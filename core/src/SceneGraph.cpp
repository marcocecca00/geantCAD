#include "SceneGraph.hh"
#include "Material.hh"
#include "Shape.hh"
#include "PhysicsConfig.hh"
#include "OutputConfig.hh"
#include "ParticleGunConfig.hh"
#include <algorithm>

namespace geantcad {

SceneGraph::SceneGraph() {
    // Create root node
    root_ = std::make_unique<VolumeNode>("World");
    auto worldShape = makeBox(1000.0, 1000.0, 1000.0);
    root_->setShape(std::move(worldShape));
    root_->setMaterial(Material::makeVacuum());
}

SceneGraph::~SceneGraph() {
    // Root will delete all children
}

VolumeNode* SceneGraph::createVolume(const std::string& name) {
    auto* node = new VolumeNode(name);
    root_->addChild(node);
    notifyNodeAdded(node);
    notifyGraphChanged();
    return node;
}

void SceneGraph::removeVolume(VolumeNode* node) {
    if (!node || node == root_.get()) return;
    
    if (selected_ == node) {
        setSelected(nullptr);
    }
    
    notifyNodeRemoved(node);
    
    // Remove from parent (which will delete it)
    VolumeNode* parent = node->getParent();
    if (parent) {
        parent->removeChild(node);
        delete node;
        notifyGraphChanged();
    }
}

VolumeNode* SceneGraph::findVolumeById(uint64_t id) {
    VolumeNode* result = nullptr;
    traverse([&](VolumeNode* node) {
        if (node->getId() == id) {
            result = node;
        }
    });
    return result;
}

VolumeNode* SceneGraph::findVolumeByName(const std::string& name) {
    VolumeNode* result = nullptr;
    traverse([&](VolumeNode* node) {
        if (node->getName() == name && !result) {
            result = node;
        }
    });
    return result;
}

void SceneGraph::setSelected(VolumeNode* node) {
    if (selected_ == node) return;
    
    selected_ = node;
    
    // Also update multi-selection to contain just this node
    multiSelection_.clear();
    if (node) {
        multiSelection_.push_back(node);
    }
    
    notifySelectionChanged();
}

void SceneGraph::clearSelection() {
    setSelected(nullptr);
    clearMultiSelection();
}

void SceneGraph::addToSelection(VolumeNode* node) {
    if (!node || isSelected(node)) return;
    
    multiSelection_.push_back(node);
    
    // Update primary selection to most recently added
    selected_ = node;
    notifySelectionChanged();
}

void SceneGraph::removeFromSelection(VolumeNode* node) {
    if (!node) return;
    
    auto it = std::find(multiSelection_.begin(), multiSelection_.end(), node);
    if (it != multiSelection_.end()) {
        multiSelection_.erase(it);
        
        // Update primary selection
        if (selected_ == node) {
            selected_ = multiSelection_.empty() ? nullptr : multiSelection_.back();
        }
        notifySelectionChanged();
    }
}

void SceneGraph::toggleSelection(VolumeNode* node) {
    if (isSelected(node)) {
        removeFromSelection(node);
    } else {
        addToSelection(node);
    }
}

bool SceneGraph::isSelected(VolumeNode* node) const {
    if (!node) return false;
    return std::find(multiSelection_.begin(), multiSelection_.end(), node) != multiSelection_.end();
}

void SceneGraph::clearMultiSelection() {
    multiSelection_.clear();
    selected_ = nullptr;
    notifySelectionChanged();
}

void SceneGraph::traverse(std::function<void(VolumeNode*)> visitor) {
    if (!root_) return;
    
    std::function<void(VolumeNode*)> visit = [&](VolumeNode* node) {
        visitor(node);
        for (auto* child : node->getChildren()) {
            visit(child);
        }
    };
    
    visit(root_.get());
}

void SceneGraph::traverseConst(std::function<void(const VolumeNode*)> visitor) const {
    if (!root_) return;
    
    std::function<void(const VolumeNode*)> visit = [&](const VolumeNode* node) {
        visitor(node);
        for (auto* child : node->getChildren()) {
            visit(child);
        }
    };
    
    visit(root_.get());
}

nlohmann::json SceneGraph::toJson() const {
    nlohmann::json j;
    if (root_) {
        j["root"] = root_->toJson();
    }
    if (selected_) {
        j["selectedId"] = selected_->getId();
    }
    j["physics"] = physicsConfig_.toJson();
    j["output"] = outputConfig_.toJson();
    j["particleGun"] = particleGunConfig_.toJson();
    return j;
}

void SceneGraph::fromJson(const nlohmann::json& j) {
    if (j.contains("root")) {
        root_ = VolumeNode::fromJson(j["root"]);
        
        if (j.contains("selectedId")) {
            uint64_t selectedId = j["selectedId"];
            selected_ = findVolumeById(selectedId);
        }
        
        if (j.contains("physics")) {
            physicsConfig_.fromJson(j["physics"]);
        }
        
        if (j.contains("output")) {
            outputConfig_.fromJson(j["output"]);
        }
        if (j.contains("particleGun")) {
            particleGunConfig_.fromJson(j["particleGun"]);
        }
        
        notifyGraphChanged();
    }
}

void SceneGraph::notifySelectionChanged() {
    if (onSelectionChanged) {
        onSelectionChanged(selected_);
    }
}

void SceneGraph::notifyNodeAdded(VolumeNode* node) {
    if (onNodeAdded) {
        onNodeAdded(node);
    }
}

void SceneGraph::notifyNodeRemoved(VolumeNode* node) {
    if (onNodeRemoved) {
        onNodeRemoved(node);
    }
}

void SceneGraph::notifyGraphChanged() {
    if (onGraphChanged) {
        onGraphChanged();
    }
}

} // namespace geantcad

