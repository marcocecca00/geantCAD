#pragma once

#include "Transform.hh"
#include "Shape.hh"
#include "Material.hh"
#include <string>
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

namespace geantcad {

/**
 * Configurazione per SensitiveDetector
 */
struct SensitiveDetectorConfig {
    bool enabled = false;
    std::string type = "calorimeter"; // "calorimeter", "tracker", "optical"
    std::string collectionName = "";
    int copyNumber = 0;
};

/**
 * Configurazione per superfici ottiche (SkinSurface)
 */
struct OpticalSurfaceConfig {
    bool enabled = false;
    std::string model = "unified"; // "unified", "glisur", "dichroic"
    std::string finish = "polished"; // "polished", "ground", "polishedfrontpainted", etc.
    double reflectivity = 0.95;
    double sigmaAlpha = 0.0; // surface roughness (degrees)
    std::string preset = ""; // "tyvek", "esr", "black"
};

/**
 * VolumeNode rappresenta un volume nella scena.
 * Organizzato in struttura gerarchica (parent-children).
 */
class VolumeNode {
public:
    VolumeNode(const std::string& name);
    ~VolumeNode();

    // Identity
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    
    uint64_t getId() const { return id_; }

    // Hierarchy
    VolumeNode* getParent() const { return parent_; }
    const std::vector<VolumeNode*>& getChildren() const { return children_; }
    void setParent(VolumeNode* parent);
    void addChild(VolumeNode* child);
    void removeChild(VolumeNode* child);
    bool isDescendantOf(const VolumeNode* ancestor) const;

    // Geometry
    std::unique_ptr<Shape> shape_;
    Transform transform_;
    
    Shape* getShape() { return shape_.get(); }
    const Shape* getShape() const { return shape_.get(); }
    void setShape(std::unique_ptr<Shape> shape);
    
    Transform& getTransform() { return transform_; }
    const Transform& getTransform() const { return transform_; }

    // Material
    std::shared_ptr<Material> material_;
    std::shared_ptr<Material> getMaterial() const { return material_; }
    void setMaterial(std::shared_ptr<Material> material) { material_ = material; }

    // Sensitive Detector
    SensitiveDetectorConfig sdConfig_;
    const SensitiveDetectorConfig& getSDConfig() const { return sdConfig_; }
    SensitiveDetectorConfig& getSDConfig() { return sdConfig_; }

    // Optical surface
    OpticalSurfaceConfig opticalConfig_;
    const OpticalSurfaceConfig& getOpticalConfig() const { return opticalConfig_; }
    OpticalSurfaceConfig& getOpticalConfig() { return opticalConfig_; }

    // World transform (combines all parent transforms)
    Transform getWorldTransform() const;

    // Serialization
    nlohmann::json toJson() const;
    static std::unique_ptr<VolumeNode> fromJson(const nlohmann::json& j);

private:
    uint64_t id_; // unique ID
    std::string name_;
    VolumeNode* parent_ = nullptr;
    std::vector<VolumeNode*> children_;
    
    static uint64_t nextId_;
};

} // namespace geantcad

