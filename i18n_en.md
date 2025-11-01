# Praat Internationalization (i18n) Guide

Praat supports multilingual UI, dynamic language switching and easy extension. This guide covers directory structure, adding new languages/translation, menu switching, python helper scripts, and sample configs.

## System Features

Praat's i18n system uses a **built-in and external hybrid approach**:
- **Built-in Language Packs**: Embedded into the executable at compile time, allowing the program to run without external files
- **External Language Packs**: Placed in `exe_directory/praat_i18n` folder, can be added or updated at any time
- **Priority**: Built-in packs take precedence, external packs serve as supplements
- **Flexibility**: Built-in and external packs can share the same language codes, allowing users to choose which version to use

## Multilingual UI Screenshots

Main window language menu example:

![Main window language menu](./docs/pictures/mainwindow.png)

Editor window in Simplified Chinese:

![Editor window in Simplified Chinese](./docs/pictures/view&editor.png)

---

## 1. Directory Structure

### Built-in Language Packs (Embedded at Compile Time)

```
sys/language_packs/
├─ en-US.json          # English (built-in)
├─ zh-CN.json          # Simplified Chinese (built-in)
├─ ko-KR.json          # Korean (built-in)
├─ ja-JP.json          # Japanese (built-in)
├─ ...                 # More built-in languages
├─ languages.json      # Built-in language registry/mapping
├─ batch_translate_de.py, comprehensive_translate_de.py
├─ compare_translations.py
```

Built-in language packs are compiled into the executable, ensuring the program can run without external files.

### External Language Packs (Loaded at Runtime)

```
exe_directory/praat_i18n/
├─ languages.json      # External language registry/mapping (optional)
├─ en-US.json          # External English pack (can override or supplement built-in)
├─ zh-CN.json          # External Chinese pack
├─ ...                 # More external languages
```

External language pack path:
- **Windows**: `executable_directory\praat_i18n`
- **Linux/Unix**: `executable_directory/praat_i18n`
- **macOS**: `executable_directory/praat_i18n` (or current directory)

### Example languages.json
```json
{
  "languages": [
    { "code": "en-US", "name": "English", "file": "en-US.json" },
    { "code": "zh-CN", "name": "简体中文", "file": "zh-CN.json" },
    ...
  ],
  "default": "en-US"
}
```

### Example zh-CN.json
```json
{
  "menu.praat": "Praat",
  "menu.new": "新建",
  "menu.save": "保存",
  ...
}
```

---

## 2. How to Add a New Language

### Adding Built-in Language Packs

1. **Duplicate** an existing JSON: Copy `sys/language_packs/en-US.json` as `sys/language_packs/fr-FR.json` and translate values
2. **Register** in `sys/language_packs/languages.json`:
```json
{ "code": "fr-FR", "name": "Français", "file": "fr-FR.json" }
```
3. **Ensure all keys are present** using `compare_translations.py` to check completeness
4. **Rebuild** Run the build script, which will automatically generate embedded language packs and compile them into the executable
   - No need to manually run `generate_embedded_packs.py` (build scripts execute it automatically)

### Adding External Language Packs

1. **Create directory** Create a `praat_i18n` folder in the same directory as the executable (if it doesn't exist)
2. **Copy JSON file** Place the language pack JSON file (e.g., `fr-FR.json`) into the `praat_i18n` directory
3. **Optional: Register language** Add language registration in `praat_i18n/languages.json` (if the file doesn't exist, the system will automatically scan the directory)
4. **Immediate availability** External language packs don't require recompilation, just restart the program to see them in the menu

### Language Pack Format

Built-in and external language packs use the same JSON format and are compatible with each other.

---

## 3. Language Switching

### Menu Structure

The i18n menu has the following structure (menu item names are internationalized and will display according to the current language):
```
i18n
├─ Built-in          # Built-in languages submenu (internationalized name, e.g., "Built-in", "内置")
│  ├─ English         # Built-in language options (with checkmark)
│  ├─ 简体中文
│  └─ ...
├─ External          # External languages submenu (internationalized name, e.g., "External", "外挂")
│  ├─ English         # External language options (if available)
│  ├─ 简体中文
│  └─ ...
└─ Simplified Chinese (Built-in)       # Currently used language (shows name and source tag, internationalized)
```

**Note**: Submenu names ("Built-in", "External") and current language tags ("(Built-in)", "(External)") will automatically display in the corresponding translation based on the currently selected interface language. For example:
- English interface: Shows "Built-in", "External", "(Built-in)"
- Chinese interface: Shows "内置", "外挂", "(内置)"
- Other languages: Shows corresponding translations

### Switching Steps

1. **Select language**: Choose a language from the "Built-in" or "External" submenu under the "i18n" menu (submenu names are internationalized)
2. **Auto-save**: Selection is saved to `praat_language_preference.txt`, recording both language code and source (built-in/external)
3. **Restart required**: After selecting a language, you need to restart the program for changes to take effect
4. **Current language display**: The bottom of the menu shows the currently used language name and source tag (internationalized, e.g., "Simplified Chinese (Built-in)" or "简体中文 (内置)")

### Language Selection Priority

- System prioritizes built-in language packs
- If built-in pack is unavailable, attempts to load external pack
- If both unavailable, falls back to English (minimal version)

---

## 4. Compile & Helper Scripts

### Build Scripts

- **`compile_praat.bat`**: Full build script
  - Automatically detects MinGW environment
  - **Automatically generates embedded language packs** (runs `generate_embedded_packs.py` before each build)
  - Uses 32-bit build by default (to avoid UTF-8 encoding issues)
  - Multi-threaded compilation (4 threads)
  - Lists available language packs and test tips after build
  - To use 64-bit build, change `msys-mingw32` to `msys-mingw64` in the script

- **`compile_quick.bat`**: Quick build script
  - Kills Praat process and performs quick compilation
  - Also automatically generates embedded language packs
  - Uses 32-bit build by default

- **`clean_praat.bat`**: Cleanup script
  - Cleans build artifacts (.o/.a files)
  - Cleans debug files and language preference files

### Embedded Language Pack Generation

**`sys/generate_embedded_packs.py`**: Script to generate embedded language packs
- **Purpose**: Compiles all language packs from `sys/language_packs/*.json` into `sys/i18n_embedded_packs.cpp`
- **Automatic execution**: Runs automatically when build scripts are executed, no manual step required
- **Manual execution**: To update embedded packs separately, run:
  ```bash
  python sys/generate_embedded_packs.py
  ```
- **Encoding handling**: Automatically handles UTF-8 BOM to ensure multi-byte characters are correctly embedded

### Build Notes

- **32-bit vs 64-bit**:
  - Default uses 32-bit build (`makefile.defs.msys-mingw32`), which better avoids UTF-8 character display issues
  - To use 64-bit build, modify the configuration in build scripts
- **Embedded language packs**:
  - Automatically generated from JSON files during build
  - Ensures each build includes the latest language pack content
  - Generated `i18n_embedded_packs.cpp` file is large (~1.7MB), which is normal

---

## 5. Python Helpers
- `batch_translate_de.py`: batch fill keys for German, serves as sample, copy for others
- `comprehensive_translate_de.py`: rule/auto fill, extensible
- `compare_translations.py` example usage:
  ```bash
  python compare_translations.py en-US.json zh-CN.json "Chinese"
  ```
  Lists untranslated/missing/short keys.

---

## 6. Typical key mapping
```json
// en-US.json
{
  "menu.praat": "Praat",
  "menu.save": "Save",
  "menu.language": "Language",
  "label.confirm": "Are you sure?",
  ...
}
// zh-CN.json
{
  "menu.praat": "Praat",
  "menu.save": "保存",
  "menu.language": "语言",
  "label.confirm": "您确定吗？",
  ...
}
```

---

## 7. Testing & Contribution

### Language Pack Testing
- Use `compare_translations.py` to check keys before publishing a new pack
- Test all menus/pages in new language
- Test built-in and external language pack loading order and priority

### Self-contained Testing
- Copy executable to a new directory (without `praat_i18n` folder)
- Verify built-in language packs work correctly
- Verify external language pack path is correctly identified

### Debug Information
- Debug log: `debug_i18n_simple.txt`
- Language preference saved in `praat_language_preference.txt` (format: `language_code:builtin/external`)
- Submit as PR with full json and languages.json

More scripting/automation/best practice: welcome to discuss via Issues!

## Translation Progress Table

Major languages automatic key count (using en-US as baseline):

| Language     | Key Count | Coverage | Notes                       |
|--------------|-----------|----------|-----------------------------|
| English      | 1999      | 100%     | Baseline                    |
| Simplified Chinese | 1999      | 99.9%+   | Fully translated UI         |
| Other major  | 1800~1990 | ≥90%     | A few menus/help lines left |
| Qǔei/lk-CN   | 1~1000    | DEMO     | Sample/incomplete           |

> Note: Use compare_translations.py regularly to ensure 99%+ key coverage.
