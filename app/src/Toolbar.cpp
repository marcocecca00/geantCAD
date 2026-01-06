#include "Toolbar.hh"
#include <QAction>
#include <QActionGroup>
#include <QStyle>
#include <QLabel>
#include <QPainter>
#include <QPixmap>

namespace geantcad {

// Helper to create simple colored icons
static QIcon createColoredIcon(const QString& text, const QColor& color = QColor("#d4d4d4")) {
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(color);
    QFont font("Segoe UI Symbol", 14);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
    return QIcon(pixmap);
}

// Helper to create shape icons
static QIcon createShapeIcon(const QString& shape, const QColor& fillColor = QColor("#3794ff")) {
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(fillColor, 1.5));
    painter.setBrush(fillColor.darker(150));
    
    if (shape == "box") {
        painter.drawRect(3, 5, 14, 10);
        // 3D effect
        painter.drawLine(3, 5, 6, 2);
        painter.drawLine(17, 5, 20, 2);
        painter.drawLine(6, 2, 20, 2);
    } else if (shape == "sphere") {
        painter.drawEllipse(2, 2, 16, 16);
    } else if (shape == "tube") {
        painter.drawEllipse(4, 2, 12, 5);
        painter.drawLine(4, 4, 4, 15);
        painter.drawLine(16, 4, 16, 15);
        painter.drawArc(4, 12, 12, 5, 0, -180 * 16);
    } else if (shape == "cone") {
        QPolygon poly;
        poly << QPoint(10, 2) << QPoint(3, 17) << QPoint(17, 17);
        painter.drawPolygon(poly);
    } else if (shape == "trd") {
        QPolygon poly;
        poly << QPoint(5, 3) << QPoint(15, 3) << QPoint(17, 17) << QPoint(3, 17);
        painter.drawPolygon(poly);
    }
    
    return QIcon(pixmap);
}

// Helper to create tool icons
static QIcon createToolIcon(const QString& tool, const QColor& color = QColor("#d4d4d4")) {
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 1.5));
    
    if (tool == "select") {
        // Arrow cursor
        QPolygon poly;
        poly << QPoint(5, 3) << QPoint(5, 16) << QPoint(8, 13) << QPoint(11, 17) << QPoint(13, 16) << QPoint(10, 12) << QPoint(14, 12);
        painter.setBrush(color);
        painter.drawPolygon(poly);
    } else if (tool == "move") {
        // Cross arrows
        painter.drawLine(10, 3, 10, 17);
        painter.drawLine(3, 10, 17, 10);
        // Arrow heads
        painter.drawLine(10, 3, 7, 6);
        painter.drawLine(10, 3, 13, 6);
        painter.drawLine(10, 17, 7, 14);
        painter.drawLine(10, 17, 13, 14);
        painter.drawLine(3, 10, 6, 7);
        painter.drawLine(3, 10, 6, 13);
        painter.drawLine(17, 10, 14, 7);
        painter.drawLine(17, 10, 14, 13);
    } else if (tool == "rotate") {
        // Circular arrow
        painter.drawArc(3, 3, 14, 14, 45 * 16, 270 * 16);
        painter.drawLine(14, 5, 17, 3);
        painter.drawLine(14, 5, 17, 8);
    } else if (tool == "scale") {
        // Corner arrows
        painter.drawRect(6, 6, 8, 8);
        painter.drawLine(3, 3, 6, 6);
        painter.drawLine(3, 3, 3, 6);
        painter.drawLine(3, 3, 6, 3);
        painter.drawLine(17, 17, 14, 14);
        painter.drawLine(17, 17, 17, 14);
        painter.drawLine(17, 17, 14, 17);
    } else if (tool == "measure") {
        // Ruler
        painter.drawLine(3, 17, 17, 3);
        painter.drawLine(3, 17, 3, 13);
        painter.drawLine(3, 17, 7, 17);
        painter.drawLine(17, 3, 17, 7);
        painter.drawLine(17, 3, 13, 3);
        // Marks
        painter.drawLine(6, 14, 8, 12);
        painter.drawLine(10, 10, 12, 8);
    } else if (tool == "clip") {
        // Clipping plane
        painter.drawLine(3, 10, 17, 10);
        painter.drawRect(5, 4, 10, 12);
        painter.setPen(QPen(QColor("#f14c4c"), 2));
        painter.drawLine(3, 10, 17, 10);
    } else if (tool == "undo") {
        painter.drawArc(5, 5, 10, 10, 90 * 16, 180 * 16);
        painter.drawLine(5, 10, 2, 7);
        painter.drawLine(5, 10, 8, 7);
    } else if (tool == "redo") {
        painter.drawArc(5, 5, 10, 10, -90 * 16, 180 * 16);
        painter.drawLine(15, 10, 18, 7);
        painter.drawLine(15, 10, 12, 7);
    } else if (tool == "delete") {
        painter.setPen(QPen(QColor("#f14c4c"), 2));
        painter.drawLine(4, 4, 16, 16);
        painter.drawLine(16, 4, 4, 16);
    } else if (tool == "duplicate") {
        painter.drawRect(3, 5, 10, 10);
        painter.drawRect(7, 3, 10, 10);
    } else if (tool == "group") {
        painter.drawRect(3, 3, 6, 6);
        painter.drawRect(11, 11, 6, 6);
        painter.setPen(QPen(color, 1, Qt::DashLine));
        painter.drawRect(2, 2, 16, 16);
    } else if (tool == "frame") {
        painter.drawRect(4, 4, 12, 12);
        painter.drawLine(10, 1, 10, 4);
        painter.drawLine(10, 16, 10, 19);
        painter.drawLine(1, 10, 4, 10);
        painter.drawLine(16, 10, 19, 10);
    } else if (tool == "reset") {
        painter.drawEllipse(4, 4, 12, 12);
        painter.drawLine(10, 4, 10, 10);
        painter.drawLine(10, 10, 14, 7);
    } else if (tool == "view") {
        // Eye icon
        painter.drawEllipse(3, 6, 14, 8);
        painter.setBrush(color);
        painter.drawEllipse(8, 8, 4, 4);
    }
    
    return QIcon(pixmap);
}

Toolbar::Toolbar(QWidget* parent)
    : QToolBar(parent)
{
    setMovable(false);
    setIconSize(QSize(22, 22));
    setToolButtonStyle(Qt::ToolButtonIconOnly); // Compact: icons only
    
    // Compact styling
    setStyleSheet(R"(
        QToolBar {
            spacing: 2px;
            padding: 2px 4px;
        }
        QToolButton {
            padding: 4px;
            margin: 1px;
            border-radius: 4px;
        }
        QToolButton:hover {
            background-color: #3a3d3e;
        }
        QToolButton:checked {
            background-color: #094771;
        }
    )");
    
    setupActions();
}

void Toolbar::setupActions() {
    createHistorySection();
    addSeparator();
    createManipulationSection();
    addSeparator();
    createShapeSection();
    addSeparator();
    createEditSection();
    addSeparator();
    createViewSection();
    addSeparator();
    createAnalysisSection();
}

void Toolbar::createHistorySection() {
    // Undo
    QAction* undoAction = addAction(createToolIcon("undo"), "Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setToolTip("Undo (Ctrl+Z)");
    connect(undoAction, &QAction::triggered, this, &Toolbar::undoAction);
    
    // Redo
    QAction* redoAction = addAction(createToolIcon("redo"), "Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setToolTip("Redo (Ctrl+Shift+Z)");
    connect(redoAction, &QAction::triggered, this, &Toolbar::redoAction);
}

void Toolbar::createViewSection() {
    // Only Frame Selection and Home View - simplified
    
    // Frame Selection (zoom to selected object)
    QAction* frameAction = addAction(createToolIcon("frame"), "Frame");
    frameAction->setShortcut(QKeySequence("F"));
    frameAction->setToolTip("Frame Selection (F)");
    connect(frameAction, &QAction::triggered, this, &Toolbar::viewFrameSelection);
    
    // Home View (reset to default view)
    QAction* resetAction = addAction(createToolIcon("reset"), "Home");
    resetAction->setShortcut(QKeySequence("Home"));
    resetAction->setToolTip("Home View (Home)");
    connect(resetAction, &QAction::triggered, this, &Toolbar::viewReset);
}

void Toolbar::createManipulationSection() {
    // Tool actions (mutually exclusive)
    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);
    
    // Select tool
    selectAction_ = addAction(createToolIcon("select"), "Select");
    selectAction_->setCheckable(true);
    selectAction_->setChecked(true);
    selectAction_->setShortcut(QKeySequence("S"));
    selectAction_->setToolTip("Select (S)");
    toolGroup->addAction(selectAction_);
    connect(selectAction_, &QAction::triggered, this, &Toolbar::toolSelect);
    
    // Move tool
    moveAction_ = addAction(createToolIcon("move"), "Move");
    moveAction_->setCheckable(true);
    moveAction_->setShortcut(QKeySequence("G"));
    moveAction_->setToolTip("Move (G)");
    toolGroup->addAction(moveAction_);
    connect(moveAction_, &QAction::triggered, this, &Toolbar::toolMove);
    
    // Rotate tool
    rotateAction_ = addAction(createToolIcon("rotate"), "Rotate");
    rotateAction_->setCheckable(true);
    rotateAction_->setShortcut(QKeySequence("R"));
    rotateAction_->setToolTip("Rotate (R)");
    toolGroup->addAction(rotateAction_);
    connect(rotateAction_, &QAction::triggered, this, &Toolbar::toolRotate);
    
    // Scale tool
    scaleAction_ = addAction(createToolIcon("scale"), "Scale");
    scaleAction_->setCheckable(true);
    scaleAction_->setShortcut(QKeySequence("T"));
    scaleAction_->setToolTip("Scale (T)");
    toolGroup->addAction(scaleAction_);
    connect(scaleAction_, &QAction::triggered, this, &Toolbar::toolScale);
}

void Toolbar::createShapeSection() {
    // Shapes dropdown menu
    QMenu* shapeMenu = new QMenu(this);
    shapeMenu->setStyleSheet("QMenu { min-width: 150px; }");
    
    QAction* boxAction = shapeMenu->addAction(createShapeIcon("box"), "Box");
    boxAction->setShortcut(QKeySequence("Ctrl+Shift+B"));
    boxAction->setToolTip("Create Box (Ctrl+Shift+B)");
    connect(boxAction, &QAction::triggered, this, &Toolbar::createBox);
    
    QAction* tubeAction = shapeMenu->addAction(createShapeIcon("tube"), "Cylinder");
    tubeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    tubeAction->setToolTip("Create Cylinder (Ctrl+Shift+T)");
    connect(tubeAction, &QAction::triggered, this, &Toolbar::createTube);
    
    QAction* sphereAction = shapeMenu->addAction(createShapeIcon("sphere"), "Sphere");
    sphereAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    sphereAction->setToolTip("Create Sphere (Ctrl+Shift+S)");
    connect(sphereAction, &QAction::triggered, this, &Toolbar::createSphere);
    
    QAction* coneAction = shapeMenu->addAction(createShapeIcon("cone"), "Cone");
    coneAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    coneAction->setToolTip("Create Cone (Ctrl+Shift+C)");
    connect(coneAction, &QAction::triggered, this, &Toolbar::createCone);
    
    QAction* trdAction = shapeMenu->addAction(createShapeIcon("trd"), "Trapezoid");
    trdAction->setShortcut(QKeySequence("Ctrl+Shift+D"));
    trdAction->setToolTip("Create Trapezoid (Ctrl+Shift+D)");
    connect(trdAction, &QAction::triggered, this, &Toolbar::createTrd);
    
    // Main button with + icon
    QPixmap plusPixmap(22, 22);
    plusPixmap.fill(Qt::transparent);
    QPainter painter(&plusPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#4ec9b0"), 2));
    painter.drawLine(11, 4, 11, 18);
    painter.drawLine(4, 11, 18, 11);
    
    QToolButton* shapeBtn = createDropdownButton(QIcon(plusPixmap), shapeMenu);
    shapeBtn->setToolTip("Add Shape");
    addWidget(shapeBtn);
}

void Toolbar::createEditSection() {
    // Duplicate
    QAction* duplicateAction = addAction(createToolIcon("duplicate"), "Duplicate");
    duplicateAction->setShortcut(QKeySequence("Ctrl+D"));
    duplicateAction->setToolTip("Duplicate (Ctrl+D)");
    connect(duplicateAction, &QAction::triggered, this, &Toolbar::duplicateSelected);
    
    // Delete
    QAction* deleteAction = addAction(createToolIcon("delete"), "Delete");
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setToolTip("Delete (Del)");
    connect(deleteAction, &QAction::triggered, this, &Toolbar::deleteSelected);
    
    // Group/Ungroup menu
    QMenu* groupMenu = new QMenu(this);
    
    QAction* groupAction = groupMenu->addAction("Group (Ctrl+G)");
    groupAction->setShortcut(QKeySequence("Ctrl+G"));
    connect(groupAction, &QAction::triggered, this, &Toolbar::groupSelected);
    
    QAction* ungroupAction = groupMenu->addAction("Ungroup (Ctrl+Shift+G)");
    ungroupAction->setShortcut(QKeySequence("Ctrl+Shift+G"));
    connect(ungroupAction, &QAction::triggered, this, &Toolbar::ungroupSelected);
    
    QToolButton* groupBtn = createDropdownButton(createToolIcon("group"), groupMenu);
    groupBtn->setToolTip("Group/Ungroup");
    addWidget(groupBtn);
}

void Toolbar::createAnalysisSection() {
    // Measure tool
    measureAction_ = addAction(createToolIcon("measure"), "Measure");
    measureAction_->setCheckable(true);
    measureAction_->setToolTip("Measure Tool");
    connect(measureAction_, &QAction::triggered, this, &Toolbar::toggleMeasureTool);
    
    // Clipping planes
    clippingAction_ = addAction(createToolIcon("clip"), "Clip");
    clippingAction_->setCheckable(true);
    clippingAction_->setToolTip("Clipping Plane");
    connect(clippingAction_, &QAction::triggered, this, &Toolbar::toggleClippingPlanes);
}

QToolButton* Toolbar::createDropdownButton(const QIcon& icon, QMenu* menu) {
    QToolButton* button = new QToolButton(this);
    button->setIcon(icon);
    button->setMenu(menu);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    
    // Style for dropdown
    button->setStyleSheet(R"(
        QToolButton {
            padding: 4px;
            border-radius: 4px;
        }
        QToolButton::menu-indicator { 
            image: none;
            width: 0;
        }
    )");
    
    return button;
}

// === Sync toolbar from external mode changes ===
void Toolbar::setSelectMode() {
    if (selectAction_ && !selectAction_->isChecked()) {
        selectAction_->setChecked(true);
    }
}

void Toolbar::setMoveMode() {
    if (moveAction_ && !moveAction_->isChecked()) {
        moveAction_->setChecked(true);
    }
}

void Toolbar::setRotateMode() {
    if (rotateAction_ && !rotateAction_->isChecked()) {
        rotateAction_->setChecked(true);
    }
}

void Toolbar::setScaleMode() {
    if (scaleAction_ && !scaleAction_->isChecked()) {
        scaleAction_->setChecked(true);
    }
}

} // namespace geantcad
