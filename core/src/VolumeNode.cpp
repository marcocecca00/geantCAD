#include "VolumeNode.hh"
#include <algorithm>

namespace geantcad {

uint64_t VolumeNode::nextId_ = 1;

VolumeNode::VolumeNode(const std::string& name)
    : id_(nextId_++)
    , name_(name)
    , transform_(Transform::identity())
{
}

VolumeNode::~VolumeNode() {
    // Remove from parent
    if (parent_) {
        parent_->removeChild(this);
    }
    
    // Remove children
    while (!children_.empty()) {
        auto* child = children_.back();
        removeChild(child);
        delete child;
    }
}

void VolumeNode::setParent(VolumeNode* parent) {
    if (parent_ == parent) return;
    
    if (parent_) {
        parent_->removeChild(this);
    }
    
    parent_ = parent;
    
    if (parent_) {
        parent_->addChild(this);
    }
}

void VolumeNode::addChild(VolumeNode* child) {
    if (!child) return;
    
    // Check if already a child
    for (auto* c : children_) {
        if (c == child) return;
    }
    
    children_.push_back(child);
    if (child->parent_ != this) {
        child->parent_ = this;
    }
}

void VolumeNode::removeChild(VolumeNode* child) {
    if (!child) return;
    
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        children_.erase(it);
        if (child->parent_ == this) {
            child->parent_ = nullptr;
        }
    }
}

bool VolumeNode::isDescendantOf(const VolumeNode* ancestor) const {
    const VolumeNode* current = parent_;
    while (current) {
        if (current == ancestor) return true;
        current = current->parent_;
    }
    return false;
}

void VolumeNode::setShape(std::unique_ptr<Shape> shape) {
    shape_ = std::move(shape);
}

Transform VolumeNode::getWorldTransform() const {
    Transform world = transform_;
    const VolumeNode* current = parent_;
    while (current) {
        world = current->transform_.combine(world);
        current = current->parent_;
    }
    return world;
}

nlohmann::json VolumeNode::toJson() const {
    nlohmann::json j;
    j["id"] = id_;
    j["name"] = name_;
    j["transform"] = transform_.toJson();
    
    if (shape_) {
        j["shape"] = shape_->toJson();
    }
    
    if (material_) {
        j["material"] = material_->toJson();
    }
    
    j["sdConfig"] = {
        {"enabled", sdConfig_.enabled},
        {"type", sdConfig_.type},
        {"collectionName", sdConfig_.collectionName},
        {"copyNumber", sdConfig_.copyNumber}
    };
    
    j["opticalConfig"] = {
        {"enabled", opticalConfig_.enabled},
        {"model", opticalConfig_.model},
        {"finish", opticalConfig_.finish},
        {"reflectivity", opticalConfig_.reflectivity},
        {"sigmaAlpha", opticalConfig_.sigmaAlpha},
        {"preset", opticalConfig_.preset}
    };
    
    // Children
    nlohmann::json childrenJson = nlohmann::json::array();
    for (auto* child : children_) {
        childrenJson.push_back(child->toJson());
    }
    j["children"] = childrenJson;
    
    return j;
}

std::unique_ptr<VolumeNode> VolumeNode::fromJson(const nlohmann::json& j) {
    auto node = std::make_unique<VolumeNode>(j["name"]);
    node->id_ = j.value("id", node->id_); // Preserve ID if provided
    
    if (j.contains("transform")) {
        node->transform_ = Transform::fromJson(j["transform"]);
    }
    
    if (j.contains("shape")) {
        node->shape_ = Shape::fromJson(j["shape"]);
    }
    
    if (j.contains("material")) {
        node->material_ = Material::fromJson(j["material"]);
    }
    
    if (j.contains("sdConfig")) {
        auto sd = j["sdConfig"];
        node->sdConfig_.enabled = sd.value("enabled", false);
        node->sdConfig_.type = sd.value("type", "calorimeter");
        node->sdConfig_.collectionName = sd.value("collectionName", "");
        node->sdConfig_.copyNumber = sd.value("copyNumber", 0);
    }
    
    if (j.contains("opticalConfig")) {
        auto opt = j["opticalConfig"];
        node->opticalConfig_.enabled = opt.value("enabled", false);
        node->opticalConfig_.model = opt.value("model", "unified");
        node->opticalConfig_.finish = opt.value("finish", "polished");
        node->opticalConfig_.reflectivity = opt.value("reflectivity", 0.95);
        node->opticalConfig_.sigmaAlpha = opt.value("sigmaAlpha", 0.0);
        node->opticalConfig_.preset = opt.value("preset", "");
    }
    
    // Children (recursive)
    if (j.contains("children")) {
        for (const auto& childJson : j["children"]) {
            auto child = fromJson(childJson);
            node->addChild(child.release());
        }
    }
    
    return node;
}

} // namespace geantcad

