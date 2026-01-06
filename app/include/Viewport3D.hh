#pragma once

#include <QWidget>
#include <QPoint>

#ifndef GEANTCAD_NO_VTK
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOrientationMarkerWidget.h>
#endif

#include "../../core/include/SceneGraph.hh"
#include "../../core/include/Shape.hh"
#include "../../core/include/CommandStack.hh"
#include <map>

// Forward declarations
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyDataAlgorithm;

namespace geantcad {

/**
 * Viewport3D: widget 3D usando VTK per rendering e interazione.
 * MVP: rendering base + camera controls (orbit/pan/zoom)
 */
#ifdef GEANTCAD_NO_VTK
class Viewport3D : public QWidget {
#else
class Viewport3D : public QVTKOpenGLNativeWidget {
#endif
    Q_OBJECT

public:
    explicit Viewport3D(QWidget* parent = nullptr);
    ~Viewport3D();

    void setSceneGraph(SceneGraph* sceneGraph);
    void setCommandStack(CommandStack* commandStack);
    
    // Interaction mode
    enum class InteractionMode {
        Select,
        Move,
        Rotate,
        Scale
    };
    void setInteractionMode(InteractionMode mode);
    InteractionMode getInteractionMode() const { return interactionMode_; }
    
    // Camera controls
    void resetView();
    void frameSelection();
    
    // Standard view presets (for view-cube)
    enum class StandardView {
        Front,      // +Y
        Back,       // -Y
        Left,       // +X
        Right,      // -X
        Top,        // +Z
        Bottom,     // -Z
        Isometric   // 45° isometric
    };
    void setStandardView(StandardView view);
    
    // Grid controls
    void setGridVisible(bool visible);
    bool isGridVisible() const { return gridVisible_; }
    void setGridSpacing(double spacing); // in mm
    double getGridSpacing() const { return gridSpacing_; }
    void setSnapToGrid(bool enabled);
    bool isSnapToGrid() const { return snapToGrid_; }
    
    // Rendering
    void refresh();
    
    // Camera control access
    vtkRenderer* getRenderer() const;
    vtkCamera* getCamera() const;

signals:
    void selectionChanged(VolumeNode* node);
    void viewChanged();
    void objectTransformed(VolumeNode* node);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void setupRenderer();
    void setupInteractor();
    void updateScene();
    void setupViewCube();
    void createGrid();
    void updateGrid();
#ifndef GEANTCAD_NO_VTK
    void updateSelectionHighlight(VolumeNode* selectedNode);
    void showContextMenu(const QPoint& pos);
#endif
    
    SceneGraph* sceneGraph_;
    CommandStack* commandStack_ = nullptr;
    InteractionMode interactionMode_ = InteractionMode::Select;
    
#ifndef GEANTCAD_NO_VTK
    vtkSmartPointer<vtkRenderer> renderer_;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow_;
    vtkSmartPointer<vtkRenderWindowInteractor> interactor_;
    
    // Actor storage (volume -> actor mapping)
    std::map<VolumeNode*, vtkSmartPointer<vtkActor>> actors_;
    
    // Grid
    vtkSmartPointer<vtkActor> gridActor_;
    vtkSmartPointer<vtkActor> axisXActor_;
    vtkSmartPointer<vtkActor> axisYActor_;
    vtkSmartPointer<vtkActor> axisZActor_;
    bool gridVisible_ = true;  // Enabled by default
    double gridSpacing_ = 50.0; // mm
    bool snapToGrid_ = false;
    
    // Picking state
    QPoint lastPickPos_;
    
    // Manipulation state
    bool isDragging_ = false;
    VolumeNode* draggedNode_ = nullptr;
    QVector3D dragStartWorldPos_;
    Transform dragStartTransform_;
    
    // View cube widget
    vtkSmartPointer<vtkOrientationMarkerWidget> viewCubeWidget_;
    
    // Helper functions for manipulation
    QVector3D screenToWorld(int x, int y, double depth = 0.0);
    double getDepthAtPosition(int x, int y);
#endif
};

} // namespace geantcad

