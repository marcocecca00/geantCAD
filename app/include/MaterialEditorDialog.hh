#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QTabWidget>
#include <QGroupBox>
#include <QColorDialog>
#include <memory>

#include "../../core/include/Material.hh"

namespace geantcad {

/**
 * @brief Dialog for creating and editing custom materials
 * 
 * Supports Geant4-style material definitions:
 * - NIST predefined materials
 * - Single element materials
 * - Compound materials (by mass fraction or atom count)
 */
class MaterialEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit MaterialEditorDialog(QWidget* parent = nullptr);
    MaterialEditorDialog(std::shared_ptr<Material> existingMaterial, QWidget* parent = nullptr);
    ~MaterialEditorDialog() override = default;
    
    std::shared_ptr<Material> getMaterial() const { return material_; }
    
signals:
    void materialCreated(std::shared_ptr<Material> material);

private slots:
    void onTypeChanged(int index);
    void onNistMaterialChanged(int index);
    void onAddElement();
    void onRemoveElement();
    void onElementChanged();
    void onColorClicked();
    void onAccept();
    void updateGeant4Preview();

private:
    void setupUI();
    void setupNistTab();
    void setupCustomTab();
    void setupPreviewTab();
    void populateNistMaterials();
    void populateElements();
    void loadMaterial(std::shared_ptr<Material> mat);
    bool validateInput();
    void buildMaterial();
    void updateColorButton();
    
    // UI Elements
    QTabWidget* tabWidget_ = nullptr;
    
    // NIST tab
    QComboBox* nistCombo_ = nullptr;
    QLabel* nistInfoLabel_ = nullptr;
    
    // Custom tab
    QLineEdit* nameEdit_ = nullptr;
    QComboBox* typeCombo_ = nullptr;
    QDoubleSpinBox* densitySpin_ = nullptr;
    QComboBox* stateCombo_ = nullptr;
    QDoubleSpinBox* temperatureSpin_ = nullptr;
    QDoubleSpinBox* pressureSpin_ = nullptr;
    
    // Single element widgets
    QWidget* singleElementWidget_ = nullptr;
    QSpinBox* atomicNumberSpin_ = nullptr;
    QDoubleSpinBox* atomicMassSpin_ = nullptr;
    
    // Compound widgets
    QWidget* compoundWidget_ = nullptr;
    QTableWidget* elementsTable_ = nullptr;
    QComboBox* addElementCombo_ = nullptr;
    QPushButton* addElementBtn_ = nullptr;
    QPushButton* removeElementBtn_ = nullptr;
    QComboBox* fractionTypeCombo_ = nullptr;  // Mass fraction vs. atom count
    
    // Visual properties
    QPushButton* colorBtn_ = nullptr;
    QDoubleSpinBox* opacitySpin_ = nullptr;
    QColor selectedColor_ = QColor(200, 200, 200);
    
    // Preview tab
    QTextEdit* geant4CodePreview_ = nullptr;
    
    // Buttons
    QPushButton* okBtn_ = nullptr;
    QPushButton* cancelBtn_ = nullptr;
    
    // Data
    std::shared_ptr<Material> material_;
    bool isEditing_ = false;
};

} // namespace geantcad

