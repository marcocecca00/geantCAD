#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include "../../core/include/CommandStack.hh"

namespace geantcad {

/**
 * @brief Panel for visualizing and navigating undo/redo history
 * 
 * Shows the command stack history and allows jumping to any state.
 */
class HistoryPanel : public QWidget {
    Q_OBJECT

public:
    explicit HistoryPanel(QWidget* parent = nullptr);
    ~HistoryPanel() override = default;

    void setCommandStack(CommandStack* commandStack);
    CommandStack* getCommandStack() const { return commandStack_; }

signals:
    void historyChanged();
    void stateRestored();

public slots:
    void refresh();
    void undo();
    void redo();
    void jumpToState(int index);

private slots:
    void onUndoClicked();
    void onRedoClicked();
    void onClearHistory();
    void onItemClicked(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void setupUI();
    void updateHistoryList();
    QString getCommandDescription(const Command* cmd) const;

    // UI elements
    QListWidget* historyList_ = nullptr;
    QPushButton* undoBtn_ = nullptr;
    QPushButton* redoBtn_ = nullptr;
    QPushButton* clearBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;

    // State
    CommandStack* commandStack_ = nullptr;
};

} // namespace geantcad

