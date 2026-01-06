#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include "ThemeManager.hh"

namespace geantcad {

/**
 * PreferencesDialog - Application settings dialog
 */
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr);
    
    // Load/Save settings
    void loadSettings();
    void saveSettings();

signals:
    void settingsChanged();

private slots:
    void onThemeChanged(int index);
    void onApply();
    void onRestoreDefaults();

private:
    void setupUI();
    void setupAppearancePage();
    void setupViewportPage();
    void setupGridPage();
    void setupGeant4Page();
    void setupShortcutsPage();
    
    QTabWidget* tabWidget_;
    
    // Appearance
    QComboBox* themeCombo_;
    QComboBox* fontCombo_;
    QSpinBox* fontSizeSpin_;
    QCheckBox* animationsCheck_;
    
    // Viewport
    QComboBox* antialiasCombo_;
    QComboBox* backgroundCombo_;
    QCheckBox* showAxesCheck_;
    QCheckBox* showViewCubeCheck_;
    QDoubleSpinBox* cameraSpeedSpin_;
    QDoubleSpinBox* zoomSpeedSpin_;
    
    // Grid
    QCheckBox* gridEnabledCheck_;
    QDoubleSpinBox* gridSpacingSpin_;
    QCheckBox* snapToGridCheck_;
    QSpinBox* gridSubdivisionsSpin_;
    
    // Geant4
    QLineEdit* geant4PathEdit_;
    QLineEdit* rootPathEdit_;
    QCheckBox* autoCompileCheck_;
    QSpinBox* numThreadsSpin_;
    
    // Buttons
    QPushButton* applyBtn_;
    QPushButton* okBtn_;
    QPushButton* cancelBtn_;
    QPushButton* restoreDefaultsBtn_;
};

} // namespace geantcad

