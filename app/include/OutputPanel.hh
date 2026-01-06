#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include "../../core/include/OutputConfig.hh"

namespace geantcad {

class OutputPanel : public QWidget {
    Q_OBJECT

public:
    OutputPanel(QWidget* parent = nullptr);
    ~OutputPanel();
    
    void setConfig(const OutputConfig& config);
    OutputConfig getConfig() const;
    
signals:
    void configChanged();

private slots:
    void onCheckboxChanged();
    void onSchemaChanged();
    void onBrowseFile();
    void onModeChanged();
    void onSelectAllFields();
    void onDeselectAllFields();

private:
    void setupUI();
    void updatePreview();
    
    // ROOT output
    QCheckBox* rootEnabledCheck_;
    QLineEdit* rootFilePathEdit_;
    QPushButton* browseButton_;
    
    // Schema
    QComboBox* schemaCombo_;
    
    // Fields
    QGroupBox* fieldsGroup_;
    QCheckBox* fieldXCheck_;
    QCheckBox* fieldYCheck_;
    QCheckBox* fieldZCheck_;
    QCheckBox* fieldEdepCheck_;
    QCheckBox* fieldEventIdCheck_;
    QCheckBox* fieldTrackIdCheck_;
    QCheckBox* fieldVolumeNameCheck_;
    QCheckBox* fieldTimeCheck_;
    QCheckBox* fieldKineticEnergyCheck_;
    
    // Mode
    QButtonGroup* modeGroup_;
    QRadioButton* perEventRadio_;
    QRadioButton* perStepRadio_;
    
    // Options
    QSpinBox* saveFrequencySpin_;
    QCheckBox* csvFallbackCheck_;
    QCheckBox* compressionCheck_;
    
    // Preview
    QLabel* previewLabel_;
};

} // namespace geantcad

