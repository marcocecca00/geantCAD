#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStatusBar>
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
#include "CameraControlWidget.hh"
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

private:
    void setupUI();
    void setupMenus();
    void setupToolbars();
    void setupStatusBar();
    void connectSignals();
    void applyStylesheet();
    void loadPreferences();
    void savePreferences();
    bool eventFilter(QObject* obj, QEvent* event) override;

    // UI Components
    QSplitter* mainSplitter_;
    QSplitter* rightSplitter_; // Vertical splitter for Properties + Simulation
    
    Viewport3D* viewport_;
    Outliner* outliner_; // Left: Scene hierarchy only
    PropertiesPanel* propertiesPanel_; // Right top: Object properties
    SimulationConfigPanel* simulationPanel_; // Right bottom: Simulation config
    CameraControlWidget* cameraControlWidget_; // Camera control overlay widget
    Toolbar* toolbar_;
    
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

