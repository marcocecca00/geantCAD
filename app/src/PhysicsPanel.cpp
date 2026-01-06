#include "PhysicsPanel.hh"
#include <QLabel>

namespace geantcad {

PhysicsPanel::PhysicsPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

PhysicsPanel::~PhysicsPanel() {
}

void PhysicsPanel::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    
    QGroupBox* groupBox = new QGroupBox("Physics Configuration", this);
    QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);
    
    // EM physics
    emCheckbox_ = new QCheckBox("Electromagnetic (EM)", this);
    emCheckbox_->setChecked(true);
    connect(emCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    groupLayout->addWidget(emCheckbox_);
    
    // Decay physics
    decayCheckbox_ = new QCheckBox("Decay", this);
    connect(decayCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    groupLayout->addWidget(decayCheckbox_);
    
    // Optical physics
    opticalCheckbox_ = new QCheckBox("Optical (Cerenkov + Scintillation)", this);
    connect(opticalCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    groupLayout->addWidget(opticalCheckbox_);
    
    // Hadronic physics
    hadronicCheckbox_ = new QCheckBox("Hadronic", this);
    hadronicCheckbox_->setChecked(true);
    connect(hadronicCheckbox_, &QCheckBox::toggled, this, &PhysicsPanel::onCheckboxChanged);
    groupLayout->addWidget(hadronicCheckbox_);
    
    // Standard list
    groupLayout->addWidget(new QLabel("Standard List:", this));
    standardListCombo_ = new QComboBox(this);
    standardListCombo_->addItem("FTFP_BERT", "FTFP_BERT");
    standardListCombo_->addItem("QGSP_BERT", "QGSP_BERT");
    standardListCombo_->addItem("FTFP_BERT_HP", "FTFP_BERT_HP");
    standardListCombo_->setCurrentIndex(0);
    connect(standardListCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PhysicsPanel::onStandardListChanged);
    groupLayout->addWidget(standardListCombo_);
    
    groupLayout->addStretch();
    
    layout->addWidget(groupBox);
    
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

void PhysicsPanel::setConfig(const PhysicsConfig& config) {
    emCheckbox_->setChecked(config.emEnabled);
    decayCheckbox_->setChecked(config.decayEnabled);
    opticalCheckbox_->setChecked(config.opticalEnabled);
    hadronicCheckbox_->setChecked(config.hadronicEnabled);
    
    int index = standardListCombo_->findData(QString::fromStdString(config.standardList));
    if (index >= 0) {
        standardListCombo_->setCurrentIndex(index);
    }
}

PhysicsConfig PhysicsPanel::getConfig() const {
    PhysicsConfig config;
    config.emEnabled = emCheckbox_->isChecked();
    config.decayEnabled = decayCheckbox_->isChecked();
    config.opticalEnabled = opticalCheckbox_->isChecked();
    config.hadronicEnabled = hadronicCheckbox_->isChecked();
    config.standardList = standardListCombo_->currentData().toString().toStdString();
    return config;
}

void PhysicsPanel::onCheckboxChanged() {
    updatePreview();
    emit configChanged();
}

void PhysicsPanel::onStandardListChanged() {
    updatePreview();
    emit configChanged();
}

void PhysicsPanel::updatePreview() {
    PhysicsConfig config = getConfig();
    
    QStringList enabledPhysics;
    if (config.emEnabled) enabledPhysics << "EM";
    if (config.decayEnabled) enabledPhysics << "Decay";
    if (config.opticalEnabled) enabledPhysics << "Optical";
    if (config.hadronicEnabled) enabledPhysics << "Hadronic";
    
    QString preview = QString("Physics List: <b>%1</b><br>")
                      .arg(QString::fromStdString(config.standardList));
    
    if (enabledPhysics.isEmpty()) {
        preview += "No physics enabled";
    } else {
        preview += "Enabled: " + enabledPhysics.join(", ");
    }
    
    previewLabel_->setText(preview);
}

} // namespace geantcad

