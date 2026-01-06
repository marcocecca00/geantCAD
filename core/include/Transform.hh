#pragma once

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include <nlohmann/json.hpp>

namespace geantcad {

/**
 * Transform rappresenta una trasformazione TRS (Translation, Rotation, Scale)
 * usando Qt per calcoli matriciali.
 * Rotazione: quaternion (più stabile di Euler per interpolazioni)
 */
class Transform {
public:
    Transform();
    explicit Transform(const QVector3D& translation);
    Transform(const QVector3D& translation, const QQuaternion& rotation, const QVector3D& scale = QVector3D(1.0f, 1.0f, 1.0f));

    // Getters
    const QVector3D& getTranslation() const { return translation_; }
    const QQuaternion& getRotation() const { return rotation_; }
    const QVector3D& getScale() const { return scale_; }

    // Setters
    void setTranslation(const QVector3D& t);
    void setRotation(const QQuaternion& r);
    void setRotationEuler(float pitch, float yaw, float roll); // degrees
    void setScale(const QVector3D& s);

    // Matrix operations
    QMatrix4x4 getMatrix() const;
    QMatrix4x4 getInverseMatrix() const;

    // Transform operations
    Transform combine(const Transform& other) const; // this * other
    QVector3D transformPoint(const QVector3D& point) const;
    QVector3D transformDirection(const QVector3D& dir) const;

    // Identity
    static Transform identity();

    // Serialization
    nlohmann::json toJson() const;
    static Transform fromJson(const nlohmann::json& j);

private:
    QVector3D translation_;
    QQuaternion rotation_; // quaternion for rotation
    QVector3D scale_;
};

} // namespace geantcad
