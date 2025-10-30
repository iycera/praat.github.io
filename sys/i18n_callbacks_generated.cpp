// 自动生成的文件 - 请勿手动编辑
// 此文件由 generate_i18n_callbacks.py 根据 languages.json 生成

#include "i18n_callbacks_generated.h"
#include "i18n_simple.h"

// 静态回调函数实现
void switchToEnglish(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToEnglish: Switching to English\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("en-US");
    }
}

void switchToChineseSimplified(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToChineseSimplified: Switching to 简体中文\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("zh-CN");
    }
}

void switchToChineseTraditional(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToChineseTraditional: Switching to 繁體中文\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("zh-TW");
    }
}

void switchToJapanese(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToJapanese: Switching to 日本語\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("ja-JP");
    }
}

void switchToKorean(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToKorean: Switching to 한국어\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("ko-KR");
    }
}

void switchToFrench(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToFrench: Switching to Français\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("fr-FR");
    }
}

void switchToGerman(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToGerman: Switching to Deutsch\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("de-DE");
    }
}

void switchToSpanish(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToSpanish: Switching to Español\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("es-ES");
    }
}

void switchToRussian(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToRussian: Switching to Русский\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("ru-RU");
    }
}

void switchToPortuguese(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToPortuguese: Switching to Português\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("pt-PT");
    }
}

void switchToArabic(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToArabic: Switching to العربية\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("lk-CN");
    }
}

void switchToitIT(Thing, GuiMenuItemEvent event) {
    if (g_i18nManager) {
        FILE* debugFile = fopen("debug_i18n_simple.txt", "a");
        fprintf(debugFile, "switchToitIT: Switching to Italiano\n");
        fclose(debugFile);
        g_i18nManager->setLanguage("it-IT");
    }
}
