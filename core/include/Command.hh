#pragma once

#include "CommandStack.hh"
#include "SceneGraph.hh"
#include "VolumeNode.hh"
#include "Shape.hh"
#include "Material.hh"
#include <memory>
#include <string>

namespace geantcad {

// Forward declaration
class CommandStack;

/**
 * Concrete commands for undo/redo
 */

class CreateVolumeCommand : public Command {
public:
    CreateVolumeCommand(SceneGraph* sceneGraph, const std::string& name, 
                       std::unique_ptr<Shape> shape, std::shared_ptr<Material> material);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Create " + volumeName_; }
    
    VolumeNode* getCreatedNode() const { return createdNode_; }

private:
    SceneGraph* sceneGraph_;
    std::string volumeName_;
    std::unique_ptr<Shape> shape_;
    std::shared_ptr<Material> material_;
    VolumeNode* createdNode_ = nullptr;
};

class DeleteVolumeCommand : public Command {
public:
    DeleteVolumeCommand(SceneGraph* sceneGraph, VolumeNode* node);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Delete " + volumeName_; }

private:
    SceneGraph* sceneGraph_;
    VolumeNode* node_;
    std::string volumeName_;
    VolumeNode* parent_;
    size_t childIndex_;
    std::unique_ptr<Shape> shape_;
    std::shared_ptr<Material> material_;
    Transform transform_;
    std::vector<VolumeNode*> children_; // Store children for undo
    nlohmann::json nodeJson_; // Full node state for undo
};

class TransformVolumeCommand : public Command {
public:
    TransformVolumeCommand(VolumeNode* node, const Transform& newTransform);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Transform " + node_->getName(); }

private:
    VolumeNode* node_;
    Transform oldTransform_;
    Transform newTransform_;
};

class DuplicateVolumeCommand : public Command {
public:
    DuplicateVolumeCommand(SceneGraph* sceneGraph, VolumeNode* sourceNode);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Duplicate " + sourceNode_->getName(); }
    
    VolumeNode* getDuplicatedNode() const { return duplicatedNode_; }

private:
    SceneGraph* sceneGraph_;
    VolumeNode* sourceNode_;
    VolumeNode* duplicatedNode_ = nullptr;
    
    VolumeNode* duplicateNodeRecursive(VolumeNode* source);
};

class ModifyShapeCommand : public Command {
public:
    ModifyShapeCommand(VolumeNode* node, const ShapeParams& newParams);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Modify Shape " + node_->getName(); }

private:
    VolumeNode* node_;
    ShapeParams oldParams_;
    ShapeParams newParams_;
};

class ModifyNameCommand : public Command {
public:
    ModifyNameCommand(VolumeNode* node, const std::string& newName);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Rename " + (node_ ? node_->getName() : "volume"); }

private:
    VolumeNode* node_;
    std::string oldName_;
    std::string newName_;
};

class ModifyMaterialCommand : public Command {
public:
    ModifyMaterialCommand(VolumeNode* node, std::shared_ptr<Material> newMaterial);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Modify Material " + (node_ ? node_->getName() : "volume"); }

private:
    VolumeNode* node_;
    std::shared_ptr<Material> oldMaterial_;
    std::shared_ptr<Material> newMaterial_;
};

class ModifySDConfigCommand : public Command {
public:
    ModifySDConfigCommand(VolumeNode* node, const SensitiveDetectorConfig& newConfig);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Modify SD Config " + (node_ ? node_->getName() : "volume"); }

private:
    VolumeNode* node_;
    SensitiveDetectorConfig oldConfig_;
    SensitiveDetectorConfig newConfig_;
};

class ModifyOpticalConfigCommand : public Command {
public:
    ModifyOpticalConfigCommand(VolumeNode* node, const OpticalSurfaceConfig& newConfig);
    void execute() override;
    void undo() override;
    std::string getDescription() const override { return "Modify Optical Config " + (node_ ? node_->getName() : "volume"); }

private:
    VolumeNode* node_;
    OpticalSurfaceConfig oldConfig_;
    OpticalSurfaceConfig newConfig_;
};

// Convenience command aliases (used by Inspector)
using SetNameCommand = ModifyNameCommand;
using SetMaterialCommand = ModifyMaterialCommand;
using SetShapeCommand = ModifyShapeCommand;
using SetSensitiveDetectorCommand = ModifySDConfigCommand;
using SetOpticalSurfaceCommand = ModifyOpticalConfigCommand;
using SetTransformCommand = TransformVolumeCommand;

} // namespace geantcad

