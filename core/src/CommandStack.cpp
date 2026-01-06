#include "CommandStack.hh"
#include <string>

namespace geantcad {

CommandStack::CommandStack(size_t maxHistory)
    : currentIndex_(0)
    , maxHistory_(maxHistory)
{
}

void CommandStack::execute(std::unique_ptr<Command> cmd) {
    // Remove any redo history
    history_.erase(history_.begin() + currentIndex_, history_.end());
    
    // Execute command
    cmd->execute();
    
    // Add to history
    history_.push_back(std::move(cmd));
    currentIndex_ = history_.size();
    
    // Limit history size
    if (history_.size() > maxHistory_) {
        history_.erase(history_.begin());
        currentIndex_--;
    }
    
    notifyHistoryChanged();
}

void CommandStack::undo() {
    if (!canUndo()) return;
    
    currentIndex_--;
    history_[currentIndex_]->undo();
    notifyHistoryChanged();
}

void CommandStack::redo() {
    if (!canRedo()) return;
    
    history_[currentIndex_]->execute();
    currentIndex_++;
    notifyHistoryChanged();
}

void CommandStack::clear() {
    history_.clear();
    currentIndex_ = 0;
    notifyHistoryChanged();
}

const std::string& CommandStack::getUndoDescription() const {
    static const std::string empty;
    if (canUndo()) {
        return history_[currentIndex_ - 1]->getDescription();
    }
    return empty;
}

const std::string& CommandStack::getRedoDescription() const {
    static const std::string empty;
    if (canRedo()) {
        return history_[currentIndex_]->getDescription();
    }
    return empty;
}

void CommandStack::notifyHistoryChanged() {
    if (onHistoryChanged) {
        onHistoryChanged();
    }
}

} // namespace geantcad

