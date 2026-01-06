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

    // UI Components
    QSplitter* mainSplitter_;
    QSplitter* rightSplitter_;
    
    Viewport3D* viewport_;
    Outliner* outliner_;
    Inspector* inspector_;
    Toolbar* toolbar_;
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

