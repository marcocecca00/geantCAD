#include "ViewCube.hh"
#include <QPainterPath>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>

namespace geantcad {

ViewCube::ViewCube(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(100, 100);
    setAttribute(Qt::WA_TranslucentBackground, false);
    
    // Initialize with isometric view orientation
    cameraOrientation_ = QQuaternion::fromEulerAngles(-30.0f, 45.0f, 0.0f);
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
    
    // Background
    painter.fillRect(rect(), QColor(30, 30, 30, 220));
    painter.setPen(QPen(QColor(80, 80, 80), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    // Draw faces (already sorted back to front)
    for (size_t i = 0; i < faces_.size(); ++i) {
        const Face& face = faces_[i];
        
        // Calculate face color based on lighting and hover
        QColor faceColor = face.baseColor;
        if (face.hovered) {
            faceColor = hoverColor_;
        }
        
        // Simple lighting based on face normal
        float lightFactor = 0.7f + 0.3f * (static_cast<float>(i) / faces_.size());
        faceColor = QColor(
            static_cast<int>(faceColor.red() * lightFactor),
            static_cast<int>(faceColor.green() * lightFactor),
            static_cast<int>(faceColor.blue() * lightFactor)
        );
        
        // Draw face
        painter.setBrush(faceColor);
        painter.setPen(QPen(edgeColor_, 1.5));
        painter.drawPolygon(face.polygon);
        
        // Draw label on front-facing faces only (last 3 in sorted order typically)
        if (i >= faces_.size() - 3) {
            QPointF center = face.polygon.boundingRect().center();
            
            QFont font = painter.font();
            font.setPointSize(7);
            font.setBold(true);
            painter.setFont(font);
            
            // Text shadow for readability
            painter.setPen(QColor(0, 0, 0, 180));
            painter.drawText(center + QPointF(1, 1), face.label);
            
            painter.setPen(textColor_);
            painter.drawText(center, face.label);
        }
    }
    
    // Draw corner indicators for isometric views
    drawCornerIndicators(painter);
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
        int hit = hitTest(event->pos());
        if (hit >= 0) {
            applyViewOrientation(faces_[hit].orientation);
        }
        isDragging_ = true;
        lastMousePos_ = event->pos();
    }
}

void ViewCube::mouseMoveEvent(QMouseEvent* event) {
    // Update hover state
    int hit = hitTest(event->pos());
    for (size_t i = 0; i < faces_.size(); ++i) {
        faces_[i].hovered = (static_cast<int>(i) == hit);
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

