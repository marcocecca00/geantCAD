#include "Command.hh"
#include "SceneGraph.hh"
#include <algorithm>
#include <iostream>

namespace geantcad {

// CreateVolumeCommand
CreateVolumeCommand::CreateVolumeCommand(SceneGraph* sceneGraph, const std::string& name,
                                       std::unique_ptr<Shape> shape, std::shared_ptr<Material> material)
    : sceneGraph_(sceneGraph)
    , volumeName_(name)
    , shape_(std::move(shape))
    , material_(material)
{
}

void CreateVolumeCommand::execute() {
    if (!sceneGraph_) return;
    
    createdNode_ = sceneGraph_->createVolume(volumeName_);
    if (createdNode_) {
        if (shape_) {
            // Set the shape (we can move it since execute() is only called once)
            createdNode_->setShape(std::move(shape_));
        }
        if (material_) {
            createdNode_->setMaterial(material_);
        }
    }
}

void CreateVolumeCommand::undo() {
    if (createdNode_ && sceneGraph_) {
        sceneGraph_->removeVolume(createdNode_);
        createdNode_ = nullptr;
    }
}

// DeleteVolumeCommand
DeleteVolumeCommand::DeleteVolumeCommand(SceneGraph* sceneGraph, VolumeNode* node)
    : sceneGraph_(sceneGraph)
    , node_(node)
{
    if (node_) {
        volumeName_ = node_->getName();
        parent_ = node_->getParent();
        
        // Find child index
        if (parent_) {
            const auto& siblings = parent_->getChildren();
            auto it = std::find(siblings.begin(), siblings.end(), node_);
            childIndex_ = (it != siblings.end()) ? std::distance(siblings.begin(), it) : 0;
        }
        
        // Save state using JSON serialization for complete state preservation
        if (node_->getShape()) {
            nlohmann::json shapeJson = node_->getShape()->toJson();
            shape_ = Shape::fromJson(shapeJson);
        }
        material_ = node_->getMaterial();
        transform_ = node_->getTransform();
        
        // Save children references (they will be deleted by execute, so we need to recreate them)
        for (auto* child : node_->getChildren()) {
            children_.push_back(child);
        }
    }
}

void DeleteVolumeCommand::execute() {
    if (node_ && sceneGraph_) {
        // Store reference before deletion
        VolumeNode* nodeToDelete = node_;
        node_ = nullptr; // Clear reference to avoid use-after-delete
        
        // Save full state using JSON before deletion
        nlohmann::json nodeJson = nodeToDelete->toJson();
        
        // Delete the node (this will also delete children)
        sceneGraph_->removeVolume(nodeToDelete);
        
        // Store JSON for undo
        nodeJson_ = nodeJson;
    }
}

void DeleteVolumeCommand::undo() {
    if (!sceneGraph_ || nodeJson_.is_null()) return;
    
    // Recreate node from JSON
    try {
        auto restoredNode = VolumeNode::fromJson(nodeJson_);
        node_ = restoredNode.release();
        
        // Reattach to parent
        if (parent_) {
            parent_->addChild(node_);
        } else {
            // If no parent, add to root
            if (sceneGraph_->getRoot()) {
                sceneGraph_->getRoot()->addChild(node_);
            }
        }
        
        // Note: SceneGraph notifications will be handled by addChild internally
    } catch (const std::exception& e) {
        std::cerr << "Error restoring deleted node: " << e.what() << std::endl;
    }
}

// TransformVolumeCommand
TransformVolumeCommand::TransformVolumeCommand(VolumeNode* node, const Transform& newTransform)
    : node_(node)
    , newTransform_(newTransform)
{
    if (node_) {
        oldTransform_ = node_->getTransform();
    }
}

void TransformVolumeCommand::execute() {
    if (node_) {
        node_->getTransform() = newTransform_;
    }
}

void TransformVolumeCommand::undo() {
    if (node_) {
        node_->getTransform() = oldTransform_;
    }
}

// DuplicateVolumeCommand
DuplicateVolumeCommand::DuplicateVolumeCommand(SceneGraph* sceneGraph, VolumeNode* sourceNode)
    : sceneGraph_(sceneGraph)
    , sourceNode_(sourceNode)
{
}

void DuplicateVolumeCommand::execute() {
    if (!sceneGraph_ || !sourceNode_) return;
    
    duplicatedNode_ = duplicateNodeRecursive(sourceNode_);
    if (duplicatedNode_ && sourceNode_->getParent()) {
        sourceNode_->getParent()->addChild(duplicatedNode_);
    }
}

void DuplicateVolumeCommand::undo() {
    if (duplicatedNode_ && sceneGraph_) {
        sceneGraph_->removeVolume(duplicatedNode_);
        duplicatedNode_ = nullptr;
    }
}

VolumeNode* DuplicateVolumeCommand::duplicateNodeRecursive(VolumeNode* source) {
    if (!source) return nullptr;
    
    // Create new node with same name + "_copy"
    std::string newName = source->getName() + "_copy";
    VolumeNode* duplicate = new VolumeNode(newName);
    
    // Copy transform
    duplicate->getTransform() = source->getTransform();
    
    // Copy material
    duplicate->setMaterial(source->getMaterial());
    
    // Clone shape using JSON serialization
    if (source->getShape()) {
        nlohmann::json shapeJson = source->getShape()->toJson();
        duplicate->setShape(Shape::fromJson(shapeJson));
    }
    
    // Copy SD config
    duplicate->getSDConfig() = source->getSDConfig();
    
    // Copy optical config
    duplicate->getOpticalConfig() = source->getOpticalConfig();
    
    // Recursively duplicate children
    for (auto* child : source->getChildren()) {
        VolumeNode* childDup = duplicateNodeRecursive(child);
        if (childDup) {
            duplicate->addChild(childDup);
        }
    }
    
    return duplicate;
}

// ModifyShapeCommand
ModifyShapeCommand::ModifyShapeCommand(VolumeNode* node, const ShapeParams& newParams)
    : node_(node)
    , newParams_(newParams)
{
    if (node_ && node_->getShape()) {
        oldParams_ = node_->getShape()->getParams();
    }
}

void ModifyShapeCommand::execute() {
    if (node_ && node_->getShape()) {
        node_->getShape()->getParams() = newParams_;
    }
}

void ModifyShapeCommand::undo() {
    if (node_ && node_->getShape()) {
        node_->getShape()->getParams() = oldParams_;
    }
}

// ModifyNameCommand
ModifyNameCommand::ModifyNameCommand(VolumeNode* node, const std::string& newName)
    : node_(node)
    , newName_(newName)
{
    if (node_) {
        oldName_ = node_->getName();
    }
}

void ModifyNameCommand::execute() {
    if (node_) {
        node_->setName(newName_);
    }
}

void ModifyNameCommand::undo() {
    if (node_) {
        node_->setName(oldName_);
    }
}

// ModifyMaterialCommand
ModifyMaterialCommand::ModifyMaterialCommand(VolumeNode* node, std::shared_ptr<Material> newMaterial)
    : node_(node)
    , newMaterial_(newMaterial)
{
    if (node_) {
        oldMaterial_ = node_->getMaterial();
    }
}

void ModifyMaterialCommand::execute() {
    if (node_) {
        node_->setMaterial(newMaterial_);
    }
}

void ModifyMaterialCommand::undo() {
    if (node_) {
        node_->setMaterial(oldMaterial_);
    }
}

// ModifySDConfigCommand
ModifySDConfigCommand::ModifySDConfigCommand(VolumeNode* node, const SensitiveDetectorConfig& newConfig)
    : node_(node)
    , newConfig_(newConfig)
{
    if (node_) {
        oldConfig_ = node_->getSDConfig();
    }
}

void ModifySDConfigCommand::execute() {
    if (node_) {
        node_->getSDConfig() = newConfig_;
    }
}

void ModifySDConfigCommand::undo() {
    if (node_) {
        node_->getSDConfig() = oldConfig_;
    }
}

// ModifyOpticalConfigCommand
ModifyOpticalConfigCommand::ModifyOpticalConfigCommand(VolumeNode* node, const OpticalSurfaceConfig& newConfig)
    : node_(node)
    , newConfig_(newConfig)
{
    if (node_) {
        oldConfig_ = node_->getOpticalConfig();
    }
}

void ModifyOpticalConfigCommand::execute() {
    if (node_) {
        node_->getOpticalConfig() = newConfig_;
    }
}

void ModifyOpticalConfigCommand::undo() {
    if (node_) {
        node_->getOpticalConfig() = oldConfig_;
    }
}

} // namespace geantcad

