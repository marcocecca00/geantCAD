#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
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
    void onStandardListChanged();

private:
    void setupUI();
    void updatePreview();
    
    QCheckBox* emCheckbox_;
    QCheckBox* decayCheckbox_;
    QCheckBox* opticalCheckbox_;
    QCheckBox* hadronicCheckbox_;
    QComboBox* standardListCombo_;
    QLabel* previewLabel_;
};

} // namespace geantcad

