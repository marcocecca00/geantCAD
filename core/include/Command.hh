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

} // namespace geantcad

