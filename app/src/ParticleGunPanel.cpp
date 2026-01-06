#include "ParticleGunPanel.hh"
#include <QLabel>

namespace geantcad {

ParticleGunPanel::ParticleGunPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    updateUI();
}

ParticleGunPanel::~ParticleGunPanel() {
}

void ParticleGunPanel::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // Particle type
    QGroupBox* particleGroup = new QGroupBox("Particle Type", this);
    QFormLayout* particleLayout = new QFormLayout(particleGroup);
    
    particleTypeCombo_ = new QComboBox(this);
    particleTypeCombo_->addItem("gamma", "gamma");
    particleTypeCombo_->addItem("e-", "e-");
    particleTypeCombo_->addItem("e+", "e+");
    particleTypeCombo_->addItem("proton", "proton");
    particleTypeCombo_->addItem("neutron", "neutron");
    particleTypeCombo_->addItem("alpha", "alpha");
    particleTypeCombo_->addItem("mu-", "mu-");
    particleTypeCombo_->addItem("mu+", "mu+");
    particleTypeCombo_->addItem("pi-", "pi-");
    particleTypeCombo_->addItem("pi+", "pi+");
    particleTypeCombo_->addItem("pi0", "pi0");
    connect(particleTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParticleGunPanel::onParticleTypeChanged);
    particleLayout->addRow("Type:", particleTypeCombo_);
    
    numberOfParticlesSpin_ = new QSpinBox(this);
    numberOfParticlesSpin_->setMinimum(1);
    numberOfParticlesSpin_->setMaximum(1000);
    numberOfParticlesSpin_->setValue(1);
    connect(numberOfParticlesSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    particleLayout->addRow("Number:", numberOfParticlesSpin_);
    
    layout->addWidget(particleGroup);
    
    // Energy configuration
    energyGroup_ = new QGroupBox("Energy", this);
    QFormLayout* energyLayout = new QFormLayout(energyGroup_);
    
    energyModeCombo_ = new QComboBox(this);
    energyModeCombo_->addItem("Mono", static_cast<int>(ParticleGunConfig::EnergyMode::Mono));
    energyModeCombo_->addItem("Uniform", static_cast<int>(ParticleGunConfig::EnergyMode::Uniform));
    energyModeCombo_->addItem("Gaussian", static_cast<int>(ParticleGunConfig::EnergyMode::Gaussian));
    connect(energyModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParticleGunPanel::onEnergyModeChanged);
    energyLayout->addRow("Mode:", energyModeCombo_);
    
    energySpin_ = new QDoubleSpinBox(this);
    energySpin_->setMinimum(0.001);
    energySpin_->setMaximum(10000.0);
    energySpin_->setValue(1.0);
    energySpin_->setSuffix(" MeV");
    energySpin_->setDecimals(3);
    connect(energySpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    energyLayout->addRow("Energy:", energySpin_);
    
    energyMinSpin_ = new QDoubleSpinBox(this);
    energyMinSpin_->setMinimum(0.001);
    energyMinSpin_->setMaximum(10000.0);
    energyMinSpin_->setValue(0.5);
    energyMinSpin_->setSuffix(" MeV");
    energyMinSpin_->setDecimals(3);
    connect(energyMinSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    energyLayout->addRow("Min:", energyMinSpin_);
    
    energyMaxSpin_ = new QDoubleSpinBox(this);
    energyMaxSpin_->setMinimum(0.001);
    energyMaxSpin_->setMaximum(10000.0);
    energyMaxSpin_->setValue(2.0);
    energyMaxSpin_->setSuffix(" MeV");
    energyMaxSpin_->setDecimals(3);
    connect(energyMaxSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    energyLayout->addRow("Max:", energyMaxSpin_);
    
    energyMeanSpin_ = new QDoubleSpinBox(this);
    energyMeanSpin_->setMinimum(0.001);
    energyMeanSpin_->setMaximum(10000.0);
    energyMeanSpin_->setValue(1.0);
    energyMeanSpin_->setSuffix(" MeV");
    energyMeanSpin_->setDecimals(3);
    connect(energyMeanSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    energyLayout->addRow("Mean:", energyMeanSpin_);
    
    energySigmaSpin_ = new QDoubleSpinBox(this);
    energySigmaSpin_->setMinimum(0.001);
    energySigmaSpin_->setMaximum(1000.0);
    energySigmaSpin_->setValue(0.1);
    energySigmaSpin_->setSuffix(" MeV");
    energySigmaSpin_->setDecimals(3);
    connect(energySigmaSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    energyLayout->addRow("Sigma:", energySigmaSpin_);
    
    layout->addWidget(energyGroup_);
    
    // Position configuration
    positionGroup_ = new QGroupBox("Position", this);
    QFormLayout* positionLayout = new QFormLayout(positionGroup_);
    
    positionModeCombo_ = new QComboBox(this);
    positionModeCombo_->addItem("Point", static_cast<int>(ParticleGunConfig::PositionMode::Point));
    positionModeCombo_->addItem("Volume", static_cast<int>(ParticleGunConfig::PositionMode::Volume));
    positionModeCombo_->addItem("Surface", static_cast<int>(ParticleGunConfig::PositionMode::Surface));
    connect(positionModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParticleGunPanel::onPositionModeChanged);
    positionLayout->addRow("Mode:", positionModeCombo_);
    
    positionXSpin_ = new QDoubleSpinBox(this);
    positionXSpin_->setMinimum(-10000.0);
    positionXSpin_->setMaximum(10000.0);
    positionXSpin_->setValue(0.0);
    positionXSpin_->setSuffix(" mm");
    positionXSpin_->setDecimals(2);
    connect(positionXSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    positionLayout->addRow("X:", positionXSpin_);
    
    positionYSpin_ = new QDoubleSpinBox(this);
    positionYSpin_->setMinimum(-10000.0);
    positionYSpin_->setMaximum(10000.0);
    positionYSpin_->setValue(0.0);
    positionYSpin_->setSuffix(" mm");
    positionYSpin_->setDecimals(2);
    connect(positionYSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    positionLayout->addRow("Y:", positionYSpin_);
    
    positionZSpin_ = new QDoubleSpinBox(this);
    positionZSpin_->setMinimum(-10000.0);
    positionZSpin_->setMaximum(10000.0);
    positionZSpin_->setValue(0.0);
    positionZSpin_->setSuffix(" mm");
    positionZSpin_->setDecimals(2);
    connect(positionZSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    positionLayout->addRow("Z:", positionZSpin_);
    
    positionRadiusSpin_ = new QDoubleSpinBox(this);
    positionRadiusSpin_->setMinimum(0.1);
    positionRadiusSpin_->setMaximum(10000.0);
    positionRadiusSpin_->setValue(10.0);
    positionRadiusSpin_->setSuffix(" mm");
    positionRadiusSpin_->setDecimals(2);
    connect(positionRadiusSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    positionLayout->addRow("Radius:", positionRadiusSpin_);
    
    positionVolumeEdit_ = new QLineEdit(this);
    positionVolumeEdit_->setPlaceholderText("Volume name (optional)");
    connect(positionVolumeEdit_, &QLineEdit::textChanged,
            this, &ParticleGunPanel::onValueChanged);
    positionLayout->addRow("Volume:", positionVolumeEdit_);
    
    layout->addWidget(positionGroup_);
    
    // Direction configuration
    directionGroup_ = new QGroupBox("Direction", this);
    QFormLayout* directionLayout = new QFormLayout(directionGroup_);
    
    directionModeCombo_ = new QComboBox(this);
    directionModeCombo_->addItem("Isotropic", static_cast<int>(ParticleGunConfig::DirectionMode::Isotropic));
    directionModeCombo_->addItem("Fixed", static_cast<int>(ParticleGunConfig::DirectionMode::Fixed));
    directionModeCombo_->addItem("Cone", static_cast<int>(ParticleGunConfig::DirectionMode::Cone));
    connect(directionModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParticleGunPanel::onDirectionModeChanged);
    directionLayout->addRow("Mode:", directionModeCombo_);
    
    directionXSpin_ = new QDoubleSpinBox(this);
    directionXSpin_->setMinimum(-1.0);
    directionXSpin_->setMaximum(1.0);
    directionXSpin_->setValue(0.0);
    directionXSpin_->setSingleStep(0.1);
    directionXSpin_->setDecimals(3);
    connect(directionXSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    directionLayout->addRow("X:", directionXSpin_);
    
    directionYSpin_ = new QDoubleSpinBox(this);
    directionYSpin_->setMinimum(-1.0);
    directionYSpin_->setMaximum(1.0);
    directionYSpin_->setValue(0.0);
    directionYSpin_->setSingleStep(0.1);
    directionYSpin_->setDecimals(3);
    connect(directionYSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    directionLayout->addRow("Y:", directionYSpin_);
    
    directionZSpin_ = new QDoubleSpinBox(this);
    directionZSpin_->setMinimum(-1.0);
    directionZSpin_->setMaximum(1.0);
    directionZSpin_->setValue(1.0);
    directionZSpin_->setSingleStep(0.1);
    directionZSpin_->setDecimals(3);
    connect(directionZSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    directionLayout->addRow("Z:", directionZSpin_);
    
    coneAngleSpin_ = new QDoubleSpinBox(this);
    coneAngleSpin_->setMinimum(0.0);
    coneAngleSpin_->setMaximum(180.0);
    coneAngleSpin_->setValue(30.0);
    coneAngleSpin_->setSuffix(" deg");
    coneAngleSpin_->setDecimals(1);
    connect(coneAngleSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParticleGunPanel::onValueChanged);
    directionLayout->addRow("Cone Angle:", coneAngleSpin_);
    
    layout->addWidget(directionGroup_);
    
    // Preview
    QGroupBox* previewGroup = new QGroupBox("Preview", this);
    QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
    previewLabel_ = new QLabel(this);
    previewLabel_->setWordWrap(true);
    previewLabel_->setStyleSheet("padding: 8px; background-color: #252525; border-radius: 3px;");
    previewLayout->addWidget(previewLabel_);
    layout->addWidget(previewGroup);
    
    layout->addStretch();
    
    updatePreview();
}

void ParticleGunPanel::updateUI() {
    // Update energy controls visibility
    ParticleGunConfig::EnergyMode energyMode = static_cast<ParticleGunConfig::EnergyMode>(
        energyModeCombo_->currentData().toInt());
    
    energySpin_->setVisible(energyMode == ParticleGunConfig::EnergyMode::Mono);
    energyMinSpin_->setVisible(energyMode == ParticleGunConfig::EnergyMode::Uniform);
    energyMaxSpin_->setVisible(energyMode == ParticleGunConfig::EnergyMode::Uniform);
    energyMeanSpin_->setVisible(energyMode == ParticleGunConfig::EnergyMode::Gaussian);
    energySigmaSpin_->setVisible(energyMode == ParticleGunConfig::EnergyMode::Gaussian);
    
    // Update position controls visibility
    ParticleGunConfig::PositionMode positionMode = static_cast<ParticleGunConfig::PositionMode>(
        positionModeCombo_->currentData().toInt());
    
    positionXSpin_->setVisible(positionMode == ParticleGunConfig::PositionMode::Point);
    positionYSpin_->setVisible(positionMode == ParticleGunConfig::PositionMode::Point);
    positionZSpin_->setVisible(positionMode == ParticleGunConfig::PositionMode::Point);
    positionRadiusSpin_->setVisible(positionMode == ParticleGunConfig::PositionMode::Volume || 
                                    positionMode == ParticleGunConfig::PositionMode::Surface);
    positionVolumeEdit_->setVisible(positionMode == ParticleGunConfig::PositionMode::Volume || 
                                    positionMode == ParticleGunConfig::PositionMode::Surface);
    
    // Update direction controls visibility
    ParticleGunConfig::DirectionMode directionMode = static_cast<ParticleGunConfig::DirectionMode>(
        directionModeCombo_->currentData().toInt());
    
    directionXSpin_->setVisible(directionMode == ParticleGunConfig::DirectionMode::Fixed || 
                                directionMode == ParticleGunConfig::DirectionMode::Cone);
    directionYSpin_->setVisible(directionMode == ParticleGunConfig::DirectionMode::Fixed || 
                                directionMode == ParticleGunConfig::DirectionMode::Cone);
    directionZSpin_->setVisible(directionMode == ParticleGunConfig::DirectionMode::Fixed || 
                                directionMode == ParticleGunConfig::DirectionMode::Cone);
    coneAngleSpin_->setVisible(directionMode == ParticleGunConfig::DirectionMode::Cone);
}

void ParticleGunPanel::setConfig(const ParticleGunConfig& config) {
    config_ = config;
    
    // Particle type
    int particleIndex = particleTypeCombo_->findData(QString::fromStdString(config.particleType));
    if (particleIndex >= 0) {
        particleTypeCombo_->setCurrentIndex(particleIndex);
    }
    numberOfParticlesSpin_->setValue(config.numberOfParticles);
    
    // Energy
    int energyIndex = energyModeCombo_->findData(static_cast<int>(config.energyMode));
    if (energyIndex >= 0) {
        energyModeCombo_->setCurrentIndex(energyIndex);
    }
    energySpin_->setValue(config.energy);
    energyMinSpin_->setValue(config.energyMin);
    energyMaxSpin_->setValue(config.energyMax);
    energyMeanSpin_->setValue(config.energyMean);
    energySigmaSpin_->setValue(config.energySigma);
    
    // Position
    int positionIndex = positionModeCombo_->findData(static_cast<int>(config.positionMode));
    if (positionIndex >= 0) {
        positionModeCombo_->setCurrentIndex(positionIndex);
    }
    positionXSpin_->setValue(config.positionX);
    positionYSpin_->setValue(config.positionY);
    positionZSpin_->setValue(config.positionZ);
    positionRadiusSpin_->setValue(config.positionRadius);
    positionVolumeEdit_->setText(QString::fromStdString(config.positionVolume));
    
    // Direction
    int directionIndex = directionModeCombo_->findData(static_cast<int>(config.directionMode));
    if (directionIndex >= 0) {
        directionModeCombo_->setCurrentIndex(directionIndex);
    }
    directionXSpin_->setValue(config.directionX);
    directionYSpin_->setValue(config.directionY);
    directionZSpin_->setValue(config.directionZ);
    coneAngleSpin_->setValue(config.coneAngle);
    
    updateUI();
}

ParticleGunConfig ParticleGunPanel::getConfig() const {
    ParticleGunConfig config;
    
    // Particle type
    config.particleType = particleTypeCombo_->currentData().toString().toStdString();
    config.numberOfParticles = numberOfParticlesSpin_->value();
    
    // Energy
    config.energyMode = static_cast<ParticleGunConfig::EnergyMode>(
        energyModeCombo_->currentData().toInt());
    config.energy = energySpin_->value();
    config.energyMin = energyMinSpin_->value();
    config.energyMax = energyMaxSpin_->value();
    config.energyMean = energyMeanSpin_->value();
    config.energySigma = energySigmaSpin_->value();
    
    // Position
    config.positionMode = static_cast<ParticleGunConfig::PositionMode>(
        positionModeCombo_->currentData().toInt());
    config.positionX = positionXSpin_->value();
    config.positionY = positionYSpin_->value();
    config.positionZ = positionZSpin_->value();
    config.positionRadius = positionRadiusSpin_->value();
    config.positionVolume = positionVolumeEdit_->text().toStdString();
    
    // Direction
    config.directionMode = static_cast<ParticleGunConfig::DirectionMode>(
        directionModeCombo_->currentData().toInt());
    config.directionX = directionXSpin_->value();
    config.directionY = directionYSpin_->value();
    config.directionZ = directionZSpin_->value();
    config.coneAngle = coneAngleSpin_->value();
    
    return config;
}

void ParticleGunPanel::onParticleTypeChanged() {
    updatePreview();
    emit configChanged();
}

void ParticleGunPanel::onEnergyModeChanged() {
    updateUI();
    updatePreview();
    emit configChanged();
}

void ParticleGunPanel::onPositionModeChanged() {
    updateUI();
    updatePreview();
    emit configChanged();
}

void ParticleGunPanel::onDirectionModeChanged() {
    updateUI();
    updatePreview();
    emit configChanged();
}

void ParticleGunPanel::onValueChanged() {
    updatePreview();
    emit configChanged();
}

void ParticleGunPanel::updatePreview() {
    ParticleGunConfig config = getConfig();
    
    QString preview = QString("Particle: <b>%1</b> × %2<br>")
                      .arg(QString::fromStdString(config.particleType))
                      .arg(config.numberOfParticles);
    
    QString energyStr;
    switch (config.energyMode) {
        case ParticleGunConfig::EnergyMode::Mono:
            energyStr = QString("%1 MeV").arg(config.energy);
            break;
        case ParticleGunConfig::EnergyMode::Uniform:
            energyStr = QString("%1 - %2 MeV").arg(config.energyMin).arg(config.energyMax);
            break;
        case ParticleGunConfig::EnergyMode::Gaussian:
            energyStr = QString("μ=%1, σ=%2 MeV").arg(config.energyMean).arg(config.energySigma);
            break;
    }
    preview += QString("Energy: <b>%1</b><br>").arg(energyStr);
    
    QString positionStr;
    switch (config.positionMode) {
        case ParticleGunConfig::PositionMode::Point:
            positionStr = QString("(%1, %2, %3) mm")
                        .arg(config.positionX).arg(config.positionY).arg(config.positionZ);
            break;
        case ParticleGunConfig::PositionMode::Volume:
            positionStr = QString("Volume: %1, r=%2 mm")
                        .arg(QString::fromStdString(config.positionVolume))
                        .arg(config.positionRadius);
            break;
        case ParticleGunConfig::PositionMode::Surface:
            positionStr = QString("Surface: %1, r=%2 mm")
                        .arg(QString::fromStdString(config.positionVolume))
                        .arg(config.positionRadius);
            break;
    }
    preview += QString("Position: <b>%1</b><br>").arg(positionStr);
    
    QString directionStr;
    switch (config.directionMode) {
        case ParticleGunConfig::DirectionMode::Isotropic:
            directionStr = "Isotropic";
            break;
        case ParticleGunConfig::DirectionMode::Fixed:
            directionStr = QString("(%1, %2, %3)")
                          .arg(config.directionX).arg(config.directionY).arg(config.directionZ);
            break;
        case ParticleGunConfig::DirectionMode::Cone:
            directionStr = QString("Cone: (%1, %2, %3), θ=%4°")
                          .arg(config.directionX).arg(config.directionY).arg(config.directionZ)
                          .arg(config.coneAngle);
            break;
    }
    preview += QString("Direction: <b>%1</b>").arg(directionStr);
    
    previewLabel_->setText(preview);
}

} // namespace geantcad

