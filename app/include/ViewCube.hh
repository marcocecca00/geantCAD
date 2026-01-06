#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

#ifndef GEANTCAD_NO_VTK
#include <vtkCamera.h>
#include <vtkRenderer.h>
#endif

namespace geantcad {

/**
 * @brief Interactive 3D ViewCube widget for viewport orientation
 * 
 * Provides quick access to standard views (Top, Front, Right, etc.)
 * and visual feedback of current camera orientation.
 */
class ViewCube : public QWidget {
    Q_OBJECT

public:
    enum class ViewOrientation {
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom,
        FrontTopRight,  // Isometric
        FrontTopLeft,
        BackTopRight,
        BackTopLeft
    };
    Q_ENUM(ViewOrientation)

    explicit ViewCube(QWidget* parent = nullptr);
    ~ViewCube() override = default;

#ifndef GEANTCAD_NO_VTK
    void setCamera(vtkCamera* camera);
    void setRenderer(vtkRenderer* renderer);
#endif

    void setCameraOrientation(const QQuaternion& orientation);
    QQuaternion getCameraOrientation() const { return cameraOrientation_; }

signals:
    void viewOrientationRequested(ViewOrientation orientation);
    void viewChanged();
    void zoomRequested(double factor);  // 1.1 for zoom in, 0.9 for zoom out

public slots:
    void updateFromCamera();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    struct Face {
        QString label;
        QPolygonF polygon;
        ViewOrientation orientation;
        QColor baseColor;
        bool hovered = false;
    };

    void updateFaces();
    QPointF project3DTo2D(const QVector3D& point) const;
    int hitTest(const QPoint& pos) const;
    void applyViewOrientation(ViewOrientation orientation);
    void drawCornerIndicators(QPainter& painter);
    void drawAxisIndicators(QPainter& painter);
    
    // Camera state
    QQuaternion cameraOrientation_;
    QVector3D cameraPosition_;
    
    // Cube faces
    std::vector<Face> faces_;
    int hoveredFace_ = -1;
    
    // Mouse interaction
    bool isDragging_ = false;
    QPoint lastMousePos_;
    
    // Zoom button state
    QPointF zoomInCenter_;
    QPointF zoomOutCenter_;
    float zoomBtnRadius_ = 10.0f;
    bool zoomInHovered_ = false;
    bool zoomOutHovered_ = false;
    
    // Visual settings
    float cubeSize_ = 50.0f;  // Cube size
    float perspective_ = 450.0f;
    
    // Modern dark theme colors (will be overridden in constructor)
    QColor frontColor_{60, 70, 85};
    QColor backColor_{50, 58, 70};
    QColor leftColor_{55, 65, 78};
    QColor rightColor_{55, 65, 78};
    QColor topColor_{70, 82, 100};
    QColor bottomColor_{45, 52, 62};
    QColor edgeColor_{90, 100, 115};
    QColor textColor_{200, 210, 220};
    QColor hoverColor_{80, 140, 220};  // Accent blue

#ifndef GEANTCAD_NO_VTK
    vtkCamera* camera_ = nullptr;
    vtkRenderer* renderer_ = nullptr;
#endif
};

} // namespace geantcad

