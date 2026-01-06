#pragma once

#include <QApplication>
#include <QPalette>
#include <QString>
#include <QFont>

namespace geantcad {

/**
 * ThemeManager - Gestisce il tema dell'applicazione
 * Supporta Dark e Light theme con palette professionali
 */
class ThemeManager {
public:
    enum class Theme {
        Dark,
        Light,
        System  // Follows system preference
    };
    
    /**
     * Apply theme to the application
     */
    static void applyTheme(Theme theme);
    
    /**
     * Get current theme
     */
    static Theme currentTheme() { return currentTheme_; }
    
    /**
     * Get stylesheet for the current theme
     */
    static QString getStyleSheet(Theme theme);
    
    /**
     * Get palette for theme
     */
    static QPalette getPalette(Theme theme);
    
    /**
     * Theme colors - for custom widgets
     */
    struct Colors {
        // Backgrounds
        QString background;
        QString backgroundAlt;
        QString backgroundHover;
        QString backgroundSelected;
        
        // Foregrounds
        QString text;
        QString textSecondary;
        QString textDisabled;
        
        // Accents
        QString accent;
        QString accentHover;
        QString accentPressed;
        
        // Borders
        QString border;
        QString borderLight;
        QString borderFocus;
        
        // Status colors
        QString success;
        QString warning;
        QString error;
        QString info;
        
        // Special
        QString shadow;
        QString highlight;
    };
    
    static Colors getColors(Theme theme);
    static Colors currentColors() { return getColors(currentTheme_); }

private:
    static Theme currentTheme_;
    static QString getDarkStyleSheet();
    static QString getLightStyleSheet();
};

} // namespace geantcad

