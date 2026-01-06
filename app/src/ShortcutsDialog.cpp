#include "ShortcutsDialog.hh"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>

namespace geantcad {

ShortcutsDialog::ShortcutsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Keyboard Shortcuts");
    setMinimumSize(600, 500);
    setupUI();
    populateShortcuts();
}

void ShortcutsDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Header
    QLabel* headerLabel = new QLabel("⌨️ <b>Keyboard Shortcuts Reference</b>", this);
    headerLabel->setStyleSheet("font-size: 16px; margin-bottom: 10px;");
    layout->addWidget(headerLabel);
    
    // Search
    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchIcon = new QLabel("🔍", this);
    searchLayout->addWidget(searchIcon);
    
    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText("Search shortcuts...");
    searchEdit_->setClearButtonEnabled(true);
    connect(searchEdit_, &QLineEdit::textChanged, this, &ShortcutsDialog::filterShortcuts);
    searchLayout->addWidget(searchEdit_);
    
    layout->addLayout(searchLayout);
    
    // Table
    table_ = new QTableWidget(this);
    table_->setColumnCount(4);
    table_->setHorizontalHeaderLabels({"Category", "Action", "Shortcut", "Description"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->verticalHeader()->setVisible(false);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);
    table_->setShowGrid(false);
    layout->addWidget(table_);
    
    // Close button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton* closeBtn = new QPushButton("Close", this);
    closeBtn->setDefault(true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeBtn);
    layout->addLayout(buttonLayout);
}

void ShortcutsDialog::populateShortcuts() {
    allShortcuts_ = {
        // File operations
        {"File", "New Project", "Ctrl+N", "Create a new project"},
        {"File", "Open Project", "Ctrl+O", "Open an existing project"},
        {"File", "Save", "Ctrl+S", "Save current project"},
        {"File", "Save As", "Ctrl+Shift+S", "Save project with new name"},
        {"File", "Quit", "Ctrl+Q", "Exit application"},
        
        // Edit operations
        {"Edit", "Undo", "Ctrl+Z", "Undo last action"},
        {"Edit", "Redo", "Ctrl+Y", "Redo undone action"},
        {"Edit", "Delete", "Delete", "Delete selected object"},
        {"Edit", "Duplicate", "Ctrl+D", "Duplicate selected object"},
        
        // Tools (Blender-like)
        {"Tools", "Select", "S", "Selection mode"},
        {"Tools", "Move", "W", "Move/translate mode"},
        {"Tools", "Rotate", "E", "Rotation mode"},
        {"Tools", "Scale", "T", "Scale mode"},
        
        // Axis constraints (during Move/Rotate/Scale)
        {"Constraint", "X Axis", "X", "Constrain to X axis"},
        {"Constraint", "Y Axis", "Y", "Constrain to Y axis"},
        {"Constraint", "Z Axis", "Z", "Constrain to Z axis"},
        
        // View controls
        {"View", "Frame Selected", "F", "Frame selected object in view"},
        {"View", "Reset View", "Home", "Reset camera to default"},
        
        // Mouse controls
        {"Mouse", "Select Object", "Left Click", "Select object under cursor"},
        {"Mouse", "Context Menu", "Right Click", "Show context menu"},
        {"Mouse", "Orbit Camera", "Middle Drag", "Rotate camera around target"},
        {"Mouse", "Pan Camera", "Shift+Middle", "Pan the view"},
        {"Mouse", "Zoom", "Scroll Wheel", "Zoom in/out"},
        {"Mouse", "Transform", "Left Drag", "Drag selected object (in W/E/T mode)"},
        
        // General
        {"General", "Show Shortcuts", "Ctrl+/", "Show this dialog"},
        {"General", "Preferences", "Ctrl+,", "Open preferences"},
    };
    
    filterShortcuts("");
}

void ShortcutsDialog::filterShortcuts(const QString& filter) {
    table_->setRowCount(0);
    
    QString lowerFilter = filter.toLower();
    QString currentCategory;
    
    for (const auto& shortcut : allShortcuts_) {
        // Filter check
        if (!filter.isEmpty()) {
            bool matches = shortcut.action.toLower().contains(lowerFilter) ||
                          shortcut.shortcut.toLower().contains(lowerFilter) ||
                          shortcut.description.toLower().contains(lowerFilter) ||
                          shortcut.category.toLower().contains(lowerFilter);
            if (!matches) continue;
        }
        
        int row = table_->rowCount();
        table_->insertRow(row);
        
        // Category (only show if different from previous)
        QTableWidgetItem* catItem = new QTableWidgetItem(shortcut.category);
        catItem->setForeground(QBrush(QColor("#858585")));
        table_->setItem(row, 0, catItem);
        
        // Action
        QTableWidgetItem* actionItem = new QTableWidgetItem(shortcut.action);
        actionItem->setFont(QFont(actionItem->font().family(), -1, QFont::Medium));
        table_->setItem(row, 1, actionItem);
        
        // Shortcut (styled like a key)
        QTableWidgetItem* shortcutItem = new QTableWidgetItem(shortcut.shortcut);
        shortcutItem->setForeground(QBrush(QColor("#4ec9b0")));
        shortcutItem->setFont(QFont("JetBrains Mono", 11, QFont::Bold));
        table_->setItem(row, 2, shortcutItem);
        
        // Description
        QTableWidgetItem* descItem = new QTableWidgetItem(shortcut.description);
        descItem->setForeground(QBrush(QColor("#858585")));
        table_->setItem(row, 3, descItem);
    }
    
    table_->resizeRowsToContents();
}

} // namespace geantcad

