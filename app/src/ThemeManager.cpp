#include "ThemeManager.hh"
#include <QStyleFactory>

namespace geantcad {

ThemeManager::Theme ThemeManager::currentTheme_ = Theme::Dark;

void ThemeManager::applyTheme(Theme theme) {
    currentTheme_ = theme;
    
    // Use Fusion style as base (works well with custom palettes)
    qApp->setStyle(QStyleFactory::create("Fusion"));
    
    // Apply palette
    qApp->setPalette(getPalette(theme));
    
    // Apply stylesheet
    qApp->setStyleSheet(getStyleSheet(theme));
}

QPalette ThemeManager::getPalette(Theme theme) {
    QPalette palette;
    Colors colors = getColors(theme);
    
    if (theme == Theme::Dark) {
        palette.setColor(QPalette::Window, QColor(colors.background));
        palette.setColor(QPalette::WindowText, QColor(colors.text));
        palette.setColor(QPalette::Base, QColor(colors.backgroundAlt));
        palette.setColor(QPalette::AlternateBase, QColor(colors.background));
        palette.setColor(QPalette::ToolTipBase, QColor("#3d3d3d"));
        palette.setColor(QPalette::ToolTipText, QColor(colors.text));
        palette.setColor(QPalette::Text, QColor(colors.text));
        palette.setColor(QPalette::Button, QColor(colors.background));
        palette.setColor(QPalette::ButtonText, QColor(colors.text));
        palette.setColor(QPalette::BrightText, QColor("#ffffff"));
        palette.setColor(QPalette::Link, QColor(colors.accent));
        palette.setColor(QPalette::Highlight, QColor(colors.accent));
        palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
        
        // Disabled colors
        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(colors.textDisabled));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(colors.textDisabled));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(colors.textDisabled));
    } else {
        palette.setColor(QPalette::Window, QColor(colors.background));
        palette.setColor(QPalette::WindowText, QColor(colors.text));
        palette.setColor(QPalette::Base, QColor(colors.backgroundAlt));
        palette.setColor(QPalette::AlternateBase, QColor(colors.background));
        palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
        palette.setColor(QPalette::ToolTipText, QColor(colors.text));
        palette.setColor(QPalette::Text, QColor(colors.text));
        palette.setColor(QPalette::Button, QColor(colors.background));
        palette.setColor(QPalette::ButtonText, QColor(colors.text));
        palette.setColor(QPalette::BrightText, QColor("#000000"));
        palette.setColor(QPalette::Link, QColor(colors.accent));
        palette.setColor(QPalette::Highlight, QColor(colors.accent));
        palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    }
    
    return palette;
}

ThemeManager::Colors ThemeManager::getColors(Theme theme) {
    Colors colors;
    
    if (theme == Theme::Dark) {
        // === GEANT4/Scientific Dark Theme ===
        // Inspired by VS Code Dark+, JetBrains Darcula
        
        // Backgrounds - Rich charcoal tones
        colors.background = "#1e1e1e";
        colors.backgroundAlt = "#252526";
        colors.backgroundHover = "#2a2d2e";
        colors.backgroundSelected = "#094771";
        
        // Text - High contrast for readability
        colors.text = "#d4d4d4";
        colors.textSecondary = "#858585";
        colors.textDisabled = "#5a5a5a";
        
        // Accent - Scientific blue (CERN-inspired)
        colors.accent = "#0078d4";
        colors.accentHover = "#1a8cff";
        colors.accentPressed = "#005a9e";
        
        // Borders
        colors.border = "#3c3c3c";
        colors.borderLight = "#454545";
        colors.borderFocus = "#0078d4";
        
        // Status
        colors.success = "#4ec9b0";
        colors.warning = "#dcdcaa";
        colors.error = "#f14c4c";
        colors.info = "#3794ff";
        
        // Special
        colors.shadow = "#000000";
        colors.highlight = "#264f78";
        
    } else {
        // === Light Theme ===
        colors.background = "#f3f3f3";
        colors.backgroundAlt = "#ffffff";
        colors.backgroundHover = "#e8e8e8";
        colors.backgroundSelected = "#cce5ff";
        
        colors.text = "#1e1e1e";
        colors.textSecondary = "#6e6e6e";
        colors.textDisabled = "#a0a0a0";
        
        colors.accent = "#0066cc";
        colors.accentHover = "#0078d4";
        colors.accentPressed = "#004c99";
        
        colors.border = "#d4d4d4";
        colors.borderLight = "#e0e0e0";
        colors.borderFocus = "#0066cc";
        
        colors.success = "#28a745";
        colors.warning = "#ffc107";
        colors.error = "#dc3545";
        colors.info = "#17a2b8";
        
        colors.shadow = "#00000020";
        colors.highlight = "#e6f2ff";
    }
    
    return colors;
}

QString ThemeManager::getStyleSheet(Theme theme) {
    if (theme == Theme::Dark) {
        return getDarkStyleSheet();
    } else {
        return getLightStyleSheet();
    }
}

QString ThemeManager::getDarkStyleSheet() {
    return R"(
/* === GEANTCAD DARK THEME === */

/* Global */
* {
    font-family: "Segoe UI", "SF Pro Display", -apple-system, sans-serif;
    font-size: 13px;
}

QMainWindow {
    background-color: #1e1e1e;
}

/* === MENUS === */
QMenuBar {
    background-color: #2d2d2d;
    color: #d4d4d4;
    border-bottom: 1px solid #3c3c3c;
    padding: 2px 0;
}

QMenuBar::item {
    padding: 5px 10px;
    border-radius: 4px;
    margin: 2px;
}

QMenuBar::item:selected {
    background-color: #3a3d3e;
}

QMenuBar::item:pressed {
    background-color: #094771;
}

QMenu {
    background-color: #2d2d2d;
    border: 1px solid #454545;
    border-radius: 6px;
    padding: 4px;
}

QMenu::item {
    padding: 6px 30px 6px 20px;
    border-radius: 4px;
    margin: 2px 4px;
}

QMenu::item:selected {
    background-color: #094771;
}

QMenu::separator {
    height: 1px;
    background-color: #3c3c3c;
    margin: 4px 10px;
}

QMenu::indicator {
    width: 16px;
    height: 16px;
    margin-left: 4px;
}

/* === TOOLBAR === */
QToolBar {
    background-color: #2d2d2d;
    border: none;
    border-bottom: 1px solid #3c3c3c;
    spacing: 4px;
    padding: 4px;
}

QToolBar::separator {
    width: 1px;
    background-color: #454545;
    margin: 4px 8px;
}

QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 6px;
    margin: 1px;
}

QToolButton:hover {
    background-color: #3a3d3e;
    border-color: #454545;
}

QToolButton:pressed, QToolButton:checked {
    background-color: #094771;
    border-color: #0078d4;
}

/* === DOCK WIDGETS === */
QDockWidget {
    color: #d4d4d4;
    titlebar-close-icon: url(close.png);
    titlebar-normal-icon: url(float.png);
}

QDockWidget::title {
    background-color: #2d2d2d;
    border: 1px solid #3c3c3c;
    border-bottom: none;
    padding: 8px 10px;
    text-align: left;
    font-weight: 600;
}

QDockWidget::close-button, QDockWidget::float-button {
    background-color: transparent;
    border: none;
    padding: 2px;
}

QDockWidget::close-button:hover, QDockWidget::float-button:hover {
    background-color: #3a3d3e;
    border-radius: 3px;
}

/* === GROUP BOX === */
QGroupBox {
    font-weight: 600;
    border: 1px solid #3c3c3c;
    border-radius: 6px;
    margin-top: 12px;
    padding-top: 10px;
    background-color: #252526;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 2px 8px;
    background-color: #2d2d2d;
    border: 1px solid #3c3c3c;
    border-radius: 4px;
    left: 10px;
}

/* === BUTTONS === */
QPushButton {
    background-color: #3c3c3c;
    color: #d4d4d4;
    border: 1px solid #454545;
    border-radius: 4px;
    padding: 6px 16px;
    min-width: 70px;
    font-weight: 500;
}

QPushButton:hover {
    background-color: #454545;
    border-color: #5a5a5a;
}

QPushButton:pressed {
    background-color: #094771;
    border-color: #0078d4;
}

QPushButton:disabled {
    background-color: #2d2d2d;
    color: #5a5a5a;
    border-color: #3c3c3c;
}

QPushButton:default {
    background-color: #0078d4;
    border-color: #0078d4;
    color: #ffffff;
}

QPushButton:default:hover {
    background-color: #1a8cff;
}

/* === INPUT FIELDS === */
QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox {
    background-color: #1e1e1e;
    color: #d4d4d4;
    border: 1px solid #3c3c3c;
    border-radius: 4px;
    padding: 5px 8px;
    selection-background-color: #094771;
}

QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus,
QSpinBox:focus, QDoubleSpinBox:focus {
    border-color: #0078d4;
}

QLineEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled {
    background-color: #252526;
    color: #5a5a5a;
}

QSpinBox::up-button, QDoubleSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    border-left: 1px solid #3c3c3c;
    border-bottom: 1px solid #3c3c3c;
    border-top-right-radius: 3px;
    width: 18px;
    background-color: #3c3c3c;
}

QSpinBox::down-button, QDoubleSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    border-left: 1px solid #3c3c3c;
    border-bottom-right-radius: 3px;
    width: 18px;
    background-color: #3c3c3c;
}

QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,
QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
    background-color: #454545;
}

/* === COMBO BOX === */
QComboBox {
    background-color: #1e1e1e;
    color: #d4d4d4;
    border: 1px solid #3c3c3c;
    border-radius: 4px;
    padding: 5px 8px;
    min-width: 100px;
}

QComboBox:hover {
    border-color: #454545;
}

QComboBox:focus {
    border-color: #0078d4;
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: center right;
    width: 20px;
    border: none;
}

QComboBox::down-arrow {
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 5px solid #858585;
    margin-right: 8px;
}

QComboBox QAbstractItemView {
    background-color: #2d2d2d;
    border: 1px solid #454545;
    border-radius: 4px;
    selection-background-color: #094771;
    outline: none;
}

QComboBox QAbstractItemView::item {
    padding: 5px 10px;
    min-height: 24px;
}

/* === CHECK BOX === */
QCheckBox {
    color: #d4d4d4;
    spacing: 8px;
}

QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border: 1px solid #5a5a5a;
    border-radius: 3px;
    background-color: #1e1e1e;
}

QCheckBox::indicator:hover {
    border-color: #0078d4;
}

QCheckBox::indicator:checked {
    background-color: #0078d4;
    border-color: #0078d4;
    image: url(check.png);
}

QCheckBox::indicator:disabled {
    background-color: #2d2d2d;
    border-color: #3c3c3c;
}

/* === TREE VIEW / LIST VIEW === */
QTreeView, QListView, QTableView {
    background-color: #1e1e1e;
    alternate-background-color: #252526;
    color: #d4d4d4;
    border: 1px solid #3c3c3c;
    border-radius: 4px;
    outline: none;
}

QTreeView::item, QListView::item {
    padding: 4px 8px;
    border-radius: 3px;
    margin: 1px 2px;
}

QTreeView::item:hover, QListView::item:hover {
    background-color: #2a2d2e;
}

QTreeView::item:selected, QListView::item:selected {
    background-color: #094771;
}

QTreeView::branch:has-siblings:!adjoins-item {
    border-image: url(vline.png) 0;
}

QTreeView::branch:has-siblings:adjoins-item {
    border-image: url(branch-more.png) 0;
}

QTreeView::branch:!has-children:!has-siblings:adjoins-item {
    border-image: url(branch-end.png) 0;
}

QHeaderView::section {
    background-color: #2d2d2d;
    color: #d4d4d4;
    padding: 6px 10px;
    border: none;
    border-right: 1px solid #3c3c3c;
    border-bottom: 1px solid #3c3c3c;
    font-weight: 600;
}

QHeaderView::section:hover {
    background-color: #3a3d3e;
}

/* === SCROLL BARS === */
QScrollBar:vertical {
    background-color: #1e1e1e;
    width: 12px;
    margin: 0;
    border-radius: 6px;
}

QScrollBar::handle:vertical {
    background-color: #5a5a5a;
    min-height: 30px;
    border-radius: 5px;
    margin: 2px;
}

QScrollBar::handle:vertical:hover {
    background-color: #787878;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background-color: #1e1e1e;
    height: 12px;
    margin: 0;
    border-radius: 6px;
}

QScrollBar::handle:horizontal {
    background-color: #5a5a5a;
    min-width: 30px;
    border-radius: 5px;
    margin: 2px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #787878;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}

/* === TAB WIDGET === */
QTabWidget::pane {
    border: 1px solid #3c3c3c;
    border-radius: 4px;
    background-color: #252526;
}

QTabBar::tab {
    background-color: #2d2d2d;
    color: #858585;
    padding: 8px 16px;
    border: 1px solid #3c3c3c;
    border-bottom: none;
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
    margin-right: 2px;
}

QTabBar::tab:hover {
    background-color: #3a3d3e;
    color: #d4d4d4;
}

QTabBar::tab:selected {
    background-color: #252526;
    color: #d4d4d4;
    border-bottom: 2px solid #0078d4;
}

/* === SLIDER === */
QSlider::groove:horizontal {
    border: 1px solid #3c3c3c;
    height: 6px;
    background-color: #1e1e1e;
    border-radius: 3px;
}

QSlider::handle:horizontal {
    background-color: #0078d4;
    border: none;
    width: 16px;
    height: 16px;
    margin: -5px 0;
    border-radius: 8px;
}

QSlider::handle:horizontal:hover {
    background-color: #1a8cff;
}

QSlider::sub-page:horizontal {
    background-color: #0078d4;
    border-radius: 3px;
}

/* === PROGRESS BAR === */
QProgressBar {
    background-color: #1e1e1e;
    border: 1px solid #3c3c3c;
    border-radius: 4px;
    height: 8px;
    text-align: center;
}

QProgressBar::chunk {
    background-color: #0078d4;
    border-radius: 3px;
}

/* === TOOLTIP === */
QToolTip {
    background-color: #3d3d3d;
    color: #d4d4d4;
    border: 1px solid #5a5a5a;
    border-radius: 4px;
    padding: 6px 10px;
}

/* === STATUS BAR === */
QStatusBar {
    background-color: #007acc;
    color: #ffffff;
    border-top: 1px solid #006bb3;
}

QStatusBar::item {
    border: none;
}

/* === SPLITTER === */
QSplitter::handle {
    background-color: #3c3c3c;
}

QSplitter::handle:horizontal {
    width: 2px;
}

QSplitter::handle:vertical {
    height: 2px;
}

QSplitter::handle:hover {
    background-color: #0078d4;
}

/* === DIALOG === */
QDialog {
    background-color: #2d2d2d;
}

QDialogButtonBox {
    button-layout: 2;
}

/* === LABEL === */
QLabel {
    color: #d4d4d4;
}

QLabel:disabled {
    color: #5a5a5a;
}

)";
}

QString ThemeManager::getLightStyleSheet() {
    return R"(
/* === GEANTCAD LIGHT THEME === */

* {
    font-family: "Segoe UI", "SF Pro Display", -apple-system, sans-serif;
    font-size: 13px;
}

QMainWindow {
    background-color: #f3f3f3;
}

QMenuBar {
    background-color: #ffffff;
    border-bottom: 1px solid #d4d4d4;
}

QMenuBar::item:selected {
    background-color: #e8e8e8;
}

QMenu {
    background-color: #ffffff;
    border: 1px solid #d4d4d4;
}

QMenu::item:selected {
    background-color: #cce5ff;
}

QToolBar {
    background-color: #ffffff;
    border-bottom: 1px solid #d4d4d4;
}

QToolButton:hover {
    background-color: #e8e8e8;
}

QToolButton:pressed, QToolButton:checked {
    background-color: #cce5ff;
}

QGroupBox {
    border: 1px solid #d4d4d4;
    background-color: #ffffff;
}

QPushButton {
    background-color: #e8e8e8;
    border: 1px solid #d4d4d4;
}

QPushButton:hover {
    background-color: #d4d4d4;
}

QPushButton:default {
    background-color: #0066cc;
    color: #ffffff;
}

QLineEdit, QTextEdit, QSpinBox, QDoubleSpinBox, QComboBox {
    background-color: #ffffff;
    border: 1px solid #d4d4d4;
}

QTreeView, QListView {
    background-color: #ffffff;
    border: 1px solid #d4d4d4;
}

QTreeView::item:selected {
    background-color: #cce5ff;
}

QStatusBar {
    background-color: #0066cc;
    color: #ffffff;
}

)";
}

} // namespace geantcad

