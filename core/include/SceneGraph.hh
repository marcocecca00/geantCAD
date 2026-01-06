#pragma once

#include "VolumeNode.hh"
#include "PhysicsConfig.hh"
#include "OutputConfig.hh"
#include "ParticleGunConfig.hh"
#include <memory>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

namespace geantcad {

/**
 * SceneGraph gestisce la gerarchia di volumi e fornisce operazioni di query/modifica.
 */
class SceneGraph {
public:
    SceneGraph();
    ~SceneGraph();

    // Root
    VolumeNode* getRoot() { return root_.get(); }
    const VolumeNode* getRoot() const { return root_.get(); }

    // Node operations
    VolumeNode* createVolume(const std::string& name);
    void removeVolume(VolumeNode* node);
    VolumeNode* findVolumeById(uint64_t id);
    VolumeNode* findVolumeByName(const std::string& name);
    
    // Selection
    VolumeNode* getSelected() const { return selected_; }
    void setSelected(VolumeNode* node);
    void clearSelection();

    // Traversal
    void traverse(std::function<void(VolumeNode*)> visitor);
    void traverseConst(std::function<void(const VolumeNode*)> visitor) const;

    // Physics configuration
    PhysicsConfig& getPhysicsConfig() { return physicsConfig_; }
    const PhysicsConfig& getPhysicsConfig() const { return physicsConfig_; }
    
    // Output configuration
    OutputConfig& getOutputConfig() { return outputConfig_; }
    const OutputConfig& getOutputConfig() const { return outputConfig_; }
    
    // ParticleGun configuration
    ParticleGunConfig& getParticleGunConfig() { return particleGunConfig_; }
    const ParticleGunConfig& getParticleGunConfig() const { return particleGunConfig_; }

    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);

    // Signals (per integrazione con GUI - usando std::function per MVP)
    std::function<void(VolumeNode*)> onSelectionChanged;
    std::function<void(VolumeNode*)> onNodeAdded;
    std::function<void(VolumeNode*)> onNodeRemoved;
    std::function<void()> onGraphChanged;

private:
    std::unique_ptr<VolumeNode> root_;
    VolumeNode* selected_ = nullptr;
    PhysicsConfig physicsConfig_;
    OutputConfig outputConfig_;
    ParticleGunConfig particleGunConfig_;
    
    void notifySelectionChanged();
    void notifyNodeAdded(VolumeNode* node);
    void notifyNodeRemoved(VolumeNode* node);
    void notifyGraphChanged();
};

} // namespace geantcad

// Forward declaration for file I/O
namespace geantcad {
    bool saveSceneToFile(SceneGraph* sceneGraph, const std::string& filePath);
    bool loadSceneFromFile(SceneGraph* sceneGraph, const std::string& filePath);
}

