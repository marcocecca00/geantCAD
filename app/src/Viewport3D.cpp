#include "Viewport3D.hh"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>

#ifndef GEANTCAD_NO_VTK
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkBox.h>
#include <vtkCylinder.h>
#include <vtkSphere.h>
#include <vtkCone.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkSphereSource.h>
#include <vtkConeSource.h>
#include <vtkSampleFunction.h>
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
// vtkTrd.h not available in VTK 9.1 - using vtkCubeSource for Trd shapes
#include <vtkCellPicker.h>
#include <vtkPropPicker.h>
#include <vtkPlaneSource.h>
#include <vtkLineSource.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLine.h>
#include <vtkCommand.h>
#include <QMenu>
#include <QContextMenuEvent>
#include "../../core/include/Command.hh"
#include "../../core/include/Transform.hh"
#endif

namespace geantcad {

#ifdef GEANTCAD_NO_VTK
Viewport3D::Viewport3D(QWidget* parent)
    : QWidget(parent)
    , sceneGraph_(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel("VTK not available.\nPlease install VTK with Qt6 support.", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}

Viewport3D::~Viewport3D() {
}
#else
Viewport3D::Viewport3D(QWidget* parent)
    : QVTKOpenGLNativeWidget(parent)
    , sceneGraph_(nullptr)
    , commandStack_(nullptr)
    , interactionMode_(InteractionMode::Select)
    , gridVisible_(false)
    , gridSpacing_(50.0)
    , snapToGrid_(false)
{
    setupRenderer();
    setupInteractor();
    
    // Setup view cube (only after interactor is set)
    setupViewCube();
}

Viewport3D::~Viewport3D() {
}

void Viewport3D::setupRenderer() {
    renderer_ = vtkSmartPointer<vtkRenderer>::New();
    renderer_->SetBackground(0.15, 0.15, 0.20); // Dark blue-gray background
    
    // QVTKOpenGLNativeWidget requires vtkGenericOpenGLRenderWindow
    renderWindow_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow_->AddRenderer(renderer_);
    renderWindow_->SetWindowName("GeantCAD Viewport");
    renderWindow_->SetSize(800, 600);
    
    setRenderWindow(renderWindow_);
    
    // Create modern grid
    createGrid();
    
    // Set up camera with better initial position
    vtkCamera* camera = renderer_->GetActiveCamera();
    camera->SetPosition(200, 200, 200);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 0, 1);
    
    // Configure camera for better interaction
    camera->SetClippingRange(1.0, 10000.0); // Wide clipping range
    camera->SetViewAngle(30.0); // Standard field of view
    
    renderer_->ResetCamera();
}

void Viewport3D::setupInteractor() {
    interactor_ = renderWindow_->GetInteractor();
    if (interactor_) {
        // Use custom interactor style that handles picking properly
        // For now, use standard trackball camera - picking will be handled separately
        vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = 
            vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        
        // Configure interaction settings for better control
        // Note: VTK 9.1 doesn't have SetRotationFactor/SetTranslationFactor
        // SetMotionFactor controls general motion sensitivity (default is 1.0)
        style->SetMotionFactor(1.0);
        
        interactor_->SetInteractorStyle(style);
        
        // Disable automatic picking by interactor to avoid conflicts
        interactor_->SetPicker(nullptr);
        
        // Enable continuous rendering for smoother interaction
        interactor_->SetDesiredUpdateRate(30.0); // 30 FPS
    }
}

void Viewport3D::setGridVisible(bool visible) {
    gridVisible_ = visible;
    updateGrid();
    if (renderWindow_) {
        renderWindow_->Render();
    }
}

void Viewport3D::setGridSpacing(double spacing) {
    if (spacing <= 0.0) return;
    gridSpacing_ = spacing;
    
    // Recreate grid with new spacing
    createGrid();
    if (renderWindow_) {
        renderWindow_->Render();
    }
}

void Viewport3D::setSnapToGrid(bool enabled) {
    snapToGrid_ = enabled;
}

void Viewport3D::createGrid() {
    if (!renderer_) return;
    
    // Remove existing grid actors
    if (gridActor_) {
        renderer_->RemoveActor(gridActor_);
        gridActor_ = nullptr;
    }
    if (axisXActor_) {
        renderer_->RemoveActor(axisXActor_);
        axisXActor_ = nullptr;
    }
    if (axisYActor_) {
        renderer_->RemoveActor(axisYActor_);
        axisYActor_ = nullptr;
    }
    
    // Grid parameters
    const double gridSize = 500.0; // Total grid size (mm)
    const int majorDivisions = static_cast<int>(gridSize / gridSpacing_);
    const int minorSubdivisions = 5; // Minor lines between major lines
    const double minorSpacing = gridSpacing_ / minorSubdivisions;
    
    // Create points and lines for the grid
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> majorLines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkCellArray> minorLines = vtkSmartPointer<vtkCellArray>::New();
    
    int pointId = 0;
    
    // Generate grid lines (XY plane at Z=0)
    for (int i = -majorDivisions; i <= majorDivisions; ++i) {
        double pos = i * gridSpacing_;
        bool isMajor = true;
        
        // Add minor lines between major lines
        for (int j = 0; j < minorSubdivisions && i < majorDivisions; ++j) {
            double minorPos = pos + j * minorSpacing;
            
            // Skip the major line itself (j=0)
            if (j == 0 && i != -majorDivisions) continue;
            
            // Lines parallel to Y axis
            points->InsertNextPoint(minorPos, -gridSize, 0);
            points->InsertNextPoint(minorPos, gridSize, 0);
            
            vtkSmartPointer<vtkLine> line1 = vtkSmartPointer<vtkLine>::New();
            line1->GetPointIds()->SetId(0, pointId++);
            line1->GetPointIds()->SetId(1, pointId++);
            
            if (j == 0) {
                majorLines->InsertNextCell(line1);
            } else {
                minorLines->InsertNextCell(line1);
            }
            
            // Lines parallel to X axis
            points->InsertNextPoint(-gridSize, minorPos, 0);
            points->InsertNextPoint(gridSize, minorPos, 0);
            
            vtkSmartPointer<vtkLine> line2 = vtkSmartPointer<vtkLine>::New();
            line2->GetPointIds()->SetId(0, pointId++);
            line2->GetPointIds()->SetId(1, pointId++);
            
            if (j == 0) {
                majorLines->InsertNextCell(line2);
            } else {
                minorLines->InsertNextCell(line2);
            }
        }
    }
    
    // Add the last major lines
    double lastPos = majorDivisions * gridSpacing_;
    // Y parallel at +gridSize
    points->InsertNextPoint(lastPos, -gridSize, 0);
    points->InsertNextPoint(lastPos, gridSize, 0);
    vtkSmartPointer<vtkLine> lineY = vtkSmartPointer<vtkLine>::New();
    lineY->GetPointIds()->SetId(0, pointId++);
    lineY->GetPointIds()->SetId(1, pointId++);
    majorLines->InsertNextCell(lineY);
    
    // X parallel at +gridSize
    points->InsertNextPoint(-gridSize, lastPos, 0);
    points->InsertNextPoint(gridSize, lastPos, 0);
    vtkSmartPointer<vtkLine> lineX = vtkSmartPointer<vtkLine>::New();
    lineX->GetPointIds()->SetId(0, pointId++);
    lineX->GetPointIds()->SetId(1, pointId++);
    majorLines->InsertNextCell(lineX);
    
    // Create polydata for major lines
    vtkSmartPointer<vtkPolyData> majorPolyData = vtkSmartPointer<vtkPolyData>::New();
    majorPolyData->SetPoints(points);
    majorPolyData->SetLines(majorLines);
    
    // Append minor lines to same polydata (simpler)
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    
    // Create separate polydata for minor lines
    vtkSmartPointer<vtkPolyData> minorPolyData = vtkSmartPointer<vtkPolyData>::New();
    minorPolyData->SetPoints(points);
    minorPolyData->SetLines(minorLines);
    
    appendFilter->AddInputData(majorPolyData);
    appendFilter->AddInputData(minorPolyData);
    appendFilter->Update();
    
    // Create mapper and actor for grid
    vtkSmartPointer<vtkPolyDataMapper> gridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    gridMapper->SetInputConnection(appendFilter->GetOutputPort());
    
    gridActor_ = vtkSmartPointer<vtkActor>::New();
    gridActor_->SetMapper(gridMapper);
    gridActor_->GetProperty()->SetColor(0.25, 0.25, 0.30); // Subtle gray-blue
    gridActor_->GetProperty()->SetLineWidth(1.0);
    gridActor_->GetProperty()->SetOpacity(0.4);
    gridActor_->SetPickable(false); // Don't interfere with selection
    
    // Add grid to renderer (at back)
    renderer_->AddActor(gridActor_);
    
    // Create X axis line (red)
    vtkSmartPointer<vtkLineSource> xAxisLine = vtkSmartPointer<vtkLineSource>::New();
    xAxisLine->SetPoint1(-gridSize, 0, 0);
    xAxisLine->SetPoint2(gridSize, 0, 0);
    
    vtkSmartPointer<vtkPolyDataMapper> xMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    xMapper->SetInputConnection(xAxisLine->GetOutputPort());
    
    axisXActor_ = vtkSmartPointer<vtkActor>::New();
    axisXActor_->SetMapper(xMapper);
    axisXActor_->GetProperty()->SetColor(0.8, 0.2, 0.2); // Red
    axisXActor_->GetProperty()->SetLineWidth(2.0);
    axisXActor_->GetProperty()->SetOpacity(0.8);
    axisXActor_->SetPickable(false);
    renderer_->AddActor(axisXActor_);
    
    // Create Y axis line (green)
    vtkSmartPointer<vtkLineSource> yAxisLine = vtkSmartPointer<vtkLineSource>::New();
    yAxisLine->SetPoint1(0, -gridSize, 0);
    yAxisLine->SetPoint2(0, gridSize, 0);
    
    vtkSmartPointer<vtkPolyDataMapper> yMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    yMapper->SetInputConnection(yAxisLine->GetOutputPort());
    
    axisYActor_ = vtkSmartPointer<vtkActor>::New();
    axisYActor_->SetMapper(yMapper);
    axisYActor_->GetProperty()->SetColor(0.2, 0.8, 0.2); // Green
    axisYActor_->GetProperty()->SetLineWidth(2.0);
    axisYActor_->GetProperty()->SetOpacity(0.8);
    axisYActor_->SetPickable(false);
    renderer_->AddActor(axisYActor_);
    
    // Apply visibility
    updateGrid();
}

void Viewport3D::updateGrid() {
    if (gridActor_) {
        gridActor_->SetVisibility(gridVisible_);
    }
    if (axisXActor_) {
        axisXActor_->SetVisibility(gridVisible_);
    }
    if (axisYActor_) {
        axisYActor_->SetVisibility(gridVisible_);
    }
}
#endif

void Viewport3D::setSceneGraph(SceneGraph* sceneGraph) {
    sceneGraph_ = sceneGraph;
    updateScene();
}

void Viewport3D::resetView() {
#ifdef GEANTCAD_NO_VTK
    (void)0; // No-op without VTK
#else
    if (renderer_) {
        renderer_->ResetCamera();
        renderWindow_->Render();
    }
#endif
}

void Viewport3D::setStandardView(StandardView view) {
#ifdef GEANTCAD_NO_VTK
    (void)view; // No-op without VTK
#else
    if (!renderer_ || !getCamera()) return;
    
    vtkCamera* camera = getCamera();
    double distance = 500.0; // Default distance
    
    switch (view) {
        case StandardView::Front: // +Y
            camera->SetPosition(0, distance, 0);
            camera->SetFocalPoint(0, 0, 0);
            camera->SetViewUp(0, 0, 1);
            break;
        case StandardView::Back: // -Y
            camera->SetPosition(0, -distance, 0);
            camera->SetFocalPoint(0, 0, 0);
            camera->SetViewUp(0, 0, 1);
            break;
        case StandardView::Left: // +X
            camera->SetPosition(distance, 0, 0);
            camera->SetFocalPoint(0, 0, 0);
            camera->SetViewUp(0, 0, 1);
            break;
        case StandardView::Right: // -X
            camera->SetPosition(-distance, 0, 0);
            camera->SetFocalPoint(0, 0, 0);
            camera->SetViewUp(0, 0, 1);
            break;
        case StandardView::Top: // +Z
            camera->SetPosition(0, 0, distance);
            camera->SetFocalPoint(0, 0, 0);
            camera->SetViewUp(0, 1, 0);
            break;
        case StandardView::Bottom: // -Z
            camera->SetPosition(0, 0, -distance);
            camera->SetFocalPoint(0, 0, 0);
            camera->SetViewUp(0, 1, 0);
            break;
        case StandardView::Isometric: // 45° isometric
            camera->SetPosition(distance, distance, distance);
            camera->SetFocalPoint(0, 0, 0);
            camera->SetViewUp(0, 0, 1);
            break;
    }
    
    renderer_->ResetCamera();
    renderWindow_->Render();
    emit viewChanged();
#endif
}

void Viewport3D::setupViewCube() {
#ifdef GEANTCAD_NO_VTK
    // No-op without VTK
#else
    if (!interactor_) return;
    
    // Create a cube actor for the view cube
    vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
    cube->SetXLength(1.0);
    cube->SetYLength(1.0);
    cube->SetZLength(1.0);
    cube->SetCenter(0, 0, 0);
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cube->GetOutputPort());
    
    vtkSmartPointer<vtkActor> cubeActor = vtkSmartPointer<vtkActor>::New();
    cubeActor->SetMapper(mapper);
    
    // Style the cube: wireframe with colored faces
    cubeActor->GetProperty()->SetRepresentationToWireframe();
    cubeActor->GetProperty()->SetColor(0.8, 0.8, 0.8);
    cubeActor->GetProperty()->SetLineWidth(2.0);
    cubeActor->GetProperty()->SetOpacity(0.8);
    
    // Create orientation marker widget with cube
    viewCubeWidget_ = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    viewCubeWidget_->SetOrientationMarker(cubeActor);
    viewCubeWidget_->SetInteractor(interactor_);
    viewCubeWidget_->SetEnabled(1);
    viewCubeWidget_->InteractiveOn();
    
    // Position in bottom-right corner
    viewCubeWidget_->SetViewport(0.0, 0.0, 0.2, 0.2);
#endif
}

void Viewport3D::frameSelection() {
#ifdef GEANTCAD_NO_VTK
    resetView();
#else
    if (!renderer_ || !sceneGraph_) {
        resetView();
        return;
    }
    
    VolumeNode* selected = sceneGraph_->getSelected();
    if (!selected || selected == sceneGraph_->getRoot()) {
        resetView();
        return;
    }
    
    // Find actor for selected node
    auto it = actors_.find(selected);
    if (it == actors_.end()) {
        resetView();
        return;
    }
    
    vtkActor* actor = it->second;
    if (!actor) {
        resetView();
        return;
    }
    
    // Get bounds of the actor
    double bounds[6];
    actor->GetBounds(bounds);
    
    // Calculate center and size
    double center[3] = {
        (bounds[0] + bounds[1]) / 2.0,
        (bounds[2] + bounds[3]) / 2.0,
        (bounds[4] + bounds[5]) / 2.0
    };
    
    double size[3] = {
        bounds[1] - bounds[0],
        bounds[3] - bounds[2],
        bounds[5] - bounds[4]
    };
    
    // Calculate distance for camera (1.5x the diagonal of bounding box)
    double diagonal = std::sqrt(size[0]*size[0] + size[1]*size[1] + size[2]*size[2]);
    double distance = diagonal * 1.5;
    
    // Position camera at a good viewing angle
    vtkCamera* camera = renderer_->GetActiveCamera();
    camera->SetFocalPoint(center);
    
    // Place camera at a nice angle (isometric view)
    double angle = 45.0 * 3.14159 / 180.0; // 45 degrees in radians
    double cameraPos[3] = {
        center[0] + distance * std::cos(angle),
        center[1] + distance * std::cos(angle),
        center[2] + distance * std::sin(angle)
    };
    
    camera->SetPosition(cameraPos);
    camera->SetViewUp(0, 0, 1);
    camera->ComputeViewPlaneNormal();
    
    // Set parallel scale for better framing
    renderer_->ResetCamera(bounds);
    
    // Render
    renderWindow_->Render();
#endif
}

void Viewport3D::refresh() {
    updateScene();
#ifndef GEANTCAD_NO_VTK
    if (renderWindow_) {
        renderWindow_->Render();
    }
#endif
}

#ifndef GEANTCAD_NO_VTK
// Helper function to create VTK source from Shape
static vtkSmartPointer<vtkPolyDataAlgorithm> createVTKSourceFromShape(const Shape* shape) {
    if (!shape) return nullptr;
    
    switch (shape->getType()) {
        case ShapeType::Box: {
            if (auto* params = shape->getParamsAs<BoxParams>()) {
                // Use vtkCubeSource for better performance
                vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
                cube->SetXLength(params->x * 2.0);
                cube->SetYLength(params->y * 2.0);
                cube->SetZLength(params->z * 2.0);
                cube->SetCenter(0, 0, 0);
                return cube;
            }
            break;
        }
        case ShapeType::Tube: {
            if (auto* params = shape->getParamsAs<TubeParams>()) {
                vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
                cylinder->SetRadius(params->rmax);
                cylinder->SetHeight(params->dz * 2.0);
                cylinder->SetResolution(32);
                cylinder->SetCenter(0, 0, 0);
                return cylinder;
            }
            break;
        }
        case ShapeType::Sphere: {
            if (auto* params = shape->getParamsAs<SphereParams>()) {
                vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
                sphere->SetRadius(params->rmax);
                sphere->SetThetaResolution(32);
                sphere->SetPhiResolution(32);
                sphere->SetCenter(0, 0, 0);
                return sphere;
            }
            break;
        }
        case ShapeType::Cone: {
            if (auto* params = shape->getParamsAs<ConeParams>()) {
                vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
                cone->SetRadius(params->rmax2); // Use larger radius
                cone->SetHeight(params->dz * 2.0);
                cone->SetResolution(32);
                cone->SetCenter(0, 0, 0);
                return cone;
            }
            break;
        }
        default:
            break;
    }
    
    return nullptr;
}
#endif

void Viewport3D::updateScene() {
#ifdef GEANTCAD_NO_VTK
    (void)0; // No-op without VTK
#else
    if (!renderer_ || !sceneGraph_) return;
    
    // Clear existing actors safely
    for (auto& pair : actors_) {
        if (pair.second) {
            renderer_->RemoveActor(pair.second);
        }
    }
    actors_.clear();
    
    // Traverse scene graph and create actors
    sceneGraph_->traverse([this](VolumeNode* node) {
        if (!node || !node->getShape()) return;
        
        // Skip hidden nodes
        if (!node->isVisible()) return;
        
        // Skip root/world node for now (or render it differently)
        if (node->getName() == "World") return;
        
        // Create VTK source from shape
        auto source = createVTKSourceFromShape(node->getShape());
        if (!source) return;
        
        // Create mapper
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(source->GetOutputPort());
        
        // Create actor
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        
        // Apply transform
        auto transform = node->getWorldTransform();
        auto matrix = transform.getMatrix();
        vtkSmartPointer<vtkTransform> vtkXForm = vtkSmartPointer<vtkTransform>::New();
        
        // Convert QMatrix4x4 to vtkMatrix4x4
        vtkSmartPointer<vtkMatrix4x4> vtkMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        const float* data = matrix.constData();
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                vtkMatrix->SetElement(i, j, data[i * 4 + j]);
            }
        }
        vtkXForm->SetMatrix(vtkMatrix);
        actor->SetUserTransform(vtkXForm);
        
        // Apply material visual properties
        if (auto material = node->getMaterial()) {
            auto& visual = material->getVisual();
            actor->GetProperty()->SetColor(visual.r, visual.g, visual.b);
            actor->GetProperty()->SetOpacity(visual.a);
            if (visual.wireframe) {
                actor->GetProperty()->SetRepresentationToWireframe();
            } else {
                actor->GetProperty()->SetRepresentationToSurface();
            }
        } else {
            // Default appearance
            actor->GetProperty()->SetColor(0.8, 0.8, 0.8);
        }
        
        // Add to renderer
        renderer_->AddActor(actor);
        actors_[node] = actor;
    });
    
    renderer_->ResetCamera();
    renderWindow_->Render();
#endif
}

void Viewport3D::setCommandStack(CommandStack* commandStack) {
    commandStack_ = commandStack;
}

void Viewport3D::setInteractionMode(InteractionMode mode) {
    interactionMode_ = mode;
    // Mode change doesn't require special handling now - selection is always enabled
}

void Viewport3D::keyPressEvent(QKeyEvent* event) {
    // Handle keyboard shortcuts
    switch (event->key()) {
        case Qt::Key_S:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+S is handled by MainWindow
                break;
            }
            setInteractionMode(InteractionMode::Select);
            emit viewChanged();
            break;
        case Qt::Key_W:
            setInteractionMode(InteractionMode::Move);
            emit viewChanged();
            break;
        case Qt::Key_E:
            setInteractionMode(InteractionMode::Rotate);
            emit viewChanged();
            break;
        case Qt::Key_G:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+G toggles grid visibility
                setGridVisible(!gridVisible_);
                break;
            }
            // G key sets Move mode
            setInteractionMode(InteractionMode::Move);
            emit viewChanged();
            break;
        case Qt::Key_R:
            setInteractionMode(InteractionMode::Rotate);
            emit viewChanged();
            break;
        case Qt::Key_T:
            setInteractionMode(InteractionMode::Scale);
            emit viewChanged();
            break;
        case Qt::Key_F:
            frameSelection();
            break;
        case Qt::Key_Home:
            resetView();
            break;
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            // Zoom in
            if (renderer_ && renderer_->GetActiveCamera()) {
                renderer_->GetActiveCamera()->Zoom(1.1);
                if (renderWindow_) renderWindow_->Render();
            }
            break;
        case Qt::Key_Minus:
        case Qt::Key_Underscore:
            // Zoom out
            if (renderer_ && renderer_->GetActiveCamera()) {
                renderer_->GetActiveCamera()->Zoom(0.9);
                if (renderWindow_) renderWindow_->Render();
            }
            break;
        case Qt::Key_Up:
            // Pan up
            if (renderer_ && renderer_->GetActiveCamera() && interactor_) {
                vtkCamera* camera = renderer_->GetActiveCamera();
                double pos[3], focal[3], up[3];
                camera->GetPosition(pos);
                camera->GetFocalPoint(focal);
                camera->GetViewUp(up);
                
                // Calculate pan distance
                double distance = std::sqrt(
                    (pos[0]-focal[0])*(pos[0]-focal[0]) +
                    (pos[1]-focal[1])*(pos[1]-focal[1]) +
                    (pos[2]-focal[2])*(pos[2]-focal[2])
                );
                double panDistance = distance * 0.05;
                
                // Pan along view up direction
                camera->SetFocalPoint(
                    focal[0] + up[0] * panDistance,
                    focal[1] + up[1] * panDistance,
                    focal[2] + up[2] * panDistance
                );
                camera->SetPosition(
                    pos[0] + up[0] * panDistance,
                    pos[1] + up[1] * panDistance,
                    pos[2] + up[2] * panDistance
                );
                if (renderWindow_) renderWindow_->Render();
            }
            break;
        case Qt::Key_Down:
            // Pan down
            if (renderer_ && renderer_->GetActiveCamera() && interactor_) {
                vtkCamera* camera = renderer_->GetActiveCamera();
                double pos[3], focal[3], up[3];
                camera->GetPosition(pos);
                camera->GetFocalPoint(focal);
                camera->GetViewUp(up);
                
                double distance = std::sqrt(
                    (pos[0]-focal[0])*(pos[0]-focal[0]) +
                    (pos[1]-focal[1])*(pos[1]-focal[1]) +
                    (pos[2]-focal[2])*(pos[2]-focal[2])
                );
                double panDistance = distance * 0.05;
                
                camera->SetFocalPoint(
                    focal[0] - up[0] * panDistance,
                    focal[1] - up[1] * panDistance,
                    focal[2] - up[2] * panDistance
                );
                camera->SetPosition(
                    pos[0] - up[0] * panDistance,
                    pos[1] - up[1] * panDistance,
                    pos[2] - up[2] * panDistance
                );
                if (renderWindow_) renderWindow_->Render();
            }
            break;
        default:
#ifndef GEANTCAD_NO_VTK
            QVTKOpenGLNativeWidget::keyPressEvent(event);
#else
            QWidget::keyPressEvent(event);
#endif
    }
}

void Viewport3D::keyReleaseEvent(QKeyEvent* event) {
#ifndef GEANTCAD_NO_VTK
    QVTKOpenGLNativeWidget::keyReleaseEvent(event);
#else
    QWidget::keyReleaseEvent(event);
#endif
}

void Viewport3D::mousePressEvent(QMouseEvent* event) {
#ifndef GEANTCAD_NO_VTK
    if (event->button() == Qt::RightButton) {
        showContextMenu(event->pos());
        return;
    }
    
    // Store mouse position for drag detection
    if (event->button() == Qt::LeftButton) {
        lastPickPos_ = event->pos();
    }
    
    QVTKOpenGLNativeWidget::mousePressEvent(event);
#else
    QWidget::mousePressEvent(event);
#endif
}

void Viewport3D::mouseReleaseEvent(QMouseEvent* event) {
#ifndef GEANTCAD_NO_VTK
    // Handle picking on left click (if not a drag)
    if (event->button() == Qt::LeftButton) {
        QPoint currentPos = event->pos();
        QPoint delta = currentPos - lastPickPos_;
        int moveThreshold = 5; // pixels
        
        // Only pick if mouse didn't move much (it was a click)
        if (delta.manhattanLength() < moveThreshold && renderer_ && renderWindow_) {
            int x = event->pos().x();
            int y = event->pos().y();
            
            vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
            picker->Pick(x, renderWindow_->GetSize()[1] - y - 1, 0, renderer_);
            
            vtkActor* pickedActor = picker->GetActor();
            VolumeNode* pickedNode = nullptr;
            
            if (pickedActor) {
                // Find VolumeNode from actor
                for (const auto& pair : actors_) {
                    if (pair.second && pair.second.GetPointer() == pickedActor) {
                        pickedNode = pair.first;
                        break;
                    }
                }
            }
            
            // Update selection
            updateSelectionHighlight(pickedNode);
            emit selectionChanged(pickedNode);
            
            if (renderWindow_) {
                renderWindow_->Render();
            }
        }
    }
    
    QVTKOpenGLNativeWidget::mouseReleaseEvent(event);
#else
    QWidget::mouseReleaseEvent(event);
#endif
}

#ifndef GEANTCAD_NO_VTK
void Viewport3D::updateSelectionHighlight(VolumeNode* selectedNode) {
    // Reset all actors to normal appearance
    for (auto& pair : actors_) {
        if (!pair.second) continue; // Safety check
        
        vtkActor* actor = pair.second.GetPointer();
        VolumeNode* node = pair.first;
        
        if (!node || !actor) continue; // Safety check
        
        if (node == selectedNode) {
            // Highlight selected: brighter color + prominent outline
            // Keep original material color but make it brighter
            if (auto material = node->getMaterial()) {
                auto& visual = material->getVisual();
                // Brighten the color for selection
                actor->GetProperty()->SetColor(
                    std::min(1.0, visual.r * 1.3),
                    std::min(1.0, visual.g * 1.3),
                    std::min(1.0, visual.b * 1.3)
                );
            } else {
                actor->GetProperty()->SetColor(1.0, 0.8, 0.0); // Yellow/orange highlight
            }
            actor->GetProperty()->SetLineWidth(4.0); // Thicker outline
            // Enable prominent outline
            actor->GetProperty()->EdgeVisibilityOn();
            actor->GetProperty()->SetEdgeColor(1.0, 1.0, 0.0); // Bright yellow edge
            actor->GetProperty()->SetAmbient(0.3); // Add ambient lighting for better visibility
            actor->GetProperty()->SetSpecular(0.8); // Add specular highlight
        } else {
            // Reset to normal
            if (auto material = node->getMaterial()) {
                auto& visual = material->getVisual();
                actor->GetProperty()->SetColor(visual.r, visual.g, visual.b);
            } else {
                actor->GetProperty()->SetColor(0.8, 0.8, 0.8);
            }
            actor->GetProperty()->SetLineWidth(1.0);
            actor->GetProperty()->EdgeVisibilityOff();
        }
    }
    
}

void Viewport3D::showContextMenu(const QPoint& pos) {
#ifndef GEANTCAD_NO_VTK
    if (!sceneGraph_) return;
    
    QMenu contextMenu(this);
    
    VolumeNode* selected = sceneGraph_->getSelected();
    
    // Tool actions
    QAction* selectAction = contextMenu.addAction("Select (S)");
    connect(selectAction, &QAction::triggered, [this]() {
        setInteractionMode(InteractionMode::Select);
        emit viewChanged();
    });
    
    QAction* moveAction = contextMenu.addAction("Move (G)");
    connect(moveAction, &QAction::triggered, [this]() {
        setInteractionMode(InteractionMode::Move);
        emit viewChanged();
    });
    
    QAction* rotateAction = contextMenu.addAction("Rotate (R)");
    connect(rotateAction, &QAction::triggered, [this]() {
        setInteractionMode(InteractionMode::Rotate);
        emit viewChanged();
    });
    
    QAction* scaleAction = contextMenu.addAction("Scale (T)");
    connect(scaleAction, &QAction::triggered, [this]() {
        setInteractionMode(InteractionMode::Scale);
        emit viewChanged();
    });
    
    contextMenu.addSeparator();
    
    // View actions
    QAction* frameAction = contextMenu.addAction("Frame Selection (F)");
    connect(frameAction, &QAction::triggered, this, &Viewport3D::frameSelection);
    
    QAction* resetAction = contextMenu.addAction("Reset View (Home)");
    connect(resetAction, &QAction::triggered, this, &Viewport3D::resetView);
    
    if (selected && selected != sceneGraph_->getRoot()) {
        contextMenu.addSeparator();
        
        QAction* deleteAction = contextMenu.addAction("Delete");
        connect(deleteAction, &QAction::triggered, [this, selected]() {
            emit selectionChanged(nullptr);
            // Delete will be handled by MainWindow
        });
    }
    
    contextMenu.exec(mapToGlobal(pos));
#endif
}

#endif // GEANTCAD_NO_VTK

vtkRenderer* Viewport3D::getRenderer() const {
#ifndef GEANTCAD_NO_VTK
    return renderer_.GetPointer();
#else
    return nullptr;
#endif
}

vtkCamera* Viewport3D::getCamera() const {
#ifndef GEANTCAD_NO_VTK
    if (renderer_) {
        return renderer_->GetActiveCamera();
    }
#endif
    return nullptr;
}

} // namespace geantcad
