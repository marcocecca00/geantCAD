#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>

namespace geantcad {

/**
 * ShortcutsDialog - Displays all keyboard shortcuts
 */
class ShortcutsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ShortcutsDialog(QWidget* parent = nullptr);

private:
    void setupUI();
    void populateShortcuts();
    void filterShortcuts(const QString& filter);
    
    QLineEdit* searchEdit_;
    QTableWidget* table_;
    
    struct Shortcut {
        QString category;
        QString action;
        QString shortcut;
        QString description;
    };
    
    QList<Shortcut> allShortcuts_;
};

} // namespace geantcad

