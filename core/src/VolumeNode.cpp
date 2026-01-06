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
    
    nlohmann::json sdJson;
    sdJson["enabled"] = sdConfig_.enabled;
    sdJson["type"] = sdConfig_.type;
    sdJson["collectionName"] = sdConfig_.collectionName;
    sdJson["copyNumber"] = sdConfig_.copyNumber;
    sdJson["usesScoringMesh"] = sdConfig_.usesScoringMesh;
    sdJson["meshSizeX"] = sdConfig_.meshSizeX;
    sdJson["meshSizeY"] = sdConfig_.meshSizeY;
    sdJson["meshSizeZ"] = sdConfig_.meshSizeZ;
    sdJson["nBinsX"] = sdConfig_.nBinsX;
    sdJson["nBinsY"] = sdConfig_.nBinsY;
    sdJson["nBinsZ"] = sdConfig_.nBinsZ;
    
    nlohmann::json scorersJson = nlohmann::json::array();
    for (const auto& scorer : sdConfig_.scorers) {
        scorersJson.push_back({
            {"name", scorer.name},
            {"type", scorer.type},
            {"particle_filter", scorer.particle_filter},
            {"min_energy", scorer.min_energy},
            {"max_energy", scorer.max_energy}
        });
    }
    sdJson["scorers"] = scorersJson;
    j["sdConfig"] = sdJson;
    
    j["opticalConfig"] = {
        {"enabled", opticalConfig_.enabled},
        {"model", opticalConfig_.model},
        {"finish", opticalConfig_.finish},
        {"reflectivity", opticalConfig_.reflectivity},
        {"sigmaAlpha", opticalConfig_.sigmaAlpha},
        {"preset", opticalConfig_.preset}
    };
    
    j["visible"] = visible_;
    
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
        node->sdConfig_.usesScoringMesh = sd.value("usesScoringMesh", false);
        node->sdConfig_.meshSizeX = sd.value("meshSizeX", 0.0);
        node->sdConfig_.meshSizeY = sd.value("meshSizeY", 0.0);
        node->sdConfig_.meshSizeZ = sd.value("meshSizeZ", 0.0);
        node->sdConfig_.nBinsX = sd.value("nBinsX", 10);
        node->sdConfig_.nBinsY = sd.value("nBinsY", 10);
        node->sdConfig_.nBinsZ = sd.value("nBinsZ", 10);
        
        if (sd.contains("scorers")) {
            node->sdConfig_.scorers.clear();
            for (const auto& scorerJson : sd["scorers"]) {
                ScorerConfig scorer;
                scorer.name = scorerJson.value("name", "");
                scorer.type = scorerJson.value("type", "energy_deposit");
                scorer.particle_filter = scorerJson.value("particle_filter", "");
                scorer.min_energy = scorerJson.value("min_energy", 0.0);
                scorer.max_energy = scorerJson.value("max_energy", 0.0);
                node->sdConfig_.scorers.push_back(scorer);
            }
        }
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
    
    node->visible_ = j.value("visible", true);
    
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

