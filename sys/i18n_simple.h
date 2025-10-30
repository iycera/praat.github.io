/* i18n_simple.h
 *
 * Copyright (C) 2025 Praat i18n Implementation - Simplified Version
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _i18n_simple_h_
#define _i18n_simple_h_

#include "melder.h"
#include "Gui.h"  // Include Gui.h for GuiWindow type
#include <vector>
#include <string>
#include <map>
#include <string>  // For std::u32string

/*
 * Simplified i18n (Internationalization) system for Praat
 * 
 * This system provides:
 * - Language switching through menu
 * - JSON-based language packs
 * - Dynamic menu translation
 * - Persistent language preference
 * 
 * Architecture:
 * - Uses standard C++ containers instead of Praat's OrderedOf
 * - Uses simple structs instead of Praat's Thing system
 * - Minimal memory management to avoid corruption
 */

// Simple language info structure
struct SimpleLanguage {
    std::string code;
    std::string name;
    std::string file;
    bool isLoaded;
    
    SimpleLanguage() : isLoaded(false) {}
    SimpleLanguage(const std::string& c, const std::string& n, const std::string& f) 
        : code(c), name(n), file(f), isLoaded(false) {}
};

// Simple translation structure
struct SimpleTranslation {
    std::string key;
    std::string value;
    
    SimpleTranslation() {}
    SimpleTranslation(const std::string& k, const std::string& v) 
        : key(k), value(v) {}
};

// Simple i18n manager
class SimpleI18nManager {
private:
    std::string currentLanguage;
    std::vector<SimpleLanguage> availableLanguages;
    std::map<std::u32string, std::u32string> translations;  // UTF-32存储
    bool isInitialized;
    
    // Menu references for updating
    GuiWindow mainWindow;
    GuiMenu newMenu;
    GuiMenu readMenu;
    GuiMenu helpMenu;
    GuiMenu i18nMenu;
    
    // Language menu items for checkmark updates
    std::map<std::string, GuiMenuItem> languageMenuItems;
    
public:
    SimpleI18nManager();
    ~SimpleI18nManager();
    
    // Core functions
    void init();
    void exit();
    
    // Menu update functions
    void updateLanguageMenuCheckmarks();
    void loadLanguagePack(const std::string& languageCode);
    void setLanguage(const std::string& languageCode);
    std::string getCurrentLanguage() const;
    std::string translate(const std::string& key) const;
    const char32* translateUTF32(const char* key) const;  // UTF-32翻译函数
    
    // Placeholder support functions
    std::string translateWithPlaceholders(const std::string& key, const std::map<std::string, std::string>& placeholders) const;
    const char32* translateUTF32WithPlaceholders(const char* key, const std::map<std::string, std::string>& placeholders) const;
    std::string replacePlaceholders(const std::string& text, const std::map<std::string, std::string>& placeholders) const;
    
    // Language management
    void registerLanguage(const std::string& code, const std::string& name, const std::string& file);
    void loadLanguageIndex();
    std::vector<SimpleLanguage> getAvailableLanguages() const;
    
    // Menu functions
    void createMenu(GuiWindow window);
    void updateAllMenus();
    
    // Menu management
    void setMenuReferences(GuiWindow window, GuiMenu newMenu, GuiMenu readMenu, GuiMenu helpMenu);
    void recreateMenus();
    void recreateI18nMenu();
    
    // Preferences
    void preferences();
    void preferencesChanged();
    void saveLanguagePreference(const std::string& languageCode);
    std::string loadLanguagePreference();
};

// Global instance
extern SimpleI18nManager* g_i18nManager;

// C-style interface for compatibility with Praat
extern "C" {
    void I18n_init();
    void I18n_exit();
    void I18n_loadLanguagePack(const char* languageCode);
    void I18n_setLanguage(const char* languageCode);
    const char* I18n_getCurrentLanguage();
    const char32* I18n_translate(const char* key);
    
    // Placeholder support C interface
    const char32* I18n_translateWithPlaceholders(const char* key, const char* placeholder1, const char* value1, ...);
    const char32* I18n_translateWithPlaceholdersMap(const char* key, const char* placeholders);
    
    void I18n_registerLanguage(const char* code, const char* name, const char* file);
    void I18n_loadLanguageIndex();
    
    void I18n_addToMenuBar(GuiWindow window);
    void I18n_updateAllMenus();
    void I18n_setMenuReferences(GuiWindow window, GuiMenu newMenu, GuiMenu readMenu, GuiMenu helpMenu);
    
    void I18n_preferences();
    void I18n_preferencesChanged();
}

// Translation macros
#define I18N_MENU_EDIT "Edit"
#define I18N_MENU_WINDOW "Window"
#define I18N_MENU_PRAAT "Praat"
#define I18N_MENU_NEW "New"
#define I18N_MENU_OPEN "Open"
#define I18N_MENU_HELP "Help"
#define I18N_MENU_CUT "Cut"
#define I18N_MENU_COPY "Copy"
#define I18N_MENU_PASTE "Paste"
#define I18N_MENU_MINIMIZE "Minimize"
#define I18N_MENU_ZOOM "Zoom"
#define I18N_MENU_CLOSE "Close"
#define I18N_MENU_NEW_SCRIPT "New Praat script..."
#define I18N_MENU_NEW_NOTEBOOK "New Praat notebook..."
#define I18N_MENU_OPEN_SCRIPT "Open Praat script..."
#define I18N_MENU_OPEN_NOTEBOOK "Open Praat notebook..."
#define I18N_MENU_GOODIES "Goodies"
#define I18N_MENU_CALCULATOR "Calculator..."
#define I18N_MENU_REPORT_DIFFERENCE "Report difference of two proportions..."
#define I18N_MENU_SAVE_DEMO_PDF "Save Demo window as PDF file..."
#define I18N_MENU_SETTINGS "Settings"
#define I18N_MENU_BUTTONS "Buttons..."

#endif /* _i18n_simple_h_ */
