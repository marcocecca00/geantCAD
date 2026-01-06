#include "MainWindow.hh"
#include "PropertiesPanel.hh"
#include "SimulationConfigPanel.hh"
#include "ViewCube.hh"
#include "ClippingPlaneWidget.hh"
#include "HistoryPanel.hh"
#include "MeasurementTool.hh"
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QActionGroup>
#include <QFile>
#include <QApplication>
#include <QSettings>
#include <QByteArray>
#include <QResizeEvent>

#ifndef GEANTCAD_NO_VTK
#include <vtkSmartPointer.h>
#include <vtkPlane.h>
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkMapper.h>
#endif
#include "../../core/include/Shape.hh"
#include "../../core/include/Material.hh"
#include "../../core/include/Serialization.hh"
#include "../../core/include/Command.hh"
#include "../../generator/include/GDMLExporter.hh"
#include "../../generator/include/Geant4ProjectGenerator.hh"
#include "BuildRunDialog.hh"
#include <QDir>
#include <QGroupBox>
using namespace geantcad;

namespace geantcad {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , sceneGraph_(new SceneGraph())
    , commandStack_(new CommandStack())
    , viewCube_(nullptr)
    , clippingWidget_(nullptr)
    , historyPanel_(nullptr)
    , measurementTool_(nullptr)
    , historyDock_(nullptr)
    , clippingDock_(nullptr)
    , measureDock_(nullptr)
{
    applyStylesheet();
    setupUI(); // Must be called before loadPreferences() so viewport_ exists
    setupMenus();
    setupToolbars();
    setupDockWidgets();
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
    
    // Add ViewCube overlay in top-right corner
    viewCube_ = new ViewCube(viewport_);
    viewCube_->setFixedSize(100, 100);
    viewCube_->setRenderer(viewport_->getRenderer());
    viewCube_->setCamera(viewport_->getCamera());
    viewCube_->show();
    viewCube_->raise();
    
    // Connect ViewCube signals
    connect(viewCube_, &ViewCube::viewChanged, this, [this]() {
        viewport_->refresh();
    });
    
    connect(viewCube_, &ViewCube::viewOrientationRequested, this, [this](ViewCube::ViewOrientation orientation) {
        switch (orientation) {
            case ViewCube::ViewOrientation::Front:
                viewport_->setStandardView(Viewport3D::StandardView::Front);
                break;
            case ViewCube::ViewOrientation::Back:
                viewport_->setStandardView(Viewport3D::StandardView::Back);
                break;
            case ViewCube::ViewOrientation::Left:
                viewport_->setStandardView(Viewport3D::StandardView::Left);
                break;
            case ViewCube::ViewOrientation::Right:
                viewport_->setStandardView(Viewport3D::StandardView::Right);
                break;
            case ViewCube::ViewOrientation::Top:
                viewport_->setStandardView(Viewport3D::StandardView::Top);
                break;
            case ViewCube::ViewOrientation::Bottom:
                viewport_->setStandardView(Viewport3D::StandardView::Bottom);
                break;
            default:
                viewport_->setStandardView(Viewport3D::StandardView::Isometric);
                break;
        }
        viewCube_->updateFromCamera();
        statusBar_->showMessage("View changed", 1000);
    });
    
    // Install event filter on viewport for resize handling
    viewport_->installEventFilter(this);
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
    
    // Panels submenu
    QMenu* panelsMenu = viewMenu->addMenu("&Panels");
    
    QAction* historyPanelAction = panelsMenu->addAction("&History Panel", this, [this]() {
        if (historyDock_) {
            historyDock_->setVisible(!historyDock_->isVisible());
        }
    });
    historyPanelAction->setCheckable(true);
    
    QAction* clippingPanelAction = panelsMenu->addAction("&Clipping Planes", this, [this]() {
        if (clippingDock_) {
            clippingDock_->setVisible(!clippingDock_->isVisible());
        }
    });
    clippingPanelAction->setCheckable(true);
    
    QAction* measurePanelAction = panelsMenu->addAction("&Measurement Tool", this, [this]() {
        if (measureDock_) {
            measureDock_->setVisible(!measureDock_->isVisible());
        }
    });
    measurePanelAction->setCheckable(true);
    
    // Update panel checkmarks when menu is about to show
    connect(viewMenu, &QMenu::aboutToShow, this, [=]() {
        historyPanelAction->setChecked(historyDock_ && historyDock_->isVisible());
        clippingPanelAction->setChecked(clippingDock_ && clippingDock_->isVisible());
        measurePanelAction->setChecked(measureDock_ && measureDock_->isVisible());
    });
    
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

void MainWindow::setupDockWidgets() {
    // === History Panel (dock on the right) ===
    historyDock_ = new QDockWidget("History", this);
    historyDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    historyDock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    
    historyPanel_ = new HistoryPanel(this);
    historyPanel_->setCommandStack(commandStack_);
    historyDock_->setWidget(historyPanel_);
    historyDock_->setMinimumWidth(200);
    historyDock_->hide(); // Hidden by default, shown via menu/toolbar
    
    addDockWidget(Qt::RightDockWidgetArea, historyDock_);
    
    // === Clipping Planes Panel (dock on the right) ===
    clippingDock_ = new QDockWidget("Clipping Planes", this);
    clippingDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    clippingDock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    
    clippingWidget_ = new ClippingPlaneWidget(this);
    clippingWidget_->setRenderer(viewport_->getRenderer());
    clippingDock_->setWidget(clippingWidget_);
    clippingDock_->setMinimumWidth(250);
    clippingDock_->hide(); // Hidden by default
    
    addDockWidget(Qt::RightDockWidgetArea, clippingDock_);
    
    // === Measurement Tool Panel (dock on the right) ===
    measureDock_ = new QDockWidget("Measurement", this);
    measureDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    measureDock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    
    measurementTool_ = new MeasurementTool(this);
    measurementTool_->setRenderer(viewport_->getRenderer());
    measureDock_->setWidget(measurementTool_);
    measureDock_->setMinimumWidth(250);
    measureDock_->hide(); // Hidden by default
    
    addDockWidget(Qt::RightDockWidgetArea, measureDock_);
    
    // Connect clipping widget to viewport refresh
    connect(clippingWidget_, &ClippingPlaneWidget::planeChanged, this, [this]() {
        // Apply clipping planes to all actors in renderer
#ifndef GEANTCAD_NO_VTK
        if (viewport_ && viewport_->getRenderer()) {
            auto renderer = viewport_->getRenderer();
            auto actors = renderer->GetActors();
            actors->InitTraversal();
            
            // Clear existing clipping planes
            vtkActor* actor;
            while ((actor = actors->GetNextActor()) != nullptr) {
                actor->GetMapper()->RemoveAllClippingPlanes();
            }
            
            // Apply active clipping planes
            auto applyPlane = [&](ClippingPlaneWidget::PlaneAxis axis) {
                if (clippingWidget_->isPlaneEnabled(axis)) {
                    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
                    double pos = clippingWidget_->getPlanePosition(axis);
                    bool flipped = clippingWidget_->isFlipped(axis);
                    double normal = flipped ? -1.0 : 1.0;
                    
                    switch (axis) {
                        case ClippingPlaneWidget::PlaneAxis::X:
                            plane->SetNormal(normal, 0, 0);
                            plane->SetOrigin(pos, 0, 0);
                            break;
                        case ClippingPlaneWidget::PlaneAxis::Y:
                            plane->SetNormal(0, normal, 0);
                            plane->SetOrigin(0, pos, 0);
                            break;
                        case ClippingPlaneWidget::PlaneAxis::Z:
                            plane->SetNormal(0, 0, normal);
                            plane->SetOrigin(0, 0, pos);
                            break;
                        default:
                            break;
                    }
                    
                    actors->InitTraversal();
                    while ((actor = actors->GetNextActor()) != nullptr) {
                        actor->GetMapper()->AddClippingPlane(plane);
                    }
                }
            };
            
            applyPlane(ClippingPlaneWidget::PlaneAxis::X);
            applyPlane(ClippingPlaneWidget::PlaneAxis::Y);
            applyPlane(ClippingPlaneWidget::PlaneAxis::Z);
            
            viewport_->refresh();
        }
#endif
    });
    
    // Connect history panel signals
    connect(historyPanel_, &HistoryPanel::historyChanged, this, [this]() {
        outliner_->refresh();
        viewport_->refresh();
        inspector_->setNode(sceneGraph_->getSelected());
    });
    
    connect(historyPanel_, &HistoryPanel::stateRestored, this, [this]() {
        outliner_->refresh();
        viewport_->refresh();
        inspector_->setNode(sceneGraph_->getSelected());
        statusBar_->showMessage("State restored", 2000);
    });
}

void MainWindow::setupStatusBar() {
    statusBar_ = statusBar();
    statusBar_->showMessage("Ready");
}

void MainWindow::connectSignals() {
    // === History actions ===
    connect(toolbar_, &Toolbar::undoAction, this, &MainWindow::onUndo);
    connect(toolbar_, &Toolbar::redoAction, this, &MainWindow::onRedo);
    
    // === View actions from toolbar ===
    connect(toolbar_, &Toolbar::viewFront, this, &MainWindow::onViewFront);
    connect(toolbar_, &Toolbar::viewBack, this, &MainWindow::onViewBack);
    connect(toolbar_, &Toolbar::viewLeft, this, &MainWindow::onViewLeft);
    connect(toolbar_, &Toolbar::viewRight, this, &MainWindow::onViewRight);
    connect(toolbar_, &Toolbar::viewTop, this, &MainWindow::onViewTop);
    connect(toolbar_, &Toolbar::viewBottom, this, &MainWindow::onViewBottom);
    connect(toolbar_, &Toolbar::viewIsometric, this, &MainWindow::onViewIsometric);
    
    connect(toolbar_, &Toolbar::viewFrameSelection, this, [this]() {
        viewport_->frameSelection();
        statusBar_->showMessage("Framed selection", 1000);
    });
    
    connect(toolbar_, &Toolbar::viewReset, this, [this]() {
        viewport_->resetView();
        if (viewCube_) viewCube_->updateFromCamera();
        statusBar_->showMessage("View reset", 1000);
    });
    
    // === Analysis tools ===
    connect(toolbar_, &Toolbar::toggleClippingPlanes, this, &MainWindow::onToggleClippingPlanes);
    connect(toolbar_, &Toolbar::toggleMeasureTool, this, &MainWindow::onToggleMeasureTool);
    
    // === Shape creation from toolbar ===
    connect(toolbar_, &Toolbar::createBox, this, [this]() {
        auto boxShape = makeBox(50.0, 50.0, 50.0);
        auto material = Material::makeAir();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Box", std::move(boxShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        if (historyPanel_) historyPanel_->refresh();
        statusBar_->showMessage("Created Box", 2000);
    });
    
    connect(toolbar_, &Toolbar::createTube, this, [this]() {
        auto tubeShape = makeTube(0.0, 30.0, 50.0);
        auto material = Material::makeWater();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Tube", std::move(tubeShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        if (historyPanel_) historyPanel_->refresh();
        statusBar_->showMessage("Created Tube", 2000);
    });
    
    connect(toolbar_, &Toolbar::createSphere, this, [this]() {
        auto sphereShape = makeSphere(0.0, 40.0);
        auto material = Material::makeAir();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Sphere", std::move(sphereShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        if (historyPanel_) historyPanel_->refresh();
        statusBar_->showMessage("Created Sphere", 2000);
    });
    
    connect(toolbar_, &Toolbar::createCone, this, [this]() {
        auto coneShape = makeCone(0.0, 20.0, 0.0, 40.0, 50.0);
        auto material = Material::makeLead();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Cone", std::move(coneShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        if (historyPanel_) historyPanel_->refresh();
        statusBar_->showMessage("Created Cone", 2000);
    });
    
    connect(toolbar_, &Toolbar::createTrd, this, [this]() {
        auto trdShape = makeTrd(30.0, 20.0, 30.0, 20.0, 50.0);
        auto material = Material::makeSilicon();
        auto cmd = std::make_unique<CreateVolumeCommand>(sceneGraph_, "Trd", std::move(trdShape), material);
        commandStack_->execute(std::move(cmd));
        outliner_->refresh();
        viewport_->refresh();
        if (historyPanel_) historyPanel_->refresh();
        statusBar_->showMessage("Created Trapezoid", 2000);
    });
    
    // === Manipulation tools ===
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
            if (historyPanel_) historyPanel_->refresh();
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
            if (historyPanel_) historyPanel_->refresh();
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

// === View action slots ===

void MainWindow::onViewFront() {
    viewport_->setStandardView(Viewport3D::StandardView::Front);
    if (viewCube_) viewCube_->updateFromCamera();
    statusBar_->showMessage("Front view", 1000);
}

void MainWindow::onViewBack() {
    viewport_->setStandardView(Viewport3D::StandardView::Back);
    if (viewCube_) viewCube_->updateFromCamera();
    statusBar_->showMessage("Back view", 1000);
}

void MainWindow::onViewLeft() {
    viewport_->setStandardView(Viewport3D::StandardView::Left);
    if (viewCube_) viewCube_->updateFromCamera();
    statusBar_->showMessage("Left view", 1000);
}

void MainWindow::onViewRight() {
    viewport_->setStandardView(Viewport3D::StandardView::Right);
    if (viewCube_) viewCube_->updateFromCamera();
    statusBar_->showMessage("Right view", 1000);
}

void MainWindow::onViewTop() {
    viewport_->setStandardView(Viewport3D::StandardView::Top);
    if (viewCube_) viewCube_->updateFromCamera();
    statusBar_->showMessage("Top view", 1000);
}

void MainWindow::onViewBottom() {
    viewport_->setStandardView(Viewport3D::StandardView::Bottom);
    if (viewCube_) viewCube_->updateFromCamera();
    statusBar_->showMessage("Bottom view", 1000);
}

void MainWindow::onViewIsometric() {
    viewport_->setStandardView(Viewport3D::StandardView::Isometric);
    if (viewCube_) viewCube_->updateFromCamera();
    statusBar_->showMessage("Isometric view", 1000);
}

// === Analysis tool slots ===

void MainWindow::onToggleClippingPlanes() {
    if (clippingDock_) {
        bool visible = !clippingDock_->isVisible();
        clippingDock_->setVisible(visible);
        statusBar_->showMessage(visible ? "Clipping planes enabled" : "Clipping planes disabled", 1000);
    }
}

void MainWindow::onToggleMeasureTool() {
    if (measureDock_) {
        bool visible = !measureDock_->isVisible();
        measureDock_->setVisible(visible);
        statusBar_->showMessage(visible ? "Measurement tool enabled" : "Measurement tool disabled", 1000);
    }
}

// === History slots ===

void MainWindow::onUndo() {
    if (commandStack_ && commandStack_->canUndo()) {
        commandStack_->undo();
        outliner_->refresh();
        viewport_->refresh();
        inspector_->setNode(sceneGraph_->getSelected());
        if (historyPanel_) historyPanel_->refresh();
        statusBar_->showMessage("Undo", 1000);
    } else {
        statusBar_->showMessage("Nothing to undo", 1000);
    }
}

void MainWindow::onRedo() {
    if (commandStack_ && commandStack_->canRedo()) {
        commandStack_->redo();
        outliner_->refresh();
        viewport_->refresh();
        inspector_->setNode(sceneGraph_->getSelected());
        if (historyPanel_) historyPanel_->refresh();
        statusBar_->showMessage("Redo", 1000);
    } else {
        statusBar_->showMessage("Nothing to redo", 1000);
    }
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

void MainWindow::updateViewCubePosition() {
    if (viewCube_ && viewport_) {
        // Position ViewCube in top-right corner of viewport
        int x = viewport_->width() - viewCube_->width() - 10;
        int y = 10;
        viewCube_->move(x, y);
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    // Keep ViewCube positioned in top-right corner of viewport
    if ((obj == viewport_->parent() || obj == viewport_) && event->type() == QEvent::Resize) {
        updateViewCubePosition();
    }
    
    return QMainWindow::eventFilter(obj, event);
}

} // namespace geantcad
