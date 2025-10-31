/* i18n_simple.cpp
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

#include "i18n_simple.h"
#include "praatP.h"
#include "Gui.h"
#include "GraphicsP.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdarg>

// Global instance
SimpleI18nManager* g_i18nManager = nullptr;

// SimpleI18nManager implementation
SimpleI18nManager::SimpleI18nManager() 
    : currentLanguage("en-US"), isInitialized(false) {
}

SimpleI18nManager::~SimpleI18nManager() {
    // Cleanup handled by destructors
}

void SimpleI18nManager::init() {
    if (isInitialized) return;
    
    // Load language index
    loadLanguageIndex();
    
    // Load saved language preference
    std::string savedLanguage = loadLanguagePreference();
    
    // Set the current language
    currentLanguage = savedLanguage;
    
    // Load the language pack
    loadLanguagePack(currentLanguage);
    
    // Update all menus and UI elements
    updateAllMenus();
    
    isInitialized = true;
}

void SimpleI18nManager::exit() {
    if (!isInitialized) return;
    
    // Cleanup
    availableLanguages.clear();
    translations.clear();
    languageMenuItems.clear();
    isInitialized = false;
}

void SimpleI18nManager::updateLanguageMenuCheckmarks() {
    // Update checkmarks for all language menu items
    for (const auto& pair : languageMenuItems) {
        const std::string& langCode = pair.first;
        GuiMenuItem menuItem = pair.second;
        
        // Check if this is the current language
        bool isCurrentLanguage = (langCode == currentLanguage);
        
        // Update checkmark state
        GuiMenuItem_check(menuItem, isCurrentLanguage);
    }
}

void SimpleI18nManager::loadLanguagePack(const std::string& languageCode) {
    // Find the language file name
    std::string fileName;
    for (const auto& lang : availableLanguages) {
        if (lang.code == languageCode) {
            fileName = lang.file;
            break;
        }
    }
    
    if (fileName.empty()) {
        // Use fallback
        fileName = languageCode + ".json";
    }
    
    // Load the language pack file
    std::string filePath = "sys/language_packs/" + fileName;
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        // Try alternative paths
        std::vector<std::string> paths = {
            "language_packs/" + fileName,
            "../sys/language_packs/" + fileName,
            "../../sys/language_packs/" + fileName
        };
        
        for (const auto& path : paths) {
            file.open(path);
            if (file.is_open()) break;
        }
    }
    
    if (!file.is_open()) {
        return;
    }
    
    // Parse JSON file and store as UTF-32
    std::string line;
    while (std::getline(file, line)) {
        // Simple JSON parsing - look for "key": "value" patterns
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Remove quotes and whitespace
            key.erase(0, key.find_first_not_of(" \t\""));
            key.erase(key.find_last_not_of(" \t\"") + 1);
            value.erase(0, value.find_first_not_of(" \t\""));
            value.erase(value.find_last_not_of(" \t\",") + 1);
            
            if (!key.empty() && !value.empty()) {
                // Convert both key and value to UTF-32 for storage
                autostring32 utf32Key = Melder_8to32(key.c_str());
                autostring32 utf32Value = Melder_8to32(value.c_str());
                translations[utf32Key.get()] = utf32Value.get();
            }
        }
    }
    
    file.close();
}

void SimpleI18nManager::setLanguage(const std::string& languageCode) {
    currentLanguage = languageCode;
    
    // Load the language pack
    loadLanguagePack(languageCode);
    
    // Save the language preference
    saveLanguagePreference(languageCode);
    
    // Update all menus and UI elements
    updateAllMenus();
    
    // Update language menu checkmarks
    updateLanguageMenuCheckmarks();
}

std::string SimpleI18nManager::getCurrentLanguage() const {
    return currentLanguage;
}

std::string SimpleI18nManager::translate(const std::string& key) const {
    autostring32 utf32Key = Melder_8to32(key.c_str());
    auto it = translations.find(utf32Key.get());
    if (it != translations.end()) {
        // Convert UTF-32 back to UTF-8 for compatibility
        autostring8 result = Melder_32to8(it->second.c_str());
        return std::string(result.get());
    }
    return key; // Return key if translation not found
}

// UTF-32 translation function - direct return without conversion
const char32* SimpleI18nManager::translateUTF32(const char* key) const {
    autostring32 utf32Key = Melder_8to32(key);
    auto it = translations.find(utf32Key.get());
    if (it != translations.end()) {
        return it->second.c_str();
    }
    return utf32Key.get();  // Return original key if not found
}

// Placeholder support functions
std::string SimpleI18nManager::translateWithPlaceholders(const std::string& key, const std::map<std::string, std::string>& placeholders) const {
    std::string translated = translate(key);
    return replacePlaceholders(translated, placeholders);
}

const char32* SimpleI18nManager::translateUTF32WithPlaceholders(const char* key, const std::map<std::string, std::string>& placeholders) const {
    autostring32 utf32Key = Melder_8to32(key);
    auto it = translations.find(utf32Key.get());
    if (it != translations.end()) {
        // Convert to string, replace placeholders, then convert back
        autostring8 temp = Melder_32to8(it->second.c_str());
        std::string result = replacePlaceholders(std::string(temp.get()), placeholders);
        autostring32 result32 = Melder_8to32(result.c_str());
        return result32.get();
    }
    return utf32Key.get();  // Return original key if not found
}

std::string SimpleI18nManager::replacePlaceholders(const std::string& text, const std::map<std::string, std::string>& placeholders) const {
    std::string result = text;
    
    // Replace placeholders in format {key}
    for (const auto& pair : placeholders) {
        std::string placeholder = "{" + pair.first + "}";
        std::string value = pair.second;
        
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    
    return result;
}

void SimpleI18nManager::registerLanguage(const std::string& code, const std::string& name, const std::string& file) {
    availableLanguages.emplace_back(code, name, file);
}

void SimpleI18nManager::loadLanguageIndex() {
    // Load languages.json
    std::string filePath = "sys/language_packs/languages.json";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        // Try alternative paths
        std::vector<std::string> paths = {
            "language_packs/languages.json",
            "../sys/language_packs/languages.json",
            "../../sys/language_packs/languages.json"
        };
        
        for (const auto& path : paths) {
            file.open(path);
            if (file.is_open()) break;
        }
    }
    
    if (!file.is_open()) {
        // Add fallback languages
        registerLanguage("en-US", "English", "en-US.json");
        registerLanguage("zh-CN", "Chinese Simplified", "zh-CN.json");
        registerLanguage("zh-TW", "Chinese Traditional", "zh-TW.json");
        registerLanguage("ja-JP", "Japanese", "ja-JP.json");
        
        return;
    }
    
    // Parse JSON file
    std::string line;
    std::string currentCode, currentName, currentFile;
    bool inLanguage = false;
    
    while (std::getline(file, line)) {
        // Simple JSON parsing - look for "field": "value" pattern
        if (line.find("\"code\"") != std::string::npos) {
            // Find the colon after "code"
            size_t colonPos = line.find(':', line.find("\"code\""));
            if (colonPos != std::string::npos) {
                // Find the first quote after the colon
                size_t start = line.find('"', colonPos);
                if (start != std::string::npos) {
                    // Find the closing quote
                    size_t end = line.find('"', start + 1);
                    if (end != std::string::npos) {
                        currentCode = line.substr(start + 1, end - start - 1);
                    }
                }
            }
        } else if (line.find("\"name\"") != std::string::npos) {
            // Find the colon after "name"
            size_t colonPos = line.find(':', line.find("\"name\""));
            if (colonPos != std::string::npos) {
                // Find the first quote after the colon
                size_t start = line.find('"', colonPos);
                if (start != std::string::npos) {
                    // Find the closing quote
                    size_t end = line.find('"', start + 1);
                    if (end != std::string::npos) {
                        currentName = line.substr(start + 1, end - start - 1);
                    }
                }
            }
        } else if (line.find("\"file\"") != std::string::npos) {
            // Find the colon after "file"
            size_t colonPos = line.find(':', line.find("\"file\""));
            if (colonPos != std::string::npos) {
                // Find the first quote after the colon
                size_t start = line.find('"', colonPos);
                if (start != std::string::npos) {
                    // Find the closing quote
                    size_t end = line.find('"', start + 1);
                    if (end != std::string::npos) {
                        currentFile = line.substr(start + 1, end - start - 1);
                    }
                }
            }
        } else if (line.find('}') != std::string::npos && !currentCode.empty()) {
            // End of language object
            registerLanguage(currentCode, currentName, currentFile);
            currentCode.clear();
            currentName.clear();
            currentFile.clear();
        }
    }
    
    file.close();
}

std::vector<SimpleLanguage> SimpleI18nManager::getAvailableLanguages() const {
    return availableLanguages;
}

// Global translation function for use by other parts of Praat
const char32* I18n_translate(const char* key) {
    if (!g_i18nManager) {
        // Return original key if i18n system is not initialized
        autostring32 result = Melder_8to32(key);
        return result.get();
    }
    
    // Use UTF-32 translation function
    return g_i18nManager->translateUTF32(key);
}

// Placeholder support C interface functions
const char32* I18n_translateWithPlaceholders(const char* key, const char* placeholder1, const char* value1, ...) {
    if (!g_i18nManager) {
        autostring32 result = Melder_8to32(key);
        return result.get();
    }
    
    std::map<std::string, std::string> placeholders;
    
    // Add first placeholder
    if (placeholder1 && value1) {
        placeholders[placeholder1] = value1;
    }
    
    // Add additional placeholders using va_list
    va_list args;
    va_start(args, value1);
    
    const char* placeholder = va_arg(args, const char*);
    const char* value = va_arg(args, const char*);
    
    while (placeholder && value) {
        placeholders[placeholder] = value;
        placeholder = va_arg(args, const char*);
        value = va_arg(args, const char*);
    }
    
    va_end(args);
    
    return g_i18nManager->translateUTF32WithPlaceholders(key, placeholders);
}

const char32* I18n_translateWithPlaceholdersMap(const char* key, const char* placeholders) {
    if (!g_i18nManager) {
        autostring32 result = Melder_8to32(key);
        return result.get();
    }
    
    // Parse placeholders from JSON-like string format: "key1:value1,key2:value2"
    std::map<std::string, std::string> placeholderMap;
    
    if (placeholders) {
        std::string input = placeholders;
        std::istringstream ss(input);
        std::string pair;
        
        while (std::getline(ss, pair, ',')) {
            size_t colonPos = pair.find(':');
            if (colonPos != std::string::npos) {
                std::string placeholder = pair.substr(0, colonPos);
                std::string value = pair.substr(colonPos + 1);
                placeholderMap[placeholder] = value;
            }
        }
    }
    
    return g_i18nManager->translateUTF32WithPlaceholders(key, placeholderMap);
}

// Static callback functions for each language
static void switchToEnglish(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("en-US");
    }
}

static void switchToChineseSimplified(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("zh-CN");
    }
}

static void switchToChineseTraditional(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("zh-TW");
    }
}

static void switchToJapanese(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("ja-JP");
    }
}

static void switchToKorean(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("ko-KR");
    }
}

static void switchToFrench(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("fr-FR");
    }
}

static void switchToGerman(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("de-DE");
    }
}

static void switchToSpanish(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("es-ES");
    }
}

static void switchToRussian(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("ru-RU");
    }
}

static void switchToPortuguese(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("pt-PT");
    }
}

static void switchToLkCn(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("lk-CN");
    }
}

static void switchToItalian(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        g_i18nManager->setLanguage("it-IT");
    }
}

void SimpleI18nManager::createMenu(GuiWindow window) {
    
    // Create the i18n menu
    GuiMenu i18nMenuRef = GuiMenu_createInWindow(window, U"i18n", 0);
    
    // Create language submenu
    GuiMenu languageSubmenu = GuiMenu_createInMenu(i18nMenuRef, I18n_translate("menu.language"), 0);
    
    // Add language options dynamically from availableLanguages
    for (const auto& lang : availableLanguages) {
        // Convert std::string to char32_t*
        autostring32 langName = Melder_8to32(lang.name.c_str());
        
        // Map language codes to specific static callbacks
        GuiMenuItemCallback callback = nullptr;
        
        if (lang.code == "en-US") {
            callback = GuiMenuItemCallback(switchToEnglish);
        } else if (lang.code == "zh-CN") {
            callback = GuiMenuItemCallback(switchToChineseSimplified);
        } else if (lang.code == "zh-TW") {
            callback = GuiMenuItemCallback(switchToChineseTraditional);
        } else if (lang.code == "ja-JP") {
            callback = GuiMenuItemCallback(switchToJapanese);
        } else if (lang.code == "ko-KR") {
            callback = GuiMenuItemCallback(switchToKorean);
        } else if (lang.code == "fr-FR") {
            callback = GuiMenuItemCallback(switchToFrench);
        } else if (lang.code == "de-DE") {
            callback = GuiMenuItemCallback(switchToGerman);
        } else if (lang.code == "es-ES") {
            callback = GuiMenuItemCallback(switchToSpanish);
        } else if (lang.code == "ru-RU") {
            callback = GuiMenuItemCallback(switchToRussian);
        } else if (lang.code == "pt-PT") {
            callback = GuiMenuItemCallback(switchToPortuguese);
        } else if (lang.code == "lk-CN") {
            callback = GuiMenuItemCallback(switchToLkCn);
        } else if (lang.code == "it-IT") {
            callback = GuiMenuItemCallback(switchToItalian);
        }
        
        if (callback) {
            // Check if this is the current language
            bool isCurrentLanguage = (lang.code == currentLanguage);
            
            // Use original language name
            autostring32 displayName = Melder_8to32(Melder_32to8(langName.get()).get());
            
            // Add menu item with callback and CHECKBUTTON flag
            uint32 flags = GuiMenu_CHECKBUTTON;
            GuiMenuItem menuItem = GuiMenu_addItem(languageSubmenu, displayName.get(), flags, callback, nullptr);
            
            // Store menu item reference for later updates
            languageMenuItems[lang.code] = menuItem;
            
            // Set initial checkmark state
            GuiMenuItem_check(menuItem, isCurrentLanguage);
        }
    }
    
    // Update checkmarks after all menu items are created
    updateLanguageMenuCheckmarks();
}

void SimpleI18nManager::updateAllMenus() {
    
    // Log some key translations for debugging
    std::vector<std::string> keyTranslations = {
        "menu.new", "menu.open", "menu.save", "menu.help", "menu.about"
    };
    
    for (const auto& key : keyTranslations) {
        // Convert key to UTF-32 for lookup
        autostring32 utf32Key = Melder_8to32(key.c_str());
        auto it = translations.find(utf32Key.get());
        if (it != translations.end()) {
            // Convert UTF-32 value back to UTF-8 for debug output
            autostring8 utf8Value = Melder_32to8(it->second.c_str());
        } else {
        }
    }
    
    // Recreate menus with new translations
    recreateMenus();
}

void SimpleI18nManager::setMenuReferences(GuiWindow window, GuiMenu newMenuRef, GuiMenu readMenuRef, GuiMenu helpMenuRef) {
    this->mainWindow = window;
    this->newMenu = newMenuRef;
    this->readMenu = readMenuRef;
    this->helpMenu = helpMenuRef;
}

void SimpleI18nManager::recreateMenus() {
    if (!mainWindow) return;
    
    // Recreate the i18n menu with updated checkmarks
    recreateI18nMenu();
    
    // For other menus, we'll implement a simple notification system
    // TODO: Implement proper menu recreation for all menus
    // This would require:
    // 1. Storing all menu items and their callbacks
    // 2. Destroying existing menus
    // 3. Recreating menus with new translations
    // 4. Re-adding all menu items with their callbacks
    
    // For demonstration, we'll just log that we would update the menus
    // For now, we'll implement a simple approach:
    // We'll show a message to the user that the language has changed
    // and they need to restart the application for the changes to take effect
    
    // TODO: Implement proper menu recreation
    // This is a limitation of Praat's GUI system
}

void SimpleI18nManager::recreateI18nMenu() {
    if (!mainWindow) return;
    
    // Note: In Praat's GUI system, we cannot easily destroy and recreate menus
    // The menu items are already created and cannot be modified after creation
    // For a proper implementation, we would need to:
    // 1. Store all menu items and their callbacks
    // 2. Destroy the existing i18n menu
    // 3. Recreate the i18n menu with updated checkmarks
    // 4. Re-add all menu items with their callbacks
    
    // For now, we'll just log that we would recreate the menu
}

void SimpleI18nManager::preferences() {
    // This will be implemented later
}

void SimpleI18nManager::saveLanguagePreference(const std::string& languageCode) {
    
    // Save to a simple text file
    FILE* prefFile = fopen("praat_language_preference.txt", "w");
    if (prefFile) {
        fprintf(prefFile, "%s", languageCode.c_str());
        fclose(prefFile);
    }
}

std::string SimpleI18nManager::loadLanguagePreference() {
    
    // Load from the preference file using binary mode to handle encoding
    FILE* prefFile = fopen("praat_language_preference.txt", "rb");
    if (prefFile) {
        // Read the file content
        fseek(prefFile, 0, SEEK_END);
        long fileSize = ftell(prefFile);
        fseek(prefFile, 0, SEEK_SET);
        
        if (fileSize > 0 && fileSize < 100) {
            char* buffer = new char[fileSize + 1];
            fread(buffer, 1, fileSize, prefFile);
            buffer[fileSize] = '\0';
            
            std::string languageCode;
            
            // Check for UTF-16 BOM
            if (fileSize >= 2 && buffer[0] == (char)0xFF && buffer[1] == (char)0xFE) {
                // UTF-16 LE encoding - extract ASCII characters
                for (int i = 2; i < fileSize; i += 2) {
                    if (buffer[i] != '\0' && buffer[i] != '\r' && buffer[i] != '\n') {
                        languageCode += buffer[i];
                    }
                }
            } else {
                // Regular ASCII/UTF-8 encoding
                languageCode = buffer;
                // Remove newline if present
                size_t len = languageCode.length();
                if (len > 0 && languageCode[len-1] == '\n') {
                    languageCode.erase(len-1);
                }
                if (len > 1 && languageCode[len-2] == '\r') {
                    languageCode.erase(len-2);
                }
            }
            
            delete[] buffer;
            fclose(prefFile);
            
            return languageCode;
        }
        fclose(prefFile);
    }
    
    return "en-US"; // Default language
}

void SimpleI18nManager::preferencesChanged() {
    // This will be implemented later
}

// C-style interface implementation
extern "C" {
    void I18n_init() {
        if (!g_i18nManager) {
            g_i18nManager = new SimpleI18nManager();
            g_i18nManager->init();
            
            // Force CJK font style to Chinese for better Chinese character rendering
            extern kGraphics_cjkFontStyle theGraphicsCjkFontStyle;
            theGraphicsCjkFontStyle = kGraphics_cjkFontStyle::CHINESE;
        }
    }
    
    void I18n_exit() {
        if (g_i18nManager) {
            g_i18nManager->exit();
            delete g_i18nManager;
            g_i18nManager = nullptr;
        }
    }
    
    void I18n_loadLanguagePack(const char* languageCode) {
        if (g_i18nManager) {
            g_i18nManager->loadLanguagePack(std::string(languageCode));
        }
    }
    
    void I18n_setLanguage(const char* languageCode) {
        if (g_i18nManager) {
            g_i18nManager->setLanguage(std::string(languageCode));
        }
    }
    
    const char* I18n_getCurrentLanguage() {
        if (g_i18nManager) {
            static std::string result = g_i18nManager->getCurrentLanguage();
            return result.c_str();
        }
        return "en-US";
    }
    
    
    void I18n_registerLanguage(const char* code, const char* name, const char* file) {
        if (g_i18nManager) {
            g_i18nManager->registerLanguage(std::string(code), std::string(name), std::string(file));
        }
    }
    
    void I18n_loadLanguageIndex() {
        if (g_i18nManager) {
            g_i18nManager->loadLanguageIndex();
        }
    }
    
    void I18n_addToMenuBar(GuiWindow window) {
        if (g_i18nManager) {
            g_i18nManager->createMenu(window);
        }
    }
    
    void I18n_createMenu() {
        // This function is deprecated - use I18n_addToMenuBar(window) instead
    }
    
    void I18n_updateAllMenus() {
        if (g_i18nManager) {
            g_i18nManager->updateAllMenus();
        }
    }
    
void I18n_setMenuReferences(GuiWindow window, GuiMenu newMenu, GuiMenu readMenu, GuiMenu helpMenu) {
    if (g_i18nManager) {
        g_i18nManager->setMenuReferences(window, newMenu, readMenu, helpMenu);
    }
}

void I18n_preferences() {
    if (g_i18nManager) {
        g_i18nManager->preferences();
    }
}

void I18n_preferencesChanged() {
    if (g_i18nManager) {
        g_i18nManager->preferencesChanged();
    }
}
}
