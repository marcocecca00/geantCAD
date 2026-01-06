#include "MainWindow.hh"
#include "PropertiesPanel.hh"
#include "SimulationConfigPanel.hh"
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QFile>
#include <QApplication>
#include <QSettings>
#include <QByteArray>
#include "../../core/include/Shape.hh"
#include "../../core/include/Material.hh"
#include "../../core/include/Serialization.hh"
#include "../../core/include/Command.hh"
#include "../../generator/include/GDMLExporter.hh"
#include "../../generator/include/Geant4ProjectGenerator.hh"
#include "BuildRunDialog.hh"
#include "CameraControlWidget.hh"
#include <QDir>
#include <QGroupBox>
using namespace geantcad;

namespace geantcad {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , sceneGraph_(new SceneGraph())
    , commandStack_(new CommandStack())
{
    applyStylesheet();
    setupUI(); // Must be called before loadPreferences() so viewport_ exists
    setupMenus();
    setupToolbars();
    setupStatusBar();
    connectSignals();
    loadPreferences(); // Load preferences after UI is set up (but after menus to avoid fullscreen issues)
}

MainWindow::~MainWindow() {
    savePreferences();
}

void MainWindow::setupUI() {
    // Create central widget with splitter (reorganized layout)
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(2);
    
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(mainSplitter_);
    
    // Left: Scene Hierarchy (Outliner only)
    QGroupBox* sceneGroup = new QGroupBox("Scene", this);
    QVBoxLayout* sceneLayout = new QVBoxLayout(sceneGroup);
    sceneLayout->setContentsMargins(2, 2, 2, 2);
    
    outliner_ = new Outliner(this);
    sceneLayout->addWidget(outliner_);
    outliner_->setSceneGraph(sceneGraph_);
    
    sceneGroup->setMinimumWidth(200);
    sceneGroup->setMaximumWidth(300);
    mainSplitter_->addWidget(sceneGroup);
    
    // Center: Viewport 3D (takes most space)
    // Create container widget for viewport + camera control overlay
    QWidget* viewportContainer = new QWidget(this);
    QVBoxLayout* viewportLayout = new QVBoxLayout(viewportContainer);
    viewportLayout->setContentsMargins(0, 0, 0, 0);
    viewportLayout->setSpacing(0);
    
    viewport_ = new Viewport3D(viewportContainer);
    viewport_->setSceneGraph(sceneGraph_);
    viewport_->setMinimumSize(400, 300);
    viewport_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    viewportLayout->addWidget(viewport_);
    
    // Add camera control widget as overlay (absolute positioning)
    cameraControlWidget_ = new CameraControlWidget(viewportContainer);
    cameraControlWidget_->setRenderer(viewport_->getRenderer());
    cameraControlWidget_->setCamera(viewport_->getCamera());
    cameraControlWidget_->setFixedSize(110, 120);
    cameraControlWidget_->setAttribute(Qt::WA_TranslucentBackground, false);
    cameraControlWidget_->setStyleSheet(
        "QWidget { background-color: rgba(30, 30, 30, 220); border: 1px solid #505050; border-radius: 4px; }"
        "QLabel { color: #e0e0e0; }"
        "QPushButton { background-color: rgba(43, 43, 43, 220); border: 1px solid #404040; border-radius: 3px; color: #e0e0e0; font-size: 14pt; min-width: 30px; min-height: 30px; }"
        "QPushButton:hover { background-color: rgba(58, 58, 58, 240); }"
        "QPushButton:pressed { background-color: rgba(0, 120, 212, 240); }"
        "QPushButton:disabled { background-color: rgba(26, 26, 26, 200); color: #606060; }"
    );
    cameraControlWidget_->raise(); // Bring to front
    cameraControlWidget_->show();
    
    // Connect camera control signals
    connect(cameraControlWidget_, &CameraControlWidget::viewChanged, this, [this]() {
        viewport_->refresh();
    });
    
    // Use event filter to keep camera widget positioned correctly
    viewportContainer->installEventFilter(this);
    
    mainSplitter_->addWidget(viewportContainer);
    
    // Right: Split verticale - Properties (top) + Simulation Config (bottom)
    rightSplitter_ = new QSplitter(Qt::Vertical, this);
    rightSplitter_->setMinimumWidth(300);
    rightSplitter_->setMaximumWidth(450);
    
    // Top: Properties Panel (Inspector with all object properties)
    propertiesPanel_ = new PropertiesPanel(this);
    propertiesPanel_->setSceneGraph(sceneGraph_);
    propertiesPanel_->setCommandStack(commandStack_);
    inspector_ = propertiesPanel_->getInspector(); // Keep reference for compatibility
    rightSplitter_->addWidget(propertiesPanel_);
    
    // Bottom: Simulation Config Panel (Physics, Source, Output, Build & Run)
    simulationPanel_ = new SimulationConfigPanel(this);
    physicsPanel_ = simulationPanel_->getPhysicsPanel();
    outputPanel_ = simulationPanel_->getOutputPanel();
    particleGunPanel_ = simulationPanel_->getParticleGunPanel();
    rightSplitter_->addWidget(simulationPanel_);
    
    // Set splitter sizes: Properties 60%, Simulation 40%
    rightSplitter_->setStretchFactor(0, 60);
    rightSplitter_->setStretchFactor(1, 40);
    rightSplitter_->setSizes({400, 300});
    
    mainSplitter_->addWidget(rightSplitter_);
    
    // Set stretch factors: Left 20%, Viewport 50%, Right 30%
    mainSplitter_->setStretchFactor(0, 20);
    mainSplitter_->setStretchFactor(1, 50);
    mainSplitter_->setStretchFactor(2, 30);
    
    // Set initial sizes
    mainSplitter_->setSizes({200, 800, 350});
    
    // Add camera control widget to viewport (overlay in corner)
    cameraControlWidget_ = new CameraControlWidget(viewport_);
    cameraControlWidget_->setRenderer(viewport_->getRenderer());
    cameraControlWidget_->setCamera(viewport_->getCamera());
    cameraControlWidget_->setFixedSize(110, 120);
    cameraControlWidget_->move(10, 10); // Top-left corner
    cameraControlWidget_->setStyleSheet(
        "QWidget { background-color: rgba(30, 30, 30, 200); border-radius: 4px; }"
        "QPushButton { background-color: rgba(43, 43, 43, 200); border: 1px solid #404040; border-radius: 3px; color: #e0e0e0; font-size: 14pt; }"
        "QPushButton:hover { background-color: rgba(58, 58, 58, 200); }"
        "QPushButton:pressed { background-color: rgba(0, 120, 212, 200); }"
        "QPushButton:disabled { background-color: rgba(26, 26, 26, 200); color: #606060; }"
    );
    cameraControlWidget_->show();
    
    // Connect camera control signals
    connect(cameraControlWidget_, &CameraControlWidget::viewChanged, this, [this]() {
        viewport_->refresh();
    });
}

void MainWindow::setupMenus() {
    QMenuBar* menuBar = this->menuBar();
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("&File");
    QAction* newAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "&New", this, [this]() { onNew(); }, QKeySequence::New);
    QAction* openAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), "&Open...", this, [this]() { onOpen(); }, QKeySequence::Open);
    QAction* saveAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "&Save", this, [this]() { onSave(); }, QKeySequence::Save);
    QAction* saveAsAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Save &As...", this, [this]() { onSaveAs(); }, QKeySequence::SaveAs);
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogCloseButton), "E&xit", this, &QWidget::close, QKeySequence::Quit);
    
    // Insert menu (for shapes/primitives)
    QMenu* insertMenu = menuBar->addMenu("&Insert");
    QMenu* shapeMenu = insertMenu->addMenu("&Shape");
    
    // Box shape
    QAction* insertBoxAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "&Box", this, [this]() {
        auto boxShape = makeBox(50.0, 50.0, 50.0);
        auto material = Material::makeAir();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Box", std::move(boxShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Box", 2000);
    });
    insertBoxAction->setShortcut(QKeySequence("Ctrl+Shift+B"));
    
    // Tube shape
    QAction* insertTubeAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_DirIcon), "&Tube", this, [this]() {
        auto tubeShape = makeTube(0.0, 30.0, 50.0);
        auto material = Material::makeWater();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Tube", std::move(tubeShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Tube", 2000);
    });
    insertTubeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    
    // Sphere shape
    QAction* insertSphereAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_ComputerIcon), "&Sphere", this, [this]() {
        auto sphereShape = makeSphere(0.0, 40.0);
        auto material = Material::makeAir();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Sphere", std::move(sphereShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Sphere", 2000);
    });
    insertSphereAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    
    // Cone shape
    QAction* insertConeAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_DriveCDIcon), "&Cone", this, [this]() {
        auto coneShape = makeCone(0.0, 20.0, 0.0, 40.0, 50.0);
        auto material = Material::makeLead();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Cone", std::move(coneShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Cone", 2000);
    });
    insertConeAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    
    // Trd shape
    QAction* insertTrdAction = shapeMenu->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "&Trd", this, [this]() {
        auto trdShape = makeTrd(30.0, 20.0, 30.0, 20.0, 50.0);
        auto material = Material::makeSilicon();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Trd", std::move(trdShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Trd", 2000);
    });
    insertTrdAction->setShortcut(QKeySequence("Ctrl+Shift+D"));
    
    // View menu
    QMenu* viewMenu = menuBar->addMenu("&View");
    QAction* frameAction = viewMenu->addAction(style()->standardIcon(QStyle::SP_FileDialogListView), "Frame &Selection", this, [this]() { 
        viewport_->frameSelection();
        statusBar_->showMessage("Framed selection", 1000);
    }, QKeySequence("F"));
    QAction* resetViewAction = viewMenu->addAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), "&Reset View", this, [this]() { 
        viewport_->resetView();
        statusBar_->showMessage("View reset", 1000);
    }, QKeySequence("Home"));
    viewMenu->addSeparator();
    QAction* toggleGridAction = viewMenu->addAction("Toggle &Grid", this, [this]() { 
        bool visible = viewport_->isGridVisible();
        viewport_->setGridVisible(!visible);
        statusBar_->showMessage(visible ? "Grid hidden" : "Grid shown", 1000);
    }, QKeySequence("Ctrl+G"));
    toggleGridAction->setCheckable(true);
    toggleGridAction->setChecked(viewport_->isGridVisible());
    
    QAction* toggleSnapAction = viewMenu->addAction("Snap to &Grid", this, [this]() { 
        bool enabled = viewport_->isSnapToGrid();
        viewport_->setSnapToGrid(!enabled);
        statusBar_->showMessage(enabled ? "Snap to grid disabled" : "Snap to grid enabled", 1000);
    });
    toggleSnapAction->setCheckable(true);
    toggleSnapAction->setChecked(viewport_->isSnapToGrid());
    
    viewMenu->addSeparator();
    
    // Grid spacing submenu
    QMenu* gridSpacingMenu = viewMenu->addMenu("Grid &Spacing");
    QActionGroup* spacingGroup = new QActionGroup(this);
    spacingGroup->setExclusive(true);
    
    QList<QPair<double, QString>> spacingOptions = {
        {10.0, "10 mm"},
        {25.0, "25 mm"},
        {50.0, "50 mm"},
        {100.0, "100 mm"},
        {250.0, "250 mm"},
        {500.0, "500 mm"}
    };
    
    for (const auto& option : spacingOptions) {
        QAction* spacingAction = gridSpacingMenu->addAction(option.second, this, [this, option]() {
            viewport_->setGridSpacing(option.first);
            statusBar_->showMessage(QString("Grid spacing set to %1").arg(option.second), 1000);
        });
        spacingAction->setCheckable(true);
        spacingGroup->addAction(spacingAction);
        
        // Check current spacing
        if (std::abs(viewport_->getGridSpacing() - option.first) < 0.1) {
            spacingAction->setChecked(true);
        }
    }
    
    // Generate menu
    QMenu* generateMenu = menuBar->addMenu("&Generate");
    QAction* genAction = generateMenu->addAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "&Generate Geant4 Project...", this, [this]() { onGenerate(); });
    QAction* buildAction = generateMenu->addAction(style()->standardIcon(QStyle::SP_MediaPlay), "&Build & Run", this, [this]() { onBuildRun(); });
}

void MainWindow::setupToolbars() {
    toolbar_ = new Toolbar(this);
    addToolBar(Qt::TopToolBarArea, toolbar_);
}

void MainWindow::setupStatusBar() {
    statusBar_ = statusBar();
    statusBar_->showMessage("Ready");
}

void MainWindow::connectSignals() {
    // Shape creation moved to Insert -> Shape menu
    // Toolbar now only handles manipulation tools
    
    connect(toolbar_, &Toolbar::toolSelect, this, [this]() {
        statusBar_->showMessage("Select tool", 1000);
    });
    
    connect(toolbar_, &Toolbar::toolMove, this, [this]() {
        statusBar_->showMessage("Move tool", 1000);
    });
    
    connect(toolbar_, &Toolbar::toolRotate, this, [this]() {
        statusBar_->showMessage("Rotate tool", 1000);
    });
    
    connect(toolbar_, &Toolbar::deleteSelected, this, [this]() {
        VolumeNode* selected = sceneGraph_->getSelected();
        if (selected && selected != sceneGraph_->getRoot()) {
            auto cmd = std::make_unique<DeleteVolumeCommand>(sceneGraph_, selected);
            commandStack_->execute(std::move(cmd));
            
            outliner_->refresh();
            viewport_->refresh();
            inspector_->clear();
            statusBar_->showMessage("Deleted volume", 2000);
        } else {
            statusBar_->showMessage("No volume selected", 2000);
        }
    });
    
    connect(toolbar_, &Toolbar::duplicateSelected, this, [this]() {
        VolumeNode* selected = sceneGraph_->getSelected();
        if (selected && selected != sceneGraph_->getRoot()) {
            auto cmd = std::make_unique<DuplicateVolumeCommand>(sceneGraph_, selected);
            commandStack_->execute(std::move(cmd));
            
            VolumeNode* duplicated = cmd->getDuplicatedNode();
            if (duplicated) {
                // Move duplicated node slightly for visibility
                auto& t = duplicated->getTransform();
                auto pos = t.getTranslation();
                t.setTranslation(QVector3D(pos.x() + 20, pos.y() + 20, pos.z()));
            }
            
            outliner_->refresh();
            viewport_->refresh();
            statusBar_->showMessage("Duplicated volume", 2000);
        } else {
            statusBar_->showMessage("No volume selected", 2000);
        }
    });
    
    // Viewport selection
    connect(viewport_, &Viewport3D::selectionChanged, this, [this](VolumeNode* node) {
        sceneGraph_->setSelected(node);
        inspector_->setNode(node);
        outliner_->refresh();
    });
    
    // Set command stack for viewport (for undo/redo of transforms)
    viewport_->setCommandStack(commandStack_);
    
    // Toolbar tool changes -> update viewport mode
    connect(toolbar_, &Toolbar::toolSelect, this, [this]() {
        viewport_->setInteractionMode(Viewport3D::InteractionMode::Select);
    });
    connect(toolbar_, &Toolbar::toolMove, this, [this]() {
        viewport_->setInteractionMode(Viewport3D::InteractionMode::Move);
    });
    connect(toolbar_, &Toolbar::toolRotate, this, [this]() {
        viewport_->setInteractionMode(Viewport3D::InteractionMode::Rotate);
    });
    connect(toolbar_, &Toolbar::toolScale, this, [this]() {
        viewport_->setInteractionMode(Viewport3D::InteractionMode::Scale);
    });
    
    // Viewport mode changes -> update toolbar
    connect(viewport_, &Viewport3D::viewChanged, this, [this]() {
        // Refresh viewport after transform
        viewport_->refresh();
        outliner_->refresh();
    });
    
    // Project Manager Panel selection
    connect(outliner_, &Outliner::nodeSelected, this, [this](VolumeNode* node) {
        sceneGraph_->setSelected(node);
        inspector_->setNode(node);
        viewport_->refresh();
    });
    
    // Inspector changes
    connect(inspector_, &Inspector::nodeChanged, this, [this](VolumeNode* node) {
        viewport_->refresh();
        outliner_->refresh();
    });
    
    // Right Panel Container signals
    connect(simulationPanel_, &SimulationConfigPanel::physicsConfigChanged, this, [this]() {
        sceneGraph_->getPhysicsConfig() = physicsPanel_->getConfig();
    });
    
    connect(simulationPanel_, &SimulationConfigPanel::outputConfigChanged, this, [this]() {
        sceneGraph_->getOutputConfig() = outputPanel_->getConfig();
    });
    
    connect(simulationPanel_, &SimulationConfigPanel::particleGunConfigChanged, this, [this]() {
        sceneGraph_->getParticleGunConfig() = particleGunPanel_->getConfig();
    });
}

void MainWindow::onNew() {
    // Reset scene - create new SceneGraph
    sceneGraph_ = new SceneGraph();
    commandStack_->clear();
    
    viewport_->setSceneGraph(sceneGraph_);
    outliner_->setSceneGraph(sceneGraph_);
    inspector_->clear();
    physicsPanel_->setConfig(sceneGraph_->getPhysicsConfig());
    outputPanel_->setConfig(sceneGraph_->getOutputConfig());
    
    currentFilePath_.clear();
    viewport_->refresh();
    outliner_->refresh();
    statusBar_->showMessage("New project", 2000);
}

void MainWindow::onOpen() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open GeantCAD Project", "", "GeantCAD Files (*.geantcad);;All Files (*)");
    if (!fileName.isEmpty()) {
        if (loadSceneFromFile(sceneGraph_, fileName.toStdString())) {
            currentFilePath_ = fileName;
            
            viewport_->setSceneGraph(sceneGraph_);
            outliner_->setSceneGraph(sceneGraph_);
            inspector_->clear();
            physicsPanel_->setConfig(sceneGraph_->getPhysicsConfig());
            outputPanel_->setConfig(sceneGraph_->getOutputConfig());
            
            viewport_->refresh();
            outliner_->refresh();
            
            statusBar_->showMessage("Opened: " + fileName, 2000);
        } else {
            QMessageBox::critical(this, "Error", "Failed to open file: " + fileName);
            statusBar_->showMessage("Failed to open file", 3000);
        }
    }
}

void MainWindow::onSave() {
    if (currentFilePath_.isEmpty()) {
        onSaveAs();
    } else {
        if (saveSceneToFile(sceneGraph_, currentFilePath_.toStdString())) {
            statusBar_->showMessage("Saved: " + currentFilePath_, 2000);
        } else {
            QMessageBox::critical(this, "Error", "Failed to save file: " + currentFilePath_);
            statusBar_->showMessage("Failed to save file", 3000);
        }
    }
}

void MainWindow::onSaveAs() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save GeantCAD Project", "", "GeantCAD Files (*.geantcad);;All Files (*)");
    if (!fileName.isEmpty()) {
        // Ensure .geantcad extension
        if (!fileName.endsWith(".geantcad", Qt::CaseInsensitive)) {
            fileName += ".geantcad";
        }
        
        if (saveSceneToFile(sceneGraph_, fileName.toStdString())) {
            currentFilePath_ = fileName;
            statusBar_->showMessage("Saved: " + fileName, 2000);
        } else {
            QMessageBox::critical(this, "Error", "Failed to save file: " + fileName);
            statusBar_->showMessage("Failed to save file", 3000);
        }
    }
}

void MainWindow::onGenerate() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Output Directory for Geant4 Project", "");
    if (!dirPath.isEmpty()) {
        Geant4ProjectGenerator generator;
        
        // Set template directory relative to executable or source
        QString appDir = QDir::currentPath();
        QString templateDir = appDir + "/templates/geant4_project";
        if (!QDir(templateDir).exists()) {
            // Try relative to source
            templateDir = appDir + "/../templates/geant4_project";
            if (!QDir(templateDir).exists()) {
                templateDir = appDir + "/../../templates/geant4_project";
            }
        }
        generator.setTemplateDir(templateDir.toStdString());
        
        if (generator.generateProject(sceneGraph_, dirPath.toStdString())) {
            statusBar_->showMessage("Generated Geant4 project: " + dirPath, 3000);
            QMessageBox::information(this, "Success", 
                "Geant4 project generated successfully in:\n" + dirPath + 
                "\n\nYou can now build it with:\n"
                "  cd " + dirPath + "\n"
                "  mkdir build && cd build\n"
                "  cmake ..\n"
                "  make -j$(nproc)");
        } else {
            statusBar_->showMessage("Failed to generate Geant4 project", 3000);
            QMessageBox::critical(this, "Error", "Failed to generate Geant4 project in:\n" + dirPath);
        }
    }
}

void MainWindow::onBuildRun() {
    // Get last generated project directory if available
    QString lastDir = ""; // TODO: Store last generated directory
    
    BuildRunDialog dialog(this);
    if (!lastDir.isEmpty()) {
        dialog.setProjectDirectory(lastDir);
    }
    dialog.exec();
}

void MainWindow::applyStylesheet() {
    // Try to load from resources first
    QFile styleFile(":/stylesheets/modern-dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(style);
        return;
    }
    
    // Fallback: try to load from file system (relative to executable or source)
    QStringList paths = {
        "app/resources/stylesheets/modern-dark.qss",
        "../app/resources/stylesheets/modern-dark.qss",
        "../../app/resources/stylesheets/modern-dark.qss",
        QApplication::applicationDirPath() + "/../app/resources/stylesheets/modern-dark.qss"
    };
    
    for (const QString& path : paths) {
        QFile fallbackFile(path);
        if (fallbackFile.open(QFile::ReadOnly)) {
            QString style = QLatin1String(fallbackFile.readAll());
            qApp->setStyleSheet(style);
            return;
        }
    }
    
    // If all else fails, use inline minimal style
    qApp->setStyleSheet(
        "QMainWindow { background-color: #1e1e1e; color: #e0e0e0; }"
        "QPushButton { background-color: #2b2b2b; border: 1px solid #404040; border-radius: 4px; padding: 6px 12px; color: #e0e0e0; }"
        "QPushButton:hover { background-color: #3a3a3a; }"
    );
}

void MainWindow::loadPreferences() {
    QSettings settings("GeantCAD", "GeantCAD");
    
    // Restore window geometry and state
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    // Restore splitter sizes
    if (mainSplitter_) {
        QList<int> sizes = settings.value("mainSplitterSizes").value<QList<int>>();
        if (!sizes.isEmpty()) {
            mainSplitter_->setSizes(sizes);
        }
    }
    
    // Restore grid settings
    if (viewport_) {
        bool gridVisible = settings.value("gridVisible", true).toBool();
        double gridSpacing = settings.value("gridSpacing", 50.0).toDouble();
        viewport_->setGridVisible(gridVisible);
        viewport_->setGridSpacing(gridSpacing);
    }
}

void MainWindow::savePreferences() {
    QSettings settings("GeantCAD", "GeantCAD");
    
    // Save window geometry and state
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    
    // Save splitter sizes
    if (mainSplitter_) {
        settings.setValue("mainSplitterSizes", QVariant::fromValue(mainSplitter_->sizes()));
    }
    
    // Save grid settings
    if (viewport_) {
        settings.setValue("gridVisible", viewport_->isGridVisible());
        settings.setValue("gridSpacing", viewport_->getGridSpacing());
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    // Keep camera control widget positioned in top-left corner of viewport container
    if (obj == viewport_->parent() && event->type() == QEvent::Resize) {
        if (cameraControlWidget_) {
            QWidget* container = qobject_cast<QWidget*>(obj);
            if (container) {
                cameraControlWidget_->move(10, 10);
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

} // namespace geantcad
