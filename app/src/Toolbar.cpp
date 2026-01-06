#include "Toolbar.hh"
#include <QAction>
#include <QActionGroup>
#include <QStyle>

namespace geantcad {

Toolbar::Toolbar(QWidget* parent)
    : QToolBar(parent)
{
    setupActions();
}

void Toolbar::setupActions() {
    // === CATEGORY: Tools (Manipulation) ===
    // Tool actions (mutually exclusive) - these are checkable and show active state
    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true); // Only one tool active at a time
    
    // Select tool - use arrow icon
    QAction* selectAction = addAction(style()->standardIcon(QStyle::SP_ArrowUp), "Select");
    connect(selectAction, &QAction::triggered, this, &Toolbar::toolSelect);
    selectAction->setCheckable(true);
    selectAction->setChecked(true); // Default active tool
    selectAction->setToolTip("Select tool (S)");
    toolGroup->addAction(selectAction);
    
    // Move tool - use arrow forward icon
    QAction* moveAction = addAction(style()->standardIcon(QStyle::SP_ArrowForward), "Move");
    connect(moveAction, &QAction::triggered, this, &Toolbar::toolMove);
    moveAction->setCheckable(true);
    moveAction->setToolTip("Move tool (G)");
    toolGroup->addAction(moveAction);
    
    // Rotate tool - use refresh/reload icon
    QAction* rotateAction = addAction(style()->standardIcon(QStyle::SP_BrowserReload), "Rotate");
    connect(rotateAction, &QAction::triggered, this, &Toolbar::toolRotate);
    rotateAction->setCheckable(true);
    rotateAction->setToolTip("Rotate tool (R)");
    toolGroup->addAction(rotateAction);
    
    // Scale tool - use detailed view icon
    QAction* scaleAction = addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Scale");
    connect(scaleAction, &QAction::triggered, this, &Toolbar::toolScale);
    scaleAction->setCheckable(true);
    scaleAction->setToolTip("Scale tool (S)");
    toolGroup->addAction(scaleAction);
    
    addSeparator();
    
    // Primitives moved to Insert -> Shape menu
    // Toolbar now only contains manipulation tools
    
    // === CATEGORY: Edit (Modify) ===
    // Edit actions - use standard icons
    QAction* duplicateAction = addAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Duplicate");
    connect(duplicateAction, &QAction::triggered, this, &Toolbar::duplicateSelected);
    duplicateAction->setToolTip("Duplicate selected");
    
    QAction* deleteAction = addAction(style()->standardIcon(QStyle::SP_TrashIcon), "Delete");
    connect(deleteAction, &QAction::triggered, this, &Toolbar::deleteSelected);
    deleteAction->setToolTip("Delete selected");
}

} // namespace geantcad

