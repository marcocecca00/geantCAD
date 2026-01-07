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
class vtkTextActor;

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
        Select,  // Also works as Pan when clicking on empty space
        Move,
        Rotate,
        Scale
    };
    void setInteractionMode(InteractionMode mode);
    InteractionMode getInteractionMode() const { return interactionMode_; }
    
    // Movement constraint planes
    enum class ConstraintPlane {
        None,       // Free movement (screen-aligned)
        XY,         // Movement constrained to XY plane (Z=const)
        XZ,         // Movement constrained to XZ plane (Y=const)
        YZ,         // Movement constrained to YZ plane (X=const)
        AxisX,      // Movement along X axis only
        AxisY,      // Movement along Y axis only
        AxisZ       // Movement along Z axis only
    };
    void setConstraintPlane(ConstraintPlane plane);
    ConstraintPlane getConstraintPlane() const { return constraintPlane_; }
    
    // Projection mode
    enum class ProjectionMode {
        Perspective,
        Orthographic
    };
    void setProjectionMode(ProjectionMode mode);
    ProjectionMode getProjectionMode() const { return projectionMode_; }
    
    // Wireframe mode
    void setWireframeMode(bool enabled);
    bool isWireframeMode() const { return wireframeMode_; }
    
    // Camera controls
    void resetView();
    void frameSelection();
    void zoom(double factor);  // 1.1 for zoom in, 0.9 for zoom out
    
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
    
    // Background color
    void setBackgroundColor(double r, double g, double b);
    void getBackgroundColor(double& r, double& g, double& b) const;
    
    // Proportional scaling toggle
    void setProportionalScaling(bool enabled) { proportionalScaling_ = enabled; }
    bool isProportionalScaling() const { return proportionalScaling_; }
    
    // Transform info text (for status display)
    QString getTransformInfo() const { return transformInfoText_; }
    
    // Rendering
    void refresh();
    
    // Camera control access
    vtkRenderer* getRenderer() const;
    vtkCamera* getCamera() const;

    // Measurement mode
    void setMeasurementMode(bool enabled);
    bool isMeasurementMode() const { return measurementMode_; }

signals:
    void selectionChanged(VolumeNode* node);
    void viewChanged();
    void objectTransformed(VolumeNode* node);
    void pointPicked(const QVector3D& point);  // For measurement tool
    void measurementModeChanged(bool enabled);  // Notify when measurement mode changes
    void interactionModeChanged(InteractionMode mode);  // Sync toolbar with viewport
    void mouseWorldCoordinates(double x, double y, double z);  // Real-time mouse position
    void objectInfoRequested(VolumeNode* node);  // Show object info in status bar
    
    // Boolean operations from context menu
    void booleanUnionRequested();
    void booleanSubtractionRequested();
    void booleanIntersectionRequested();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setupRenderer();
    void setupInteractor();
    void updateScene();
    void setupViewCube();
    void createGrid();
    void updateGrid();
    void createWorldBox();  // Geant4 world volume wireframe
#ifndef GEANTCAD_NO_VTK
    void updateSelectionHighlight(VolumeNode* selectedNode);
    void showContextMenu(const QPoint& pos);
#endif
    
    SceneGraph* sceneGraph_;
    CommandStack* commandStack_ = nullptr;
    InteractionMode interactionMode_ = InteractionMode::Select;
    ConstraintPlane constraintPlane_ = ConstraintPlane::XY; // Default to XY plane (ground)
    ProjectionMode projectionMode_ = ProjectionMode::Orthographic;  // CAD default
    bool measurementMode_ = false;  // For measurement tool picking
    bool wireframeMode_ = false;  // Toggle solid/wireframe
    
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
    
    // World box (Geant4 world volume - 1m x 1m x 1m wireframe)
    vtkSmartPointer<vtkActor> worldBoxActor_;
    double gridSpacing_ = 10.0; // mm - 5 squares = 50mm (typical object size)
    bool snapToGrid_ = false;
    
    // Picking state
    QPoint lastPickPos_;
    
    // Manipulation state
    bool isDragging_ = false;
    VolumeNode* draggedNode_ = nullptr;
    QVector3D dragStartWorldPos_;
    Transform dragStartTransform_;
    
    // Pan mode state
    bool isPanning_ = false;
    QPoint lastPanPos_;
    
    // View cube widget
    vtkSmartPointer<vtkOrientationMarkerWidget> viewCubeWidget_;
    
    // Gizmo actors
    vtkSmartPointer<vtkActor> gizmoXArrow_;
    vtkSmartPointer<vtkActor> gizmoYArrow_;
    vtkSmartPointer<vtkActor> gizmoZArrow_;
    vtkSmartPointer<vtkActor> gizmoXYPlane_;
    vtkSmartPointer<vtkActor> gizmoXZPlane_;
    vtkSmartPointer<vtkActor> gizmoYZPlane_;
    vtkSmartPointer<vtkActor> gizmoRotateX_;
    vtkSmartPointer<vtkActor> gizmoRotateY_;
    vtkSmartPointer<vtkActor> gizmoRotateZ_;
    vtkSmartPointer<vtkActor> gizmoScaleX_;
    vtkSmartPointer<vtkActor> gizmoScaleY_;
    vtkSmartPointer<vtkActor> gizmoScaleZ_;
    int activeGizmoAxis_ = -1; // -1 = none, 0 = X, 1 = Y, 2 = Z, 3 = XY, 4 = XZ, 5 = YZ
    
    // Smart guides (alignment lines)
    std::vector<vtkSmartPointer<vtkActor>> guideActors_;
    double snapThreshold_ = 10.0; // Distance threshold for snap suggestion (mm)
    
    // Axis labels
    std::vector<vtkSmartPointer<vtkActor>> axisLabels_;
    
    // Background color
    double bgColorR_ = 0.15, bgColorG_ = 0.15, bgColorB_ = 0.20;
    
    // Proportional scaling (false = free scaling like BambuLab slicer)
    bool proportionalScaling_ = false; // Default to free scaling
    
    // Transform info text overlay
    vtkSmartPointer<vtkTextActor> transformInfoActor_;
    QString transformInfoText_;
    void updateTransformTextOverlay(const QString& text);
    void hideTransformTextOverlay();
    
    // Grid scale indicator
    vtkSmartPointer<vtkTextActor> gridScaleActor_;
    
    // Helper functions for manipulation
    QVector3D screenToWorld(int x, int y, double depth = 0.0);
    double getDepthAtPosition(int x, int y);
    QVector3D projectToPlane(const QVector3D& worldPos, ConstraintPlane plane, const QVector3D& planePoint);
    void createGizmos();
    void updateGizmoPosition();
    void showGizmo(bool show);
    void updateGizmoHighlight(int hoveredAxis);
    int pickGizmoAxis(int x, int y);
    
    // Smart guides system
    struct AlignmentGuide {
        QVector3D start;
        QVector3D end;
        double distance; // Distance to alignment point
        enum Type { CenterX, CenterY, CenterZ, EdgeAlign, DistanceEqual } type;
    };
    std::vector<AlignmentGuide> findAlignments(VolumeNode* movingNode, const QVector3D& newPos);
    void updateSmartGuides(VolumeNode* movingNode, const QVector3D& newPos);
    void clearSmartGuides();
    QVector3D applySmartSnap(const QVector3D& pos, VolumeNode* movingNode);
#endif
};

} // namespace geantcad

