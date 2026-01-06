#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include "../../core/include/PhysicsConfig.hh"

namespace geantcad {

class PhysicsPanel : public QWidget {
    Q_OBJECT

public:
    PhysicsPanel(QWidget* parent = nullptr);
    ~PhysicsPanel();
    
    void setConfig(const PhysicsConfig& config);
    PhysicsConfig getConfig() const;
    
signals:
    void configChanged();

private slots:
    void onCheckboxChanged();
    void onComboChanged();
    void onCutsChanged();

private:
    void setupUI();
    void updatePreview();
    void updateEnabledStates();
    
    // Physics toggles
    QCheckBox* emCheckbox_;
    QCheckBox* decayCheckbox_;
    QCheckBox* opticalCheckbox_;
    QCheckBox* hadronicCheckbox_;
    QCheckBox* ionCheckbox_;
    QCheckBox* radioactiveCheckbox_;
    QCheckBox* stepLimiterCheckbox_;
    
    // Physics options
    QComboBox* emOptionCombo_;
    QComboBox* hadronicModelCombo_;
    
    // Production cuts
    QDoubleSpinBox* gammaCutSpin_;
    QDoubleSpinBox* electronCutSpin_;
    QDoubleSpinBox* positronCutSpin_;
    QDoubleSpinBox* protonCutSpin_;
    
    // Preview
    QLabel* previewLabel_;
    
    bool updating_;
};

} // namespace geantcad

