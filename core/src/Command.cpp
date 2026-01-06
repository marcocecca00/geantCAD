#include "Command.hh"
#include "SceneGraph.hh"
#include <algorithm>

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
        
        // Save state
        if (node_->getShape()) {
            // Clone shape params (simplified - would need proper cloning)
        }
        material_ = node_->getMaterial();
        transform_ = node_->getTransform();
        
        // Save children
        for (auto* child : node_->getChildren()) {
            children_.push_back(child);
        }
    }
}

void DeleteVolumeCommand::execute() {
    if (node_ && sceneGraph_) {
        sceneGraph_->removeVolume(node_);
    }
}

void DeleteVolumeCommand::undo() {
    if (!node_ || !sceneGraph_) return;
    
    // Recreate node (simplified - full implementation would restore all state)
    // For MVP, this is a placeholder
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
    
    // Copy shape (simplified - would need proper cloning)
    // For MVP, we'll recreate from shape type
    if (source->getShape()) {
        const Shape* shape = source->getShape();
        // TODO: Clone shape properly
        // For now, just copy the shape pointer (not ideal, but works for MVP)
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

} // namespace geantcad

