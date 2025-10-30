// 自动生成的文件 - 请勿手动编辑
// 此文件由 generate_i18n_callbacks.py 根据 languages.json 生成

#ifndef I18N_CALLBACKS_GENERATED_H
#define I18N_CALLBACKS_GENERATED_H

// 静态回调函数声明
void switchToEnglish(Thing, GuiMenuItemEvent event);
void switchToChineseSimplified(Thing, GuiMenuItemEvent event);
void switchToChineseTraditional(Thing, GuiMenuItemEvent event);
void switchToJapanese(Thing, GuiMenuItemEvent event);
void switchToKorean(Thing, GuiMenuItemEvent event);
void switchToFrench(Thing, GuiMenuItemEvent event);
void switchToGerman(Thing, GuiMenuItemEvent event);
void switchToSpanish(Thing, GuiMenuItemEvent event);
void switchToRussian(Thing, GuiMenuItemEvent event);
void switchToPortuguese(Thing, GuiMenuItemEvent event);
void switchToArabic(Thing, GuiMenuItemEvent event);
void switchToitIT(Thing, GuiMenuItemEvent event);

// 回调映射宏
#define GENERATE_CALLBACK_MAPPING(lang) \
    } else if (lang.code == "en-US") {
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
            callback = GuiMenuItemCallback(switchToArabic);
    } else if (lang.code == "it-IT") {
            callback = GuiMenuItemCallback(switchToitIT);

#endif // I18N_CALLBACKS_GENERATED_H
