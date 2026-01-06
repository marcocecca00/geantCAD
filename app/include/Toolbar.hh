#pragma once

#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QWidgetAction>

namespace geantcad {

/**
 * @brief Enhanced toolbar with categorized tool groups
 * 
 * Organized into categories following CAD best practices:
 * - View tools (camera control)
 * - Manipulation tools (select, move, rotate, scale)
 * - Create tools (shapes/primitives)
 * - Edit tools (modify, duplicate, delete)
 * - Analysis tools (measure, clip)
 */
class Toolbar : public QToolBar {
    Q_OBJECT

public:
    explicit Toolbar(QWidget* parent = nullptr);
    
public slots:
    // Sync toolbar with viewport mode changes
    void setSelectMode();
    void setMoveMode();
    void setRotateMode();
    void setScaleMode();

signals:
    // === View ===
    void viewFront();
    void viewBack();
    void viewLeft();
    void viewRight();
    void viewTop();
    void viewBottom();
    void viewIsometric();
    void viewReset();
    void viewFrameSelection();
    
    // === Manipulation Tools ===
    void toolSelect();
    void toolMove();
    void toolRotate();
    void toolScale();
    
    // === Create (Shapes) ===
    void createBox();
    void createTube();
    void createSphere();
    void createCone();
    void createTrd();
    
    // === Edit ===
    void deleteSelected();
    void duplicateSelected();
    void groupSelected();
    void ungroupSelected();
    
    // === Analysis ===
    void toggleMeasureTool();
    void toggleClippingPlanes();
    
    // === History ===
    void undoAction();
    void redoAction();

private:
    void setupActions();
    void createViewSection();
    void createManipulationSection();
    void createShapeSection();
    void createEditSection();
    void createAnalysisSection();
    void createHistorySection();
    
    QToolButton* createDropdownButton(const QIcon& icon, QMenu* menu);
    
    // Tool buttons for state tracking
    QAction* selectAction_ = nullptr;
    QAction* moveAction_ = nullptr;
    QAction* rotateAction_ = nullptr;
    QAction* scaleAction_ = nullptr;
    
    QAction* measureAction_ = nullptr;
    QAction* clippingAction_ = nullptr;
};

} // namespace geantcad
