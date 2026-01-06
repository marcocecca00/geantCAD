#include "Inspector.hh"
#include "CollapsibleGroupBox.hh"
#include "../../core/include/Material.hh"
#include "../../core/include/Shape.hh"
#include "../../core/include/Command.hh"
#include "../../core/include/VolumeNode.hh"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QColor>
#include <cmath>

namespace geantcad {

Inspector::Inspector(QWidget* parent)
    : QWidget(parent)
    , currentNode_(nullptr)
    , updating_(false)
    , updatingShape_(false)
{
    setupUI();
}

void Inspector::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Name
    QGroupBox* nameGroup = new QGroupBox("Name", this);
    QVBoxLayout* nameLayout = new QVBoxLayout(nameGroup);
    nameEdit_ = new QLineEdit(this);
    connect(nameEdit_, &QLineEdit::textChanged, this, &Inspector::onNameChanged);
    nameLayout->addWidget(nameEdit_);
    layout->addWidget(nameGroup);
    
    // Transform (collapsible, expanded by default)
    QWidget* transformContent = new QWidget(this);
    QFormLayout* transformLayout = new QFormLayout(transformContent);
    transformLayout->setContentsMargins(8, 8, 8, 8);
    
    posX_ = new QDoubleSpinBox(this);
    posY_ = new QDoubleSpinBox(this);
    posZ_ = new QDoubleSpinBox(this);
    posX_->setRange(-10000, 10000);
    posY_->setRange(-10000, 10000);
    posZ_->setRange(-10000, 10000);
    posX_->setSuffix(" mm");
    posY_->setSuffix(" mm");
    posZ_->setSuffix(" mm");
    
    rotX_ = new QDoubleSpinBox(this);
    rotY_ = new QDoubleSpinBox(this);
    rotZ_ = new QDoubleSpinBox(this);
    rotX_->setRange(-360, 360);
    rotY_->setRange(-360, 360);
    rotZ_->setRange(-360, 360);
    rotX_->setSuffix(" °");
    rotY_->setSuffix(" °");
    rotZ_->setSuffix(" °");
    
    transformLayout->addRow("Position X:", posX_);
    transformLayout->addRow("Position Y:", posY_);
    transformLayout->addRow("Position Z:", posZ_);
    transformLayout->addRow("Rotation X:", rotX_);
    transformLayout->addRow("Rotation Y:", rotY_);
    transformLayout->addRow("Rotation Z:", rotZ_);
    
    connect(posX_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onTransformChanged);
    connect(posY_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onTransformChanged);
    connect(posZ_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onTransformChanged);
    connect(rotX_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onTransformChanged);
    connect(rotY_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onTransformChanged);
    connect(rotZ_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onTransformChanged);
    
    CollapsibleGroupBox* transformCollapsible = new CollapsibleGroupBox("Transform", this);
    transformCollapsible->setContent(transformContent);
    transformCollapsible->setExpanded(true);
    layout->addWidget(transformCollapsible);
    
    // Material (collapsible, expanded by default)
    QWidget* materialContent = new QWidget(this);
    QVBoxLayout* materialLayout = new QVBoxLayout(materialContent);
    materialLayout->setContentsMargins(8, 8, 8, 8);
    
    QHBoxLayout* materialRowLayout = new QHBoxLayout();
    materialCombo_ = new QComboBox(this);
    connect(materialCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Inspector::onMaterialChanged);
    materialCombo_->addItem("Air", "G4_AIR");
    materialCombo_->addItem("Water", "G4_WATER");
    materialCombo_->addItem("Silicon", "G4_Si");
    materialCombo_->addItem("Lead", "G4_Pb");
    materialCombo_->addItem("Vacuum", "G4_Galactic");
    materialRowLayout->addWidget(materialCombo_);
    
    // Color preview
    materialColorPreview_ = new QLabel(this);
    materialColorPreview_->setMinimumSize(30, 30);
    materialColorPreview_->setMaximumSize(30, 30);
    materialColorPreview_->setStyleSheet("border: 1px solid #404040; border-radius: 3px;");
    materialColorPreview_->setToolTip("Material color preview");
    materialRowLayout->addWidget(materialColorPreview_);
    
    materialLayout->addLayout(materialRowLayout);
    
    CollapsibleGroupBox* materialCollapsible = new CollapsibleGroupBox("Material", this);
    materialCollapsible->setContent(materialContent);
    materialCollapsible->setExpanded(true);
    layout->addWidget(materialCollapsible);
    
    // Geometry (Shape Parameters) - collapsible, expanded by default
    geometryContent_ = new QWidget(this);
    geometryLayout_ = new QFormLayout(geometryContent_);
    geometryLayout_->setContentsMargins(8, 8, 8, 8);
    
    // Box parameters
    boxX_ = new QDoubleSpinBox(this);
    boxY_ = new QDoubleSpinBox(this);
    boxZ_ = new QDoubleSpinBox(this);
    boxX_->setRange(0.1, 10000);
    boxY_->setRange(0.1, 10000);
    boxZ_->setRange(0.1, 10000);
    boxX_->setSuffix(" mm");
    boxY_->setSuffix(" mm");
    boxZ_->setSuffix(" mm");
    boxX_->setDecimals(2);
    boxY_->setDecimals(2);
    boxZ_->setDecimals(2);
    connect(boxX_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(boxY_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(boxZ_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    
    // Tube parameters
    tubeRmin_ = new QDoubleSpinBox(this);
    tubeRmax_ = new QDoubleSpinBox(this);
    tubeDz_ = new QDoubleSpinBox(this);
    tubeRmin_->setRange(0.0, 10000);
    tubeRmax_->setRange(0.1, 10000);
    tubeDz_->setRange(0.1, 10000);
    tubeRmin_->setSuffix(" mm");
    tubeRmax_->setSuffix(" mm");
    tubeDz_->setSuffix(" mm");
    tubeRmin_->setDecimals(2);
    tubeRmax_->setDecimals(2);
    tubeDz_->setDecimals(2);
    connect(tubeRmin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(tubeRmax_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(tubeDz_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    
    // Sphere parameters
    sphereRmin_ = new QDoubleSpinBox(this);
    sphereRmax_ = new QDoubleSpinBox(this);
    sphereRmin_->setRange(0.0, 10000);
    sphereRmax_->setRange(0.1, 10000);
    sphereRmin_->setSuffix(" mm");
    sphereRmax_->setSuffix(" mm");
    sphereRmin_->setDecimals(2);
    sphereRmax_->setDecimals(2);
    connect(sphereRmin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(sphereRmax_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    
    // Cone parameters
    coneRmin1_ = new QDoubleSpinBox(this);
    coneRmax1_ = new QDoubleSpinBox(this);
    coneRmin2_ = new QDoubleSpinBox(this);
    coneRmax2_ = new QDoubleSpinBox(this);
    coneDz_ = new QDoubleSpinBox(this);
    coneRmin1_->setRange(0.0, 10000);
    coneRmax1_->setRange(0.1, 10000);
    coneRmin2_->setRange(0.0, 10000);
    coneRmax2_->setRange(0.1, 10000);
    coneDz_->setRange(0.1, 10000);
    coneRmin1_->setSuffix(" mm");
    coneRmax1_->setSuffix(" mm");
    coneRmin2_->setSuffix(" mm");
    coneRmax2_->setSuffix(" mm");
    coneDz_->setSuffix(" mm");
    coneRmin1_->setDecimals(2);
    coneRmax1_->setDecimals(2);
    coneRmin2_->setDecimals(2);
    coneRmax2_->setDecimals(2);
    coneDz_->setDecimals(2);
    connect(coneRmin1_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(coneRmax1_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(coneRmin2_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(coneRmax2_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(coneDz_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    
    // Trd parameters
    trdDx1_ = new QDoubleSpinBox(this);
    trdDx2_ = new QDoubleSpinBox(this);
    trdDy1_ = new QDoubleSpinBox(this);
    trdDy2_ = new QDoubleSpinBox(this);
    trdDz_ = new QDoubleSpinBox(this);
    trdDx1_->setRange(0.1, 10000);
    trdDx2_->setRange(0.1, 10000);
    trdDy1_->setRange(0.1, 10000);
    trdDy2_->setRange(0.1, 10000);
    trdDz_->setRange(0.1, 10000);
    trdDx1_->setSuffix(" mm");
    trdDx2_->setSuffix(" mm");
    trdDy1_->setSuffix(" mm");
    trdDy2_->setSuffix(" mm");
    trdDz_->setSuffix(" mm");
    trdDx1_->setDecimals(2);
    trdDx2_->setDecimals(2);
    trdDy1_->setDecimals(2);
    trdDy2_->setDecimals(2);
    trdDz_->setDecimals(2);
    connect(trdDx1_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(trdDx2_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(trdDy1_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(trdDy2_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    connect(trdDz_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onShapeParamsChanged);
    
    geometryGroup_ = new CollapsibleGroupBox("Geometry", this);
    geometryGroup_->setContent(geometryContent_);
    geometryGroup_->setExpanded(true);
    layout->addWidget(geometryGroup_);
    
    // Initially hide all shape widgets
    hideAllShapeWidgets();
    
    // Sensitive Detector (collapsible, collapsed by default)
    QWidget* sdContent = new QWidget(this);
    QVBoxLayout* sdLayout = new QVBoxLayout(sdContent);
    sdLayout->setContentsMargins(8, 8, 8, 8);
    
    sdEnabledCheck_ = new QCheckBox("Enable Sensitive Detector", this);
    connect(sdEnabledCheck_, &QCheckBox::toggled, this, &Inspector::onSDChanged);
    connect(sdEnabledCheck_, &QCheckBox::toggled, this, [this](bool enabled) {
        sdTypeCombo_->setEnabled(enabled);
        sdCollectionEdit_->setEnabled(enabled);
        sdCopyNumberSpin_->setEnabled(enabled);
    });
    sdLayout->addWidget(sdEnabledCheck_);
    
    QFormLayout* sdFormLayout = new QFormLayout();
    
    sdTypeCombo_ = new QComboBox(this);
    sdTypeCombo_->addItem("Calorimeter", "calorimeter");
    sdTypeCombo_->addItem("Tracker", "tracker");
    sdTypeCombo_->addItem("Optical", "optical");
    connect(sdTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Inspector::onSDChanged);
    sdFormLayout->addRow("Type:", sdTypeCombo_);
    
    sdCollectionEdit_ = new QLineEdit(this);
    connect(sdCollectionEdit_, &QLineEdit::textChanged, this, &Inspector::onSDChanged);
    sdFormLayout->addRow("Collection Name:", sdCollectionEdit_);
    
    sdCopyNumberSpin_ = new QSpinBox(this);
    sdCopyNumberSpin_->setRange(0, 10000);
    connect(sdCopyNumberSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &Inspector::onSDChanged);
    sdFormLayout->addRow("Copy Number:", sdCopyNumberSpin_);
    
    sdLayout->addLayout(sdFormLayout);
    
    CollapsibleGroupBox* sdCollapsible = new CollapsibleGroupBox("Sensitive Detector", this);
    sdCollapsible->setContent(sdContent);
    sdCollapsible->setExpanded(false); // Collapsed by default
    layout->addWidget(sdCollapsible);
    
    // Store reference for enable/disable logic
    sdGroup_ = nullptr; // Not used anymore, but keep for compatibility
    
    // Initially disable SD controls
    sdTypeCombo_->setEnabled(false);
    sdCollectionEdit_->setEnabled(false);
    sdCopyNumberSpin_->setEnabled(false);
    
    // Optical Surface (collapsible, collapsed by default)
    QWidget* opticalContent = new QWidget(this);
    QVBoxLayout* opticalLayout = new QVBoxLayout(opticalContent);
    opticalLayout->setContentsMargins(8, 8, 8, 8);
    
    opticalEnabledCheck_ = new QCheckBox("Enable Optical Surface", this);
    connect(opticalEnabledCheck_, &QCheckBox::toggled, this, &Inspector::onOpticalChanged);
    connect(opticalEnabledCheck_, &QCheckBox::toggled, this, [this](bool enabled) {
        opticalModelCombo_->setEnabled(enabled);
        opticalFinishCombo_->setEnabled(enabled);
        opticalPresetCombo_->setEnabled(enabled);
        opticalReflectivitySpin_->setEnabled(enabled);
        opticalSigmaAlphaSpin_->setEnabled(enabled);
    });
    opticalLayout->addWidget(opticalEnabledCheck_);
    
    QFormLayout* opticalFormLayout = new QFormLayout();
    
    opticalModelCombo_ = new QComboBox(this);
    opticalModelCombo_->addItem("Unified", "unified");
    opticalModelCombo_->addItem("GLISUR", "glisur");
    opticalModelCombo_->addItem("Dichroic", "dichroic");
    connect(opticalModelCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Inspector::onOpticalChanged);
    opticalFormLayout->addRow("Model:", opticalModelCombo_);
    
    opticalFinishCombo_ = new QComboBox(this);
    opticalFinishCombo_->addItem("Polished", "polished");
    opticalFinishCombo_->addItem("Ground", "ground");
    opticalFinishCombo_->addItem("Polished Front Painted", "polishedfrontpainted");
    opticalFinishCombo_->addItem("Polished Back Painted", "polishedbackpainted");
    opticalFinishCombo_->addItem("Ground Front Painted", "groundfrontpainted");
    opticalFinishCombo_->addItem("Ground Back Painted", "groundbackpainted");
    connect(opticalFinishCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Inspector::onOpticalChanged);
    opticalFormLayout->addRow("Finish:", opticalFinishCombo_);
    
    opticalPresetCombo_ = new QComboBox(this);
    opticalPresetCombo_->addItem("None", "");
    opticalPresetCombo_->addItem("Tyvek", "tyvek");
    opticalPresetCombo_->addItem("ESR (Enhanced Specular Reflector)", "esr");
    opticalPresetCombo_->addItem("Black", "black");
    connect(opticalPresetCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Inspector::onOpticalPresetChanged);
    opticalFormLayout->addRow("Preset:", opticalPresetCombo_);
    
    opticalReflectivitySpin_ = new QDoubleSpinBox(this);
    opticalReflectivitySpin_->setRange(0.0, 1.0);
    opticalReflectivitySpin_->setSingleStep(0.01);
    opticalReflectivitySpin_->setDecimals(3);
    connect(opticalReflectivitySpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onOpticalChanged);
    opticalFormLayout->addRow("Reflectivity:", opticalReflectivitySpin_);
    
    opticalSigmaAlphaSpin_ = new QDoubleSpinBox(this);
    opticalSigmaAlphaSpin_->setRange(0.0, 90.0);
    opticalSigmaAlphaSpin_->setSingleStep(0.1);
    opticalSigmaAlphaSpin_->setSuffix(" °");
    connect(opticalSigmaAlphaSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Inspector::onOpticalChanged);
    opticalFormLayout->addRow("Sigma Alpha:", opticalSigmaAlphaSpin_);
    
    opticalLayout->addLayout(opticalFormLayout);
    
    CollapsibleGroupBox* opticalCollapsible = new CollapsibleGroupBox("Optical Surface", this);
    opticalCollapsible->setContent(opticalContent);
    opticalCollapsible->setExpanded(false); // Collapsed by default
    layout->addWidget(opticalCollapsible);
    
    // Store reference for enable/disable logic
    opticalGroup_ = nullptr; // Not used anymore, but keep for compatibility
    
    // Initially disable optical controls
    opticalModelCombo_->setEnabled(false);
    opticalFinishCombo_->setEnabled(false);
    opticalPresetCombo_->setEnabled(false);
    opticalReflectivitySpin_->setEnabled(false);
    opticalSigmaAlphaSpin_->setEnabled(false);
    
    layout->addStretch();
}

void Inspector::setNode(VolumeNode* node) {
    currentNode_ = node;
    updateUI();
}

void Inspector::clear() {
    currentNode_ = nullptr;
    updating_ = true;
    nameEdit_->clear();
    posX_->setValue(0);
    posY_->setValue(0);
    posZ_->setValue(0);
    rotX_->setValue(0);
    rotY_->setValue(0);
    rotZ_->setValue(0);
    updating_ = false;
}

void Inspector::updateUI() {
    updating_ = true;
    
    if (currentNode_) {
        nameEdit_->setText(QString::fromStdString(currentNode_->getName()));
        
        auto& t = currentNode_->getTransform();
        auto pos = t.getTranslation();
        posX_->setValue(pos.x());
        posY_->setValue(pos.y());
        posZ_->setValue(pos.z());
        
        // Extract Euler angles from quaternion (ZYX convention: roll, pitch, yaw)
        auto rot = t.getRotation();
        // QQuaternion stores as (x, y, z, scalar) but constructor is (scalar, x, y, z)
        float w = rot.scalar();
        float x = rot.x();
        float y = rot.y();
        float z = rot.z();
        
        // Convert to Euler angles (ZYX convention: roll around X, pitch around Y, yaw around Z)
        // Roll (X-axis rotation)
        float sinr_cosp = 2.0f * (w * x + y * z);
        float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
        float roll = atan2f(sinr_cosp, cosr_cosp) * 180.0f / M_PI;
        
        // Pitch (Y-axis rotation)
        float sinp = 2.0f * (w * y - z * x);
        float pitch;
        if (fabsf(sinp) >= 1.0f) {
            pitch = copysignf(M_PI / 2.0f, sinp) * 180.0f / M_PI;
        } else {
            pitch = asinf(sinp) * 180.0f / M_PI;
        }
        
        // Yaw (Z-axis rotation)
        float siny_cosp = 2.0f * (w * z + x * y);
        float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
        float yaw = atan2f(siny_cosp, cosy_cosp) * 180.0f / M_PI;
        
        rotX_->setValue(roll);
        rotY_->setValue(pitch);
        rotZ_->setValue(yaw);
        
        // Material
        if (auto material = currentNode_->getMaterial()) {
            QString nistName = QString::fromStdString(material->getNistName());
            int index = materialCombo_->findData(nistName);
            if (index >= 0) {
                materialCombo_->setCurrentIndex(index);
            }
            // Update color preview
            updateMaterialColorPreview(material);
        } else {
            materialColorPreview_->setStyleSheet("border: 1px solid #404040; border-radius: 3px; background-color: #2b2b2b;");
        }
        
        // Sensitive Detector
        auto& sdConfig = currentNode_->getSDConfig();
        sdEnabledCheck_->setChecked(sdConfig.enabled);
        
        if (sdConfig.enabled) {
            int typeIndex = sdTypeCombo_->findData(QString::fromStdString(sdConfig.type));
            if (typeIndex >= 0) sdTypeCombo_->setCurrentIndex(typeIndex);
            
            QString collectionName = QString::fromStdString(sdConfig.collectionName);
            sdCollectionEdit_->setText(collectionName);
            sdCopyNumberSpin_->setValue(sdConfig.copyNumber);
        }
        
        sdTypeCombo_->setEnabled(sdConfig.enabled);
        sdCollectionEdit_->setEnabled(sdConfig.enabled);
        sdCopyNumberSpin_->setEnabled(sdConfig.enabled);
        
        // Optical Surface
        auto& opticalConfig = currentNode_->getOpticalConfig();
        opticalEnabledCheck_->setChecked(opticalConfig.enabled);
        
        if (opticalConfig.enabled) {
            int modelIndex = opticalModelCombo_->findData(QString::fromStdString(opticalConfig.model));
            if (modelIndex >= 0) opticalModelCombo_->setCurrentIndex(modelIndex);
            
            int finishIndex = opticalFinishCombo_->findData(QString::fromStdString(opticalConfig.finish));
            if (finishIndex >= 0) opticalFinishCombo_->setCurrentIndex(finishIndex);
            
            int presetIndex = opticalPresetCombo_->findData(QString::fromStdString(opticalConfig.preset));
            if (presetIndex >= 0) opticalPresetCombo_->setCurrentIndex(presetIndex);
            
            opticalReflectivitySpin_->setValue(opticalConfig.reflectivity);
            opticalSigmaAlphaSpin_->setValue(opticalConfig.sigmaAlpha);
        }
        
        opticalModelCombo_->setEnabled(opticalConfig.enabled);
        opticalFinishCombo_->setEnabled(opticalConfig.enabled);
        opticalPresetCombo_->setEnabled(opticalConfig.enabled);
        opticalReflectivitySpin_->setEnabled(opticalConfig.enabled);
        opticalSigmaAlphaSpin_->setEnabled(opticalConfig.enabled);
        
        // Update shape UI
        updateShapeUI();
    } else {
        clear();
    }
    
    updating_ = false;
}

void Inspector::onTransformChanged() {
    if (updating_ || !currentNode_) return;
    
    Transform newTransform = currentNode_->getTransform();
    
    // Update position
    QVector3D pos(posX_->value(), posY_->value(), posZ_->value());
    newTransform.setTranslation(pos);
    
    // Update rotation (Euler angles in degrees)
    newTransform.setRotationEuler(rotX_->value(), rotY_->value(), rotZ_->value());
    
    if (commandStack_) {
        auto cmd = std::make_unique<TransformVolumeCommand>(currentNode_, newTransform);
        commandStack_->execute(std::move(cmd));
    } else {
        currentNode_->getTransform() = newTransform;
    }
    
    emit nodeChanged(currentNode_);
    
    // Refresh viewport to show changes
    // Note: We can't access viewport from here, so we emit signal
    // MainWindow will handle the refresh
}

void Inspector::onNameChanged() {
    if (updating_ || !currentNode_) return;
    
    std::string newName = nameEdit_->text().toStdString();
    if (commandStack_) {
        auto cmd = std::make_unique<ModifyNameCommand>(currentNode_, newName);
        commandStack_->execute(std::move(cmd));
    } else {
        currentNode_->setName(newName);
    }
    emit nodeChanged(currentNode_);
}

void Inspector::onMaterialChanged() {
    if (updating_ || !currentNode_) return;
    
    QString nistName = materialCombo_->currentData().toString();
    auto material = Material::makeNist(nistName.toStdString());
    
    if (commandStack_) {
        auto cmd = std::make_unique<ModifyMaterialCommand>(currentNode_, material);
        commandStack_->execute(std::move(cmd));
    } else {
        currentNode_->setMaterial(material);
    }
    
    // Update color preview
    updateMaterialColorPreview(material);
    
    emit nodeChanged(currentNode_);
}

void Inspector::updateMaterialColorPreview(std::shared_ptr<Material> material) {
    if (!material || !materialColorPreview_) return;
    
    const auto& visual = material->getVisual();
    QColor color;
    color.setRgbF(visual.r, visual.g, visual.b, visual.a);
    
    QString styleSheet = QString("border: 1px solid #404040; border-radius: 3px; background-color: %1;")
                        .arg(color.name());
    materialColorPreview_->setStyleSheet(styleSheet);
    
    // Update tooltip with color info
    QString tooltip = QString("Material: %1\nColor: RGB(%2, %3, %4)")
                     .arg(QString::fromStdString(material->getName()))
                     .arg(static_cast<int>(visual.r * 255))
                     .arg(static_cast<int>(visual.g * 255))
                     .arg(static_cast<int>(visual.b * 255));
    materialColorPreview_->setToolTip(tooltip);
}

void Inspector::onSDChanged() {
    if (updating_ || !currentNode_) return;
    
    SensitiveDetectorConfig newConfig = currentNode_->getSDConfig();
    newConfig.enabled = sdEnabledCheck_->isChecked();
    
    if (newConfig.enabled) {
        newConfig.type = sdTypeCombo_->currentData().toString().toStdString();
        QString collectionName = sdCollectionEdit_->text().trimmed();
        if (collectionName.isEmpty()) {
            // Auto-generate collection name from node name
            collectionName = QString::fromStdString(currentNode_->getName()) + "HitsCollection";
            sdCollectionEdit_->setText(collectionName);
        }
        newConfig.collectionName = collectionName.toStdString();
        newConfig.copyNumber = sdCopyNumberSpin_->value();
    }
    
    if (commandStack_) {
        auto cmd = std::make_unique<ModifySDConfigCommand>(currentNode_, newConfig);
        commandStack_->execute(std::move(cmd));
    } else {
        currentNode_->getSDConfig() = newConfig;
    }
    
    emit nodeChanged(currentNode_);
}

void Inspector::onOpticalChanged() {
    if (updating_ || !currentNode_) return;
    
    OpticalSurfaceConfig newConfig = currentNode_->getOpticalConfig();
    newConfig.enabled = opticalEnabledCheck_->isChecked();
    
    if (newConfig.enabled) {
        newConfig.model = opticalModelCombo_->currentData().toString().toStdString();
        newConfig.finish = opticalFinishCombo_->currentData().toString().toStdString();
        newConfig.reflectivity = opticalReflectivitySpin_->value();
        newConfig.sigmaAlpha = opticalSigmaAlphaSpin_->value();
        newConfig.preset = opticalPresetCombo_->currentData().toString().toStdString();
    }
    
    if (commandStack_) {
        auto cmd = std::make_unique<ModifyOpticalConfigCommand>(currentNode_, newConfig);
        commandStack_->execute(std::move(cmd));
    } else {
        currentNode_->getOpticalConfig() = newConfig;
    }
    
    emit nodeChanged(currentNode_);
}

void Inspector::onOpticalPresetChanged() {
    if (updating_ || !currentNode_) return;
    
    QString preset = opticalPresetCombo_->currentData().toString();
    auto& opticalConfig = currentNode_->getOpticalConfig();
    opticalConfig.preset = preset.toStdString();
    
    if (!preset.isEmpty()) {
        // Apply preset values
        if (preset == "tyvek") {
            opticalConfig.reflectivity = 0.98;
            opticalConfig.sigmaAlpha = 2.5;
            opticalConfig.finish = "ground";
            opticalConfig.model = "unified";
        } else if (preset == "esr") {
            opticalConfig.reflectivity = 0.98;
            opticalConfig.sigmaAlpha = 0.0;
            opticalConfig.finish = "polished";
            opticalConfig.model = "unified";
        } else if (preset == "black") {
            opticalConfig.reflectivity = 0.0;
            opticalConfig.sigmaAlpha = 0.0;
            opticalConfig.finish = "ground";
            opticalConfig.model = "unified";
        }
        
        // Update UI to reflect preset values
        updating_ = true;
        int finishIndex = opticalFinishCombo_->findData(QString::fromStdString(opticalConfig.finish));
        if (finishIndex >= 0) opticalFinishCombo_->setCurrentIndex(finishIndex);
        opticalReflectivitySpin_->setValue(opticalConfig.reflectivity);
        opticalSigmaAlphaSpin_->setValue(opticalConfig.sigmaAlpha);
        updating_ = false;
    }
    
    emit nodeChanged(currentNode_);
}

void Inspector::hideAllShapeWidgets() {
    // Remove all rows from geometry layout
    while (geometryLayout_->rowCount() > 0) {
        geometryLayout_->removeRow(0);
    }
}

void Inspector::updateShapeUI() {
    if (!currentNode_ || !currentNode_->getShape()) {
        hideAllShapeWidgets();
        return;
    }
    
    updatingShape_ = true;
    hideAllShapeWidgets();
    
    Shape* shape = currentNode_->getShape();
    ShapeType type = shape->getType();
    
    switch (type) {
        case ShapeType::Box: {
            if (auto* params = shape->getParamsAs<BoxParams>()) {
                boxX_->setValue(params->x);
                boxY_->setValue(params->y);
                boxZ_->setValue(params->z);
                geometryLayout_->addRow("Half-length X:", boxX_);
                geometryLayout_->addRow("Half-length Y:", boxY_);
                geometryLayout_->addRow("Half-length Z:", boxZ_);
            }
            break;
        }
        case ShapeType::Tube: {
            if (auto* params = shape->getParamsAs<TubeParams>()) {
                tubeRmin_->setValue(params->rmin);
                tubeRmax_->setValue(params->rmax);
                tubeDz_->setValue(params->dz);
                // Update max for rmin to ensure rmax > rmin
                tubeRmin_->setMaximum(tubeRmax_->value() - 0.01);
                geometryLayout_->addRow("Inner Radius (Rmin):", tubeRmin_);
                geometryLayout_->addRow("Outer Radius (Rmax):", tubeRmax_);
                geometryLayout_->addRow("Half-height (Dz):", tubeDz_);
            }
            break;
        }
        case ShapeType::Sphere: {
            if (auto* params = shape->getParamsAs<SphereParams>()) {
                sphereRmin_->setValue(params->rmin);
                sphereRmax_->setValue(params->rmax);
                // Update max for rmin to ensure rmax > rmin
                sphereRmin_->setMaximum(sphereRmax_->value() - 0.01);
                geometryLayout_->addRow("Inner Radius (Rmin):", sphereRmin_);
                geometryLayout_->addRow("Outer Radius (Rmax):", sphereRmax_);
            }
            break;
        }
        case ShapeType::Cone: {
            if (auto* params = shape->getParamsAs<ConeParams>()) {
                coneRmin1_->setValue(params->rmin1);
                coneRmax1_->setValue(params->rmax1);
                coneRmin2_->setValue(params->rmin2);
                coneRmax2_->setValue(params->rmax2);
                coneDz_->setValue(params->dz);
                // Update max for rmin to ensure rmax > rmin
                coneRmin1_->setMaximum(coneRmax1_->value() - 0.01);
                coneRmin2_->setMaximum(coneRmax2_->value() - 0.01);
                geometryLayout_->addRow("Inner Radius -Z (Rmin1):", coneRmin1_);
                geometryLayout_->addRow("Outer Radius -Z (Rmax1):", coneRmax1_);
                geometryLayout_->addRow("Inner Radius +Z (Rmin2):", coneRmin2_);
                geometryLayout_->addRow("Outer Radius +Z (Rmax2):", coneRmax2_);
                geometryLayout_->addRow("Half-height (Dz):", coneDz_);
            }
            break;
        }
        case ShapeType::Trd: {
            if (auto* params = shape->getParamsAs<TrdParams>()) {
                trdDx1_->setValue(params->dx1);
                trdDx2_->setValue(params->dx2);
                trdDy1_->setValue(params->dy1);
                trdDy2_->setValue(params->dy2);
                trdDz_->setValue(params->dz);
                geometryLayout_->addRow("Half-length X at -Z (Dx1):", trdDx1_);
                geometryLayout_->addRow("Half-length X at +Z (Dx2):", trdDx2_);
                geometryLayout_->addRow("Half-length Y at -Z (Dy1):", trdDy1_);
                geometryLayout_->addRow("Half-length Y at +Z (Dy2):", trdDy2_);
                geometryLayout_->addRow("Half-height (Dz):", trdDz_);
            }
            break;
        }
        default:
            break;
    }
    
    updatingShape_ = false;
}

void Inspector::onShapeParamsChanged() {
    if (updatingShape_ || !currentNode_ || !currentNode_->getShape()) return;
    
    Shape* shape = currentNode_->getShape();
    ShapeType type = shape->getType();
    ShapeParams oldParams = shape->getParams();
    ShapeParams newParams = oldParams;
    
    bool valid = true;
    
    switch (type) {
        case ShapeType::Box: {
            if (auto* params = std::get_if<BoxParams>(&newParams)) {
                params->x = boxX_->value();
                params->y = boxY_->value();
                params->z = boxZ_->value();
                valid = params->x > 0 && params->y > 0 && params->z > 0;
            }
            break;
        }
        case ShapeType::Tube: {
            if (auto* params = std::get_if<TubeParams>(&newParams)) {
                params->rmin = tubeRmin_->value();
                params->rmax = tubeRmax_->value();
                params->dz = tubeDz_->value();
                valid = params->rmax > params->rmin && params->dz > 0;
                // Update rmin max when rmax changes
                if (tubeRmax_->hasFocus()) {
                    tubeRmin_->setMaximum(params->rmax - 0.01);
                }
            }
            break;
        }
        case ShapeType::Sphere: {
            if (auto* params = std::get_if<SphereParams>(&newParams)) {
                params->rmin = sphereRmin_->value();
                params->rmax = sphereRmax_->value();
                valid = params->rmax > params->rmin;
                // Update rmin max when rmax changes
                if (sphereRmax_->hasFocus()) {
                    sphereRmin_->setMaximum(params->rmax - 0.01);
                }
            }
            break;
        }
        case ShapeType::Cone: {
            if (auto* params = std::get_if<ConeParams>(&newParams)) {
                params->rmin1 = coneRmin1_->value();
                params->rmax1 = coneRmax1_->value();
                params->rmin2 = coneRmin2_->value();
                params->rmax2 = coneRmax2_->value();
                params->dz = coneDz_->value();
                valid = params->rmax1 > params->rmin1 && 
                        params->rmax2 > params->rmin2 && 
                        params->dz > 0;
                // Update rmin max when rmax changes
                if (coneRmax1_->hasFocus()) {
                    coneRmin1_->setMaximum(params->rmax1 - 0.01);
                }
                if (coneRmax2_->hasFocus()) {
                    coneRmin2_->setMaximum(params->rmax2 - 0.01);
                }
            }
            break;
        }
        case ShapeType::Trd: {
            if (auto* params = std::get_if<TrdParams>(&newParams)) {
                params->dx1 = trdDx1_->value();
                params->dx2 = trdDx2_->value();
                params->dy1 = trdDy1_->value();
                params->dy2 = trdDy2_->value();
                params->dz = trdDz_->value();
                valid = params->dx1 > 0 && params->dx2 > 0 && 
                        params->dy1 > 0 && params->dy2 > 0 && 
                        params->dz > 0;
            }
            break;
        }
        default:
            return;
    }
    
    if (!valid) {
        // Revert to old params if invalid
        updatingShape_ = true;
        updateShapeUI();
        updatingShape_ = false;
        return;
    }
    
    // Use command stack for undo/redo if available
    if (commandStack_) {
        auto cmd = std::make_unique<ModifyShapeCommand>(currentNode_, newParams);
        commandStack_->execute(std::move(cmd));
    } else {
        // Apply directly if no command stack
        shape->getParams() = newParams;
    }
    
    emit nodeChanged(currentNode_);
}

} // namespace geantcad

