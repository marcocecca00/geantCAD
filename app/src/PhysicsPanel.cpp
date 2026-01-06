#include "PhysicsPanel.hh"
#include <QLabel>
#include <QFormLayout>
#include <QScrollArea>

namespace geantcad {

PhysicsPanel::PhysicsPanel(QWidget* parent)
    : QWidget(parent)
    , updating_(false)
{
    setupUI();
}

PhysicsPanel::~PhysicsPanel() {
}

void PhysicsPanel::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(10);
    
    // === Electromagnetic Physics ===
    QGroupBox* emGroup = new QGroupBox("Electromagnetic Physics", this);
    QVBoxLayout* emLayout = new QVBoxLayout(emGroup);
    
    emCheckbox_ = new QCheckBox("Enable EM Physics", this);
    emCheckbox_->setChecked(true);
    emCheckbox_->setToolTip("Standard electromagnetic interactions");
    connect(emCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    emLayout->addWidget(emCheckbox_);
    
    QFormLayout* emOptionLayout = new QFormLayout();
    emOptionCombo_ = new QComboBox(this);
    emOptionCombo_->addItem("Standard (default)", "Standard");
    emOptionCombo_->addItem("Option 1 (high energy)", "Option1");
    emOptionCombo_->addItem("Option 2 (low energy)", "Option2");
    emOptionCombo_->addItem("Option 3 (WVI)", "Option3");
    emOptionCombo_->addItem("Option 4 (precision)", "Option4");
    emOptionCombo_->addItem("Penelope (medical)", "Penelope");
    emOptionCombo_->addItem("Livermore (low energy)", "Livermore");
    emOptionCombo_->setToolTip("Select EM physics model variant");
    connect(emOptionCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PhysicsPanel::onComboChanged);
    emOptionLayout->addRow("EM Model:", emOptionCombo_);
    emLayout->addLayout(emOptionLayout);
    
    scrollLayout->addWidget(emGroup);
    
    // === Hadronic Physics ===
    QGroupBox* hadGroup = new QGroupBox("Hadronic Physics", this);
    QVBoxLayout* hadLayout = new QVBoxLayout(hadGroup);
    
    hadronicCheckbox_ = new QCheckBox("Enable Hadronic Physics", this);
    hadronicCheckbox_->setChecked(true);
    hadronicCheckbox_->setToolTip("Strong interactions for hadrons");
    connect(hadronicCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    hadLayout->addWidget(hadronicCheckbox_);
    
    QFormLayout* hadOptionLayout = new QFormLayout();
    hadronicModelCombo_ = new QComboBox(this);
    hadronicModelCombo_->addItem("FTFP_BERT (default)", "FTFP_BERT");
    hadronicModelCombo_->addItem("QGSP_BERT", "QGSP_BERT");
    hadronicModelCombo_->addItem("QGSP_BIC (binary cascade)", "QGSP_BIC");
    hadronicModelCombo_->addItem("FTFP_INCLXX (INCL++)", "FTFP_INCLXX");
    hadronicModelCombo_->setToolTip("Select hadronic physics model");
    connect(hadronicModelCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PhysicsPanel::onComboChanged);
    hadOptionLayout->addRow("Hadronic Model:", hadronicModelCombo_);
    hadLayout->addLayout(hadOptionLayout);
    
    ionCheckbox_ = new QCheckBox("Ion Physics", this);
    ionCheckbox_->setToolTip("Heavy ion interactions");
    connect(ionCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    hadLayout->addWidget(ionCheckbox_);
    
    scrollLayout->addWidget(hadGroup);
    
    // === Additional Physics ===
    QGroupBox* addGroup = new QGroupBox("Additional Physics", this);
    QVBoxLayout* addLayout = new QVBoxLayout(addGroup);
    
    decayCheckbox_ = new QCheckBox("Decay Physics", this);
    decayCheckbox_->setToolTip("Particle decay (stable particles)");
    connect(decayCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    addLayout->addWidget(decayCheckbox_);
    
    radioactiveCheckbox_ = new QCheckBox("Radioactive Decay", this);
    radioactiveCheckbox_->setToolTip("Radioactive nuclear decay");
    connect(radioactiveCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    addLayout->addWidget(radioactiveCheckbox_);
    
    opticalCheckbox_ = new QCheckBox("Optical Physics (Cerenkov + Scintillation)", this);
    opticalCheckbox_->setToolTip("Optical photon processes");
    connect(opticalCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    addLayout->addWidget(opticalCheckbox_);
    
    stepLimiterCheckbox_ = new QCheckBox("Step Limiter", this);
    stepLimiterCheckbox_->setToolTip("Limit step size for precision");
    connect(stepLimiterCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    addLayout->addWidget(stepLimiterCheckbox_);
    
    scrollLayout->addWidget(addGroup);
    
    // === Production Cuts ===
    QGroupBox* cutsGroup = new QGroupBox("Production Cuts", this);
    QFormLayout* cutsLayout = new QFormLayout(cutsGroup);
    
    auto createCutSpin = [this]() {
        QDoubleSpinBox* spin = new QDoubleSpinBox(this);
        spin->setRange(0.001, 100.0);
        spin->setValue(0.1);
        spin->setSuffix(" mm");
        spin->setDecimals(3);
        spin->setToolTip("Secondary particle production threshold");
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PhysicsPanel::onCutsChanged);
        return spin;
    };
    
    gammaCutSpin_ = createCutSpin();
    cutsLayout->addRow("Gamma:", gammaCutSpin_);
    
    electronCutSpin_ = createCutSpin();
    cutsLayout->addRow("Electron (e⁻):", electronCutSpin_);
    
    positronCutSpin_ = createCutSpin();
    cutsLayout->addRow("Positron (e⁺):", positronCutSpin_);
    
    protonCutSpin_ = createCutSpin();
    cutsLayout->addRow("Proton:", protonCutSpin_);
    
    scrollLayout->addWidget(cutsGroup);
    
    // === Preview ===
    QGroupBox* previewGroup = new QGroupBox("Configuration Preview", this);
    QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
    previewLabel_ = new QLabel(this);
    previewLabel_->setWordWrap(true);
    previewLabel_->setStyleSheet("padding: 8px; background-color: #252525; border-radius: 3px; font-family: monospace;");
    previewLayout->addWidget(previewLabel_);
    scrollLayout->addWidget(previewGroup);
    
    scrollLayout->addStretch();
    
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea);
    
    updateEnabledStates();
    updatePreview();
}

void PhysicsPanel::setConfig(const PhysicsConfig& config) {
    updating_ = true;
    
    emCheckbox_->setChecked(config.emEnabled);
    decayCheckbox_->setChecked(config.decayEnabled);
    opticalCheckbox_->setChecked(config.opticalEnabled);
    hadronicCheckbox_->setChecked(config.hadronicEnabled);
    ionCheckbox_->setChecked(config.ionPhysicsEnabled);
    radioactiveCheckbox_->setChecked(config.radioactiveDecayEnabled);
    stepLimiterCheckbox_->setChecked(config.stepLimiterEnabled);
    
    int emIndex = emOptionCombo_->findData(QString::fromStdString(PhysicsConfig::emOptionToString(config.emOption)));
    if (emIndex >= 0) emOptionCombo_->setCurrentIndex(emIndex);
    
    int hadIndex = hadronicModelCombo_->findData(QString::fromStdString(PhysicsConfig::hadronicModelToString(config.hadronicModel)));
    if (hadIndex >= 0) hadronicModelCombo_->setCurrentIndex(hadIndex);
    
    gammaCutSpin_->setValue(config.gammaCut);
    electronCutSpin_->setValue(config.electronCut);
    positronCutSpin_->setValue(config.positronCut);
    protonCutSpin_->setValue(config.protonCut);
    
    updating_ = false;
    updateEnabledStates();
    updatePreview();
}

PhysicsConfig PhysicsPanel::getConfig() const {
    PhysicsConfig config;
    config.emEnabled = emCheckbox_->isChecked();
    config.decayEnabled = decayCheckbox_->isChecked();
    config.opticalEnabled = opticalCheckbox_->isChecked();
    config.hadronicEnabled = hadronicCheckbox_->isChecked();
    config.ionPhysicsEnabled = ionCheckbox_->isChecked();
    config.radioactiveDecayEnabled = radioactiveCheckbox_->isChecked();
    config.stepLimiterEnabled = stepLimiterCheckbox_->isChecked();
    
    config.emOption = PhysicsConfig::stringToEMOption(emOptionCombo_->currentData().toString().toStdString());
    config.hadronicModel = PhysicsConfig::stringToHadronicModel(hadronicModelCombo_->currentData().toString().toStdString());
    
    config.gammaCut = gammaCutSpin_->value();
    config.electronCut = electronCutSpin_->value();
    config.positronCut = positronCutSpin_->value();
    config.protonCut = protonCutSpin_->value();
    
    return config;
}

void PhysicsPanel::onCheckboxChanged() {
    updateEnabledStates();
    updatePreview();
    if (!updating_) emit configChanged();
}

void PhysicsPanel::onComboChanged() {
    updatePreview();
    if (!updating_) emit configChanged();
}

void PhysicsPanel::onCutsChanged() {
    updatePreview();
    if (!updating_) emit configChanged();
}

void PhysicsPanel::updateEnabledStates() {
    emOptionCombo_->setEnabled(emCheckbox_->isChecked());
    hadronicModelCombo_->setEnabled(hadronicCheckbox_->isChecked());
    ionCheckbox_->setEnabled(hadronicCheckbox_->isChecked());
}

void PhysicsPanel::updatePreview() {
    PhysicsConfig config = getConfig();
    
    QStringList enabledPhysics;
    if (config.emEnabled) {
        enabledPhysics << QString("EM (%1)").arg(QString::fromStdString(PhysicsConfig::emOptionToString(config.emOption)));
    }
    if (config.hadronicEnabled) {
        enabledPhysics << QString("Hadronic (%1)").arg(QString::fromStdString(PhysicsConfig::hadronicModelToString(config.hadronicModel)));
    }
    if (config.decayEnabled) enabledPhysics << "Decay";
    if (config.radioactiveDecayEnabled) enabledPhysics << "Radioactive";
    if (config.opticalEnabled) enabledPhysics << "Optical";
    if (config.ionPhysicsEnabled) enabledPhysics << "Ions";
    if (config.stepLimiterEnabled) enabledPhysics << "StepLimiter";
    
    QString preview;
    preview += "<b>Enabled Physics:</b><br>";
    if (enabledPhysics.isEmpty()) {
        preview += "⚠️ No physics enabled<br>";
    } else {
        preview += enabledPhysics.join(" • ") + "<br>";
    }
    
    preview += "<br><b>Production Cuts:</b><br>";
    preview += QString("γ: %1mm | e⁻: %2mm | e⁺: %3mm | p: %4mm")
        .arg(config.gammaCut)
        .arg(config.electronCut)
        .arg(config.positronCut)
        .arg(config.protonCut);
    
    previewLabel_->setText(preview);
}

} // namespace geantcad

