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
#include "i18n_embedded_packs.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <utility>  // for std::pair
#if defined (_WIN32)
#include <windows.h>
#elif defined (__linux__) || defined (__unix__)
#include <unistd.h>
#include <limits.h>
#endif

// Global instance
SimpleI18nManager* g_i18nManager = nullptr;

// Generic callback function for language switching
// Uses closure to store language code and isBuiltin flag
struct LanguageSwitchClosure {
    std::string languageCode;
    bool isBuiltin;
};

// Store closures for language switching (will be cleaned up in exit)
static std::vector<LanguageSwitchClosure*> g_languageClosures;

// SimpleI18nManager implementation
SimpleI18nManager::SimpleI18nManager() 
    : currentLanguage("en-US"), currentLanguageIsBuiltin(true), isInitialized(false) {
}

SimpleI18nManager::~SimpleI18nManager() {
    // Cleanup handled by destructors
}

void SimpleI18nManager::init() {
    if (isInitialized) return;
    
    // Initialize external language pack path (exe directory/praat_i18n)
    // Try to get executable directory
    #if defined (_WIN32)
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string exeDir(exePath);
        size_t lastSlash = exeDir.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            exeDir = exeDir.substr(0, lastSlash + 1);
        }
        externalLanguagePackPath = exeDir + "praat_i18n";
        // Normalize path separators for Windows
        for (size_t i = 0; i < externalLanguagePackPath.length(); i++) {
            if (externalLanguagePackPath[i] == '/') {
                externalLanguagePackPath[i] = '\\';
            }
        }
    #elif defined (macintosh)
        // For macOS, we might need to get bundle path
        // For now, use current directory as fallback
        externalLanguagePackPath = "./praat_i18n";
    #else  // Unix/Linux
        // Try to get executable path from /proc/self/exe or use argv[0] if available
        char exePath[1024];
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (len != -1) {
            exePath[len] = '\0';
            std::string exeDir(exePath);
            size_t lastSlash = exeDir.find_last_of("/");
            if (lastSlash != std::string::npos) {
                exeDir = exeDir.substr(0, lastSlash + 1);
            }
            externalLanguagePackPath = exeDir + "praat_i18n";
        } else {
            externalLanguagePackPath = "./praat_i18n";
        }
    #endif
    
    // Load language index (both builtin and external)
    loadLanguageIndex();
    
    // Load saved language preference
    auto savedPref = loadLanguagePreference();
    currentLanguage = savedPref.first;
    currentLanguageIsBuiltin = savedPref.second;
    
    // Verify that the saved language pack is still available
    // If not, fallback to English (which is always available)
    if (!isLanguagePackAvailable(currentLanguage, currentLanguageIsBuiltin)) {
        currentLanguage = "en-US";
        currentLanguageIsBuiltin = true;
    }
    
    // Load the language pack (builtin first, then external as supplement)
    loadLanguagePack(currentLanguage, currentLanguageIsBuiltin);
    
    // Update all menus and UI elements
    updateAllMenus();
    
    isInitialized = true;
}

void SimpleI18nManager::exit() {
    if (!isInitialized) return;
    
    // Cleanup language closures
    for (auto* closure : g_languageClosures) {
        delete closure;
    }
    g_languageClosures.clear();
    
    // Cleanup
    availableLanguages.clear();
    translations.clear();
    languageMenuItems.clear();
    isInitialized = false;
}

void SimpleI18nManager::updateLanguageMenuCheckmarks() {
    // Update checkmarks for all language menu items
    // Format: "builtin:langCode" or "external:langCode"
    for (const auto& pair : languageMenuItems) {
        const std::string& langKey = pair.first;  // format: "builtin:langCode" or "external:langCode"
        GuiMenuItem menuItem = pair.second;
        
        // Extract isBuiltin and langCode from key
        size_t colonPos = langKey.find(':');
        if (colonPos != std::string::npos) {
            bool itemIsBuiltin = (langKey.substr(0, colonPos) == "builtin");
            std::string itemLangCode = langKey.substr(colonPos + 1);
        
        // Check if this is the current language
            bool isCurrentLanguage = (itemLangCode == currentLanguage && 
                                     itemIsBuiltin == currentLanguageIsBuiltin);
        
        // Update checkmark state
        GuiMenuItem_check(menuItem, isCurrentLanguage);
    }
    }
}

// Helper function to load translations from a JSON string
static void loadTranslationsFromString(const std::string& content, std::map<std::u32string, std::u32string>& translations, bool overwrite) {
    if (content.empty()) {
        return;
    }
    
    // Parse JSON string and store as UTF-32
    // Improved parsing to handle multi-line and proper JSON structure
    size_t pos = 0;
    while (pos < content.length()) {
        // Find the next key - look for "key":
        size_t keyStart = content.find('"', pos);
        if (keyStart == std::string::npos) break;
        
        size_t keyEnd = content.find('"', keyStart + 1);
        if (keyEnd == std::string::npos) break;
        
        std::string key = content.substr(keyStart + 1, keyEnd - keyStart - 1);
        
        // Find the colon after the key
        size_t colonPos = content.find(':', keyEnd);
        if (colonPos == std::string::npos) {
            pos = keyEnd + 1;
            continue;
        }
        
        // Find the value - look for "value"
        size_t valueStart = content.find('"', colonPos);
        if (valueStart == std::string::npos) {
            pos = colonPos + 1;
            continue;
        }
        
        size_t valueEnd = content.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) {
            pos = valueStart + 1;
            continue;
        }
        
        // Extract value (may contain escaped characters, but we'll handle basic cases)
        std::string value = content.substr(valueStart + 1, valueEnd - valueStart - 1);
        
        // Handle escaped characters (basic support)
        size_t escapePos = 0;
        while ((escapePos = value.find("\\\"", escapePos)) != std::string::npos) {
            value.replace(escapePos, 2, "\"");
            escapePos += 1;
        }
        while ((escapePos = value.find("\\\\", escapePos)) != std::string::npos) {
            value.replace(escapePos, 2, "\\");
            escapePos += 1;
        }
        while ((escapePos = value.find("\\n", escapePos)) != std::string::npos) {
            value.replace(escapePos, 2, "\n");
            escapePos += 1;
        }
        
        if (!key.empty() && !value.empty()) {
            // Convert both key and value to UTF-32 for storage
            autostring32 utf32Key = Melder_8to32(key.c_str());
            autostring32 utf32Value = Melder_8to32(value.c_str());
            
            // If overwrite is false (external), only add if key doesn't exist (supplement mode)
            if (overwrite || translations.find(utf32Key.get()) == translations.end()) {
                translations[utf32Key.get()] = utf32Value.get();
            }
        }
        
        // Move to next potential entry (after the closing quote and comma)
        pos = valueEnd + 1;
        // Skip comma if present
        while (pos < content.length() && (content[pos] == ',' || content[pos] == ' ' || content[pos] == '\t' || content[pos] == '\n' || content[pos] == '\r')) {
            pos++;
        }
    }
}

// Helper function to load translations from a JSON file
static void loadTranslationsFromFile(const std::string& filePath, std::map<std::u32string, std::u32string>& translations, bool overwrite) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return;
    }
    
    // Read entire file content
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Load from string
    loadTranslationsFromString(content, translations, overwrite);
}

void SimpleI18nManager::loadLanguagePack(const std::string& languageCode, bool isBuiltin) {
    // Clear existing translations
    translations.clear();
    
    std::string builtinFileName;
    std::string externalFileName;
    
    // Find file names from available languages
    for (const auto& lang : availableLanguages) {
        if (lang.code == languageCode) {
            if (lang.isBuiltin) {
                builtinFileName = lang.file;
            } else {
                externalFileName = lang.file;
            }
        }
    }
    
    // Use fallback if not found
    if (builtinFileName.empty()) {
        builtinFileName = languageCode + ".json";
    }
    if (externalFileName.empty()) {
        externalFileName = languageCode + ".json";
    }
    
    // Strategy: Always load builtin first (if exists), then external as supplement
    // This way, external can supplement builtin even when builtin is selected
    bool builtinLoaded = false;
    
    // First, try to load from embedded language pack (always available if embedded)
    if (isLanguagePackEmbedded(languageCode)) {
        std::string embeddedContent = getEmbeddedLanguagePack(languageCode);
        if (!embeddedContent.empty()) {
            loadTranslationsFromString(embeddedContent, translations, true);  // overwrite mode
            if (!translations.empty()) {
                builtinLoaded = true;
            }
        }
    }
    
    // Always try to load from file system (for development/testing, can override/supplement embedded)
    // This allows file system version to add new keys or override existing ones
    std::vector<std::string> builtinPaths = {
        "sys/language_packs/" + builtinFileName,
        "language_packs/" + builtinFileName,
        "../sys/language_packs/" + builtinFileName,
        "../../sys/language_packs/" + builtinFileName
    };
    
    for (const auto& path : builtinPaths) {
        // Load file system version in overwrite mode to allow new keys to be added
        loadTranslationsFromFile(path, translations, true);  // overwrite mode - allows file system to supplement embedded
        if (!builtinLoaded && !translations.empty()) {
            builtinLoaded = true;
            break;
        }
        // If embedded was loaded, file system version supplements it (same overwrite mode)
        if (!translations.empty()) {
            break;  // Found and loaded file system version
        }
    }
    
    // Final fallback: minimal embedded English (only if nothing else worked)
    if (!builtinLoaded && languageCode == "en-US") {
        // Minimal embedded English translations (critical items only)
        static const char* MINIMAL_EN_US = R"({
  "menu.praat": "Praat",
  "menu.new": "New",
  "menu.open": "Open",
  "menu.save": "Save",
  "menu.help": "Help",
  "menu.i18n": "i18n",
  "menu.language": "Language",
  "menu.about": "About",
  "menu.quit": "Quit",
  "menu.preferences": "Preferences"
})";
        loadTranslationsFromString(std::string(MINIMAL_EN_US), translations, true);
        builtinLoaded = !translations.empty();
    }
    
    // Then, try to load external language pack as supplement (non-overwrite mode)
    // This allows external to supplement builtin
    // Use appropriate path separator
    std::string separator = "/";
    #if defined (_WIN32)
        separator = "\\";
    #endif
    std::string externalPath = externalLanguagePackPath + separator + externalFileName;
    loadTranslationsFromFile(externalPath, translations, false);  // supplement mode (non-overwrite)
    
    // If external was requested but no builtin was loaded, try external-only mode
    if (!isBuiltin && !builtinLoaded && translations.empty()) {
        // Try external-only (overwrite mode since translations is empty)
        loadTranslationsFromFile(externalPath, translations, true);
    }
}

void SimpleI18nManager::setLanguage(const std::string& languageCode, bool isBuiltin) {
    currentLanguage = languageCode;
    currentLanguageIsBuiltin = isBuiltin;
    
    // Load the language pack (builtin first, then external as supplement)
    loadLanguagePack(languageCode, isBuiltin);
    
    // Save the language preference
    saveLanguagePreference(languageCode, isBuiltin);
    
    // Update all menus and UI elements
    updateAllMenus();
    
    // Update language menu checkmarks
    updateLanguageMenuCheckmarks();
}

std::string SimpleI18nManager::getCurrentLanguage() const {
    return currentLanguage;
}

bool SimpleI18nManager::getCurrentLanguageIsBuiltin() const {
    return currentLanguageIsBuiltin;
}

std::string SimpleI18nManager::getExternalLanguagePackPath() const {
    return externalLanguagePackPath;
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

void SimpleI18nManager::registerLanguage(const std::string& code, const std::string& name, const std::string& file, bool isBuiltin) {
    availableLanguages.emplace_back(code, name, file, isBuiltin);
}

// Embedded builtin languages.json content (compiled into exe)
static const char* EMBEDDED_LANGUAGES_JSON = R"({
  "languages": [
    {
      "code": "en-US",
      "name": "English",
      "file": "en-US.json"
    },
    {
      "code": "zh-CN",
      "name": "简体中文",
      "file": "zh-CN.json"
    },
    {
      "code": "zh-TW",
      "name": "繁體中文",
      "file": "zh-TW.json"
    },
    {
      "code": "ja-JP",
      "name": "日本語",
      "file": "ja-JP.json"
    },
    {
      "code": "ko-KR",
      "name": "한국어",
      "file": "ko-KR.json"
    },
    {
      "code": "fr-FR",
      "name": "Français",
      "file": "fr-FR.json"
    },
    {
      "code": "de-DE",
      "name": "Deutsch",
      "file": "de-DE.json"
    },
    {
      "code": "es-ES",
      "name": "Español",
      "file": "es-ES.json"
    },
    {
      "code": "ru-RU",
      "name": "Русский",
      "file": "ru-RU.json"
    },
    {
      "code": "pt-PT",
      "name": "Português",
      "file": "pt-PT.json"
    },
    {
      "code": "lk-CN",
      "name": "Qǔei",
      "file": "lk-CN.json"
    },
    {
      "code": "it-IT",
      "name": "Italiano",
      "file": "it-IT.json"
    }
  ],
  "default": "en-US"
})";

// Helper function to parse languages.json from string content
static void parseLanguagesJsonFromString(const std::string& content, std::vector<SimpleLanguage>& languages, bool isBuiltin) {
    if (content.empty()) {
        return;
    }
    
    // Parse JSON file - look for language objects
    std::string currentCode, currentName, currentFile;
    size_t pos = 0;
    
    while ((pos = content.find("\"code\"", pos)) != std::string::npos) {
            // Find the colon after "code"
        size_t colonPos = content.find(':', pos);
            if (colonPos != std::string::npos) {
                // Find the first quote after the colon
            size_t start = content.find('"', colonPos);
                if (start != std::string::npos) {
                    // Find the closing quote
                size_t end = content.find('"', start + 1);
                    if (end != std::string::npos) {
                    currentCode = content.substr(start + 1, end - start - 1);
                }
            }
        }
        
        // Find "name" field (should be after "code" in same object)
        size_t namePos = content.find("\"name\"", pos);
        if (namePos != std::string::npos && namePos < pos + 200) {  // within reasonable distance
            size_t nameColon = content.find(':', namePos);
            if (nameColon != std::string::npos) {
                size_t nameStart = content.find('"', nameColon);
                if (nameStart != std::string::npos) {
                    size_t nameEnd = content.find('"', nameStart + 1);
                    if (nameEnd != std::string::npos) {
                        currentName = content.substr(nameStart + 1, nameEnd - nameStart - 1);
                    }
                }
            }
        }
        
        // Find "file" field (should be after "name" in same object)
        size_t filePos = content.find("\"file\"", pos);
        if (filePos != std::string::npos && filePos < pos + 300) {  // within reasonable distance
            size_t fileColon = content.find(':', filePos);
            if (fileColon != std::string::npos) {
                size_t fileStart = content.find('"', fileColon);
                if (fileStart != std::string::npos) {
                    size_t fileEnd = content.find('"', fileStart + 1);
                    if (fileEnd != std::string::npos) {
                        currentFile = content.substr(fileStart + 1, fileEnd - fileStart - 1);
                    }
                }
            }
        }
        
        // If we have all three fields, add the language
        if (!currentCode.empty() && !currentName.empty() && !currentFile.empty()) {
            languages.emplace_back(currentCode, currentName, currentFile, isBuiltin);
            currentCode.clear();
            currentName.clear();
            currentFile.clear();
        }
        
        // Move to next potential language entry
        pos += 7;  // length of "code"
    }
}

// Helper function to parse languages.json file
static void parseLanguagesJson(const std::string& filePath, std::vector<SimpleLanguage>& languages, bool isBuiltin) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return;
    }
    
    // Read entire file content
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Parse from string
    parseLanguagesJsonFromString(content, languages, isBuiltin);
}

void SimpleI18nManager::loadBuiltinLanguageIndex() {
    // First try to load from embedded content (always available)
    int beforeSize = availableLanguages.size();
    parseLanguagesJsonFromString(std::string(EMBEDDED_LANGUAGES_JSON), availableLanguages, true);
    int afterSize = availableLanguages.size();
    int languagesLoaded = afterSize - beforeSize;
    
    // If embedded content didn't work, try to load from file system (for development)
    if (languagesLoaded == 0) {
        std::vector<std::string> paths = {
            "sys/language_packs/languages.json",
            "language_packs/languages.json",
            "../sys/language_packs/languages.json",
            "../../sys/language_packs/languages.json"
        };
        
        for (const auto& path : paths) {
            beforeSize = availableLanguages.size();
            parseLanguagesJson(path, availableLanguages, true);  // isBuiltin = true
            afterSize = availableLanguages.size();
            languagesLoaded = afterSize - beforeSize;
            if (languagesLoaded > 0) {
                break;
            }
        }
    }
    
    // Final fallback: hardcoded languages
    if (languagesLoaded == 0) {
        // Add fallback languages (builtin) - but use proper native names from languages.json
        registerLanguage("en-US", "English", "en-US.json", true);
        registerLanguage("zh-CN", "简体中文", "zh-CN.json", true);
        registerLanguage("zh-TW", "繁體中文", "zh-TW.json", true);
        registerLanguage("ja-JP", "日本語", "ja-JP.json", true);
        registerLanguage("ko-KR", "한국어", "ko-KR.json", true);
        registerLanguage("fr-FR", "Français", "fr-FR.json", true);
        registerLanguage("de-DE", "Deutsch", "de-DE.json", true);
        registerLanguage("es-ES", "Español", "es-ES.json", true);
        registerLanguage("ru-RU", "Русский", "ru-RU.json", true);
        registerLanguage("pt-PT", "Português", "pt-PT.json", true);
        registerLanguage("lk-CN", "Qǔei", "lk-CN.json", true);
        registerLanguage("it-IT", "Italiano", "it-IT.json", true);
    }
}

void SimpleI18nManager::loadExternalLanguageIndex() {
    // Load external languages.json from praat_i18n directory
    // Use appropriate path separator
    std::string separator = "/";
    #if defined (_WIN32)
        separator = "\\";
    #endif
    std::string externalPath = externalLanguagePackPath + separator + "languages.json";
    parseLanguagesJson(externalPath, availableLanguages, false);  // isBuiltin = false
}

void SimpleI18nManager::loadLanguageIndex() {
    // Load builtin languages first
    loadBuiltinLanguageIndex();
    
    // Then load external languages (they can have same codes)
    loadExternalLanguageIndex();
}

std::vector<SimpleLanguage> SimpleI18nManager::getAvailableLanguages() const {
    return availableLanguages;
}

bool SimpleI18nManager::isLanguagePackAvailable(const std::string& languageCode, bool isBuiltin) const {
    // Check if embedded version exists (always available)
    if (isBuiltin && isLanguagePackEmbedded(languageCode)) {
        return true;
    }
    
    // English is always available (has minimal embedded fallback)
    if (languageCode == "en-US" && isBuiltin) {
        return true;
    }
    
    std::string fileName;
    // Find file name from available languages
    for (const auto& lang : availableLanguages) {
        if (lang.code == languageCode && lang.isBuiltin == isBuiltin) {
            fileName = lang.file;
            break;
        }
    }
    
    if (fileName.empty()) {
        fileName = languageCode + ".json";
    }
    
    if (isBuiltin) {
        // Check if builtin language pack file exists
        std::vector<std::string> builtinPaths = {
            "sys/language_packs/" + fileName,
            "language_packs/" + fileName,
            "../sys/language_packs/" + fileName,
            "../../sys/language_packs/" + fileName
        };
        
        for (const auto& path : builtinPaths) {
            std::ifstream file(path);
            if (file.is_open()) {
                file.close();
                return true;
            }
        }
    } else {
        // Check if external language pack file exists
        std::string separator = "/";
        #if defined (_WIN32)
            separator = "\\";
        #endif
        std::string externalPath = externalLanguagePackPath + separator + fileName;
        std::ifstream file(externalPath);
        if (file.is_open()) {
            file.close();
            return true;
        }
    }
    
    return false;
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

static void switchLanguageCallback(Thing closure, GuiMenuItemEvent event) {
    LanguageSwitchClosure* data = reinterpret_cast<LanguageSwitchClosure*>(closure);
    if (g_i18nManager && data) {
        g_i18nManager->setLanguage(data->languageCode, data->isBuiltin);
    }
}

void SimpleI18nManager::createMenu(GuiWindow window) {
    
    // Create the i18n menu
    GuiMenu i18nMenuRef = GuiMenu_createInWindow(window, U"i18n", 0);
    this->i18nMenu = i18nMenuRef;
    this->mainWindow = window;
    
    // Create builtin language submenu (use translation key via I18n_translate for consistency)
    const char32* builtinMenuName32 = I18n_translate("menu.i18n.builtin");
    GuiMenu builtinSubmenu = GuiMenu_createInMenu(i18nMenuRef, builtinMenuName32, 0);
    
    // Create external language submenu (use translation key via I18n_translate for consistency)
    const char32* externalMenuName32 = I18n_translate("menu.i18n.external");
    GuiMenu externalSubmenu = GuiMenu_createInMenu(i18nMenuRef, externalMenuName32, 0);
    
    // Add separator
    GuiMenu_addSeparator(i18nMenuRef);
    
    // Add current language display (third menu item) - use translation key via I18n_translate
    // Find current language name from availableLanguages
    std::string currentLangDisplay = currentLanguage;
    for (const auto& lang : availableLanguages) {
        if (lang.code == currentLanguage && lang.isBuiltin == currentLanguageIsBuiltin) {
            currentLangDisplay = lang.name;
            break;
        }
    }
    // Add suffix based on builtin/external status
    currentLangDisplay += " ";
    // Convert I18n_translate result (char32*) to UTF-8 string for concatenation
    autostring8 suffix8 = Melder_32to8(currentLanguageIsBuiltin ? 
                                        I18n_translate("menu.i18n.current.builtin") : 
                                        I18n_translate("menu.i18n.current.external"));
    currentLangDisplay += std::string(suffix8.get());
    autostring32 currentLangDisplay32 = Melder_8to32(currentLangDisplay.c_str());
    GuiMenu_addItem(i18nMenuRef, currentLangDisplay32.get(), GuiMenu_INSENSITIVE, nullptr, nullptr);
    
    // Add language options dynamically from availableLanguages
    // Only show languages that have available language packs
    for (const auto& lang : availableLanguages) {
        // Check if language pack is available before adding to menu
        if (!isLanguagePackAvailable(lang.code, lang.isBuiltin)) {
            continue;  // Skip languages without available packs
        }
        
        // Convert std::string to char32_t*
        autostring32 langName = Melder_8to32(lang.name.c_str());
        
        // Select the appropriate submenu
        GuiMenu targetSubmenu = lang.isBuiltin ? builtinSubmenu : externalSubmenu;
        
        // Create closure for callback
        LanguageSwitchClosure* closure = new LanguageSwitchClosure();
        closure->languageCode = lang.code;
        closure->isBuiltin = lang.isBuiltin;
        g_languageClosures.push_back(closure);
        
        // Check if this is the current language
        bool isCurrentLanguage = (lang.code == currentLanguage && lang.isBuiltin == currentLanguageIsBuiltin);
        
        // Use original language name directly (already converted to UTF-32)
        // Note: langName is autostring32, use .get() directly, don't copy
        
        // Add menu item with callback and CHECKBUTTON flag
        uint32 flags = GuiMenu_CHECKBUTTON;
        GuiMenuItem menuItem = GuiMenu_addItem(targetSubmenu, langName.get(), flags,
                                               switchLanguageCallback, reinterpret_cast<Thing>(closure));
            
            // Store menu item reference for later updates
        // Format: "builtin:langCode" or "external:langCode"
        std::string langKey = (lang.isBuiltin ? "builtin:" : "external:") + lang.code;
        languageMenuItems[langKey] = menuItem;
        
        // Set initial checkmark state
        GuiMenuItem_check(menuItem, isCurrentLanguage);
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

void SimpleI18nManager::saveLanguagePreference(const std::string& languageCode, bool isBuiltin) {
    // Save to a simple text file
    // Format: "builtin:en-US" or "external:en-US"
    FILE* prefFile = fopen("praat_language_preference.txt", "w");
    if (prefFile) {
        std::string pref = (isBuiltin ? "builtin:" : "external:") + languageCode;
        fprintf(prefFile, "%s", pref.c_str());
        fclose(prefFile);
    }
}

std::pair<std::string, bool> SimpleI18nManager::loadLanguagePreference() {
    // Load from the preference file using binary mode to handle encoding
    // Returns (languageCode, isBuiltin)
    FILE* prefFile = fopen("praat_language_preference.txt", "rb");
    if (prefFile) {
        // Read the file content
        fseek(prefFile, 0, SEEK_END);
        long fileSize = ftell(prefFile);
        fseek(prefFile, 0, SEEK_SET);
        
        if (fileSize > 0 && fileSize < 200) {
            char* buffer = new char[fileSize + 1];
            fread(buffer, 1, fileSize, prefFile);
            buffer[fileSize] = '\0';
            
            std::string pref;
            
            // Check for UTF-16 BOM
            if (fileSize >= 2 && buffer[0] == (char)0xFF && buffer[1] == (char)0xFE) {
                // UTF-16 LE encoding - extract ASCII characters
                for (int i = 2; i < fileSize; i += 2) {
                    if (buffer[i] != '\0' && buffer[i] != '\r' && buffer[i] != '\n') {
                        pref += buffer[i];
                    }
                }
            } else {
                // Regular ASCII/UTF-8 encoding
                pref = buffer;
                // Remove newline if present
                size_t len = pref.length();
                if (len > 0 && pref[len-1] == '\n') {
                    pref.erase(len-1);
                }
                if (len > 1 && pref[len-2] == '\r') {
                    pref.erase(len-2);
                }
            }
            
            delete[] buffer;
            fclose(prefFile);
            
            // Parse format: "builtin:en-US" or "external:en-US"
            size_t colonPos = pref.find(':');
            if (colonPos != std::string::npos) {
                std::string type = pref.substr(0, colonPos);
                std::string langCode = pref.substr(colonPos + 1);
                bool isBuiltin = (type == "builtin");
                return std::make_pair(langCode, isBuiltin);
            } else {
                // Old format (no prefix) - assume builtin
                return std::make_pair(pref, true);
            }
        }
        fclose(prefFile);
    }
    
    return std::make_pair("en-US", true); // Default language (builtin)
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
            // Default to builtin for backward compatibility
            g_i18nManager->loadLanguagePack(std::string(languageCode), true);
        }
    }
    
    void I18n_setLanguage(const char* languageCode) {
        if (g_i18nManager) {
            // Default to builtin for backward compatibility
            g_i18nManager->setLanguage(std::string(languageCode), true);
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
            // Default to builtin for backward compatibility
            g_i18nManager->registerLanguage(std::string(code), std::string(name), std::string(file), true);
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
