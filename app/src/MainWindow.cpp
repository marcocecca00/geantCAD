#include "MainWindow.hh"
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QFile>
#include <QApplication>
#include "../../core/include/Shape.hh"
#include "../../core/include/Material.hh"
#include "../../core/include/Serialization.hh"
#include "../../core/include/Command.hh"
#include "../../generator/include/GDMLExporter.hh"
#include "../../generator/include/Geant4ProjectGenerator.hh"
#include "BuildRunDialog.hh"
#include <QDir>

namespace geantcad {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , sceneGraph_(new SceneGraph())
    , commandStack_(new CommandStack())
{
    applyStylesheet();
    setupUI();
    setupMenus();
    setupToolbars();
    setupStatusBar();
    connectSignals();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // Create central widget with splitter
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);
    
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(mainSplitter_);
    
    // Left: Viewport (takes most space, minimum 400px width)
    viewport_ = new Viewport3D(this);
    viewport_->setSceneGraph(sceneGraph_);
    viewport_->setMinimumSize(400, 300);
    viewport_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainSplitter_->addWidget(viewport_);
    
    // Right: Outliner + Inspector + PhysicsPanel (minimum 250px width)
    rightSplitter_ = new QSplitter(Qt::Vertical, this);
    rightSplitter_->setMinimumWidth(250);
    
    outliner_ = new Outliner(this);
    outliner_->setSceneGraph(sceneGraph_);
    outliner_->setMinimumHeight(150);
    outliner_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightSplitter_->addWidget(outliner_);
    
    inspector_ = new Inspector(this);
    inspector_->setMinimumHeight(200);
    inspector_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    inspector_->setCommandStack(commandStack_);
    rightSplitter_->addWidget(inspector_);
    
    physicsPanel_ = new PhysicsPanel(this);
    physicsPanel_->hide(); // Hidden, accessible via menu/dialog
    
    outputPanel_ = new OutputPanel(this);
    outputPanel_->hide(); // Hidden, accessible via menu/dialog
    
    particleGunPanel_ = new ParticleGunPanel(this);
    particleGunPanel_->hide(); // Hidden, accessible via menu/dialog
    
    // Set stretch factors: outliner gets less space, inspector gets more
    rightSplitter_->setStretchFactor(0, 2);
    rightSplitter_->setStretchFactor(1, 3);
    
    mainSplitter_->addWidget(rightSplitter_);
    
    // Set stretch factors: viewport gets 70%, right panel gets 30%
    mainSplitter_->setStretchFactor(0, 7);
    mainSplitter_->setStretchFactor(1, 3);
    
    // Set initial sizes: viewport gets 70%, right panel gets 30%
    mainSplitter_->setSizes({700, 300});
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
    // Toolbar signals - create primitives (with undo/redo)
    connect(toolbar_, &Toolbar::createBox, this, [this]() {
        auto boxShape = makeBox(50.0, 50.0, 50.0); // 100x100x100 mm (half-lengths)
        auto material = Material::makeAir();
        
        auto cmd = std::make_unique<CreateVolumeCommand>(
            sceneGraph_, "Box", std::move(boxShape), material);
        
        commandStack_->execute(std::move(cmd));
        
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Box", 2000);
    });
    
    connect(toolbar_, &Toolbar::createTube, this, [this]() {
        VolumeNode* node = sceneGraph_->createVolume("Tube");
        auto tubeShape = makeTube(0.0, 30.0, 50.0); // rmax=30mm, dz=50mm
        node->setShape(std::move(tubeShape));
        node->setMaterial(Material::makeWater());
        
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Tube", 2000);
    });
    
    connect(toolbar_, &Toolbar::createSphere, this, [this]() {
        VolumeNode* node = sceneGraph_->createVolume("Sphere");
        auto sphereShape = makeSphere(0.0, 40.0); // rmax=40mm
        node->setShape(std::move(sphereShape));
        node->setMaterial(Material::makeAir());
        
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Sphere", 2000);
    });
    
    connect(toolbar_, &Toolbar::createCone, this, [this]() {
        VolumeNode* node = sceneGraph_->createVolume("Cone");
        auto coneShape = makeCone(0.0, 20.0, 0.0, 40.0, 50.0);
        node->setShape(std::move(coneShape));
        node->setMaterial(Material::makeLead());
        
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Cone", 2000);
    });
    
    connect(toolbar_, &Toolbar::createTrd, this, [this]() {
        VolumeNode* node = sceneGraph_->createVolume("Trd");
        auto trdShape = makeTrd(30.0, 20.0, 30.0, 20.0, 50.0);
        node->setShape(std::move(trdShape));
        node->setMaterial(Material::makeSilicon());
        
        outliner_->refresh();
        viewport_->refresh();
        statusBar_->showMessage("Created Trd", 2000);
    });
    
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
    
    // Outliner selection
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
    
    // Physics panel changes
    connect(physicsPanel_, &PhysicsPanel::configChanged, this, [this]() {
        sceneGraph_->getPhysicsConfig() = physicsPanel_->getConfig();
    });
    
    // Output panel changes
    connect(outputPanel_, &OutputPanel::configChanged, this, [this]() {
        sceneGraph_->getOutputConfig() = outputPanel_->getConfig();
    });
    
    connect(particleGunPanel_, &ParticleGunPanel::configChanged, this, [this]() {
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

} // namespace geantcad
