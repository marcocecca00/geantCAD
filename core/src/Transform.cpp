#include "Transform.hh"
#include <cmath>

namespace geantcad {

Transform::Transform()
    : translation_(0.0f, 0.0f, 0.0f)
    , rotation_(1.0f, 0.0f, 0.0f, 0.0f) // identity quaternion
    , scale_(1.0f, 1.0f, 1.0f)
{
}

Transform::Transform(const QVector3D& translation)
    : translation_(translation)
    , rotation_(1.0f, 0.0f, 0.0f, 0.0f)
    , scale_(1.0f, 1.0f, 1.0f)
{
}

Transform::Transform(const QVector3D& translation, const QQuaternion& rotation, const QVector3D& scale)
    : translation_(translation)
    , rotation_(rotation)
    , scale_(scale)
{
}

void Transform::setTranslation(const QVector3D& t) {
    translation_ = t;
}

void Transform::setRotation(const QQuaternion& r) {
    rotation_ = r.normalized();
}

void Transform::setRotationEuler(float pitch, float yaw, float roll) {
    // QQuaternion::fromEulerAngles expects degrees
    rotation_ = QQuaternion::fromEulerAngles(pitch, yaw, roll);
}

void Transform::setScale(const QVector3D& s) {
    scale_ = s;
}

QMatrix4x4 Transform::getMatrix() const {
    QMatrix4x4 m;
    m.translate(translation_);
    m.rotate(rotation_);
    m.scale(scale_);
    return m;
}

QMatrix4x4 Transform::getInverseMatrix() const {
    QMatrix4x4 m = getMatrix();
    bool invertible;
    QMatrix4x4 inv = m.inverted(&invertible);
    if (!invertible) {
        // Return identity if not invertible
        return QMatrix4x4();
    }
    return inv;
}

Transform Transform::combine(const Transform& other) const {
    // Combine: this * other
    // First apply other, then this
    QMatrix4x4 thisMat = getMatrix();
    QMatrix4x4 otherMat = other.getMatrix();
    QMatrix4x4 combined = thisMat * otherMat;
    
    // Extract components (approximate, since scale/rotation can't be cleanly separated)
    QVector3D combinedTrans = QVector3D(combined(0, 3), combined(1, 3), combined(2, 3));
    
    // For proper decomposition, we'd need more complex math. For MVP, we combine matrices.
    // Simplified: assume no scale for now
    QQuaternion combinedRot = rotation_ * other.rotation_;
    QVector3D combinedScale = QVector3D(
        scale_.x() * other.scale_.x(),
        scale_.y() * other.scale_.y(),
        scale_.z() * other.scale_.z()
    );
    
    return Transform(combinedTrans, combinedRot, combinedScale);
}

QVector3D Transform::transformPoint(const QVector3D& point) const {
    QMatrix4x4 m = getMatrix();
    QVector4D result = m * QVector4D(point, 1.0f);
    return QVector3D(result.x(), result.y(), result.z());
}

QVector3D Transform::transformDirection(const QVector3D& dir) const {
    QMatrix4x4 m = getMatrix();
    // Remove translation for directions
    QMatrix4x4 rotScaleOnly = m;
    rotScaleOnly(0, 3) = 0;
    rotScaleOnly(1, 3) = 0;
    rotScaleOnly(2, 3) = 0;
    rotScaleOnly(3, 3) = 1;
    
    QVector4D result = rotScaleOnly * QVector4D(dir, 0.0f);
    return QVector3D(result.x(), result.y(), result.z()).normalized();
}

Transform Transform::identity() {
    return Transform();
}

nlohmann::json Transform::toJson() const {
    nlohmann::json j;
    j["translation"] = {translation_.x(), translation_.y(), translation_.z()};
    j["rotation"] = {rotation_.x(), rotation_.y(), rotation_.z(), rotation_.scalar()};
    j["scale"] = {scale_.x(), scale_.y(), scale_.z()};
    return j;
}

Transform Transform::fromJson(const nlohmann::json& j) {
    Transform t;
    
    if (j.contains("translation")) {
        auto trans = j["translation"];
        t.translation_ = QVector3D(trans[0], trans[1], trans[2]);
    }
    
    if (j.contains("rotation")) {
        auto rot = j["rotation"];
        t.rotation_ = QQuaternion(rot[3], rot[0], rot[1], rot[2]); // scalar, x, y, z
    }
    
    if (j.contains("scale")) {
        auto scl = j["scale"];
        t.scale_ = QVector3D(scl[0], scl[1], scl[2]);
    }
    
    return t;
}

} // namespace geantcad

