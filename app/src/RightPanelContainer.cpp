#include "RightPanelContainer.hh"
#include "BuildRunDialog.hh"
#include <QLabel>
#include <QGroupBox>
#include <QStyle>

namespace geantcad {

RightPanelContainer::RightPanelContainer(QWidget* parent)
    : QWidget(parent)
    , currentTabIndex_(0)
{
    setupUI();
}

RightPanelContainer::~RightPanelContainer() {
}

void RightPanelContainer::setupUI() {
    mainLayout_ = new QHBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    
    // Create stacked widget for panels
    stackedWidget_ = new QStackedWidget(this);
    
    // Create panels
    physicsPanel_ = new PhysicsPanel(this);
    outputPanel_ = new OutputPanel(this);
    particleGunPanel_ = new ParticleGunPanel(this);
    
    // Simulation panel (placeholder for now, can integrate BuildRunDialog later)
    simulationPanel_ = new QWidget(this);
    QVBoxLayout* simLayout = new QVBoxLayout(simulationPanel_);
    QLabel* simLabel = new QLabel("Simulation Panel\n\nBuild & Run functionality", simulationPanel_);
    simLabel->setAlignment(Qt::AlignCenter);
    simLabel->setStyleSheet("color: #888; padding: 20px;");
    simLayout->addWidget(simLabel);
    
    // Add panels to stacked widget
    stackedWidget_->addWidget(physicsPanel_);      // Index 0: Materials/Physics
    stackedWidget_->addWidget(particleGunPanel_);  // Index 1: Source
    stackedWidget_->addWidget(outputPanel_);        // Index 2: Analysis
    stackedWidget_->addWidget(physicsPanel_);        // Index 3: Physics (duplicate for now)
    stackedWidget_->addWidget(simulationPanel_);   // Index 4: Simulation
    
    // Setup vertical tabs
    setupVerticalTabs();
    
    // Add tabs and stacked widget to layout
    mainLayout_->addWidget(stackedWidget_, 1); // Stacked widget takes most space
    mainLayout_->addLayout(tabLayout_, 0);     // Tabs on the right
    
    // Connect panel signals
    connect(physicsPanel_, &PhysicsPanel::configChanged, this, &RightPanelContainer::physicsConfigChanged);
    connect(outputPanel_, &OutputPanel::configChanged, this, &RightPanelContainer::outputConfigChanged);
    connect(particleGunPanel_, &ParticleGunPanel::configChanged, this, &RightPanelContainer::particleGunConfigChanged);
    
    // Show first panel
    stackedWidget_->setCurrentIndex(0);
}

void RightPanelContainer::setupVerticalTabs() {
    tabLayout_ = new QVBoxLayout();
    tabLayout_->setContentsMargins(0, 0, 0, 0);
    tabLayout_->setSpacing(2);
    
    tabButtonGroup_ = new QButtonGroup(this);
    
    // Materials tab (Physics panel)
    materialsTab_ = new QPushButton("Materials", this);
    materialsTab_->setCheckable(true);
    materialsTab_->setChecked(true);
    materialsTab_->setMinimumHeight(40);
    materialsTab_->setMaximumWidth(120);
    materialsTab_->setStyleSheet(
        "QPushButton {"
        "  text-align: left;"
        "  padding: 8px;"
        "  border: none;"
        "  background-color: #252525;"
        "  color: #e0e0e0;"
        "}"
        "QPushButton:hover {"
        "  background-color: #3a3a3a;"
        "}"
        "QPushButton:checked {"
        "  background-color: #0078d4;"
        "  color: white;"
        "}"
    );
    tabButtonGroup_->addButton(materialsTab_, 0);
    tabLayout_->addWidget(materialsTab_);
    
    // Source tab
    sourceTab_ = new QPushButton("Source", this);
    sourceTab_->setCheckable(true);
    sourceTab_->setMinimumHeight(40);
    sourceTab_->setMaximumWidth(120);
    sourceTab_->setStyleSheet(materialsTab_->styleSheet());
    tabButtonGroup_->addButton(sourceTab_, 1);
    tabLayout_->addWidget(sourceTab_);
    
    // Analysis tab
    analysisTab_ = new QPushButton("Analysis", this);
    analysisTab_->setCheckable(true);
    analysisTab_->setMinimumHeight(40);
    analysisTab_->setMaximumWidth(120);
    analysisTab_->setStyleSheet(materialsTab_->styleSheet());
    tabButtonGroup_->addButton(analysisTab_, 2);
    tabLayout_->addWidget(analysisTab_);
    
    // Physics tab
    physicsTab_ = new QPushButton("Physics", this);
    physicsTab_->setCheckable(true);
    physicsTab_->setMinimumHeight(40);
    physicsTab_->setMaximumWidth(120);
    physicsTab_->setStyleSheet(materialsTab_->styleSheet());
    tabButtonGroup_->addButton(physicsTab_, 3);
    tabLayout_->addWidget(physicsTab_);
    
    // Simulation tab
    simulationTab_ = new QPushButton("Simulation", this);
    simulationTab_->setCheckable(true);
    simulationTab_->setMinimumHeight(40);
    simulationTab_->setMaximumWidth(120);
    simulationTab_->setStyleSheet(materialsTab_->styleSheet());
    tabButtonGroup_->addButton(simulationTab_, 4);
    tabLayout_->addWidget(simulationTab_);
    
    // Add stretch at bottom
    tabLayout_->addStretch();
    
    // Connect button group
    connect(tabButtonGroup_, QOverload<int>::of(&QButtonGroup::idClicked), 
            this, &RightPanelContainer::onTabClicked);
}

void RightPanelContainer::onTabClicked(int index) {
    if (index >= 0 && index < stackedWidget_->count()) {
        stackedWidget_->setCurrentIndex(index);
        currentTabIndex_ = index;
    }
}

} // namespace geantcad

