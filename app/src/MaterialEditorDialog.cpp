#include "MaterialEditorDialog.hh"
#include <QMessageBox>
#include <QHeaderView>

namespace geantcad {

MaterialEditorDialog::MaterialEditorDialog(QWidget* parent)
    : QDialog(parent)
    , isEditing_(false)
{
    setWindowTitle("Create Custom Material");
    setupUI();
}

MaterialEditorDialog::MaterialEditorDialog(std::shared_ptr<Material> existingMaterial, QWidget* parent)
    : QDialog(parent)
    , material_(existingMaterial)
    , isEditing_(true)
{
    setWindowTitle("Edit Material");
    setupUI();
    if (material_) {
        loadMaterial(material_);
    }
}

void MaterialEditorDialog::setupUI() {
    setMinimumSize(500, 550);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    
    tabWidget_ = new QTabWidget(this);
    
    setupNistTab();
    setupCustomTab();
    setupPreviewTab();
    
    mainLayout->addWidget(tabWidget_);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    cancelBtn_ = new QPushButton("Cancel", this);
    connect(cancelBtn_, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn_);
    
    okBtn_ = new QPushButton("Create Material", this);
    okBtn_->setDefault(true);
    connect(okBtn_, &QPushButton::clicked, this, &MaterialEditorDialog::onAccept);
    buttonLayout->addWidget(okBtn_);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect tab change to update preview
    connect(tabWidget_, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 2) { // Preview tab
            updateGeant4Preview();
        }
    });
}

void MaterialEditorDialog::setupNistTab() {
    QWidget* nistTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(nistTab);
    
    QLabel* infoLabel = new QLabel("Select a predefined NIST material:", this);
    layout->addWidget(infoLabel);
    
    nistCombo_ = new QComboBox(this);
    populateNistMaterials();
    connect(nistCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MaterialEditorDialog::onNistMaterialChanged);
    layout->addWidget(nistCombo_);
    
    nistInfoLabel_ = new QLabel(this);
    nistInfoLabel_->setStyleSheet("color: #888; font-style: italic;");
    nistInfoLabel_->setWordWrap(true);
    layout->addWidget(nistInfoLabel_);
    
    // Visual properties for NIST materials
    QGroupBox* visualGroup = new QGroupBox("Visual Properties", this);
    QFormLayout* visualLayout = new QFormLayout(visualGroup);
    
    colorBtn_ = new QPushButton(this);
    colorBtn_->setFixedSize(60, 24);
    updateColorButton();
    connect(colorBtn_, &QPushButton::clicked, this, &MaterialEditorDialog::onColorClicked);
    visualLayout->addRow("Color:", colorBtn_);
    
    opacitySpin_ = new QDoubleSpinBox(this);
    opacitySpin_->setRange(0.0, 1.0);
    opacitySpin_->setSingleStep(0.1);
    opacitySpin_->setValue(1.0);
    visualLayout->addRow("Opacity:", opacitySpin_);
    
    layout->addWidget(visualGroup);
    layout->addStretch();
    
    tabWidget_->addTab(nistTab, "NIST Material");
    
    // Trigger initial info display
    onNistMaterialChanged(0);
}

void MaterialEditorDialog::setupCustomTab() {
    QWidget* customTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(customTab);
    
    // Basic properties
    QGroupBox* basicGroup = new QGroupBox("Basic Properties", this);
    QFormLayout* basicLayout = new QFormLayout(basicGroup);
    
    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Enter material name");
    basicLayout->addRow("Name:", nameEdit_);
    
    typeCombo_ = new QComboBox(this);
    typeCombo_->addItem("Single Element", static_cast<int>(Material::Type::SingleElement));
    typeCombo_->addItem("Compound (Elements)", static_cast<int>(Material::Type::Compound));
    connect(typeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MaterialEditorDialog::onTypeChanged);
    basicLayout->addRow("Type:", typeCombo_);
    
    densitySpin_ = new QDoubleSpinBox(this);
    densitySpin_->setRange(0.0001, 30.0);
    densitySpin_->setDecimals(4);
    densitySpin_->setSingleStep(0.1);
    densitySpin_->setValue(1.0);
    densitySpin_->setSuffix(" g/cm³");
    basicLayout->addRow("Density:", densitySpin_);
    
    stateCombo_ = new QComboBox(this);
    stateCombo_->addItem("Solid", static_cast<int>(Material::State::Solid));
    stateCombo_->addItem("Liquid", static_cast<int>(Material::State::Liquid));
    stateCombo_->addItem("Gas", static_cast<int>(Material::State::Gas));
    basicLayout->addRow("State:", stateCombo_);
    
    temperatureSpin_ = new QDoubleSpinBox(this);
    temperatureSpin_->setRange(0.0, 10000.0);
    temperatureSpin_->setValue(293.15);
    temperatureSpin_->setSuffix(" K");
    basicLayout->addRow("Temperature:", temperatureSpin_);
    
    pressureSpin_ = new QDoubleSpinBox(this);
    pressureSpin_->setRange(0.0, 1000.0);
    pressureSpin_->setDecimals(4);
    pressureSpin_->setValue(1.0);
    pressureSpin_->setSuffix(" atm");
    basicLayout->addRow("Pressure:", pressureSpin_);
    
    layout->addWidget(basicGroup);
    
    // Single Element widget
    singleElementWidget_ = new QWidget(this);
    QFormLayout* singleLayout = new QFormLayout(singleElementWidget_);
    singleLayout->setContentsMargins(0, 0, 0, 0);
    
    atomicNumberSpin_ = new QSpinBox(this);
    atomicNumberSpin_->setRange(1, 118);
    atomicNumberSpin_->setValue(6);  // Carbon
    singleLayout->addRow("Atomic Number (Z):", atomicNumberSpin_);
    
    atomicMassSpin_ = new QDoubleSpinBox(this);
    atomicMassSpin_->setRange(1.0, 300.0);
    atomicMassSpin_->setDecimals(3);
    atomicMassSpin_->setValue(12.011);  // Carbon
    atomicMassSpin_->setSuffix(" g/mol");
    singleLayout->addRow("Atomic Mass (A):", atomicMassSpin_);
    
    layout->addWidget(singleElementWidget_);
    
    // Compound widget
    compoundWidget_ = new QWidget(this);
    QVBoxLayout* compoundLayout = new QVBoxLayout(compoundWidget_);
    compoundLayout->setContentsMargins(0, 0, 0, 0);
    
    // Fraction type selector
    QHBoxLayout* fractionLayout = new QHBoxLayout();
    QLabel* fractionLabel = new QLabel("Composition by:", this);
    fractionTypeCombo_ = new QComboBox(this);
    fractionTypeCombo_->addItem("Mass Fraction (0-1)");
    fractionTypeCombo_->addItem("Atom Count");
    fractionLayout->addWidget(fractionLabel);
    fractionLayout->addWidget(fractionTypeCombo_);
    fractionLayout->addStretch();
    compoundLayout->addLayout(fractionLayout);
    
    // Elements table
    elementsTable_ = new QTableWidget(this);
    elementsTable_->setColumnCount(4);
    elementsTable_->setHorizontalHeaderLabels({"Element", "Symbol", "Z", "Fraction/Atoms"});
    elementsTable_->horizontalHeader()->setStretchLastSection(true);
    elementsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    elementsTable_->setMinimumHeight(120);
    compoundLayout->addWidget(elementsTable_);
    
    // Add/Remove buttons
    QHBoxLayout* elemBtnLayout = new QHBoxLayout();
    addElementCombo_ = new QComboBox(this);
    populateElements();
    elemBtnLayout->addWidget(addElementCombo_);
    
    addElementBtn_ = new QPushButton("Add", this);
    connect(addElementBtn_, &QPushButton::clicked, this, &MaterialEditorDialog::onAddElement);
    elemBtnLayout->addWidget(addElementBtn_);
    
    removeElementBtn_ = new QPushButton("Remove", this);
    connect(removeElementBtn_, &QPushButton::clicked, this, &MaterialEditorDialog::onRemoveElement);
    elemBtnLayout->addWidget(removeElementBtn_);
    
    elemBtnLayout->addStretch();
    compoundLayout->addLayout(elemBtnLayout);
    
    layout->addWidget(compoundWidget_);
    compoundWidget_->hide();
    
    layout->addStretch();
    
    tabWidget_->addTab(customTab, "Custom Material");
}

void MaterialEditorDialog::setupPreviewTab() {
    QWidget* previewTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(previewTab);
    
    QLabel* label = new QLabel("Geant4 C++ Code:", this);
    layout->addWidget(label);
    
    geant4CodePreview_ = new QTextEdit(this);
    geant4CodePreview_->setReadOnly(true);
    geant4CodePreview_->setFontFamily("monospace");
    geant4CodePreview_->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4;");
    layout->addWidget(geant4CodePreview_);
    
    tabWidget_->addTab(previewTab, "Geant4 Code");
}

void MaterialEditorDialog::populateNistMaterials() {
    QStringList materials = {
        "G4_AIR", "G4_WATER", "G4_Galactic",
        "G4_Al", "G4_Si", "G4_Fe", "G4_Cu", "G4_Pb", "G4_Ti",
        "G4_STAINLESS-STEEL", "G4_BRASS", "G4_BRONZE",
        "G4_GLASS_PLATE", "G4_Pyrex_Glass",
        "G4_POLYSTYRENE", "G4_POLYETHYLENE", "G4_PLEXIGLASS",
        "G4_CARBON_DIOXIDE", "G4_Ar", "G4_He", "G4_N", "G4_O",
        "G4_CESIUM_IODIDE", "G4_SODIUM_IODIDE",
        "G4_BGO", "G4_LYSO", "G4_PbWO4",
        "G4_CONCRETE", "G4_BONE_COMPACT_ICRU",
        "G4_MUSCLE_SKELETAL_ICRP", "G4_TISSUE_SOFT_ICRP"
    };
    
    nistCombo_->addItems(materials);
}

void MaterialEditorDialog::populateElements() {
    struct ElemInfo { QString name; QString symbol; int z; double a; };
    QList<ElemInfo> elements = {
        {"Hydrogen", "H", 1, 1.008},
        {"Helium", "He", 2, 4.003},
        {"Carbon", "C", 6, 12.011},
        {"Nitrogen", "N", 7, 14.007},
        {"Oxygen", "O", 8, 15.999},
        {"Fluorine", "F", 9, 18.998},
        {"Sodium", "Na", 11, 22.990},
        {"Aluminum", "Al", 13, 26.982},
        {"Silicon", "Si", 14, 28.086},
        {"Phosphorus", "P", 15, 30.974},
        {"Sulfur", "S", 16, 32.065},
        {"Chlorine", "Cl", 17, 35.453},
        {"Argon", "Ar", 18, 39.948},
        {"Potassium", "K", 19, 39.098},
        {"Calcium", "Ca", 20, 40.078},
        {"Iron", "Fe", 26, 55.845},
        {"Copper", "Cu", 29, 63.546},
        {"Zinc", "Zn", 30, 65.380},
        {"Germanium", "Ge", 32, 72.630},
        {"Bromine", "Br", 35, 79.904},
        {"Yttrium", "Y", 39, 88.906},
        {"Iodine", "I", 53, 126.904},
        {"Cesium", "Cs", 55, 132.905},
        {"Barium", "Ba", 56, 137.327},
        {"Lutetium", "Lu", 71, 174.967},
        {"Tungsten", "W", 74, 183.840},
        {"Lead", "Pb", 82, 207.200},
        {"Bismuth", "Bi", 83, 208.980}
    };
    
    for (const auto& elem : elements) {
        QString text = QString("%1 (%2) - Z=%3").arg(elem.name).arg(elem.symbol).arg(elem.z);
        addElementCombo_->addItem(text, QVariant::fromValue(
            QList<QVariant>{elem.symbol, elem.name, elem.z, elem.a}));
    }
}

void MaterialEditorDialog::onTypeChanged(int index) {
    Material::Type type = static_cast<Material::Type>(typeCombo_->currentData().toInt());
    
    singleElementWidget_->setVisible(type == Material::Type::SingleElement);
    compoundWidget_->setVisible(type == Material::Type::Compound);
    
    updateGeant4Preview();
}

void MaterialEditorDialog::onNistMaterialChanged(int index) {
    QString material = nistCombo_->currentText();
    
    // Provide info about common materials
    QString info;
    if (material == "G4_AIR") {
        info = "Standard air at STP. Density: 0.00120 g/cm³";
    } else if (material == "G4_WATER") {
        info = "Liquid water (H2O). Density: 1.00 g/cm³";
    } else if (material == "G4_Galactic") {
        info = "Galactic vacuum. Extremely low density.";
    } else if (material.startsWith("G4_") && material.length() <= 5) {
        info = "Pure element. Check NIST database for properties.";
    } else if (material == "G4_STAINLESS-STEEL") {
        info = "Stainless steel. Density: ~8.0 g/cm³";
    } else if (material.contains("IODIDE")) {
        info = "Scintillator crystal. Common for radiation detection.";
    }
    
    nistInfoLabel_->setText(info);
    updateGeant4Preview();
}

void MaterialEditorDialog::onAddElement() {
    QVariant data = addElementCombo_->currentData();
    if (!data.isValid()) return;
    
    QList<QVariant> elemData = data.value<QList<QVariant>>();
    if (elemData.size() < 4) return;
    
    int row = elementsTable_->rowCount();
    elementsTable_->insertRow(row);
    
    // Name
    QTableWidgetItem* nameItem = new QTableWidgetItem(elemData[1].toString());
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    elementsTable_->setItem(row, 0, nameItem);
    
    // Symbol
    QTableWidgetItem* symbolItem = new QTableWidgetItem(elemData[0].toString());
    symbolItem->setFlags(symbolItem->flags() & ~Qt::ItemIsEditable);
    elementsTable_->setItem(row, 1, symbolItem);
    
    // Z
    QTableWidgetItem* zItem = new QTableWidgetItem(QString::number(elemData[2].toInt()));
    zItem->setFlags(zItem->flags() & ~Qt::ItemIsEditable);
    zItem->setData(Qt::UserRole, elemData[3].toDouble());  // Store A in user data
    elementsTable_->setItem(row, 2, zItem);
    
    // Fraction (editable)
    QTableWidgetItem* fracItem = new QTableWidgetItem("0.0");
    elementsTable_->setItem(row, 3, fracItem);
    
    updateGeant4Preview();
}

void MaterialEditorDialog::onRemoveElement() {
    int row = elementsTable_->currentRow();
    if (row >= 0) {
        elementsTable_->removeRow(row);
        updateGeant4Preview();
    }
}

void MaterialEditorDialog::onElementChanged() {
    updateGeant4Preview();
}

void MaterialEditorDialog::onColorClicked() {
    QColor color = QColorDialog::getColor(selectedColor_, this, "Select Material Color");
    if (color.isValid()) {
        selectedColor_ = color;
        updateColorButton();
    }
}

void MaterialEditorDialog::updateColorButton() {
    if (colorBtn_) {
        QString style = QString("background-color: rgb(%1, %2, %3);")
            .arg(selectedColor_.red())
            .arg(selectedColor_.green())
            .arg(selectedColor_.blue());
        colorBtn_->setStyleSheet(style);
    }
}

void MaterialEditorDialog::onAccept() {
    if (!validateInput()) {
        return;
    }
    
    buildMaterial();
    emit materialCreated(material_);
    accept();
}

bool MaterialEditorDialog::validateInput() {
    // Check if we're on NIST tab or custom tab
    if (tabWidget_->currentIndex() == 0) {
        // NIST - always valid
        return true;
    }
    
    // Custom material validation
    if (nameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a material name.");
        return false;
    }
    
    Material::Type type = static_cast<Material::Type>(typeCombo_->currentData().toInt());
    
    if (type == Material::Type::Compound && elementsTable_->rowCount() == 0) {
        QMessageBox::warning(this, "Validation Error", "Please add at least one element to the compound.");
        return false;
    }
    
    return true;
}

void MaterialEditorDialog::buildMaterial() {
    // Check which tab is active
    if (tabWidget_->currentIndex() == 0) {
        // NIST material
        QString nistName = nistCombo_->currentText();
        material_ = Material::makeNist(nistName.toStdString());
    } else {
        // Custom material
        QString name = nameEdit_->text().trimmed();
        double density = densitySpin_->value();
        Material::Type type = static_cast<Material::Type>(typeCombo_->currentData().toInt());
        
        if (type == Material::Type::SingleElement) {
            Element elem;
            elem.atomicNumber = atomicNumberSpin_->value();
            elem.atomicMass = atomicMassSpin_->value();
            material_ = Material::makeFromElement(name.toStdString(), density, elem);
        } else {
            // Compound
            bool byAtoms = (fractionTypeCombo_->currentIndex() == 1);
            
            if (byAtoms) {
                std::vector<std::pair<Element, int>> elements;
                for (int i = 0; i < elementsTable_->rowCount(); ++i) {
                    Element elem;
                    elem.symbol = elementsTable_->item(i, 1)->text().toStdString();
                    elem.name = elementsTable_->item(i, 0)->text().toStdString();
                    elem.atomicNumber = elementsTable_->item(i, 2)->text().toInt();
                    elem.atomicMass = elementsTable_->item(i, 2)->data(Qt::UserRole).toDouble();
                    
                    int nAtoms = elementsTable_->item(i, 3)->text().toInt();
                    elements.push_back({elem, nAtoms});
                }
                material_ = Material::makeCompoundByAtoms(name.toStdString(), density, elements);
            } else {
                std::vector<std::pair<Element, double>> elements;
                for (int i = 0; i < elementsTable_->rowCount(); ++i) {
                    Element elem;
                    elem.symbol = elementsTable_->item(i, 1)->text().toStdString();
                    elem.name = elementsTable_->item(i, 0)->text().toStdString();
                    elem.atomicNumber = elementsTable_->item(i, 2)->text().toInt();
                    elem.atomicMass = elementsTable_->item(i, 2)->data(Qt::UserRole).toDouble();
                    
                    double fraction = elementsTable_->item(i, 3)->text().toDouble();
                    elements.push_back({elem, fraction});
                }
                material_ = Material::makeCompoundByMass(name.toStdString(), density, elements);
            }
        }
        
        // Set state properties
        material_->setState(static_cast<Material::State>(stateCombo_->currentData().toInt()));
        material_->setTemperature(temperatureSpin_->value());
        material_->setPressure(pressureSpin_->value());
    }
    
    // Set visual properties
    if (material_) {
        auto& visual = material_->getVisual();
        visual.r = selectedColor_.redF();
        visual.g = selectedColor_.greenF();
        visual.b = selectedColor_.blueF();
        visual.a = opacitySpin_ ? opacitySpin_->value() : 1.0f;
    }
}

void MaterialEditorDialog::updateGeant4Preview() {
    if (!geant4CodePreview_) return;
    
    // Build a temporary material to get the code
    buildMaterial();
    
    if (material_) {
        QString code = QString::fromStdString(material_->toGeant4Code());
        geant4CodePreview_->setText(code);
    } else {
        geant4CodePreview_->setText("// No material defined");
    }
}

void MaterialEditorDialog::loadMaterial(std::shared_ptr<Material> mat) {
    if (!mat) return;
    
    // Determine which tab to show
    if (mat->getMaterialType() == Material::Type::NIST) {
        tabWidget_->setCurrentIndex(0);
        int idx = nistCombo_->findText(QString::fromStdString(mat->getNistName()));
        if (idx >= 0) nistCombo_->setCurrentIndex(idx);
    } else {
        tabWidget_->setCurrentIndex(1);
        nameEdit_->setText(QString::fromStdString(mat->getName()));
        densitySpin_->setValue(mat->getDensity());
        
        // Type
        int typeIdx = typeCombo_->findData(static_cast<int>(mat->getMaterialType()));
        if (typeIdx >= 0) typeCombo_->setCurrentIndex(typeIdx);
        
        // State
        int stateIdx = stateCombo_->findData(static_cast<int>(mat->getState()));
        if (stateIdx >= 0) stateCombo_->setCurrentIndex(stateIdx);
        
        temperatureSpin_->setValue(mat->getTemperature());
        pressureSpin_->setValue(mat->getPressure());
        
        if (mat->getMaterialType() == Material::Type::SingleElement) {
            atomicNumberSpin_->setValue(mat->getAtomicNumber());
            atomicMassSpin_->setValue(mat->getAtomicMass());
        }
        
        // Components would need to be loaded into the table...
    }
    
    // Visual
    auto& visual = mat->getVisual();
    selectedColor_ = QColor::fromRgbF(visual.r, visual.g, visual.b);
    updateColorButton();
    if (opacitySpin_) opacitySpin_->setValue(visual.a);
    
    okBtn_->setText("Update Material");
}

} // namespace geantcad

