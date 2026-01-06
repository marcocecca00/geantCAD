#include "Viewport3D.hh"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QVector3D>
#include <cmath>

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
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
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
#include <vtkArrowSource.h>
#include <vtkDiskSource.h>
#include <vtkTubeFilter.h>
#include <vtkRegularPolygonSource.h>
// vtkVectorText requires FreeType - using cone markers instead
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
    
    // Create manipulation gizmos
    createGizmos();
    
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

void Viewport3D::setBackgroundColor(double r, double g, double b) {
    bgColorR_ = r;
    bgColorG_ = g;
    bgColorB_ = b;
#ifndef GEANTCAD_NO_VTK
    if (renderer_) {
        renderer_->SetBackground(r, g, b);
        if (renderWindow_) renderWindow_->Render();
    }
#endif
}

void Viewport3D::getBackgroundColor(double& r, double& g, double& b) const {
    r = bgColorR_;
    g = bgColorG_;
    b = bgColorB_;
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
    if (axisZActor_) {
        renderer_->RemoveActor(axisZActor_);
        axisZActor_ = nullptr;
    }
    
    // Remove axis labels
    for (auto& label : axisLabels_) {
        if (label) renderer_->RemoveActor(label);
    }
    axisLabels_.clear();
    
    // Grid parameters - LARGER grid for better visibility
    const double gridSize = 1000.0; // Total grid size (mm) - extended
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
    
    // Create Z axis line (blue) - vertical
    vtkSmartPointer<vtkLineSource> zAxisLine = vtkSmartPointer<vtkLineSource>::New();
    zAxisLine->SetPoint1(0, 0, -gridSize);
    zAxisLine->SetPoint2(0, 0, gridSize);
    
    vtkSmartPointer<vtkPolyDataMapper> zMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    zMapper->SetInputConnection(zAxisLine->GetOutputPort());
    
    axisZActor_ = vtkSmartPointer<vtkActor>::New();
    axisZActor_->SetMapper(zMapper);
    axisZActor_->GetProperty()->SetColor(0.2, 0.4, 0.9); // Blue
    axisZActor_->GetProperty()->SetLineWidth(2.0);
    axisZActor_->GetProperty()->SetOpacity(0.8);
    axisZActor_->SetPickable(false);
    renderer_->AddActor(axisZActor_);
    
    // Create axis markers (cones at positive ends) instead of text labels
    auto createAxisMarker = [&](double x, double y, double z, double dirX, double dirY, double dirZ, double r, double g, double b) {
        vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
        cone->SetHeight(30);
        cone->SetRadius(8);
        cone->SetResolution(16);
        cone->SetDirection(dirX, dirY, dirZ);
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(cone->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(r, g, b);
        actor->SetPosition(x, y, z);
        actor->SetPickable(false);
        
        renderer_->AddActor(actor);
        axisLabels_.push_back(actor);
    };
    
    // Add cone markers at the positive end of each axis
    createAxisMarker(gridSize + 15, 0, 0, 1, 0, 0, 0.8, 0.2, 0.2);  // Red X
    createAxisMarker(0, gridSize + 15, 0, 0, 1, 0, 0.2, 0.8, 0.2);  // Green Y
    createAxisMarker(0, 0, gridSize + 15, 0, 0, 1, 0.2, 0.4, 0.9);  // Blue Z
    
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
    if (axisZActor_) {
        axisZActor_->SetVisibility(gridVisible_);
    }
    // Update axis labels visibility
    for (auto& label : axisLabels_) {
        if (label) {
            label->SetVisibility(gridVisible_);
        }
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
    // The Qt-based ViewCube in top-right is the only one we use now
    // No VTK orientation marker widget needed - it's redundant
    viewCubeWidget_ = nullptr;
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
        // QMatrix4x4 uses column-major storage, VTK uses row-major
        // QMatrix4x4::constData() returns data in column-major order
        // So we need to transpose when setting VTK matrix
        vtkSmartPointer<vtkMatrix4x4> vtkMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        const float* data = matrix.constData();
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                // Transpose: swap i and j when reading from Qt data
                vtkMatrix->SetElement(i, j, data[j * 4 + i]);
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
    
    // Don't reset camera during drag operations
    if (!isDragging_) {
        // Only reset if scene changed significantly
        if (actors_.empty()) {
            renderer_->ResetCamera();
        }
    }
    
    // Preserve selection highlight after scene update
    if (sceneGraph_) {
        VolumeNode* selected = sceneGraph_->getSelected();
        if (selected) {
            updateSelectionHighlight(selected);
        }
    }
    
    renderWindow_->Render();
#endif
}

void Viewport3D::setCommandStack(CommandStack* commandStack) {
    commandStack_ = commandStack;
}

void Viewport3D::setInteractionMode(InteractionMode mode) {
    interactionMode_ = mode;
    
    // Deactivate measurement mode when switching tools
    if (mode != InteractionMode::Select && measurementMode_) {
        measurementMode_ = false;
        emit measurementModeChanged(false);
    }
    
#ifndef GEANTCAD_NO_VTK
    // Show/hide gizmos based on mode
    updateGizmoPosition();
    if (renderWindow_) renderWindow_->Render();
#endif
}

void Viewport3D::setConstraintPlane(ConstraintPlane plane) {
    constraintPlane_ = plane;
#ifndef GEANTCAD_NO_VTK
    updateGizmoPosition();
    if (renderWindow_) renderWindow_->Render();
#endif
}

void Viewport3D::setProjectionMode(ProjectionMode mode) {
    projectionMode_ = mode;
#ifndef GEANTCAD_NO_VTK
    if (renderer_ && renderer_->GetActiveCamera()) {
        vtkCamera* camera = renderer_->GetActiveCamera();
        if (mode == ProjectionMode::Orthographic) {
            camera->ParallelProjectionOn();
        } else {
            camera->ParallelProjectionOff();
        }
        if (renderWindow_) renderWindow_->Render();
    }
#endif
}

void Viewport3D::setMeasurementMode(bool enabled) {
    measurementMode_ = enabled;
    if (enabled) {
        // Switch to select mode when measurement is active
        interactionMode_ = InteractionMode::Select;
    }
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
        case Qt::Key_X:
            // Constrain to X axis or YZ plane
            if (event->modifiers() & Qt::ShiftModifier) {
                setConstraintPlane(ConstraintPlane::YZ);
            } else {
                setConstraintPlane(ConstraintPlane::AxisX);
            }
            emit viewChanged();
            break;
        case Qt::Key_Y:
            // Constrain to Y axis or XZ plane
            if (event->modifiers() & Qt::ShiftModifier) {
                setConstraintPlane(ConstraintPlane::XZ);
            } else {
                setConstraintPlane(ConstraintPlane::AxisY);
            }
            emit viewChanged();
            break;
        case Qt::Key_Z:
            // Constrain to Z axis or XY plane
            if (event->modifiers() & Qt::ShiftModifier) {
                setConstraintPlane(ConstraintPlane::XY);
            } else {
                setConstraintPlane(ConstraintPlane::AxisZ);
            }
            emit viewChanged();
            break;
        case Qt::Key_5:
            // Toggle perspective/orthographic (like Blender)
            if (projectionMode_ == ProjectionMode::Perspective) {
                setProjectionMode(ProjectionMode::Orthographic);
            } else {
                setProjectionMode(ProjectionMode::Perspective);
            }
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
        int x = event->pos().x();
        int y = event->pos().y();
        
        // Check if we clicked on a gizmo
        if (interactionMode_ != InteractionMode::Select && sceneGraph_) {
            int gizmoAxis = pickGizmoAxis(x, y);
            if (gizmoAxis >= 0) {
                activeGizmoAxis_ = gizmoAxis;
                
                // Set constraint based on clicked gizmo part
                if (gizmoAxis == 0) setConstraintPlane(ConstraintPlane::AxisX);
                else if (gizmoAxis == 1) setConstraintPlane(ConstraintPlane::AxisY);
                else if (gizmoAxis == 2) setConstraintPlane(ConstraintPlane::AxisZ);
                else if (gizmoAxis == 3) setConstraintPlane(ConstraintPlane::XY);
                else if (gizmoAxis == 4) setConstraintPlane(ConstraintPlane::XZ);
                else if (gizmoAxis == 5) setConstraintPlane(ConstraintPlane::YZ);
                
                // Start manipulation
                VolumeNode* selected = sceneGraph_->getSelected();
                if (selected && selected != sceneGraph_->getRoot()) {
                    isDragging_ = true;
                    draggedNode_ = selected;
                    dragStartTransform_ = selected->getTransform();
                    dragStartWorldPos_ = screenToWorld(x, y, getDepthAtPosition(x, y));
                    return;
                }
            }
        }
        
        // Check if we should start manipulation (click on object)
        if ((interactionMode_ == InteractionMode::Move || 
             interactionMode_ == InteractionMode::Rotate ||
             interactionMode_ == InteractionMode::Scale) && sceneGraph_) {
            VolumeNode* selected = sceneGraph_->getSelected();
            if (selected && selected != sceneGraph_->getRoot()) {
                vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
                picker->Pick(x, renderWindow_->GetSize()[1] - y - 1, 0, renderer_);
                
                vtkActor* pickedActor = picker->GetActor();
                VolumeNode* pickedNode = nullptr;
                
                if (pickedActor) {
                    for (const auto& pair : actors_) {
                        if (pair.second && pair.second.GetPointer() == pickedActor) {
                            pickedNode = pair.first;
                            break;
                        }
                    }
                }
                
                // Start dragging if we clicked on the selected object
                if (pickedNode == selected) {
                    isDragging_ = true;
                    draggedNode_ = selected;
                    dragStartTransform_ = selected->getTransform();
                    dragStartWorldPos_ = screenToWorld(x, y, getDepthAtPosition(x, y));
                    return;
                }
            }
        }
    }
    
    QVTKOpenGLNativeWidget::mousePressEvent(event);
#else
    QWidget::mousePressEvent(event);
#endif
}

void Viewport3D::mouseMoveEvent(QMouseEvent* event) {
#ifndef GEANTCAD_NO_VTK
    if (isDragging_ && draggedNode_) {
        // IMPORTANT: Don't call parent class when dragging to prevent camera movement
        int x = event->pos().x();
        int y = event->pos().y();
        
        // Calculate screen delta for predictable movement
        int deltaX = x - lastPickPos_.x();
        int deltaY = y - lastPickPos_.y();
        
        if (interactionMode_ == InteractionMode::Move) {
            // Use screen-space delta for more predictable movement
            // REDUCED sensitivity for better control
            vtkCamera* camera = renderer_->GetActiveCamera();
            double cameraDistance = camera->GetDistance();
            double moveFactor = cameraDistance / 800.0; // Reduced from 500 for slower movement
            moveFactor = std::max(0.05, std::min(2.0, moveFactor)); // Tighter clamp
            
            // Calculate world delta based on camera orientation
            double* viewUp = camera->GetViewUp();
            double viewRight[3];
            double* viewDir = camera->GetDirectionOfProjection();
            
            // Cross product for right vector (viewUp x viewDir)
            viewRight[0] = viewUp[1] * viewDir[2] - viewUp[2] * viewDir[1];
            viewRight[1] = viewUp[2] * viewDir[0] - viewUp[0] * viewDir[2];
            viewRight[2] = viewUp[0] * viewDir[1] - viewUp[1] * viewDir[0];
            
            // Normalize
            double rightLen = std::sqrt(viewRight[0]*viewRight[0] + viewRight[1]*viewRight[1] + viewRight[2]*viewRight[2]);
            if (rightLen > 0.0001) {
                viewRight[0] /= rightLen;
                viewRight[1] /= rightLen;
                viewRight[2] /= rightLen;
            }
            
            // Use smaller delta multiplier for smoother motion
            QVector3D delta(
                (viewRight[0] * deltaX - viewUp[0] * deltaY) * moveFactor * 0.3,
                (viewRight[1] * deltaX - viewUp[1] * deltaY) * moveFactor * 0.3,
                (viewRight[2] * deltaX - viewUp[2] * deltaY) * moveFactor * 0.3
            );
            
            // Add delta to current position
            QVector3D currentPos = draggedNode_->getTransform().getTranslation();
            QVector3D newPos = currentPos + delta;
            
            // Apply smart snap (alignment with other objects)
            newPos = applySmartSnap(newPos, draggedNode_);
            
            // Update smart guides visualization
            updateSmartGuides(draggedNode_, newPos);
            
            // Snap to grid if enabled (after smart snap)
            if (snapToGrid_ && gridSpacing_ > 0) {
                newPos.setX(std::round(newPos.x() / gridSpacing_) * gridSpacing_);
                newPos.setY(std::round(newPos.y() / gridSpacing_) * gridSpacing_);
                newPos.setZ(std::round(newPos.z() / gridSpacing_) * gridSpacing_);
            }
            
            // Update transform
            Transform newTransform = draggedNode_->getTransform();
            newTransform.setTranslation(newPos);
            draggedNode_->getTransform() = newTransform;
            
            // Calculate and display transform info in viewport
            QVector3D totalMove = newPos - dragStartTransform_.getTranslation();
            transformInfoText_ = QString("Move: Δ(%1, %2, %3) mm")
                .arg(totalMove.x(), 0, 'f', 1)
                .arg(totalMove.y(), 0, 'f', 1)
                .arg(totalMove.z(), 0, 'f', 1);
            updateTransformTextOverlay(transformInfoText_);
            
            // Update last position for next frame
            lastPickPos_ = event->pos();
            
            // Update gizmo position
            updateGizmoPosition();
            refresh();
            emit objectTransformed(draggedNode_);
            return; // Don't call parent - prevents camera from moving
        }
        else if (interactionMode_ == InteractionMode::Rotate) {
            // Calculate rotation from mouse movement (use delta from last frame)
            double deltaX = (x - lastPickPos_.x()) * 0.5; // Degrees per pixel
            double deltaY = (y - lastPickPos_.y()) * 0.5;
            
            // Get current rotation and add incremental rotation
            QQuaternion currentRotation = draggedNode_->getTransform().getRotation();
            QString axisName = "Z";
            double rotationAngle = deltaX;
            
            switch (constraintPlane_) {
                case ConstraintPlane::AxisX:
                case ConstraintPlane::YZ:
                    // Rotate around X axis
                    currentRotation = QQuaternion::fromAxisAndAngle(1, 0, 0, deltaY) * currentRotation;
                    axisName = "X";
                    rotationAngle = deltaY;
                    break;
                case ConstraintPlane::AxisY:
                case ConstraintPlane::XZ:
                    // Rotate around Y axis
                    currentRotation = QQuaternion::fromAxisAndAngle(0, 1, 0, deltaX) * currentRotation;
                    axisName = "Y";
                    break;
                case ConstraintPlane::AxisZ:
                case ConstraintPlane::XY:
                default:
                    // Rotate around Z axis
                    currentRotation = QQuaternion::fromAxisAndAngle(0, 0, 1, deltaX) * currentRotation;
                    break;
            }
            
            draggedNode_->getTransform().setRotation(currentRotation);
            
            // Calculate total rotation from start (approximate using Euler angles)
            QVector3D startEuler = dragStartTransform_.getRotation().toEulerAngles();
            QVector3D currentEuler = currentRotation.toEulerAngles();
            QVector3D deltaEuler = currentEuler - startEuler;
            
            transformInfoText_ = QString("Rotate %1: Δ%2°")
                .arg(axisName)
                .arg(deltaEuler.z(), 0, 'f', 1); // Show delta around active axis
            updateTransformTextOverlay(transformInfoText_);
            
            // Update last position for next frame
            lastPickPos_ = event->pos();
            
            updateGizmoPosition();
            refresh();
            emit objectTransformed(draggedNode_);
            return; // Don't call parent - prevents camera from moving
        }
        else if (interactionMode_ == InteractionMode::Scale) {
            // Calculate scale from mouse movement (right = bigger, left = smaller)
            double scaleFactor = 1.0 + (x - lastPickPos_.x()) * 0.01;
            scaleFactor = std::max(0.9, std::min(1.1, scaleFactor)); // Clamp for stability
            
            // Determine if we should scale proportionally
            bool scaleAll = proportionalScaling_ || constraintPlane_ == ConstraintPlane::None;
            
            // For shapes, we modify the shape parameters based on type
            // This is more Geant4-like (shape dimensions matter, not transform scale)
            if (draggedNode_->getShape()) {
                Shape* shape = draggedNode_->getShape();
                ShapeType type = shape->getType();
                
                // Scale based on shape type and constraint
                switch (type) {
                    case ShapeType::Box: {
                        if (auto* p = shape->getParamsAs<BoxParams>()) {
                            if (scaleAll || constraintPlane_ == ConstraintPlane::AxisX)
                                p->x = std::max(1.0, p->x * scaleFactor);
                            if (scaleAll || constraintPlane_ == ConstraintPlane::AxisY)
                                p->y = std::max(1.0, p->y * scaleFactor);
                            if (scaleAll || constraintPlane_ == ConstraintPlane::AxisZ)
                                p->z = std::max(1.0, p->z * scaleFactor);
                        }
                        break;
                    }
                    case ShapeType::Tube: {
                        if (auto* p = shape->getParamsAs<TubeParams>()) {
                            if (scaleAll || constraintPlane_ == ConstraintPlane::AxisZ)
                                p->dz = std::max(1.0, p->dz * scaleFactor);
                            if (scaleAll || constraintPlane_ != ConstraintPlane::AxisZ) {
                                p->rmax = std::max(1.0, p->rmax * scaleFactor);
                                p->rmin = std::max(0.0, p->rmin * scaleFactor);
                            }
                        }
                        break;
                    }
                    case ShapeType::Sphere: {
                        if (auto* p = shape->getParamsAs<SphereParams>()) {
                            p->rmax = std::max(1.0, p->rmax * scaleFactor);
                            p->rmin = std::max(0.0, p->rmin * scaleFactor);
                        }
                        break;
                    }
                    case ShapeType::Cone: {
                        if (auto* p = shape->getParamsAs<ConeParams>()) {
                            if (scaleAll || constraintPlane_ == ConstraintPlane::AxisZ)
                                p->dz = std::max(1.0, p->dz * scaleFactor);
                            if (scaleAll || constraintPlane_ != ConstraintPlane::AxisZ) {
                                p->rmax1 = std::max(1.0, p->rmax1 * scaleFactor);
                                p->rmax2 = std::max(0.0, p->rmax2 * scaleFactor);
                            }
                        }
                        break;
                    }
                    case ShapeType::Trd: {
                        if (auto* p = shape->getParamsAs<TrdParams>()) {
                            if (scaleAll || constraintPlane_ == ConstraintPlane::AxisX) {
                                p->dx1 = std::max(1.0, p->dx1 * scaleFactor);
                                p->dx2 = std::max(1.0, p->dx2 * scaleFactor);
                            }
                            if (scaleAll || constraintPlane_ == ConstraintPlane::AxisY) {
                                p->dy1 = std::max(1.0, p->dy1 * scaleFactor);
                                p->dy2 = std::max(1.0, p->dy2 * scaleFactor);
                            }
                            if (scaleAll || constraintPlane_ == ConstraintPlane::AxisZ)
                                p->dz = std::max(1.0, p->dz * scaleFactor);
                        }
                        break;
                    }
                    default:
                        break;
                }
                
                // Calculate scale info text
                double scalePercent = (scaleFactor - 1.0) * 100.0;
                transformInfoText_ = QString("Scale: %1%2%")
                    .arg(scalePercent > 0 ? "+" : "")
                    .arg(scalePercent, 0, 'f', 1);
                updateTransformTextOverlay(transformInfoText_);
            }
            
            // Update for continuous feedback
            lastPickPos_ = event->pos();
            
            refresh();
            emit objectTransformed(draggedNode_);
            return;
        }
    }
    
    QVTKOpenGLNativeWidget::mouseMoveEvent(event);
#else
    QWidget::mouseMoveEvent(event);
#endif
}

void Viewport3D::mouseReleaseEvent(QMouseEvent* event) {
#ifndef GEANTCAD_NO_VTK
    // Handle end of drag operation
    if (isDragging_ && draggedNode_ && event->button() == Qt::LeftButton) {
        // Clear smart guides and hide transform text
        clearSmartGuides();
        hideTransformTextOverlay();
        
        // Create undo command for the entire drag operation
        if (commandStack_) {
            Transform finalTransform = draggedNode_->getTransform();
            // Restore original transform first
            draggedNode_->getTransform() = dragStartTransform_;
            // Then execute command (which will apply the new transform)
            auto cmd = std::make_unique<TransformVolumeCommand>(draggedNode_, finalTransform);
            commandStack_->execute(std::move(cmd));
        }
        
        emit objectTransformed(draggedNode_);
        
        isDragging_ = false;
        draggedNode_ = nullptr;
        
        refresh();
        return;
    }
    
    // Handle picking on left click (if not a drag)
    if (event->button() == Qt::LeftButton) {
        QPoint currentPos = event->pos();
        QPoint delta = currentPos - lastPickPos_;
        int moveThreshold = 5; // pixels
        
        // Only pick if mouse didn't move much (it was a click)
        if (delta.manhattanLength() < moveThreshold && renderer_ && renderWindow_) {
            int x = event->pos().x();
            int y = event->pos().y();
            
            // Measurement mode: emit 3D point for measurement tool
            if (measurementMode_) {
                vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
                picker->SetTolerance(0.005);
                
                if (picker->Pick(x, renderWindow_->GetSize()[1] - y - 1, 0, renderer_)) {
                    double* pickPos = picker->GetPickPosition();
                    QVector3D point(pickPos[0], pickPos[1], pickPos[2]);
                    emit pointPicked(point);
                } else {
                    // If no object was hit, project onto the XY plane at Z=0
                    QVector3D rayStart = screenToWorld(x, y, 0.0);
                    QVector3D rayEnd = screenToWorld(x, y, 1.0);
                    QVector3D rayDir = (rayEnd - rayStart).normalized();
                    
                    // Intersect with Z=0 plane
                    if (std::abs(rayDir.z()) > 0.0001) {
                        double t = -rayStart.z() / rayDir.z();
                        if (t > 0) {
                            QVector3D point = rayStart + t * rayDir;
                            emit pointPicked(point);
                        }
                    }
                }
                
                if (renderWindow_) {
                    renderWindow_->Render();
                }
                return;
            }
            
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
    
    // Update gizmo position for new selection
    updateGizmoPosition();
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

QVector3D Viewport3D::screenToWorld(int x, int y, double depth) {
    if (!renderer_ || !renderWindow_) {
        return QVector3D(0, 0, 0);
    }
    
    // Get display coordinates (VTK uses bottom-left origin)
    int* size = renderWindow_->GetSize();
    double displayX = static_cast<double>(x);
    double displayY = static_cast<double>(size[1] - y - 1);
    
    // Use renderer to convert display to world coordinates
    renderer_->SetDisplayPoint(displayX, displayY, depth);
    renderer_->DisplayToWorld();
    double* worldPoint = renderer_->GetWorldPoint();
    
    // Handle homogeneous coordinates
    if (worldPoint[3] != 0.0) {
        return QVector3D(
            worldPoint[0] / worldPoint[3],
            worldPoint[1] / worldPoint[3],
            worldPoint[2] / worldPoint[3]
        );
    }
    
    return QVector3D(worldPoint[0], worldPoint[1], worldPoint[2]);
}

double Viewport3D::getDepthAtPosition(int x, int y) {
    if (!renderer_ || !renderWindow_) {
        return 0.5; // Middle depth as fallback
    }
    
    // Get display coordinates (VTK uses bottom-left origin)
    int* size = renderWindow_->GetSize();
    double displayY = static_cast<double>(size[1] - y - 1);
    
    // Pick to get depth at the mouse position
    vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
    if (picker->Pick(x, displayY, 0, renderer_)) {
        double* pickPosition = picker->GetPickPosition();
        // Convert world position back to normalized depth
        renderer_->SetWorldPoint(pickPosition[0], pickPosition[1], pickPosition[2], 1.0);
        renderer_->WorldToDisplay();
        double* displayPoint = renderer_->GetDisplayPoint();
        return displayPoint[2];
    }
    
    // If no pick, use a default depth (center of view)
    return 0.5;
}

void Viewport3D::createGizmos() {
    if (!renderer_) return;
    
    const double arrowLength = 40.0;
    const double arrowRadius = 2.0;
    const double planeSize = 15.0;
    
    // === Create arrow gizmos for translation ===
    auto createArrow = [&](double r, double g, double b, double dirX, double dirY, double dirZ) {
        vtkSmartPointer<vtkArrowSource> arrow = vtkSmartPointer<vtkArrowSource>::New();
        arrow->SetTipResolution(16);
        arrow->SetShaftResolution(16);
        arrow->SetTipRadius(0.15);
        arrow->SetTipLength(0.3);
        arrow->SetShaftRadius(0.05);
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(arrow->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(r, g, b);
        actor->GetProperty()->SetAmbient(0.5);
        actor->GetProperty()->SetDiffuse(0.8);
        actor->SetScale(arrowLength, arrowLength, arrowLength);
        
        // Orient arrow along direction
        if (dirX > 0) {
            actor->RotateZ(0);   // Default X direction
        } else if (dirY > 0) {
            actor->RotateZ(90);  // Y direction
        } else if (dirZ > 0) {
            actor->RotateY(-90); // Z direction
        }
        
        actor->SetPickable(true);
        return actor;
    };
    
    gizmoXArrow_ = createArrow(1.0, 0.2, 0.2, 1, 0, 0); // Red - X
    gizmoYArrow_ = createArrow(0.2, 1.0, 0.2, 0, 1, 0); // Green - Y
    gizmoZArrow_ = createArrow(0.2, 0.4, 1.0, 0, 0, 1); // Blue - Z
    
    // === Create plane gizmos (small squares) ===
    auto createPlaneGizmo = [&](double r, double g, double b, const char* plane) {
        vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
        planeSource->SetXResolution(1);
        planeSource->SetYResolution(1);
        
        if (strcmp(plane, "XY") == 0) {
            planeSource->SetOrigin(5, 5, 0);
            planeSource->SetPoint1(planeSize, 5, 0);
            planeSource->SetPoint2(5, planeSize, 0);
        } else if (strcmp(plane, "XZ") == 0) {
            planeSource->SetOrigin(5, 0, 5);
            planeSource->SetPoint1(planeSize, 0, 5);
            planeSource->SetPoint2(5, 0, planeSize);
        } else { // YZ
            planeSource->SetOrigin(0, 5, 5);
            planeSource->SetPoint1(0, planeSize, 5);
            planeSource->SetPoint2(0, 5, planeSize);
        }
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(planeSource->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(r, g, b);
        actor->GetProperty()->SetOpacity(0.4);
        actor->SetPickable(true);
        return actor;
    };
    
    gizmoXYPlane_ = createPlaneGizmo(0.2, 0.4, 1.0, "XY"); // Blue - XY plane
    gizmoXZPlane_ = createPlaneGizmo(0.2, 1.0, 0.2, "XZ"); // Green - XZ plane
    gizmoYZPlane_ = createPlaneGizmo(1.0, 0.2, 0.2, "YZ"); // Red - YZ plane
    
    // === Create rotation rings ===
    auto createRing = [&](double r, double g, double b, const char* axis) {
        vtkSmartPointer<vtkDiskSource> disk = vtkSmartPointer<vtkDiskSource>::New();
        disk->SetInnerRadius(25);
        disk->SetOuterRadius(28);
        disk->SetRadialResolution(1);
        disk->SetCircumferentialResolution(64);
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(disk->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(r, g, b);
        actor->GetProperty()->SetOpacity(0.8);
        actor->GetProperty()->SetLineWidth(3);
        
        // Orient ring perpendicular to axis
        if (strcmp(axis, "X") == 0) {
            actor->RotateY(90);
        } else if (strcmp(axis, "Y") == 0) {
            actor->RotateX(90);
        }
        // Z axis ring needs no rotation
        
        actor->SetPickable(true);
        return actor;
    };
    
    gizmoRotateX_ = createRing(1.0, 0.2, 0.2, "X");
    gizmoRotateY_ = createRing(0.2, 1.0, 0.2, "Y");
    gizmoRotateZ_ = createRing(0.2, 0.4, 1.0, "Z");
    
    // === Create scale boxes ===
    auto createScaleBox = [&](double r, double g, double b, double x, double y, double z) {
        vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
        cube->SetXLength(6);
        cube->SetYLength(6);
        cube->SetZLength(6);
        cube->SetCenter(x * 35, y * 35, z * 35);
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(cube->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(r, g, b);
        actor->SetPickable(true);
        return actor;
    };
    
    gizmoScaleX_ = createScaleBox(1.0, 0.2, 0.2, 1, 0, 0);
    gizmoScaleY_ = createScaleBox(0.2, 1.0, 0.2, 0, 1, 0);
    gizmoScaleZ_ = createScaleBox(0.2, 0.4, 1.0, 0, 0, 1);
    
    // Create transform info text overlay
    transformInfoActor_ = vtkSmartPointer<vtkTextActor>::New();
    transformInfoActor_->SetInput("");
    transformInfoActor_->SetPosition(10, 40);  // Bottom-left corner
    transformInfoActor_->GetTextProperty()->SetFontSize(18);
    transformInfoActor_->GetTextProperty()->SetColor(1.0, 1.0, 0.3); // Yellow text
    transformInfoActor_->GetTextProperty()->SetFontFamilyToCourier();
    transformInfoActor_->GetTextProperty()->SetBold(true);
    transformInfoActor_->GetTextProperty()->SetShadow(true);
    transformInfoActor_->GetTextProperty()->SetBackgroundOpacity(0.5);
    transformInfoActor_->GetTextProperty()->SetBackgroundColor(0.1, 0.1, 0.1);
    transformInfoActor_->VisibilityOff();  // Hidden initially
    renderer_->AddActor2D(transformInfoActor_);
    
    // Initially hide all gizmos
    showGizmo(false);
}

void Viewport3D::updateGizmoPosition() {
    if (!sceneGraph_ || !renderer_) return;
    
    VolumeNode* selected = sceneGraph_->getSelected();
    bool hasSelection = selected && selected != sceneGraph_->getRoot();
    
    // Hide gizmos if nothing selected or in Select mode
    if (!hasSelection || interactionMode_ == InteractionMode::Select) {
        showGizmo(false);
        return;
    }
    
    // Get object position
    QVector3D pos = selected->getTransform().getTranslation();
    
    // Calculate object bounding box size to offset gizmos outside object
    double objectRadius = 30.0; // Default minimum offset
    if (selected->getShape()) {
        Shape* shape = selected->getShape();
        switch (shape->getType()) {
            case ShapeType::Box:
                if (auto* p = shape->getParamsAs<BoxParams>()) {
                    objectRadius = std::max({p->x, p->y, p->z}) * 0.5 + 10.0;
                }
                break;
            case ShapeType::Sphere:
                if (auto* p = shape->getParamsAs<SphereParams>()) {
                    objectRadius = p->rmax + 10.0;
                }
                break;
            case ShapeType::Tube:
                if (auto* p = shape->getParamsAs<TubeParams>()) {
                    objectRadius = std::max(p->rmax, p->dz) + 10.0;
                }
                break;
            case ShapeType::Cone:
                if (auto* p = shape->getParamsAs<ConeParams>()) {
                    objectRadius = std::max(std::max(p->rmax1, p->rmax2), p->dz) + 10.0;
                }
                break;
            case ShapeType::Trd:
                if (auto* p = shape->getParamsAs<TrdParams>()) {
                    objectRadius = std::max({p->dx1, p->dx2, p->dy1, p->dy2, p->dz}) + 10.0;
                }
                break;
            default:
                break;
        }
    }
    
    // Gizmo scale based on object size (minimum 40, scale with object)
    double gizmoScale = std::max(40.0, objectRadius * 1.2);
    
    // Position all gizmo actors at object position
    auto positionActor = [&](vtkSmartPointer<vtkActor>& actor) {
        if (actor) {
            actor->SetPosition(pos.x(), pos.y(), pos.z());
        }
    };
    
    // Show appropriate gizmos based on mode
    showGizmo(false); // Hide all first
    
    if (interactionMode_ == InteractionMode::Move) {
        positionActor(gizmoXArrow_);
        positionActor(gizmoYArrow_);
        positionActor(gizmoZArrow_);
        positionActor(gizmoXYPlane_);
        positionActor(gizmoXZPlane_);
        positionActor(gizmoYZPlane_);
        
        if (gizmoXArrow_) { renderer_->AddActor(gizmoXArrow_); gizmoXArrow_->VisibilityOn(); }
        if (gizmoYArrow_) { renderer_->AddActor(gizmoYArrow_); gizmoYArrow_->VisibilityOn(); }
        if (gizmoZArrow_) { renderer_->AddActor(gizmoZArrow_); gizmoZArrow_->VisibilityOn(); }
        if (gizmoXYPlane_) { renderer_->AddActor(gizmoXYPlane_); gizmoXYPlane_->VisibilityOn(); }
        if (gizmoXZPlane_) { renderer_->AddActor(gizmoXZPlane_); gizmoXZPlane_->VisibilityOn(); }
        if (gizmoYZPlane_) { renderer_->AddActor(gizmoYZPlane_); gizmoYZPlane_->VisibilityOn(); }
        
        // Highlight active constraint
        if (gizmoXArrow_) gizmoXArrow_->GetProperty()->SetOpacity(constraintPlane_ == ConstraintPlane::AxisX ? 1.0 : 0.6);
        if (gizmoYArrow_) gizmoYArrow_->GetProperty()->SetOpacity(constraintPlane_ == ConstraintPlane::AxisY ? 1.0 : 0.6);
        if (gizmoZArrow_) gizmoZArrow_->GetProperty()->SetOpacity(constraintPlane_ == ConstraintPlane::AxisZ ? 1.0 : 0.6);
        if (gizmoXYPlane_) gizmoXYPlane_->GetProperty()->SetOpacity(constraintPlane_ == ConstraintPlane::XY ? 0.6 : 0.3);
        if (gizmoXZPlane_) gizmoXZPlane_->GetProperty()->SetOpacity(constraintPlane_ == ConstraintPlane::XZ ? 0.6 : 0.3);
        if (gizmoYZPlane_) gizmoYZPlane_->GetProperty()->SetOpacity(constraintPlane_ == ConstraintPlane::YZ ? 0.6 : 0.3);
    }
    else if (interactionMode_ == InteractionMode::Rotate) {
        positionActor(gizmoRotateX_);
        positionActor(gizmoRotateY_);
        positionActor(gizmoRotateZ_);
        
        if (gizmoRotateX_) { renderer_->AddActor(gizmoRotateX_); gizmoRotateX_->VisibilityOn(); }
        if (gizmoRotateY_) { renderer_->AddActor(gizmoRotateY_); gizmoRotateY_->VisibilityOn(); }
        if (gizmoRotateZ_) { renderer_->AddActor(gizmoRotateZ_); gizmoRotateZ_->VisibilityOn(); }
    }
    else if (interactionMode_ == InteractionMode::Scale) {
        positionActor(gizmoScaleX_);
        positionActor(gizmoScaleY_);
        positionActor(gizmoScaleZ_);
        
        if (gizmoScaleX_) { renderer_->AddActor(gizmoScaleX_); gizmoScaleX_->VisibilityOn(); }
        if (gizmoScaleY_) { renderer_->AddActor(gizmoScaleY_); gizmoScaleY_->VisibilityOn(); }
        if (gizmoScaleZ_) { renderer_->AddActor(gizmoScaleZ_); gizmoScaleZ_->VisibilityOn(); }
    }
}

void Viewport3D::showGizmo(bool show) {
    auto setVisibility = [&](vtkSmartPointer<vtkActor>& actor) {
        if (actor) {
            if (show) {
                actor->VisibilityOn();
            } else {
                actor->VisibilityOff();
                renderer_->RemoveActor(actor);
            }
        }
    };
    
    setVisibility(gizmoXArrow_);
    setVisibility(gizmoYArrow_);
    setVisibility(gizmoZArrow_);
    setVisibility(gizmoXYPlane_);
    setVisibility(gizmoXZPlane_);
    setVisibility(gizmoYZPlane_);
    setVisibility(gizmoRotateX_);
    setVisibility(gizmoRotateY_);
    setVisibility(gizmoRotateZ_);
    setVisibility(gizmoScaleX_);
    setVisibility(gizmoScaleY_);
    setVisibility(gizmoScaleZ_);
}

void Viewport3D::updateTransformTextOverlay(const QString& text) {
    if (!transformInfoActor_) return;
    
    transformInfoActor_->SetInput(text.toStdString().c_str());
    transformInfoActor_->VisibilityOn();
    
    if (renderWindow_) {
        renderWindow_->Render();
    }
}

void Viewport3D::hideTransformTextOverlay() {
    if (!transformInfoActor_) return;
    
    transformInfoActor_->VisibilityOff();
    transformInfoText_.clear();
    
    if (renderWindow_) {
        renderWindow_->Render();
    }
}

int Viewport3D::pickGizmoAxis(int x, int y) {
    if (!renderer_ || !renderWindow_) return -1;
    
    int* size = renderWindow_->GetSize();
    double displayY = static_cast<double>(size[1] - y - 1);
    
    vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
    picker->Pick(x, displayY, 0, renderer_);
    
    vtkActor* pickedActor = picker->GetActor();
    if (!pickedActor) return -1;
    
    if (pickedActor == gizmoXArrow_.GetPointer()) return 0; // X axis
    if (pickedActor == gizmoYArrow_.GetPointer()) return 1; // Y axis
    if (pickedActor == gizmoZArrow_.GetPointer()) return 2; // Z axis
    if (pickedActor == gizmoXYPlane_.GetPointer()) return 3; // XY plane
    if (pickedActor == gizmoXZPlane_.GetPointer()) return 4; // XZ plane
    if (pickedActor == gizmoYZPlane_.GetPointer()) return 5; // YZ plane
    if (pickedActor == gizmoRotateX_.GetPointer()) return 0;
    if (pickedActor == gizmoRotateY_.GetPointer()) return 1;
    if (pickedActor == gizmoRotateZ_.GetPointer()) return 2;
    if (pickedActor == gizmoScaleX_.GetPointer()) return 0;
    if (pickedActor == gizmoScaleY_.GetPointer()) return 1;
    if (pickedActor == gizmoScaleZ_.GetPointer()) return 2;
    
    return -1;
}

QVector3D Viewport3D::projectToPlane(const QVector3D& worldPos, ConstraintPlane plane, const QVector3D& planePoint) {
    QVector3D result = worldPos;
    
    switch (plane) {
        case ConstraintPlane::XY:
            result.setZ(planePoint.z());
            break;
        case ConstraintPlane::XZ:
            result.setY(planePoint.y());
            break;
        case ConstraintPlane::YZ:
            result.setX(planePoint.x());
            break;
        case ConstraintPlane::AxisX:
            result.setY(planePoint.y());
            result.setZ(planePoint.z());
            break;
        case ConstraintPlane::AxisY:
            result.setX(planePoint.x());
            result.setZ(planePoint.z());
            break;
        case ConstraintPlane::AxisZ:
            result.setX(planePoint.x());
            result.setY(planePoint.y());
            break;
        default:
            break;
    }
    
    return result;
}

std::vector<Viewport3D::AlignmentGuide> Viewport3D::findAlignments(VolumeNode* movingNode, const QVector3D& newPos) {
    std::vector<AlignmentGuide> guides;
    if (!sceneGraph_ || !movingNode) return guides;
    
    // Collect all other nodes' positions
    std::vector<QVector3D> otherPositions;
    sceneGraph_->traverse([&](VolumeNode* node) {
        if (node != movingNode && node != sceneGraph_->getRoot() && node->getShape()) {
            otherPositions.push_back(node->getTransform().getTranslation());
        }
    });
    
    // Check alignment with each other object
    for (const auto& otherPos : otherPositions) {
        // X alignment (centers aligned on X)
        if (std::abs(newPos.x() - otherPos.x()) < snapThreshold_) {
            AlignmentGuide guide;
            guide.type = AlignmentGuide::CenterX;
            guide.start = QVector3D(otherPos.x(), std::min(newPos.y(), otherPos.y()) - 50, newPos.z());
            guide.end = QVector3D(otherPos.x(), std::max(newPos.y(), otherPos.y()) + 50, newPos.z());
            guide.distance = std::abs(newPos.x() - otherPos.x());
            guides.push_back(guide);
        }
        
        // Y alignment
        if (std::abs(newPos.y() - otherPos.y()) < snapThreshold_) {
            AlignmentGuide guide;
            guide.type = AlignmentGuide::CenterY;
            guide.start = QVector3D(std::min(newPos.x(), otherPos.x()) - 50, otherPos.y(), newPos.z());
            guide.end = QVector3D(std::max(newPos.x(), otherPos.x()) + 50, otherPos.y(), newPos.z());
            guide.distance = std::abs(newPos.y() - otherPos.y());
            guides.push_back(guide);
        }
        
        // Z alignment
        if (std::abs(newPos.z() - otherPos.z()) < snapThreshold_) {
            AlignmentGuide guide;
            guide.type = AlignmentGuide::CenterZ;
            guide.start = QVector3D(newPos.x(), newPos.y(), otherPos.z());
            guide.end = QVector3D(otherPos.x(), otherPos.y(), otherPos.z());
            guide.distance = std::abs(newPos.z() - otherPos.z());
            guides.push_back(guide);
        }
    }
    
    return guides;
}

void Viewport3D::updateSmartGuides(VolumeNode* movingNode, const QVector3D& newPos) {
    // Clear existing guides
    clearSmartGuides();
    
    if (!renderer_) return;
    
    // Find alignments
    auto alignments = findAlignments(movingNode, newPos);
    
    // Create guide line actors
    for (const auto& guide : alignments) {
        vtkSmartPointer<vtkLineSource> line = vtkSmartPointer<vtkLineSource>::New();
        line->SetPoint1(guide.start.x(), guide.start.y(), guide.start.z());
        line->SetPoint2(guide.end.x(), guide.end.y(), guide.end.z());
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(line->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        
        // Color based on alignment type
        switch (guide.type) {
            case AlignmentGuide::CenterX:
                actor->GetProperty()->SetColor(1.0, 0.4, 0.4); // Red for X
                break;
            case AlignmentGuide::CenterY:
                actor->GetProperty()->SetColor(0.4, 1.0, 0.4); // Green for Y
                break;
            case AlignmentGuide::CenterZ:
                actor->GetProperty()->SetColor(0.4, 0.6, 1.0); // Blue for Z
                break;
            default:
                actor->GetProperty()->SetColor(1.0, 0.8, 0.2); // Yellow default
        }
        
        actor->GetProperty()->SetLineWidth(2.0);
        actor->GetProperty()->SetOpacity(0.8);
        actor->SetPickable(false);
        
        renderer_->AddActor(actor);
        guideActors_.push_back(actor);
    }
}

void Viewport3D::clearSmartGuides() {
    if (!renderer_) return;
    
    for (auto& actor : guideActors_) {
        if (actor) {
            renderer_->RemoveActor(actor);
        }
    }
    guideActors_.clear();
}

QVector3D Viewport3D::applySmartSnap(const QVector3D& pos, VolumeNode* movingNode) {
    if (!sceneGraph_ || !movingNode) return pos;
    
    QVector3D result = pos;
    
    // Collect all other nodes' positions
    sceneGraph_->traverse([&](VolumeNode* node) {
        if (node != movingNode && node != sceneGraph_->getRoot() && node->getShape()) {
            QVector3D otherPos = node->getTransform().getTranslation();
            
            // Snap X if close
            if (std::abs(pos.x() - otherPos.x()) < snapThreshold_) {
                result.setX(otherPos.x());
            }
            
            // Snap Y if close  
            if (std::abs(pos.y() - otherPos.y()) < snapThreshold_) {
                result.setY(otherPos.y());
            }
            
            // Snap Z if close
            if (std::abs(pos.z() - otherPos.z()) < snapThreshold_) {
                result.setZ(otherPos.z());
            }
        }
    });
    
    return result;
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
