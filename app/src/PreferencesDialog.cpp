#include "PreferencesDialog.hh"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QFontDatabase>
#include <QThread>

namespace geantcad {

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Preferences");
    setMinimumSize(550, 500);
    setupUI();
    loadSettings();
}

void PreferencesDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    
    tabWidget_ = new QTabWidget(this);
    
    setupAppearancePage();
    setupViewportPage();
    setupGridPage();
    setupGeant4Page();
    
    mainLayout->addWidget(tabWidget_);
    
    // Button row
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    restoreDefaultsBtn_ = new QPushButton("Restore Defaults", this);
    connect(restoreDefaultsBtn_, &QPushButton::clicked, this, &PreferencesDialog::onRestoreDefaults);
    buttonLayout->addWidget(restoreDefaultsBtn_);
    
    buttonLayout->addStretch();
    
    applyBtn_ = new QPushButton("Apply", this);
    connect(applyBtn_, &QPushButton::clicked, this, &PreferencesDialog::onApply);
    buttonLayout->addWidget(applyBtn_);
    
    cancelBtn_ = new QPushButton("Cancel", this);
    connect(cancelBtn_, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn_);
    
    okBtn_ = new QPushButton("OK", this);
    okBtn_->setDefault(true);
    connect(okBtn_, &QPushButton::clicked, this, [this]() {
        saveSettings();
        accept();
    });
    buttonLayout->addWidget(okBtn_);
    
    mainLayout->addLayout(buttonLayout);
}

void PreferencesDialog::setupAppearancePage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Theme Group
    QGroupBox* themeGroup = new QGroupBox("Theme", page);
    QFormLayout* themeLayout = new QFormLayout(themeGroup);
    
    themeCombo_ = new QComboBox(page);
    themeCombo_->addItem("🌙 Dark (Default)", static_cast<int>(ThemeManager::Theme::Dark));
    themeCombo_->addItem("☀️ Light", static_cast<int>(ThemeManager::Theme::Light));
    themeCombo_->addItem("🖥️ System", static_cast<int>(ThemeManager::Theme::System));
    connect(themeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PreferencesDialog::onThemeChanged);
    themeLayout->addRow("Color Theme:", themeCombo_);
    
    layout->addWidget(themeGroup);
    
    // Font Group
    QGroupBox* fontGroup = new QGroupBox("Font", page);
    QFormLayout* fontLayout = new QFormLayout(fontGroup);
    
    fontCombo_ = new QComboBox(page);
    for (const QString& family : QFontDatabase::families()) {
        fontCombo_->addItem(family);
    }
    fontCombo_->setCurrentText("Segoe UI");
    fontLayout->addRow("Font Family:", fontCombo_);
    
    fontSizeSpin_ = new QSpinBox(page);
    fontSizeSpin_->setRange(8, 24);
    fontSizeSpin_->setValue(13);
    fontSizeSpin_->setSuffix(" pt");
    fontLayout->addRow("Font Size:", fontSizeSpin_);
    
    layout->addWidget(fontGroup);
    
    // Animations Group
    QGroupBox* animGroup = new QGroupBox("Animations", page);
    QVBoxLayout* animLayout = new QVBoxLayout(animGroup);
    
    animationsCheck_ = new QCheckBox("Enable UI animations", page);
    animationsCheck_->setChecked(true);
    animationsCheck_->setToolTip("Enable smooth animations for panels and transitions");
    animLayout->addWidget(animationsCheck_);
    
    layout->addWidget(animGroup);
    
    layout->addStretch();
    
    tabWidget_->addTab(page, "🎨 Appearance");
}

void PreferencesDialog::setupViewportPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Rendering Group
    QGroupBox* renderGroup = new QGroupBox("Rendering", page);
    QFormLayout* renderLayout = new QFormLayout(renderGroup);
    
    antialiasCombo_ = new QComboBox(page);
    antialiasCombo_->addItem("Off", 0);
    antialiasCombo_->addItem("2x MSAA", 2);
    antialiasCombo_->addItem("4x MSAA", 4);
    antialiasCombo_->addItem("8x MSAA", 8);
    antialiasCombo_->setCurrentIndex(2);
    renderLayout->addRow("Anti-aliasing:", antialiasCombo_);
    
    backgroundCombo_ = new QComboBox(page);
    backgroundCombo_->addItem("Gradient (Dark)", "gradient_dark");
    backgroundCombo_->addItem("Gradient (Light)", "gradient_light");
    backgroundCombo_->addItem("Solid Black", "solid_black");
    backgroundCombo_->addItem("Solid Gray", "solid_gray");
    backgroundCombo_->addItem("Solid White", "solid_white");
    renderLayout->addRow("Background:", backgroundCombo_);
    
    layout->addWidget(renderGroup);
    
    // Helpers Group
    QGroupBox* helpersGroup = new QGroupBox("Visual Helpers", page);
    QVBoxLayout* helpersLayout = new QVBoxLayout(helpersGroup);
    
    showAxesCheck_ = new QCheckBox("Show coordinate axes", page);
    showAxesCheck_->setChecked(true);
    helpersLayout->addWidget(showAxesCheck_);
    
    showViewCubeCheck_ = new QCheckBox("Show view cube", page);
    showViewCubeCheck_->setChecked(true);
    helpersLayout->addWidget(showViewCubeCheck_);
    
    layout->addWidget(helpersGroup);
    
    // Navigation Group
    QGroupBox* navGroup = new QGroupBox("Navigation", page);
    QFormLayout* navLayout = new QFormLayout(navGroup);
    
    cameraSpeedSpin_ = new QDoubleSpinBox(page);
    cameraSpeedSpin_->setRange(0.1, 10.0);
    cameraSpeedSpin_->setValue(1.0);
    cameraSpeedSpin_->setSingleStep(0.1);
    navLayout->addRow("Rotation Speed:", cameraSpeedSpin_);
    
    zoomSpeedSpin_ = new QDoubleSpinBox(page);
    zoomSpeedSpin_->setRange(0.1, 10.0);
    zoomSpeedSpin_->setValue(1.0);
    zoomSpeedSpin_->setSingleStep(0.1);
    navLayout->addRow("Zoom Speed:", zoomSpeedSpin_);
    
    layout->addWidget(navGroup);
    
    layout->addStretch();
    
    tabWidget_->addTab(page, "🖼️ Viewport");
}

void PreferencesDialog::setupGridPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Grid Settings
    QGroupBox* gridGroup = new QGroupBox("Grid Settings", page);
    QFormLayout* gridLayout = new QFormLayout(gridGroup);
    
    gridEnabledCheck_ = new QCheckBox(page);
    gridEnabledCheck_->setChecked(false);
    gridLayout->addRow("Enable Grid:", gridEnabledCheck_);
    
    gridSpacingSpin_ = new QDoubleSpinBox(page);
    gridSpacingSpin_->setRange(1.0, 1000.0);
    gridSpacingSpin_->setValue(10.0);
    gridSpacingSpin_->setSuffix(" mm");
    gridLayout->addRow("Grid Spacing:", gridSpacingSpin_);
    
    gridSubdivisionsSpin_ = new QSpinBox(page);
    gridSubdivisionsSpin_->setRange(1, 10);
    gridSubdivisionsSpin_->setValue(5);
    gridLayout->addRow("Subdivisions:", gridSubdivisionsSpin_);
    
    layout->addWidget(gridGroup);
    
    // Snapping Group
    QGroupBox* snapGroup = new QGroupBox("Snapping", page);
    QVBoxLayout* snapLayout = new QVBoxLayout(snapGroup);
    
    snapToGridCheck_ = new QCheckBox("Snap to grid", page);
    snapToGridCheck_->setChecked(false);
    snapLayout->addWidget(snapToGridCheck_);
    
    layout->addWidget(snapGroup);
    
    layout->addStretch();
    
    tabWidget_->addTab(page, "📐 Grid");
}

void PreferencesDialog::setupGeant4Page() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Paths Group
    QGroupBox* pathsGroup = new QGroupBox("External Programs", page);
    QFormLayout* pathsLayout = new QFormLayout(pathsGroup);
    
    QHBoxLayout* g4Layout = new QHBoxLayout();
    geant4PathEdit_ = new QLineEdit(page);
    geant4PathEdit_->setPlaceholderText("Auto-detect or specify path...");
    g4Layout->addWidget(geant4PathEdit_);
    QPushButton* g4BrowseBtn = new QPushButton("Browse...", page);
    connect(g4BrowseBtn, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Geant4 Installation");
        if (!dir.isEmpty()) geant4PathEdit_->setText(dir);
    });
    g4Layout->addWidget(g4BrowseBtn);
    pathsLayout->addRow("Geant4 Path:", g4Layout);
    
    QHBoxLayout* rootLayout = new QHBoxLayout();
    rootPathEdit_ = new QLineEdit(page);
    rootPathEdit_->setPlaceholderText("Optional - for ROOT output");
    rootLayout->addWidget(rootPathEdit_);
    QPushButton* rootBrowseBtn = new QPushButton("Browse...", page);
    connect(rootBrowseBtn, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select ROOT Installation");
        if (!dir.isEmpty()) rootPathEdit_->setText(dir);
    });
    rootLayout->addWidget(rootBrowseBtn);
    pathsLayout->addRow("ROOT Path:", rootLayout);
    
    layout->addWidget(pathsGroup);
    
    // Build Group
    QGroupBox* buildGroup = new QGroupBox("Build Settings", page);
    QFormLayout* buildLayout = new QFormLayout(buildGroup);
    
    autoCompileCheck_ = new QCheckBox(page);
    autoCompileCheck_->setChecked(false);
    autoCompileCheck_->setToolTip("Automatically compile after generating code");
    buildLayout->addRow("Auto-compile:", autoCompileCheck_);
    
    numThreadsSpin_ = new QSpinBox(page);
    numThreadsSpin_->setRange(1, QThread::idealThreadCount() * 2);
    numThreadsSpin_->setValue(QThread::idealThreadCount());
    numThreadsSpin_->setToolTip("Number of parallel compilation jobs");
    buildLayout->addRow("Build Threads:", numThreadsSpin_);
    
    layout->addWidget(buildGroup);
    
    layout->addStretch();
    
    tabWidget_->addTab(page, "⚛️ Geant4");
}

void PreferencesDialog::loadSettings() {
    QSettings settings("GeantCAD", "GeantCAD");
    
    // Appearance
    int themeIndex = settings.value("appearance/theme", 0).toInt();
    themeCombo_->setCurrentIndex(themeIndex);
    
    fontCombo_->setCurrentText(settings.value("appearance/fontFamily", "Segoe UI").toString());
    fontSizeSpin_->setValue(settings.value("appearance/fontSize", 13).toInt());
    animationsCheck_->setChecked(settings.value("appearance/animations", true).toBool());
    
    // Viewport
    int aaIndex = antialiasCombo_->findData(settings.value("viewport/antialiasing", 4).toInt());
    if (aaIndex >= 0) antialiasCombo_->setCurrentIndex(aaIndex);
    
    int bgIndex = backgroundCombo_->findData(settings.value("viewport/background", "gradient_dark").toString());
    if (bgIndex >= 0) backgroundCombo_->setCurrentIndex(bgIndex);
    
    showAxesCheck_->setChecked(settings.value("viewport/showAxes", true).toBool());
    showViewCubeCheck_->setChecked(settings.value("viewport/showViewCube", true).toBool());
    cameraSpeedSpin_->setValue(settings.value("viewport/cameraSpeed", 1.0).toDouble());
    zoomSpeedSpin_->setValue(settings.value("viewport/zoomSpeed", 1.0).toDouble());
    
    // Grid
    gridEnabledCheck_->setChecked(settings.value("grid/enabled", false).toBool());
    gridSpacingSpin_->setValue(settings.value("grid/spacing", 10.0).toDouble());
    gridSubdivisionsSpin_->setValue(settings.value("grid/subdivisions", 5).toInt());
    snapToGridCheck_->setChecked(settings.value("grid/snapToGrid", false).toBool());
    
    // Geant4
    geant4PathEdit_->setText(settings.value("geant4/path", "").toString());
    rootPathEdit_->setText(settings.value("geant4/rootPath", "").toString());
    autoCompileCheck_->setChecked(settings.value("geant4/autoCompile", false).toBool());
    numThreadsSpin_->setValue(settings.value("geant4/numThreads", QThread::idealThreadCount()).toInt());
}

void PreferencesDialog::saveSettings() {
    QSettings settings("GeantCAD", "GeantCAD");
    
    // Appearance
    settings.setValue("appearance/theme", themeCombo_->currentIndex());
    settings.setValue("appearance/fontFamily", fontCombo_->currentText());
    settings.setValue("appearance/fontSize", fontSizeSpin_->value());
    settings.setValue("appearance/animations", animationsCheck_->isChecked());
    
    // Viewport
    settings.setValue("viewport/antialiasing", antialiasCombo_->currentData().toInt());
    settings.setValue("viewport/background", backgroundCombo_->currentData().toString());
    settings.setValue("viewport/showAxes", showAxesCheck_->isChecked());
    settings.setValue("viewport/showViewCube", showViewCubeCheck_->isChecked());
    settings.setValue("viewport/cameraSpeed", cameraSpeedSpin_->value());
    settings.setValue("viewport/zoomSpeed", zoomSpeedSpin_->value());
    
    // Grid
    settings.setValue("grid/enabled", gridEnabledCheck_->isChecked());
    settings.setValue("grid/spacing", gridSpacingSpin_->value());
    settings.setValue("grid/subdivisions", gridSubdivisionsSpin_->value());
    settings.setValue("grid/snapToGrid", snapToGridCheck_->isChecked());
    
    // Geant4
    settings.setValue("geant4/path", geant4PathEdit_->text());
    settings.setValue("geant4/rootPath", rootPathEdit_->text());
    settings.setValue("geant4/autoCompile", autoCompileCheck_->isChecked());
    settings.setValue("geant4/numThreads", numThreadsSpin_->value());
    
    // Apply theme
    ThemeManager::Theme theme = static_cast<ThemeManager::Theme>(themeCombo_->currentData().toInt());
    ThemeManager::applyTheme(theme);
    
    emit settingsChanged();
}

void PreferencesDialog::onThemeChanged(int index) {
    // Preview theme immediately
    ThemeManager::Theme theme = static_cast<ThemeManager::Theme>(themeCombo_->currentData().toInt());
    ThemeManager::applyTheme(theme);
}

void PreferencesDialog::onApply() {
    saveSettings();
}

void PreferencesDialog::onRestoreDefaults() {
    // Appearance
    themeCombo_->setCurrentIndex(0);
    fontCombo_->setCurrentText("Segoe UI");
    fontSizeSpin_->setValue(13);
    animationsCheck_->setChecked(true);
    
    // Viewport
    antialiasCombo_->setCurrentIndex(2);
    backgroundCombo_->setCurrentIndex(0);
    showAxesCheck_->setChecked(true);
    showViewCubeCheck_->setChecked(true);
    cameraSpeedSpin_->setValue(1.0);
    zoomSpeedSpin_->setValue(1.0);
    
    // Grid
    gridEnabledCheck_->setChecked(false);
    gridSpacingSpin_->setValue(10.0);
    gridSubdivisionsSpin_->setValue(5);
    snapToGridCheck_->setChecked(false);
    
    // Geant4
    geant4PathEdit_->clear();
    rootPathEdit_->clear();
    autoCompileCheck_->setChecked(false);
    numThreadsSpin_->setValue(QThread::idealThreadCount());
}

} // namespace geantcad

