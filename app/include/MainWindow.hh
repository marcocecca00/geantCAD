#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStatusBar>
#include <QDockWidget>
#include "Viewport3D.hh"
#include "Outliner.hh"
#include "Inspector.hh"
#include "Toolbar.hh"
#include "PhysicsPanel.hh"
#include "OutputPanel.hh"
#include "ParticleGunPanel.hh"
#include "BuildRunDialog.hh"
#include "PropertiesPanel.hh"
#include "SimulationConfigPanel.hh"
#include "ViewCube.hh"
#include "ClippingPlaneWidget.hh"
#include "HistoryPanel.hh"
#include "MeasurementTool.hh"
#include "../../core/include/SceneGraph.hh"
#include "../../core/include/CommandStack.hh"

namespace geantcad {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onGenerate();
    void onBuildRun();
    
    // View actions
    void onViewFront();
    void onViewBack();
    void onViewLeft();
    void onViewRight();
    void onViewTop();
    void onViewBottom();
    void onViewIsometric();
    
    // Analysis tools
    void onToggleClippingPlanes();
    void onToggleMeasureTool();
    
    // History
    void onUndo();
    void onRedo();

private:
    void setupUI();
    void setupMenus();
    void setupToolbars();
    void setupStatusBar();
    void setupDockWidgets();
    void connectSignals();
    void applyStylesheet();
    void loadPreferences();
    void savePreferences();
    void updateViewCubePosition();
    bool eventFilter(QObject* obj, QEvent* event) override;

    // UI Components
    QSplitter* mainSplitter_;
    QSplitter* rightSplitter_; // Vertical splitter for Properties + Simulation
    
    Viewport3D* viewport_;
    Outliner* outliner_; // Left: Scene hierarchy only
    PropertiesPanel* propertiesPanel_; // Right top: Object properties
    SimulationConfigPanel* simulationPanel_; // Right bottom: Simulation config
    Toolbar* toolbar_;
    
    // New integrated widgets
    ViewCube* viewCube_;                      // ViewCube overlay
    ClippingPlaneWidget* clippingWidget_;     // Clipping planes
    HistoryPanel* historyPanel_;              // History dock
    MeasurementTool* measurementTool_;        // Measurement tool
    
    // Dock widgets
    QDockWidget* historyDock_;
    QDockWidget* clippingDock_;
    QDockWidget* measureDock_;
    
    // Keep references for backward compatibility
    Inspector* inspector_;
    PhysicsPanel* physicsPanel_;
    OutputPanel* outputPanel_;
    ParticleGunPanel* particleGunPanel_;
    
    QStatusBar* statusBar_;
    
    // Core
    SceneGraph* sceneGraph_;
    CommandStack* commandStack_;
    
    QString currentFilePath_;
};

} // namespace geantcad

