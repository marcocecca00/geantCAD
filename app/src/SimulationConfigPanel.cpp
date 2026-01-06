#include "SimulationConfigPanel.hh"
#include "BuildRunDialog.hh"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

namespace geantcad {

SimulationConfigPanel::SimulationConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

SimulationConfigPanel::~SimulationConfigPanel() {
}

void SimulationConfigPanel::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Tab widget for different simulation config sections
    tabWidget_ = new QTabWidget(this);
    tabWidget_->setTabPosition(QTabWidget::North);
    
    // Physics tab
    physicsPanel_ = new PhysicsPanel(this);
    tabWidget_->addTab(physicsPanel_, "Physics");
    
    // Particle Gun (Source) tab
    particleGunPanel_ = new ParticleGunPanel(this);
    tabWidget_->addTab(particleGunPanel_, "Source");
    
    // Output/Analysis tab
    outputPanel_ = new OutputPanel(this);
    tabWidget_->addTab(outputPanel_, "Output");
    
    // Build & Run tab
    buildRunWidget_ = new QWidget(this);
    QVBoxLayout* buildRunLayout = new QVBoxLayout(buildRunWidget_);
    buildRunLayout->setContentsMargins(10, 10, 10, 10);
    
    QLabel* infoLabel = new QLabel(
        "Build & Run Configuration\n\n"
        "Use the 'Build & Run Geant4 Project...' option from the Tools menu\n"
        "to compile and execute your Geant4 simulation.",
        buildRunWidget_
    );
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignTop);
    buildRunLayout->addWidget(infoLabel);
    
    QPushButton* buildRunButton = new QPushButton("Build & Run...", buildRunWidget_);
    connect(buildRunButton, &QPushButton::clicked, this, &SimulationConfigPanel::onBuildRun);
    buildRunLayout->addWidget(buildRunButton);
    buildRunLayout->addStretch();
    
    tabWidget_->addTab(buildRunWidget_, "Build & Run");
    
    layout->addWidget(tabWidget_);
    
    // Connect signals
    connect(physicsPanel_, &PhysicsPanel::configChanged, this, &SimulationConfigPanel::physicsConfigChanged);
    connect(outputPanel_, &OutputPanel::configChanged, this, &SimulationConfigPanel::outputConfigChanged);
    connect(particleGunPanel_, &ParticleGunPanel::configChanged, this, &SimulationConfigPanel::particleGunConfigChanged);
}

void SimulationConfigPanel::onBuildRun() {
    BuildRunDialog dialog(parentWidget());
    dialog.exec();
}

} // namespace geantcad

