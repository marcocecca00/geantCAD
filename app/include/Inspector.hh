#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include "CollapsibleGroupBox.hh"
#include "../../core/include/VolumeNode.hh"
#include "../../core/include/CommandStack.hh"
#include "../../core/include/Material.hh"
#include <QLabel>
#include <memory>

namespace geantcad {

class Inspector : public QWidget {
    Q_OBJECT

public:
    explicit Inspector(QWidget* parent = nullptr);
    
    void setNode(VolumeNode* node);
    void clear();
    void setCommandStack(CommandStack* commandStack) { commandStack_ = commandStack; }

signals:
    void nodeChanged(VolumeNode* node);

private slots:
    void onTransformChanged();
    void onNameChanged();
    void onMaterialChanged();
    void onSDChanged();
    void onOpticalChanged();
    void onOpticalPresetChanged();
    void onShapeParamsChanged();

private:
    void setupUI();
    void updateUI();
    void updateShapeUI();
    void hideAllShapeWidgets();
    void updateMaterialColorPreview(std::shared_ptr<Material> material);
    
    VolumeNode* currentNode_;
    CommandStack* commandStack_ = nullptr;
    
    // Transform
    QDoubleSpinBox* posX_, *posY_, *posZ_;
    QDoubleSpinBox* rotX_, *rotY_, *rotZ_;
    
    // Name
    QLineEdit* nameEdit_;
    
    // Material
    QComboBox* materialCombo_;
    QLabel* materialColorPreview_;
    
    // Geometry (Shape Parameters)
    CollapsibleGroupBox* geometryGroup_;
    QWidget* geometryContent_;
    QFormLayout* geometryLayout_;
    
    // Box parameters
    QDoubleSpinBox* boxX_, *boxY_, *boxZ_;
    
    // Tube parameters
    QDoubleSpinBox* tubeRmin_, *tubeRmax_, *tubeDz_;
    
    // Sphere parameters
    QDoubleSpinBox* sphereRmin_, *sphereRmax_;
    
    // Cone parameters
    QDoubleSpinBox* coneRmin1_, *coneRmax1_, *coneRmin2_, *coneRmax2_, *coneDz_;
    
    // Trd parameters
    QDoubleSpinBox* trdDx1_, *trdDx2_, *trdDy1_, *trdDy2_, *trdDz_;
    
    // Sensitive Detector
    QGroupBox* sdGroup_;
    QCheckBox* sdEnabledCheck_;
    QComboBox* sdTypeCombo_;
    QLineEdit* sdCollectionEdit_;
    QSpinBox* sdCopyNumberSpin_;
    
    // Optical Surface
    QGroupBox* opticalGroup_;
    QCheckBox* opticalEnabledCheck_;
    QComboBox* opticalModelCombo_;
    QComboBox* opticalFinishCombo_;
    QComboBox* opticalPresetCombo_;
    QDoubleSpinBox* opticalReflectivitySpin_;
    QDoubleSpinBox* opticalSigmaAlphaSpin_;
    
    bool updating_;
    bool updatingShape_;
};

} // namespace geantcad

