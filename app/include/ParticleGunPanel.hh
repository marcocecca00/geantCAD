#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include "../../core/include/ParticleGunConfig.hh"

namespace geantcad {

class ParticleGunPanel : public QWidget {
    Q_OBJECT

public:
    explicit ParticleGunPanel(QWidget* parent = nullptr);
    ~ParticleGunPanel();
    
    void setConfig(const ParticleGunConfig& config);
    ParticleGunConfig getConfig() const;

signals:
    void configChanged();

private slots:
    void onParticleTypeChanged();
    void onEnergyModeChanged();
    void onPositionModeChanged();
    void onDirectionModeChanged();
    void onValueChanged();

private:
    void setupUI();
    void updateUI();
    void updatePreview();
    
    // Particle type
    QComboBox* particleTypeCombo_;
    
    // Energy
    QComboBox* energyModeCombo_;
    QDoubleSpinBox* energySpin_;
    QDoubleSpinBox* energyMinSpin_;
    QDoubleSpinBox* energyMaxSpin_;
    QDoubleSpinBox* energyMeanSpin_;
    QDoubleSpinBox* energySigmaSpin_;
    QGroupBox* energyGroup_;
    
    // Position
    QComboBox* positionModeCombo_;
    QDoubleSpinBox* positionXSpin_;
    QDoubleSpinBox* positionYSpin_;
    QDoubleSpinBox* positionZSpin_;
    QDoubleSpinBox* positionRadiusSpin_;
    QLineEdit* positionVolumeEdit_;
    QGroupBox* positionGroup_;
    
    // Direction
    QComboBox* directionModeCombo_;
    QDoubleSpinBox* directionXSpin_;
    QDoubleSpinBox* directionYSpin_;
    QDoubleSpinBox* directionZSpin_;
    QDoubleSpinBox* coneAngleSpin_;
    QGroupBox* directionGroup_;
    
    // Number of particles
    QSpinBox* numberOfParticlesSpin_;
    
    // Preview
    QLabel* previewLabel_;
    
    ParticleGunConfig config_;
};

} // namespace geantcad

