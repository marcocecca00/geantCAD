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
 * Scorer configuration for MultiFunctionalDetector
 */
struct ScorerConfig {
    std::string name;
    std::string type;  // "energy_deposit", "track_length", "n_of_step", "flux", "dose"
    std::string particle_filter;  // empty = all, or specific particle name
    double min_energy = 0.0;      // MeV
    double max_energy = 0.0;      // MeV (0 = unlimited)
};

/**
 * Configurazione per SensitiveDetector
 */
struct SensitiveDetectorConfig {
    bool enabled = false;
    std::string type = "calorimeter"; // "calorimeter", "tracker", "optical", "multifunctional"
    std::string collectionName = "";
    int copyNumber = 0;
    
    // For MultiFunctionalDetector
    std::vector<ScorerConfig> scorers;
    
    // Scoring mesh settings (optional, for creating a separate scoring mesh)
    bool usesScoringMesh = false;
    double meshSizeX = 0.0;  // If 0, uses volume dimensions
    double meshSizeY = 0.0;
    double meshSizeZ = 0.0;
    int nBinsX = 10;
    int nBinsY = 10;
    int nBinsZ = 10;
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

    // Visibility (for viewport display, not affecting export)
    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }

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
    bool visible_ = true; // visibility in viewport
    
    static uint64_t nextId_;
};

} // namespace geantcad

