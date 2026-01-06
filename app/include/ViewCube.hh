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
    
    // Camera state
    QQuaternion cameraOrientation_;
    QVector3D cameraPosition_;
    
    // Cube faces
    std::vector<Face> faces_;
    int hoveredFace_ = -1;
    
    // Mouse interaction
    bool isDragging_ = false;
    QPoint lastMousePos_;
    
    // Visual settings
    float cubeSize_ = 60.0f;
    float perspective_ = 500.0f;
    
    // Colors
    QColor frontColor_{0, 120, 212};     // Blue
    QColor backColor_{100, 100, 100};    // Gray
    QColor leftColor_{212, 120, 0};      // Orange
    QColor rightColor_{0, 180, 100};     // Green
    QColor topColor_{180, 60, 60};       // Red
    QColor bottomColor_{120, 60, 180};   // Purple
    QColor edgeColor_{60, 60, 60};
    QColor textColor_{255, 255, 255};
    QColor hoverColor_{255, 220, 100};   // Yellow highlight

#ifndef GEANTCAD_NO_VTK
    vtkCamera* camera_ = nullptr;
    vtkRenderer* renderer_ = nullptr;
#endif
};

} // namespace geantcad

