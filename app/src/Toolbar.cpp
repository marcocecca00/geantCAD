#include "Toolbar.hh"
#include <QAction>
#include <QActionGroup>
#include <QStyle>
#include <QLabel>

namespace geantcad {

Toolbar::Toolbar(QWidget* parent)
    : QToolBar(parent)
{
    setMovable(false);
    setIconSize(QSize(20, 20));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    setupActions();
}

void Toolbar::setupActions() {
    createHistorySection();
    addSeparator();
    createViewSection();
    addSeparator();
    createManipulationSection();
    addSeparator();
    createShapeSection();
    addSeparator();
    createEditSection();
    addSeparator();
    createAnalysisSection();
}

void Toolbar::createHistorySection() {
    // Category label
    QLabel* label = new QLabel(" History ");
    label->setStyleSheet("color: #888; font-size: 9pt;");
    addWidget(label);
    
    // Undo
    QAction* undoAction = addAction(style()->standardIcon(QStyle::SP_ArrowBack), "Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setToolTip("Undo (Ctrl+Z)");
    connect(undoAction, &QAction::triggered, this, &Toolbar::undoAction);
    
    // Redo
    QAction* redoAction = addAction(style()->standardIcon(QStyle::SP_ArrowForward), "Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setToolTip("Redo (Ctrl+Shift+Z)");
    connect(redoAction, &QAction::triggered, this, &Toolbar::redoAction);
}

void Toolbar::createViewSection() {
    // Category label
    QLabel* label = new QLabel(" View ");
    label->setStyleSheet("color: #888; font-size: 9pt;");
    addWidget(label);
    
    // View dropdown button
    QMenu* viewMenu = new QMenu(this);
    
    viewMenu->addAction("Front (Numpad 1)", this, &Toolbar::viewFront);
    viewMenu->addAction("Back (Ctrl+Numpad 1)", this, &Toolbar::viewBack);
    viewMenu->addAction("Left (Numpad 3)", this, &Toolbar::viewLeft);
    viewMenu->addAction("Right (Ctrl+Numpad 3)", this, &Toolbar::viewRight);
    viewMenu->addAction("Top (Numpad 7)", this, &Toolbar::viewTop);
    viewMenu->addAction("Bottom (Ctrl+Numpad 7)", this, &Toolbar::viewBottom);
    viewMenu->addSeparator();
    viewMenu->addAction("Isometric (Numpad 0)", this, &Toolbar::viewIsometric);
    
    QToolButton* viewBtn = createDropdownButton("Views", style()->standardIcon(QStyle::SP_ComputerIcon), viewMenu);
    viewBtn->setToolTip("Standard view orientations");
    addWidget(viewBtn);
    
    // Frame Selection
    QAction* frameAction = addAction(style()->standardIcon(QStyle::SP_FileDialogListView), "Frame");
    frameAction->setShortcut(QKeySequence("F"));
    frameAction->setToolTip("Frame selection (F)");
    connect(frameAction, &QAction::triggered, this, &Toolbar::viewFrameSelection);
    
    // Reset View
    QAction* resetAction = addAction(style()->standardIcon(QStyle::SP_DialogResetButton), "Reset");
    resetAction->setShortcut(QKeySequence("Home"));
    resetAction->setToolTip("Reset view (Home)");
    connect(resetAction, &QAction::triggered, this, &Toolbar::viewReset);
}

void Toolbar::createManipulationSection() {
    // Category label
    QLabel* label = new QLabel(" Tools ");
    label->setStyleSheet("color: #888; font-size: 9pt;");
    addWidget(label);
    
    // Tool actions (mutually exclusive)
    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);
    
    // Select tool
    selectAction_ = addAction(style()->standardIcon(QStyle::SP_ArrowUp), "Select");
    selectAction_->setCheckable(true);
    selectAction_->setChecked(true);
    selectAction_->setShortcut(QKeySequence("S"));
    selectAction_->setToolTip("Select tool (S) - Click to select objects");
    toolGroup->addAction(selectAction_);
    connect(selectAction_, &QAction::triggered, this, &Toolbar::toolSelect);
    
    // Move tool
    moveAction_ = addAction(style()->standardIcon(QStyle::SP_DialogApplyButton), "Move");
    moveAction_->setCheckable(true);
    moveAction_->setShortcut(QKeySequence("G"));
    moveAction_->setToolTip("Move tool (G) - Translate selected objects");
    toolGroup->addAction(moveAction_);
    connect(moveAction_, &QAction::triggered, this, &Toolbar::toolMove);
    
    // Rotate tool
    rotateAction_ = addAction(style()->standardIcon(QStyle::SP_BrowserReload), "Rotate");
    rotateAction_->setCheckable(true);
    rotateAction_->setShortcut(QKeySequence("R"));
    rotateAction_->setToolTip("Rotate tool (R) - Rotate selected objects");
    toolGroup->addAction(rotateAction_);
    connect(rotateAction_, &QAction::triggered, this, &Toolbar::toolRotate);
    
    // Scale tool
    scaleAction_ = addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Scale");
    scaleAction_->setCheckable(true);
    scaleAction_->setShortcut(QKeySequence("T"));
    scaleAction_->setToolTip("Scale tool (T) - Scale selected objects");
    toolGroup->addAction(scaleAction_);
    connect(scaleAction_, &QAction::triggered, this, &Toolbar::toolScale);
}

void Toolbar::createShapeSection() {
    // Category label
    QLabel* label = new QLabel(" Shapes ");
    label->setStyleSheet("color: #888; font-size: 9pt;");
    addWidget(label);
    
    // Shapes dropdown menu
    QMenu* shapeMenu = new QMenu(this);
    
    QAction* boxAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "Box");
    boxAction->setShortcut(QKeySequence("Ctrl+Shift+B"));
    boxAction->setToolTip("Create Box (Ctrl+Shift+B)");
    connect(boxAction, &QAction::triggered, this, &Toolbar::createBox);
    
    QAction* tubeAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_DirIcon), "Tube");
    tubeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    tubeAction->setToolTip("Create Tube/Cylinder (Ctrl+Shift+T)");
    connect(tubeAction, &QAction::triggered, this, &Toolbar::createTube);
    
    QAction* sphereAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_ComputerIcon), "Sphere");
    sphereAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    sphereAction->setToolTip("Create Sphere (Ctrl+Shift+S)");
    connect(sphereAction, &QAction::triggered, this, &Toolbar::createSphere);
    
    QAction* coneAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_DriveCDIcon), "Cone");
    coneAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    coneAction->setToolTip("Create Cone (Ctrl+Shift+C)");
    connect(coneAction, &QAction::triggered, this, &Toolbar::createCone);
    
    QAction* trdAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Trapezoid");
    trdAction->setShortcut(QKeySequence("Ctrl+Shift+D"));
    trdAction->setToolTip("Create Trapezoid/Trd (Ctrl+Shift+D)");
    connect(trdAction, &QAction::triggered, this, &Toolbar::createTrd);
    
    QToolButton* shapeBtn = createDropdownButton("Add Shape", style()->standardIcon(QStyle::SP_FileDialogNewFolder), shapeMenu);
    shapeBtn->setToolTip("Add primitive shapes to scene");
    addWidget(shapeBtn);
}

void Toolbar::createEditSection() {
    // Category label
    QLabel* label = new QLabel(" Edit ");
    label->setStyleSheet("color: #888; font-size: 9pt;");
    addWidget(label);
    
    // Duplicate
    QAction* duplicateAction = addAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Duplicate");
    duplicateAction->setShortcut(QKeySequence("Ctrl+D"));
    duplicateAction->setToolTip("Duplicate selected (Ctrl+D)");
    connect(duplicateAction, &QAction::triggered, this, &Toolbar::duplicateSelected);
    
    // Delete
    QAction* deleteAction = addAction(style()->standardIcon(QStyle::SP_TrashIcon), "Delete");
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setToolTip("Delete selected (Delete)");
    connect(deleteAction, &QAction::triggered, this, &Toolbar::deleteSelected);
    
    // Group/Ungroup menu
    QMenu* groupMenu = new QMenu(this);
    
    QAction* groupAction = groupMenu->addAction("Group (Ctrl+G)");
    groupAction->setShortcut(QKeySequence("Ctrl+G"));
    connect(groupAction, &QAction::triggered, this, &Toolbar::groupSelected);
    
    QAction* ungroupAction = groupMenu->addAction("Ungroup (Ctrl+Shift+G)");
    ungroupAction->setShortcut(QKeySequence("Ctrl+Shift+G"));
    connect(ungroupAction, &QAction::triggered, this, &Toolbar::ungroupSelected);
    
    QToolButton* groupBtn = createDropdownButton("Group", style()->standardIcon(QStyle::SP_DirIcon), groupMenu);
    groupBtn->setToolTip("Group/Ungroup objects");
    addWidget(groupBtn);
}

void Toolbar::createAnalysisSection() {
    // Category label
    QLabel* label = new QLabel(" Analysis ");
    label->setStyleSheet("color: #888; font-size: 9pt;");
    addWidget(label);
    
    // Measure tool
    measureAction_ = addAction(style()->standardIcon(QStyle::SP_FileDialogInfoView), "Measure");
    measureAction_->setCheckable(true);
    measureAction_->setToolTip("Measurement tool - distance, angle, coordinates");
    connect(measureAction_, &QAction::triggered, this, &Toolbar::toggleMeasureTool);
    
    // Clipping planes
    clippingAction_ = addAction(style()->standardIcon(QStyle::SP_TitleBarNormalButton), "Clip");
    clippingAction_->setCheckable(true);
    clippingAction_->setToolTip("Clipping planes - section view");
    connect(clippingAction_, &QAction::triggered, this, &Toolbar::toggleClippingPlanes);
}

QToolButton* Toolbar::createDropdownButton(const QString& text, const QIcon& icon, QMenu* menu) {
    QToolButton* button = new QToolButton(this);
    button->setText(text);
    button->setIcon(icon);
    button->setMenu(menu);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // Style for dropdown
    button->setStyleSheet(
        "QToolButton { padding: 4px 8px; }"
        "QToolButton::menu-indicator { image: none; }"
    );
    
    return button;
}

} // namespace geantcad
