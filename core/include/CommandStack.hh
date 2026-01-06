#pragma once

#include <memory>
#include <vector>
#include <functional>

namespace geantcad {

/**
 * Command pattern per undo/redo.
 */
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getDescription() const = 0;
};

/**
 * CommandStack gestisce undo/redo.
 */
class CommandStack {
public:
    CommandStack(size_t maxHistory = 100);
    
    void execute(std::unique_ptr<Command> cmd);
    void undo();
    void redo();
    void clear();
    
    bool canUndo() const { return currentIndex_ > 0; }
    bool canRedo() const { return currentIndex_ < history_.size(); }
    
    const std::string& getUndoDescription() const;
    const std::string& getRedoDescription() const;
    
    // Signals
    std::function<void()> onHistoryChanged;

private:
    std::vector<std::unique_ptr<Command>> history_;
    size_t currentIndex_;
    size_t maxHistory_;
    
    void notifyHistoryChanged();
};

} // namespace geantcad

