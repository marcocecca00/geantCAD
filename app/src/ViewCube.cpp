#include "ViewCube.hh"
#include <QPainterPath>
#include <QFontMetrics>
#include <QLineF>
#include <cmath>
#include <algorithm>

namespace geantcad {

ViewCube::ViewCube(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(150, 200);  // Larger: bigger cube + more space for controls
    setAttribute(Qt::WA_TranslucentBackground, true);
    
    // Colored faces like axis colors (more distinct)
    frontColor_ = QColor(80, 150, 80);    // Green (Y+)
    backColor_ = QColor(60, 100, 60);     // Dark green (Y-)
    leftColor_ = QColor(150, 80, 80);     // Red (X-)
    rightColor_ = QColor(200, 100, 100);  // Light red (X+)
    topColor_ = QColor(100, 130, 200);    // Blue (Z+)
    bottomColor_ = QColor(70, 90, 140);   // Dark blue (Z-)
    edgeColor_ = QColor(40, 40, 45);
    textColor_ = QColor(255, 255, 255);
    hoverColor_ = QColor(255, 200, 100);  // Bright orange on hover
    
    // Initialize with isometric view orientation
    cameraOrientation_ = QQuaternion::fromEulerAngles(-30.0f, 45.0f, 0.0f);
    cubeSize_ = 60.0f;  // Larger cube for the camera
    
    // Initialize zoom button positions (will be updated in paintEvent)
    zoomInCenter_ = QPointF(90, 170);
    zoomOutCenter_ = QPointF(122, 170);
    zoomBtnRadius_ = 11.0f;
    
    updateFaces();
}

#ifndef GEANTCAD_NO_VTK
void ViewCube::setCamera(vtkCamera* camera) {
    camera_ = camera;
    updateFromCamera();
}

void ViewCube::setRenderer(vtkRenderer* renderer) {
    renderer_ = renderer;
}

void ViewCube::updateFromCamera() {
    if (!camera_) return;
    
    // Get camera direction vectors
    double pos[3], focal[3], up[3];
    camera_->GetPosition(pos);
    camera_->GetFocalPoint(focal);
    camera_->GetViewUp(up);
    
    // Calculate view direction
    QVector3D viewDir(focal[0] - pos[0], focal[1] - pos[1], focal[2] - pos[2]);
    viewDir.normalize();
    
    QVector3D upVec(up[0], up[1], up[2]);
    upVec.normalize();
    
    QVector3D rightVec = QVector3D::crossProduct(viewDir, upVec);
    rightVec.normalize();
    
    // Recalculate up to ensure orthogonality
    upVec = QVector3D::crossProduct(rightVec, viewDir);
    
    // Build rotation matrix and convert to quaternion
    QMatrix4x4 rotMatrix;
    rotMatrix.setColumn(0, QVector4D(rightVec, 0));
    rotMatrix.setColumn(1, QVector4D(upVec, 0));
    rotMatrix.setColumn(2, QVector4D(-viewDir, 0));
    rotMatrix.setColumn(3, QVector4D(0, 0, 0, 1));
    
    cameraOrientation_ = QQuaternion::fromRotationMatrix(rotMatrix.toGenericMatrix<3, 3>());
    
    updateFaces();
    update();
}
#endif

void ViewCube::setCameraOrientation(const QQuaternion& orientation) {
    cameraOrientation_ = orientation;
    updateFaces();
    update();
}

QPointF ViewCube::project3DTo2D(const QVector3D& point) const {
    // Apply camera orientation
    QVector3D rotated = cameraOrientation_.rotatedVector(point);
    
    // Simple perspective projection
    float z = rotated.z() + perspective_;
    float scale = perspective_ / z;
    
    float centerX = width() / 2.0f;
    float centerY = height() / 2.0f;
    
    return QPointF(
        centerX + rotated.x() * scale,
        centerY - rotated.y() * scale  // Invert Y for screen coordinates
    );
}

void ViewCube::updateFaces() {
    faces_.clear();
    
    float s = cubeSize_ / 2.0f;
    
    // Define cube vertices
    QVector3D vertices[8] = {
        {-s, -s, -s}, { s, -s, -s}, { s,  s, -s}, {-s,  s, -s},  // Back face
        {-s, -s,  s}, { s, -s,  s}, { s,  s,  s}, {-s,  s,  s}   // Front face
    };
    
    // Define faces (indices and orientations)
    struct FaceDef {
        int v[4];
        QString label;
        ViewOrientation orientation;
        QColor color;
    };
    
    FaceDef faceDefs[] = {
        {{4, 5, 6, 7}, "FRONT", ViewOrientation::Front, frontColor_},
        {{1, 0, 3, 2}, "BACK", ViewOrientation::Back, backColor_},
        {{0, 4, 7, 3}, "LEFT", ViewOrientation::Left, leftColor_},
        {{5, 1, 2, 6}, "RIGHT", ViewOrientation::Right, rightColor_},
        {{7, 6, 2, 3}, "TOP", ViewOrientation::Top, topColor_},
        {{0, 1, 5, 4}, "BOTTOM", ViewOrientation::Bottom, bottomColor_}
    };
    
    // Calculate face depths for sorting (painter's algorithm)
    struct FaceWithDepth {
        Face face;
        float depth;
    };
    std::vector<FaceWithDepth> facesWithDepth;
    
    for (const auto& def : faceDefs) {
        Face face;
        face.label = def.label;
        face.orientation = def.orientation;
        face.baseColor = def.color;
        
        QPolygonF polygon;
        QVector3D center(0, 0, 0);
        
        for (int i = 0; i < 4; ++i) {
            QPointF projected = project3DTo2D(vertices[def.v[i]]);
            polygon << projected;
            center += vertices[def.v[i]];
        }
        center /= 4.0f;
        
        face.polygon = polygon;
        
        // Calculate depth (Z after rotation)
        QVector3D rotatedCenter = cameraOrientation_.rotatedVector(center);
        
        FaceWithDepth fwd;
        fwd.face = face;
        fwd.depth = rotatedCenter.z();
        facesWithDepth.push_back(fwd);
    }
    
    // Sort by depth (back to front)
    std::sort(facesWithDepth.begin(), facesWithDepth.end(),
              [](const FaceWithDepth& a, const FaceWithDepth& b) {
                  return a.depth < b.depth;  // Draw back faces first
              });
    
    for (const auto& fwd : facesWithDepth) {
        faces_.push_back(fwd.face);
    }
}

void ViewCube::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // NO background container - just draw the cube directly
    
    // Draw faces (already sorted back to front)
    for (size_t i = 0; i < faces_.size(); ++i) {
        const Face& face = faces_[i];
        
        // Modern gradient fill for faces
        QColor faceColor = face.baseColor;
        if (face.hovered) {
            faceColor = hoverColor_;
        }
        
        // Smooth lighting
        float lightFactor = 0.75f + 0.25f * (static_cast<float>(i) / faces_.size());
        faceColor = QColor(
            static_cast<int>(faceColor.red() * lightFactor),
            static_cast<int>(faceColor.green() * lightFactor),
            static_cast<int>(faceColor.blue() * lightFactor),
            240
        );
        
        // Draw face
        painter.setBrush(faceColor);
        painter.setPen(QPen(QColor(edgeColor_.red(), edgeColor_.green(), edgeColor_.blue(), 150), 1.5));
        painter.drawPolygon(face.polygon);
        
        // Draw label on visible faces
        if (i >= faces_.size() - 3) {
            QPointF center = face.polygon.boundingRect().center();
            
            QFont font("Segoe UI", 8);
            font.setBold(true);
            painter.setFont(font);
            
            // Text shadow
            painter.setPen(QColor(0, 0, 0, 180));
            painter.drawText(QRectF(center.x() - 20 + 1, center.y() - 6 + 1, 40, 12), Qt::AlignCenter, face.label);
            
            // Main text
            painter.setPen(face.hovered ? Qt::black : textColor_);
            painter.drawText(QRectF(center.x() - 20, center.y() - 6, 40, 12), Qt::AlignCenter, face.label);
        }
    }
    
    // === Bottom section: axis indicator on left, zoom buttons on right ===
    // More space between cube and controls
    float bottomY = height() - 30;  // Controls at bottom
    
    // Small axis indicator (left side)
    float axisX = 30;
    float axisY = bottomY;
    float axisLen = 20;
    
    // Transform axes by camera orientation
    QVector3D xAxis = cameraOrientation_.rotatedVector(QVector3D(1, 0, 0));
    QVector3D yAxis = cameraOrientation_.rotatedVector(QVector3D(0, 1, 0));
    QVector3D zAxis = cameraOrientation_.rotatedVector(QVector3D(0, 0, 1));
    
    // X axis (red)
    painter.setPen(QPen(QColor(230, 80, 80), 2.5));
    painter.drawLine(QPointF(axisX, axisY), QPointF(axisX + xAxis.x() * axisLen, axisY - xAxis.y() * axisLen));
    
    // Y axis (green)
    painter.setPen(QPen(QColor(80, 210, 80), 2.5));
    painter.drawLine(QPointF(axisX, axisY), QPointF(axisX + yAxis.x() * axisLen, axisY - yAxis.y() * axisLen));
    
    // Z axis (blue)
    painter.setPen(QPen(QColor(80, 130, 230), 2.5));
    painter.drawLine(QPointF(axisX, axisY), QPointF(axisX + zAxis.x() * axisLen, axisY - zAxis.y() * axisLen));
    
    // Axis labels
    QFont smallFont("Segoe UI", 8);
    smallFont.setBold(true);
    painter.setFont(smallFont);
    painter.setPen(QColor(230, 80, 80));
    painter.drawText(QPointF(axisX + xAxis.x() * axisLen + 3, axisY - xAxis.y() * axisLen + 4), "X");
    painter.setPen(QColor(80, 210, 80));
    painter.drawText(QPointF(axisX + yAxis.x() * axisLen + 3, axisY - yAxis.y() * axisLen + 4), "Y");
    painter.setPen(QColor(80, 130, 230));
    painter.drawText(QPointF(axisX + zAxis.x() * axisLen + 3, axisY - zAxis.y() * axisLen + 4), "Z");
    
    // === Zoom buttons (right side, more spaced) ===
    float zoomBtnY = bottomY;
    float zoomInX = width() - 60;
    float zoomOutX = width() - 28;
    float btnRadius = 11;
    
    // Store button positions for click handling
    zoomInCenter_ = QPointF(zoomInX, zoomBtnY);
    zoomOutCenter_ = QPointF(zoomOutX, zoomBtnY);
    zoomBtnRadius_ = btnRadius;
    
    // Zoom In button (+)
    QColor zoomInColor = zoomInHovered_ ? QColor(100, 180, 255) : QColor(70, 80, 95);
    painter.setBrush(zoomInColor);
    painter.setPen(QPen(QColor(120, 130, 150), 1.5));
    painter.drawEllipse(QPointF(zoomInX, zoomBtnY), btnRadius, btnRadius);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(QPointF(zoomInX - 5, zoomBtnY), QPointF(zoomInX + 5, zoomBtnY));
    painter.drawLine(QPointF(zoomInX, zoomBtnY - 5), QPointF(zoomInX, zoomBtnY + 5));
    
    // Zoom Out button (-)
    QColor zoomOutColor = zoomOutHovered_ ? QColor(100, 180, 255) : QColor(70, 80, 95);
    painter.setBrush(zoomOutColor);
    painter.setPen(QPen(QColor(120, 130, 150), 1.5));
    painter.drawEllipse(QPointF(zoomOutX, zoomBtnY), btnRadius, btnRadius);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(QPointF(zoomOutX - 5, zoomBtnY), QPointF(zoomOutX + 5, zoomBtnY));
}

void ViewCube::drawAxisIndicators(QPainter& painter) {
    // Small axis indicator in bottom-left corner
    float cx = 22;
    float cy = height() - 22;
    float len = 12;
    
    // Transform axes by camera orientation
    QVector3D xAxis = cameraOrientation_.rotatedVector(QVector3D(1, 0, 0));
    QVector3D yAxis = cameraOrientation_.rotatedVector(QVector3D(0, 1, 0));
    QVector3D zAxis = cameraOrientation_.rotatedVector(QVector3D(0, 0, 1));
    
    // Draw axes (X=red, Y=green, Z=blue)
    painter.setPen(QPen(QColor(230, 80, 80), 2));
    painter.drawLine(QPointF(cx, cy), QPointF(cx + xAxis.x() * len, cy - xAxis.y() * len));
    
    painter.setPen(QPen(QColor(80, 200, 80), 2));
    painter.drawLine(QPointF(cx, cy), QPointF(cx + yAxis.x() * len, cy - yAxis.y() * len));
    
    painter.setPen(QPen(QColor(80, 140, 230), 2));
    painter.drawLine(QPointF(cx, cy), QPointF(cx + zAxis.x() * len, cy - zAxis.y() * len));
}

void ViewCube::drawCornerIndicators(QPainter& painter) {
    float s = cubeSize_ / 2.0f;
    
    // Corner positions
    QVector3D corners[] = {
        { s,  s,  s},  // Front-Top-Right
        {-s,  s,  s},  // Front-Top-Left
        { s,  s, -s},  // Back-Top-Right
        {-s,  s, -s}   // Back-Top-Left
    };
    
    painter.setBrush(QColor(255, 255, 255, 100));
    painter.setPen(Qt::NoPen);
    
    for (const auto& corner : corners) {
        QPointF pos = project3DTo2D(corner);
        float radius = 4.0f;
        
        // Only draw if visible (positive Z after rotation)
        QVector3D rotated = cameraOrientation_.rotatedVector(corner);
        if (rotated.z() > 0) {
            painter.drawEllipse(pos, radius, radius);
        }
    }
}

int ViewCube::hitTest(const QPoint& pos) const {
    // Test in reverse order (front to back)
    for (int i = static_cast<int>(faces_.size()) - 1; i >= 0; --i) {
        if (faces_[i].polygon.containsPoint(pos, Qt::OddEvenFill)) {
            return i;
        }
    }
    return -1;
}

void ViewCube::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->pos();
        
        // Check zoom buttons first
        double distToZoomIn = QLineF(pos, zoomInCenter_).length();
        double distToZoomOut = QLineF(pos, zoomOutCenter_).length();
        
        if (distToZoomIn <= zoomBtnRadius_) {
            emit zoomRequested(1.2);  // Zoom in 20%
            return;
        }
        if (distToZoomOut <= zoomBtnRadius_) {
            emit zoomRequested(0.8);  // Zoom out 20%
            return;
        }
        
        // Check cube faces
        int hit = hitTest(event->pos());
        if (hit >= 0) {
            applyViewOrientation(faces_[hit].orientation);
        }
        isDragging_ = true;
        lastMousePos_ = event->pos();
    }
}

void ViewCube::mouseMoveEvent(QMouseEvent* event) {
    QPointF pos = event->pos();
    
    // Update zoom button hover state
    double distToZoomIn = QLineF(pos, zoomInCenter_).length();
    double distToZoomOut = QLineF(pos, zoomOutCenter_).length();
    bool wasZoomInHovered = zoomInHovered_;
    bool wasZoomOutHovered = zoomOutHovered_;
    zoomInHovered_ = (distToZoomIn <= zoomBtnRadius_);
    zoomOutHovered_ = (distToZoomOut <= zoomBtnRadius_);
    
    // Update face hover state
    int hit = hitTest(event->pos());
    for (size_t i = 0; i < faces_.size(); ++i) {
        faces_[i].hovered = (static_cast<int>(i) == hit);
    }
    
    // Repaint if hover changed
    if (wasZoomInHovered != zoomInHovered_ || wasZoomOutHovered != zoomOutHovered_) {
        update();
    }
    
    if (isDragging_ && (event->buttons() & Qt::LeftButton)) {
        // Rotate the view based on mouse movement
        QPoint delta = event->pos() - lastMousePos_;
        
        float rotX = delta.y() * 0.5f;
        float rotY = delta.x() * 0.5f;
        
        QQuaternion rotation = QQuaternion::fromEulerAngles(-rotX, rotY, 0);
        cameraOrientation_ = rotation * cameraOrientation_;
        
#ifndef GEANTCAD_NO_VTK
        if (camera_ && renderer_) {
            // Apply rotation to actual camera
            double pos[3], focal[3];
            camera_->GetPosition(pos);
            camera_->GetFocalPoint(focal);
            
            QVector3D camPos(pos[0] - focal[0], pos[1] - focal[1], pos[2] - focal[2]);
            QVector3D newPos = rotation.rotatedVector(camPos);
            
            camera_->SetPosition(
                focal[0] + newPos.x(),
                focal[1] + newPos.y(),
                focal[2] + newPos.z()
            );
            
            // Update up vector
            double up[3];
            camera_->GetViewUp(up);
            QVector3D upVec(up[0], up[1], up[2]);
            QVector3D newUp = rotation.rotatedVector(upVec);
            camera_->SetViewUp(newUp.x(), newUp.y(), newUp.z());
            
            emit viewChanged();
        }
#endif
        
        lastMousePos_ = event->pos();
        updateFaces();
    }
    
    hoveredFace_ = hit;
    setCursor(hit >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    update();
}

void ViewCube::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isDragging_ = false;
    }
}

void ViewCube::enterEvent(QEnterEvent* /*event*/) {
    update();
}

void ViewCube::leaveEvent(QEvent* /*event*/) {
    for (auto& face : faces_) {
        face.hovered = false;
    }
    hoveredFace_ = -1;
    setCursor(Qt::ArrowCursor);
    update();
}

void ViewCube::applyViewOrientation(ViewOrientation orientation) {
    QVector3D cameraDir;
    QVector3D upVector(0, 0, 1);  // Default Z-up
    
    switch (orientation) {
        case ViewOrientation::Front:
            cameraDir = QVector3D(0, -1, 0);
            break;
        case ViewOrientation::Back:
            cameraDir = QVector3D(0, 1, 0);
            break;
        case ViewOrientation::Left:
            cameraDir = QVector3D(-1, 0, 0);
            break;
        case ViewOrientation::Right:
            cameraDir = QVector3D(1, 0, 0);
            break;
        case ViewOrientation::Top:
            cameraDir = QVector3D(0, 0, 1);
            upVector = QVector3D(0, 1, 0);
            break;
        case ViewOrientation::Bottom:
            cameraDir = QVector3D(0, 0, -1);
            upVector = QVector3D(0, -1, 0);
            break;
        case ViewOrientation::FrontTopRight:
            cameraDir = QVector3D(1, -1, 1).normalized();
            break;
        case ViewOrientation::FrontTopLeft:
            cameraDir = QVector3D(-1, -1, 1).normalized();
            break;
        case ViewOrientation::BackTopRight:
            cameraDir = QVector3D(1, 1, 1).normalized();
            break;
        case ViewOrientation::BackTopLeft:
            cameraDir = QVector3D(-1, 1, 1).normalized();
            break;
    }

#ifndef GEANTCAD_NO_VTK
    if (camera_ && renderer_) {
        double focal[3];
        camera_->GetFocalPoint(focal);
        
        // Get current distance to maintain zoom level
        double pos[3];
        camera_->GetPosition(pos);
        double distance = std::sqrt(
            (pos[0] - focal[0]) * (pos[0] - focal[0]) +
            (pos[1] - focal[1]) * (pos[1] - focal[1]) +
            (pos[2] - focal[2]) * (pos[2] - focal[2])
        );
        
        // Set new camera position
        camera_->SetPosition(
            focal[0] + cameraDir.x() * distance,
            focal[1] + cameraDir.y() * distance,
            focal[2] + cameraDir.z() * distance
        );
        
        camera_->SetViewUp(upVector.x(), upVector.y(), upVector.z());
        
        emit viewChanged();
    }
#endif
    
    emit viewOrientationRequested(orientation);
    updateFromCamera();
}


} // namespace geantcad

