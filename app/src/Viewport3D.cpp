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
#include <vtkBoxWidget.h>
#include <vtkCommand.h>
#include <vtkTransform.h>
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
    , gridVisible_(true)
    , gridSpacing_(50.0)
    , snapToGrid_(false)
    , manipulatedNode_(nullptr)
    , isManipulating_(false)
    , isPicking_(false)
{
    setupRenderer();
    setupInteractor();
    
    // Setup view cube (only after interactor is set)
    setupViewCube();
    
    setupManipulator();
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
    
    // Add grid for reference
    addGrid();
    
    // Note: Axes at origin removed - using orientation widget in corner instead
    
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

void Viewport3D::addGrid() {
    // Grid disabled - using only coordinate axes for orientation
    // The ViewCube provides better orientation feedback
    if (!renderer_) return;
    
    // Remove existing grid if present
    if (gridActor_) {
        renderer_->RemoveActor(gridActor_);
        gridActor_ = nullptr;
    }
    
    // Only add simple origin axes (short, visible at origin)
    if (!gridVisible_) return;
    
    const double axisLength = 100.0; // 100mm axes at origin
    
    // Create colored axes at origin
    auto createAxisActor = [this](double x1, double y1, double z1, 
                                   double x2, double y2, double z2,
                                   double r, double g, double b) {
        vtkSmartPointer<vtkLineSource> line = vtkSmartPointer<vtkLineSource>::New();
        line->SetPoint1(x1, y1, z1);
        line->SetPoint2(x2, y2, z2);
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(line->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(r, g, b);
        actor->GetProperty()->SetLineWidth(2.0);
        actor->GetProperty()->SetOpacity(0.8);
        
        return actor;
    };
    
    // X axis (red)
    auto xAxis = createAxisActor(0, 0, 0, axisLength, 0, 0, 1.0, 0.3, 0.3);
    renderer_->AddActor(xAxis);
    
    // Y axis (green)  
    auto yAxis = createAxisActor(0, 0, 0, 0, axisLength, 0, 0.3, 1.0, 0.3);
    renderer_->AddActor(yAxis);
    
    // Z axis (blue)
    auto zAxis = createAxisActor(0, 0, 0, 0, 0, axisLength, 0.3, 0.3, 1.0);
    renderer_->AddActor(zAxis);
}

void Viewport3D::setGridVisible(bool visible) {
    gridVisible_ = visible;
    if (!renderer_) return; // Safety check - renderer not initialized yet
    addGrid(); // Recreate grid with new visibility
    if (renderWindow_) {
        renderWindow_->Render();
    }
}

void Viewport3D::setGridSpacing(double spacing) {
    if (spacing <= 0.0) return;
    if (!renderer_) return; // Safety check - renderer not initialized yet
    gridSpacing_ = spacing;
    addGrid(); // Recreate grid with new spacing
    if (renderWindow_) {
        renderWindow_->Render();
    }
}

void Viewport3D::setSnapToGrid(bool enabled) {
    snapToGrid_ = enabled;
    // Snap-to-grid will be applied during transform operations
}

// Removed addAxesAtOrigin() - using orientation widget in corner instead
#endif

void Viewport3D::setSceneGraph(SceneGraph* sceneGraph) {
    sceneGraph_ = sceneGraph;
    updateScene();
    updateManipulator();
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
    
    // Disable manipulator before clearing actors to prevent crashes
    if (boxWidget_) {
        boxWidget_->Off();
        manipulatedNode_ = nullptr;
    }
    
    // Clear existing actors safely
    // First, remove all actors from renderer
    for (auto& pair : actors_) {
        if (pair.second) {
            renderer_->RemoveActor(pair.second);
        }
    }
    
    // Force render to ensure actors are removed before clearing map
    if (renderWindow_) {
        renderWindow_->Render();
    }
    
    // Now clear the map - actors will be destroyed when smart pointers go out of scope
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
    
    updateManipulator();
#endif
}

void Viewport3D::setCommandStack(CommandStack* commandStack) {
    commandStack_ = commandStack;
}

void Viewport3D::setInteractionMode(InteractionMode mode) {
    interactionMode_ = mode;
    updateManipulator();
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
        // Show context menu
        showContextMenu(event->pos());
        return;
    }
    
    // Store mouse position for drag detection
    if (event->button() == Qt::LeftButton) {
        lastPickPos_ = event->pos();
        isPicking_ = false; // Will be set to true only if we actually pick
    }
    
    // Always let VTK handle mouse events first for camera rotation
    // We'll do picking in mouseReleaseEvent if it was a click (not drag)
    QVTKOpenGLNativeWidget::mousePressEvent(event);
#else
    QWidget::mousePressEvent(event);
#endif
}

void Viewport3D::mouseReleaseEvent(QMouseEvent* event) {
#ifndef GEANTCAD_NO_VTK
    // Handle picking ONLY if it was a click (not a drag)
    if (event->button() == Qt::LeftButton && interactionMode_ == InteractionMode::Select) {
        QPoint currentPos = event->pos();
        QPoint delta = currentPos - lastPickPos_;
        int moveThreshold = 3; // pixels - very small threshold
        
        // Only do picking if mouse didn't move much (it was a click, not drag)
        if (delta.manhattanLength() < moveThreshold && interactor_ && renderer_) {
            // It was a click - do picking now
            int x = event->pos().x();
            int y = event->pos().y();
            
            vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
            picker->Pick(x, renderWindow_->GetSize()[1] - y - 1, 0, renderer_);
            
            vtkActor* pickedActor = picker->GetActor();
            if (pickedActor) {
                // Find VolumeNode from actor - verify it's still valid
                VolumeNode* pickedNode = nullptr;
                for (const auto& pair : actors_) {
                    if (pair.second && pair.second.GetPointer() == pickedActor) {
                        pickedNode = pair.first;
                        break;
                    }
                }
                
                if (pickedNode && actors_.find(pickedNode) != actors_.end()) {
                    // Verify actor is still valid
                    if (actors_[pickedNode] && actors_[pickedNode].GetPointer() == pickedActor) {
                        updateSelectionHighlight(pickedNode);
                        emit selectionChanged(pickedNode);
                    }
                }
            } else {
                // Click on empty space - clear selection
                updateSelectionHighlight(nullptr);
                emit selectionChanged(nullptr);
            }
            
            // Ensure manipulator is OFF in Select mode
            if (boxWidget_) {
                boxWidget_->Off();
                manipulatedNode_ = nullptr;
            }
            
            // Render selection highlight
            if (renderWindow_) {
                renderWindow_->Render();
            }
        }
        // If it was a drag, VTK already handled camera rotation - do nothing
    }
    
    // Handle manipulator completion
    if (isManipulating_ && manipulatedNode_ && commandStack_) {
        // Verify node still exists
        if (actors_.find(manipulatedNode_) != actors_.end()) {
            Transform newTransform = manipulatedNode_->getTransform();
            auto cmd = std::make_unique<TransformVolumeCommand>(manipulatedNode_, newTransform);
            commandStack_->execute(std::move(cmd));
        }
        
        isManipulating_ = false;
        emit viewChanged();
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
    
    // CRITICAL: Don't render immediately during picking to avoid glitch
    // Render will be triggered naturally by VTK event processing or after mouse release
    // Only render if not currently picking
    if (renderWindow_ && !isPicking_) {
        renderWindow_->Render();
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

void Viewport3D::setupManipulator() {
#ifndef GEANTCAD_NO_VTK
    if (!interactor_ || !renderer_) return;
    
    boxWidget_ = vtkSmartPointer<vtkBoxWidget>::New();
    boxWidget_->SetInteractor(interactor_);
    boxWidget_->SetPlaceFactor(1.25);
    boxWidget_->SetRotationEnabled(interactionMode_ == InteractionMode::Rotate);
    boxWidget_->SetTranslationEnabled(interactionMode_ == InteractionMode::Move);
    boxWidget_->SetScalingEnabled(interactionMode_ == InteractionMode::Scale);
    boxWidget_->Off();
    
    // Connect callback for transform updates using command pattern
    class ManipulatorCallback : public vtkCommand {
    public:
        static ManipulatorCallback* New() { return new ManipulatorCallback; }
        Viewport3D* viewport = nullptr;
        void Execute(vtkObject* caller, unsigned long eventId, void* callData) override {
            if (viewport && eventId == vtkCommand::InteractionEvent) {
                viewport->onManipulatorInteraction();
            }
        }
    };
    
    vtkSmartPointer<ManipulatorCallback> callback = vtkSmartPointer<ManipulatorCallback>::New();
    callback->viewport = this;
    boxWidget_->AddObserver(vtkCommand::InteractionEvent, callback);
#endif
}

void Viewport3D::updateManipulator() {
#ifndef GEANTCAD_NO_VTK
    if (!boxWidget_ || !sceneGraph_) return;
    
    VolumeNode* selected = sceneGraph_->getSelected();
    
    // Disable manipulator if no selection or in Select mode
    if (!selected || selected == sceneGraph_->getRoot() || interactionMode_ == InteractionMode::Select) {
        boxWidget_->Off();
        manipulatedNode_ = nullptr;
        return;
    }
    
    // Find actor for selected node
    auto it = actors_.find(selected);
    if (it == actors_.end() || !it->second) {
        boxWidget_->Off();
        manipulatedNode_ = nullptr;
        return;
    }
    
    vtkActor* actor = it->second.GetPointer();
    if (!actor) {
        boxWidget_->Off();
        manipulatedNode_ = nullptr;
        return;
    }
    
    // CRITICAL: Safely get bounds with error checking
    try {
        double bounds[6];
        actor->GetBounds(bounds);
        
        // Validate bounds
        bool validBounds = true;
        for (int i = 0; i < 6; ++i) {
            if (!std::isfinite(bounds[i])) {
                validBounds = false;
                break;
            }
        }
        
        if (!validBounds) {
            boxWidget_->Off();
            manipulatedNode_ = nullptr;
            return;
        }
        
        // Configure manipulator based on mode
        boxWidget_->SetRotationEnabled(interactionMode_ == InteractionMode::Rotate);
        boxWidget_->SetTranslationEnabled(interactionMode_ == InteractionMode::Move);
        boxWidget_->SetScalingEnabled(interactionMode_ == InteractionMode::Scale);
        
        // Place box widget around selected actor
        boxWidget_->SetPlaceFactor(1.25);
        boxWidget_->PlaceWidget(bounds);
        boxWidget_->On();
        
        manipulatedNode_ = selected;
    } catch (...) {
        // If anything fails, disable manipulator safely
        boxWidget_->Off();
        manipulatedNode_ = nullptr;
    }
    
    if (renderWindow_) {
        renderWindow_->Render();
    }
#endif
}

void Viewport3D::onManipulatorInteraction() {
#ifndef GEANTCAD_NO_VTK
    if (!boxWidget_ || !manipulatedNode_ || !sceneGraph_) return;
    
    isManipulating_ = true;
    
    // Get transform from box widget
    vtkSmartPointer<vtkTransform> vtkXForm = vtkSmartPointer<vtkTransform>::New();
    boxWidget_->GetTransform(vtkXForm);
    
    // Convert VTK transform to our Transform
    vtkMatrix4x4* vtkMatrix = vtkXForm->GetMatrix();
    
    // Extract translation
    QVector3D translation(
        vtkMatrix->GetElement(0, 3),
        vtkMatrix->GetElement(1, 3),
        vtkMatrix->GetElement(2, 3)
    );
    
    // Extract rotation (from rotation part of matrix)
    QMatrix4x4 qtMatrix;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            qtMatrix(i, j) = vtkMatrix->GetElement(i, j);
        }
    }
    QQuaternion rotation = QQuaternion::fromRotationMatrix(qtMatrix.toGenericMatrix<3, 3>());
    
    // Extract scale (from diagonal of rotation matrix)
    QVector3D scale(
        QVector3D(qtMatrix(0, 0), qtMatrix(0, 1), qtMatrix(0, 2)).length(),
        QVector3D(qtMatrix(1, 0), qtMatrix(1, 1), qtMatrix(1, 2)).length(),
        QVector3D(qtMatrix(2, 0), qtMatrix(2, 1), qtMatrix(2, 2)).length()
    );
    
    // Update transform based on mode
    Transform& nodeTransform = manipulatedNode_->getTransform();
    
    if (interactionMode_ == InteractionMode::Move) {
        nodeTransform.setTranslation(translation);
    } else if (interactionMode_ == InteractionMode::Rotate) {
        nodeTransform.setRotation(rotation);
    } else if (interactionMode_ == InteractionMode::Scale) {
        nodeTransform.setScale(scale);
    }
    
    // Update actor transform - verify node still exists in map
    auto it = actors_.find(manipulatedNode_);
    if (it != actors_.end() && it->second) {
        vtkActor* actor = it->second.GetPointer();
        if (!actor) return; // Safety check
        auto worldTransform = manipulatedNode_->getWorldTransform();
        auto matrix = worldTransform.getMatrix();
        
        vtkSmartPointer<vtkTransform> vtkActorTransform = vtkSmartPointer<vtkTransform>::New();
        vtkSmartPointer<vtkMatrix4x4> vtkMatrix2 = vtkSmartPointer<vtkMatrix4x4>::New();
        const float* data = matrix.constData();
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                vtkMatrix2->SetElement(i, j, data[i * 4 + j]);
            }
        }
        vtkActorTransform->SetMatrix(vtkMatrix2);
        actor->SetUserTransform(vtkActorTransform);
    }
    
    if (renderWindow_) {
        renderWindow_->Render();
    }
    
    emit viewChanged();
#endif
}
#endif

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
