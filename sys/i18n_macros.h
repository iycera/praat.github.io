// i18n_macros.h - Macros for internationalization

#ifndef _i18n_macros_h_
#define _i18n_macros_h_

// Macro to translate text with fallback
#define I18N_TEXT(key, fallback) I18n_translateWithFallback(U##key, U##fallback)

// Macro for menu creation with translation
#define I18N_MENU(name, fallback) I18N_TEXT("menu." name, fallback)

// Macro for button creation with translation
#define I18N_BUTTON(name, fallback) I18N_TEXT("button." name, fallback)

// Macro for dialog creation with translation
#define I18N_DIALOG(name, fallback) I18N_TEXT("dialog." name, fallback)

// Macro for status messages with translation
#define I18N_STATUS(name, fallback) I18N_TEXT("status." name, fallback)

// Macro for error messages with translation
#define I18N_ERROR(name, fallback) I18N_TEXT("error." name, fallback)

// Macro for general messages with translation
#define I18N_MESSAGE(name, fallback) I18N_TEXT("message." name, fallback)

// Common menu translations
#define I18N_MENU_PRAAT I18N_MENU("praat", "Praat")
#define I18N_MENU_NEW I18N_MENU("new", "New")
#define I18N_MENU_OPEN I18N_MENU("open", "Open")
#define I18N_MENU_SAVE I18N_MENU("save", "Save")
#define I18N_MENU_HELP I18N_MENU("help", "Help")
#define I18N_MENU_EDIT I18N_MENU("edit", "Edit")
#define I18N_MENU_VIEW I18N_MENU("view", "View")
#define I18N_MENU_WINDOW I18N_MENU("window", "Window")
#define I18N_MENU_FILE I18N_MENU("file", "File")
#define I18N_MENU_TOOLS I18N_MENU("tools", "Tools")
#define I18N_MENU_GOODIES I18N_MENU("goodies", "Goodies")
#define I18N_MENU_TECHNICAL I18N_MENU("technical", "Technical")
#define I18N_MENU_ABOUT I18N_MENU("about", "About")
#define I18N_MENU_QUIT I18N_MENU("quit", "Quit")
#define I18N_MENU_PREFERENCES I18N_MENU("preferences", "Preferences")
#define I18N_MENU_SETTINGS I18N_MENU("settings", "Settings")
#define I18N_MENU_CUT I18N_MENU("cut", "Cut")
#define I18N_MENU_COPY I18N_MENU("copy", "Copy")
#define I18N_MENU_PASTE I18N_MENU("paste", "Paste")
#define I18N_MENU_MINIMIZE I18N_MENU("minimize", "Minimize")
#define I18N_MENU_ZOOM I18N_MENU("zoom", "Zoom")
#define I18N_MENU_CLOSE I18N_MENU("close", "Close")
#define I18N_MENU_NEW_SCRIPT I18N_MENU("new_script", "New Praat script")
#define I18N_MENU_NEW_NOTEBOOK I18N_MENU("new_notebook", "New Praat notebook")
#define I18N_MENU_OPEN_SCRIPT I18N_MENU("open_script", "Open Praat script...")
#define I18N_MENU_OPEN_NOTEBOOK I18N_MENU("open_notebook", "Open Praat notebook...")
#define I18N_MENU_ADD_MENU_COMMAND I18N_MENU("add_menu_command", "Add menu command...")
#define I18N_MENU_HIDE_MENU_COMMAND I18N_MENU("hide_menu_command", "Hide menu command...")
#define I18N_MENU_SHOW_MENU_COMMAND I18N_MENU("show_menu_command", "Show menu command...")
#define I18N_MENU_ADD_ACTION_COMMAND I18N_MENU("add_action_command", "Add action command...")
#define I18N_MENU_HIDE_ACTION_COMMAND I18N_MENU("hide_action_command", "Hide action command...")
#define I18N_MENU_SHOW_ACTION_COMMAND I18N_MENU("show_action_command", "Show action command...")
#define I18N_MENU_CALCULATOR I18N_MENU("calculator", "Calculator...")
#define I18N_MENU_REPORT_DIFFERENCE I18N_MENU("report_difference", "Report difference of two proportions...")
#define I18N_MENU_SAVE_DEMO_PDF I18N_MENU("save_demo_pdf", "Save Demo window as PDF file...")
#define I18N_MENU_BUTTONS I18N_MENU("buttons", "Buttons...")

// Common button translations
#define I18N_BUTTON_OK I18N_BUTTON("ok", "OK")
#define I18N_BUTTON_CANCEL I18N_BUTTON("cancel", "Cancel")
#define I18N_BUTTON_APPLY I18N_BUTTON("apply", "Apply")
#define I18N_BUTTON_CLOSE I18N_BUTTON("close", "Close")
#define I18N_BUTTON_SAVE I18N_BUTTON("save", "Save")
#define I18N_BUTTON_OPEN I18N_BUTTON("open", "Open")
#define I18N_BUTTON_NEW I18N_BUTTON("new", "New")
#define I18N_BUTTON_DELETE I18N_BUTTON("delete", "Delete")
#define I18N_BUTTON_COPY I18N_BUTTON("copy", "Copy")
#define I18N_BUTTON_PASTE I18N_BUTTON("paste", "Paste")
#define I18N_BUTTON_CUT I18N_BUTTON("cut", "Cut")
#define I18N_BUTTON_UNDO I18N_BUTTON("undo", "Undo")
#define I18N_BUTTON_REDO I18N_BUTTON("redo", "Redo")
#define I18N_BUTTON_FIND I18N_BUTTON("find", "Find")
#define I18N_BUTTON_REPLACE I18N_BUTTON("replace", "Replace")
#define I18N_BUTTON_SELECT_ALL I18N_BUTTON("select_all", "Select All")

// Common dialog translations
#define I18N_DIALOG_FILE_OPEN I18N_DIALOG("file_open", "Open File")
#define I18N_DIALOG_FILE_SAVE I18N_DIALOG("file_save", "Save File")
#define I18N_DIALOG_FILE_SAVE_AS I18N_DIALOG("file_save_as", "Save As")
#define I18N_DIALOG_PREFERENCES I18N_DIALOG("preferences", "Preferences")
#define I18N_DIALOG_SETTINGS I18N_DIALOG("settings", "Settings")
#define I18N_DIALOG_ABOUT I18N_DIALOG("about", "About Praat")

// Common status translations
#define I18N_STATUS_READY I18N_STATUS("ready", "Ready")
#define I18N_STATUS_LOADING I18N_STATUS("loading", "Loading...")
#define I18N_STATUS_SAVING I18N_STATUS("saving", "Saving...")
#define I18N_STATUS_PROCESSING I18N_STATUS("processing", "Processing...")

// Common error translations
#define I18N_ERROR_FILE_NOT_FOUND I18N_ERROR("file_not_found", "File not found")
#define I18N_ERROR_PERMISSION_DENIED I18N_ERROR("permission_denied", "Permission denied")
#define I18N_ERROR_INVALID_FORMAT I18N_ERROR("invalid_format", "Invalid format")
#define I18N_ERROR_OUT_OF_MEMORY I18N_ERROR("out_of_memory", "Out of memory")
#define I18N_ERROR_UNKNOWN I18N_ERROR("unknown", "Unknown error")

// Common message translations
#define I18N_MESSAGE_CONFIRM_QUIT I18N_MESSAGE("confirm_quit", "Are you sure you want to quit?")
#define I18N_MESSAGE_CONFIRM_DELETE I18N_MESSAGE("confirm_delete", "Are you sure you want to delete this item?")
#define I18N_MESSAGE_SAVE_CHANGES I18N_MESSAGE("save_changes", "Do you want to save changes?")
#define I18N_MESSAGE_LANGUAGE_CHANGED I18N_MESSAGE("language_changed", "Language changed. Please restart Praat to apply changes.")
#define I18N_MESSAGE_LANGUAGE_PACK_LOADED I18N_MESSAGE("language_pack_loaded", "Language pack loaded successfully")
#define I18N_MESSAGE_LANGUAGE_PACK_ERROR I18N_MESSAGE("language_pack_error", "Error loading language pack")

#endif // _i18n_macros_h_
