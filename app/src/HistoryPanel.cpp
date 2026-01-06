#include "HistoryPanel.hh"
#include <QGroupBox>
#include <QStyle>

namespace geantcad {

HistoryPanel::HistoryPanel(QWidget* parent)
    : QWidget(parent)
    , commandStack_(nullptr)
{
    setupUI();
}

void HistoryPanel::setCommandStack(CommandStack* commandStack) {
    commandStack_ = commandStack;
    refresh();
}

void HistoryPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);
    
    // Title
    QLabel* titleLabel = new QLabel("History", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 11pt; color: #e0e0e0;");
    mainLayout->addWidget(titleLabel);
    
    // Undo/Redo buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    
    undoBtn_ = new QPushButton("⟵ Undo", this);
    undoBtn_->setToolTip("Undo last action (Ctrl+Z)");
    undoBtn_->setShortcut(QKeySequence::Undo);
    connect(undoBtn_, &QPushButton::clicked, this, &HistoryPanel::onUndoClicked);
    btnLayout->addWidget(undoBtn_);
    
    redoBtn_ = new QPushButton("Redo ⟶", this);
    redoBtn_->setToolTip("Redo last undone action (Ctrl+Shift+Z)");
    redoBtn_->setShortcut(QKeySequence::Redo);
    connect(redoBtn_, &QPushButton::clicked, this, &HistoryPanel::onRedoClicked);
    btnLayout->addWidget(redoBtn_);
    
    mainLayout->addLayout(btnLayout);
    
    // History list
    QGroupBox* listGroup = new QGroupBox("Command History", this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    listLayout->setContentsMargins(4, 4, 4, 4);
    
    historyList_ = new QListWidget(this);
    historyList_->setMinimumHeight(150);
    historyList_->setToolTip("Double-click to jump to a state");
    connect(historyList_, &QListWidget::itemClicked, this, &HistoryPanel::onItemClicked);
    connect(historyList_, &QListWidget::itemDoubleClicked, this, &HistoryPanel::onItemDoubleClicked);
    listLayout->addWidget(historyList_);
    
    // Clear button
    clearBtn_ = new QPushButton("Clear History", this);
    clearBtn_->setToolTip("Clear all history (cannot be undone)");
    connect(clearBtn_, &QPushButton::clicked, this, &HistoryPanel::onClearHistory);
    listLayout->addWidget(clearBtn_);
    
    mainLayout->addWidget(listGroup);
    
    // Status
    statusLabel_ = new QLabel("No history", this);
    statusLabel_->setStyleSheet("color: #a0a0a0; font-style: italic;");
    mainLayout->addWidget(statusLabel_);
    
    mainLayout->addStretch();
    
    refresh();
}

void HistoryPanel::refresh() {
    updateHistoryList();
    
    if (commandStack_) {
        undoBtn_->setEnabled(commandStack_->canUndo());
        redoBtn_->setEnabled(commandStack_->canRedo());
        clearBtn_->setEnabled(commandStack_->getHistorySize() > 0);
        
        int current = commandStack_->getCurrentIndex();
        int total = commandStack_->getHistorySize();
        
        if (total > 0) {
            statusLabel_->setText(QString("State %1 of %2").arg(current + 1).arg(total));
        } else {
            statusLabel_->setText("No history");
        }
    } else {
        undoBtn_->setEnabled(false);
        redoBtn_->setEnabled(false);
        clearBtn_->setEnabled(false);
        statusLabel_->setText("No command stack");
    }
}

void HistoryPanel::updateHistoryList() {
    historyList_->clear();
    
    if (!commandStack_) return;
    
    int currentIndex = commandStack_->getCurrentIndex();
    
    // Add initial state marker
    QListWidgetItem* initialItem = new QListWidgetItem("🔵 Initial State");
    initialItem->setData(Qt::UserRole, -1);
    if (currentIndex < 0) {
        initialItem->setBackground(QColor(0, 120, 212, 100));
        initialItem->setForeground(Qt::white);
    }
    historyList_->addItem(initialItem);
    
    // Add command history
    const auto& history = commandStack_->getHistory();
    for (size_t i = 0; i < history.size(); ++i) {
        QString desc = getCommandDescription(history[i].get());
        QString icon;
        
        // Determine icon based on command type
        if (desc.contains("Create")) {
            icon = "➕";
        } else if (desc.contains("Delete")) {
            icon = "➖";
        } else if (desc.contains("Transform") || desc.contains("Move") || desc.contains("Rotate")) {
            icon = "🔄";
        } else if (desc.contains("Duplicate")) {
            icon = "📋";
        } else if (desc.contains("Material")) {
            icon = "🎨";
        } else {
            icon = "•";
        }
        
        QString text = QString("%1 %2").arg(icon).arg(desc);
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, static_cast<int>(i));
        
        // Highlight current position
        if (static_cast<int>(i) == currentIndex) {
            item->setBackground(QColor(0, 120, 212, 100));
            item->setForeground(Qt::white);
        } else if (static_cast<int>(i) > currentIndex) {
            // Grayed out for redo-able commands
            item->setForeground(QColor(128, 128, 128));
        }
        
        historyList_->addItem(item);
    }
    
    // Scroll to current position
    if (currentIndex >= 0 && currentIndex < historyList_->count()) {
        historyList_->setCurrentRow(currentIndex + 1);  // +1 for initial state
        historyList_->scrollToItem(historyList_->currentItem());
    }
}

QString HistoryPanel::getCommandDescription(const Command* cmd) const {
    if (!cmd) return "Unknown";
    return QString::fromStdString(cmd->getDescription());
}

void HistoryPanel::undo() {
    if (commandStack_ && commandStack_->canUndo()) {
        commandStack_->undo();
        refresh();
        emit historyChanged();
    }
}

void HistoryPanel::redo() {
    if (commandStack_ && commandStack_->canRedo()) {
        commandStack_->redo();
        refresh();
        emit historyChanged();
    }
}

void HistoryPanel::jumpToState(int index) {
    if (!commandStack_) return;
    
    int currentIndex = commandStack_->getCurrentIndex();
    
    if (index < currentIndex) {
        // Undo to reach target state
        while (commandStack_->getCurrentIndex() > index && commandStack_->canUndo()) {
            commandStack_->undo();
        }
    } else if (index > currentIndex) {
        // Redo to reach target state
        while (commandStack_->getCurrentIndex() < index && commandStack_->canRedo()) {
            commandStack_->redo();
        }
    }
    
    refresh();
    emit stateRestored();
}

void HistoryPanel::onUndoClicked() {
    undo();
}

void HistoryPanel::onRedoClicked() {
    redo();
}

void HistoryPanel::onClearHistory() {
    if (commandStack_) {
        commandStack_->clear();
        refresh();
        emit historyChanged();
    }
}

void HistoryPanel::onItemClicked(QListWidgetItem* item) {
    // Just highlight, don't jump yet
    Q_UNUSED(item);
}

void HistoryPanel::onItemDoubleClicked(QListWidgetItem* item) {
    if (!item) return;
    
    int index = item->data(Qt::UserRole).toInt();
    jumpToState(index);
}

} // namespace geantcad

