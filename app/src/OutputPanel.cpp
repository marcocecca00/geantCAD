#include "OutputPanel.hh"
#include <QLabel>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>

namespace geantcad {

OutputPanel::OutputPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

OutputPanel::~OutputPanel() {
}

void OutputPanel::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // ROOT Output Group
    QGroupBox* rootGroup = new QGroupBox("ROOT Output", this);
    QVBoxLayout* rootLayout = new QVBoxLayout(rootGroup);
    
    rootEnabledCheck_ = new QCheckBox("Enable ROOT Output", this);
    connect(rootEnabledCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    rootLayout->addWidget(rootEnabledCheck_);
    
    QHBoxLayout* fileLayout = new QHBoxLayout();
    fileLayout->addWidget(new QLabel("File:", this));
    rootFilePathEdit_ = new QLineEdit("output.root", this);
    fileLayout->addWidget(rootFilePathEdit_);
    browseButton_ = new QPushButton("Browse...", this);
    connect(browseButton_, &QPushButton::clicked, this, &OutputPanel::onBrowseFile);
    fileLayout->addWidget(browseButton_);
    rootLayout->addLayout(fileLayout);
    
    layout->addWidget(rootGroup);
    
    // Schema
    QGroupBox* schemaGroup = new QGroupBox("Schema", this);
    QVBoxLayout* schemaLayout = new QVBoxLayout(schemaGroup);
    
    schemaCombo_ = new QComboBox(this);
    schemaCombo_->addItem("Event Summary", static_cast<int>(OutputConfig::Schema::EventSummary));
    schemaCombo_->addItem("Step Hits", static_cast<int>(OutputConfig::Schema::StepHits));
    schemaCombo_->addItem("Custom", static_cast<int>(OutputConfig::Schema::Custom));
    connect(schemaCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutputPanel::onSchemaChanged);
    schemaLayout->addWidget(schemaCombo_);
    
    layout->addWidget(schemaGroup);
    
    // Fields
    fieldsGroup_ = new QGroupBox("Fields", this);
    QVBoxLayout* fieldsLayout = new QVBoxLayout(fieldsGroup_);
    
    fieldXCheck_ = new QCheckBox("x", this);
    fieldYCheck_ = new QCheckBox("y", this);
    fieldZCheck_ = new QCheckBox("z", this);
    fieldEdepCheck_ = new QCheckBox("edep", this);
    fieldEventIdCheck_ = new QCheckBox("event_id", this);
    fieldTrackIdCheck_ = new QCheckBox("track_id", this);
    fieldVolumeNameCheck_ = new QCheckBox("volume_name", this);
    fieldTimeCheck_ = new QCheckBox("time", this);
    fieldKineticEnergyCheck_ = new QCheckBox("kinetic_energy", this);
    
    connect(fieldXCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    connect(fieldYCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    connect(fieldZCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    connect(fieldEdepCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    connect(fieldEventIdCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    connect(fieldTrackIdCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    connect(fieldVolumeNameCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    connect(fieldTimeCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    connect(fieldKineticEnergyCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    
    fieldsLayout->addWidget(fieldXCheck_);
    fieldsLayout->addWidget(fieldYCheck_);
    fieldsLayout->addWidget(fieldZCheck_);
    fieldsLayout->addWidget(fieldEdepCheck_);
    fieldsLayout->addWidget(fieldEventIdCheck_);
    fieldsLayout->addWidget(fieldTrackIdCheck_);
    fieldsLayout->addWidget(fieldVolumeNameCheck_);
    fieldsLayout->addWidget(fieldTimeCheck_);
    fieldsLayout->addWidget(fieldKineticEnergyCheck_);
    
    layout->addWidget(fieldsGroup_);
    
    // Mode
    QGroupBox* modeGroup = new QGroupBox("Output Mode", this);
    QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);
    
    modeGroup_ = new QButtonGroup(this);
    perEventRadio_ = new QRadioButton("Per-event", this);
    perStepRadio_ = new QRadioButton("Per-step", this);
    perEventRadio_->setChecked(true);
    
    modeGroup_->addButton(perEventRadio_, 0);
    modeGroup_->addButton(perStepRadio_, 1);
    connect(modeGroup_, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &OutputPanel::onModeChanged);
    
    modeLayout->addWidget(perEventRadio_);
    modeLayout->addWidget(perStepRadio_);
    
    layout->addWidget(modeGroup);
    
    // Options
    QGroupBox* optionsGroup = new QGroupBox("Options", this);
    QFormLayout* optionsLayout = new QFormLayout(optionsGroup);
    
    saveFrequencySpin_ = new QSpinBox(this);
    saveFrequencySpin_->setRange(1, 10000);
    saveFrequencySpin_->setValue(1);
    connect(saveFrequencySpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OutputPanel::onCheckboxChanged);
    optionsLayout->addRow("Save Frequency:", saveFrequencySpin_);
    
    csvFallbackCheck_ = new QCheckBox("Fallback to CSV if ROOT unavailable", this);
    csvFallbackCheck_->setChecked(true);
    connect(csvFallbackCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    optionsLayout->addRow("", csvFallbackCheck_);
    
    compressionCheck_ = new QCheckBox("Enable Compression", this);
    connect(compressionCheck_, &QCheckBox::toggled, this, &OutputPanel::onCheckboxChanged);
    optionsLayout->addRow("", compressionCheck_);
    
    layout->addWidget(optionsGroup);
    layout->addStretch();
}

void OutputPanel::setConfig(const OutputConfig& config) {
    rootEnabledCheck_->setChecked(config.rootEnabled);
    rootFilePathEdit_->setText(QString::fromStdString(config.rootFilePath));
    
    int schemaIndex = schemaCombo_->findData(static_cast<int>(config.schema));
    if (schemaIndex >= 0) {
        schemaCombo_->setCurrentIndex(schemaIndex);
    }
    
    fieldXCheck_->setChecked(config.fieldX);
    fieldYCheck_->setChecked(config.fieldY);
    fieldZCheck_->setChecked(config.fieldZ);
    fieldEdepCheck_->setChecked(config.fieldEdep);
    fieldEventIdCheck_->setChecked(config.fieldEventId);
    fieldTrackIdCheck_->setChecked(config.fieldTrackId);
    fieldVolumeNameCheck_->setChecked(config.fieldVolumeName);
    fieldTimeCheck_->setChecked(config.fieldTime);
    fieldKineticEnergyCheck_->setChecked(config.fieldKineticEnergy);
    
    perEventRadio_->setChecked(config.perEvent);
    perStepRadio_->setChecked(!config.perEvent);
    
    saveFrequencySpin_->setValue(config.saveFrequency);
    csvFallbackCheck_->setChecked(config.csvFallback);
    compressionCheck_->setChecked(config.compression);
}

OutputConfig OutputPanel::getConfig() const {
    OutputConfig config;
    config.rootEnabled = rootEnabledCheck_->isChecked();
    config.rootFilePath = rootFilePathEdit_->text().toStdString();
    config.schema = static_cast<OutputConfig::Schema>(schemaCombo_->currentData().toInt());
    config.fieldX = fieldXCheck_->isChecked();
    config.fieldY = fieldYCheck_->isChecked();
    config.fieldZ = fieldZCheck_->isChecked();
    config.fieldEdep = fieldEdepCheck_->isChecked();
    config.fieldEventId = fieldEventIdCheck_->isChecked();
    config.fieldTrackId = fieldTrackIdCheck_->isChecked();
    config.fieldVolumeName = fieldVolumeNameCheck_->isChecked();
    config.fieldTime = fieldTimeCheck_->isChecked();
    config.fieldKineticEnergy = fieldKineticEnergyCheck_->isChecked();
    config.perEvent = perEventRadio_->isChecked();
    config.saveFrequency = saveFrequencySpin_->value();
    config.csvFallback = csvFallbackCheck_->isChecked();
    config.compression = compressionCheck_->isChecked();
    return config;
}

void OutputPanel::onCheckboxChanged() {
    emit configChanged();
}

void OutputPanel::onSchemaChanged() {
    emit configChanged();
}

void OutputPanel::onBrowseFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Select ROOT Output File", 
                                                     rootFilePathEdit_->text(),
                                                     "ROOT Files (*.root);;All Files (*)");
    if (!fileName.isEmpty()) {
        rootFilePathEdit_->setText(fileName);
        emit configChanged();
    }
}

void OutputPanel::onModeChanged() {
    emit configChanged();
}

} // namespace geantcad

