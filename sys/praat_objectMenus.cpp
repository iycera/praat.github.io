/* praat_objectMenus.cpp
 *
 * Copyright (C) 1992-2024 Paul Boersma
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
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

#include "praatP.h"
#include "praatM.h"
#include "praat_script.h"
#include "ScriptEditor.h"
#include "NotebookEditor.h"
#include "ButtonEditor.h"
#include "DataEditor.h"
#include "site.h"
#include "GraphicsP.h"
#include "DemoEditor.h"
#include "i18n_simple.h"
// #include "i18n_macros.h"  // Removed to avoid macro redefinition

#define EDITOR  theCurrentPraatObjects -> list [IOBJECT]. editors

/********** Callbacks of the fixed buttons. **********/

DIRECT (PRAAT_Remove) {
	WHERE_DOWN (SELECTED)
		praat_removeObject (IOBJECT);
	praat_show ();
	END_NO_NEW_DATA
}

FORM (MODIFY_Rename, I18n_translate("form.rename_object"), I18n_translate("form.rename_dialog")) {
	TEXTFIELD (newName, I18n_translate("form.new_name"), U"", 3)
OK
	WHERE (SELECTED)
		SET_STRING (newName, NAME)
DO
	if (theCurrentPraatObjects -> totalSelection == 0)
		Melder_throw (I18n_translate("error.selection_changed"), U"\n", I18n_translate("error.no_object_selected"), U" ", I18n_translate("error.cannot_rename"));
	if (theCurrentPraatObjects -> totalSelection > 1)
		Melder_throw (I18n_translate("error.selection_changed"), U"\n", I18n_translate("error.cannot_rename_multiple"));
	WHERE (SELECTED)
		break;
	static MelderString string;
	MelderString_copy (& string, newName);
	praat_cleanUpName (string.string);
	static MelderString fullName;
	MelderString_copy (& fullName, Thing_className (OBJECT), U" ", string.string);
	if (! str32equ (fullName.string, FULL_NAME)) {
		theCurrentPraatObjects -> list [IOBJECT]. name = Melder_dup_f (fullName.string);
		autoMelderString listName;
		MelderString_append (& listName, ID, U". ", fullName.string);
		praat_list_renameAndSelect (IOBJECT, listName.string);
		for (int ieditor = 0; ieditor < praat_MAXNUM_EDITORS; ieditor ++)
			if (EDITOR [ieditor])
				Thing_setName (EDITOR [ieditor], listName.string);
		Thing_setName (OBJECT, string.string);
	}
	END_NO_NEW_DATA
}

FORM (NEW1_Copy, I18n_translate("form.copy_object"), I18n_translate("form.copy_dialog")) {
	TEXTFIELD (newName, I18n_translate("form.name_of_new_object"), U"", 3)
OK
	WHERE (SELECTED)
		SET_STRING (newName, NAME)
DO
	if (theCurrentPraatObjects -> totalSelection == 0)
		Melder_throw (I18n_translate("error.selection_changed"), U"\n", I18n_translate("error.no_object_selected"), U" ", I18n_translate("error.cannot_copy"));
	if (theCurrentPraatObjects -> totalSelection > 1)
		Melder_throw (I18n_translate("error.selection_changed"), U"\n", I18n_translate("error.cannot_copy_multiple"));
	if (interpreter)
		interpreter -> returnType = kInterpreter_ReturnType::OBJECT_;
	WHERE (SELECTED)
		praat_new (Data_copy ((Daata) OBJECT), newName);
	END_WITH_NEW_DATA
}

DIRECT (INFO_Info) {
	INFO_NONE
		if (theCurrentPraatObjects -> totalSelection == 0)
			Melder_throw (I18n_translate("error.selection_changed"), U"\n", I18n_translate("error.no_object_selected"), U" ", I18n_translate("error.cannot_query"));
		if (theCurrentPraatObjects -> totalSelection > 1)
			Melder_throw (I18n_translate("error.selection_changed"), U"\n", I18n_translate("error.cannot_query_multiple"));
		WHERE (SELECTED) Thing_infoWithIdAndFile (OBJECT, ID, & theCurrentPraatObjects -> list [IOBJECT]. file);
	INFO_NONE_END
}

DIRECT (PRAAT__Inspect) {
	PRAAT
		if (theCurrentPraatObjects -> totalSelection == 0)
			Melder_throw (I18n_translate("error.selection_changed"), U"\n", I18n_translate("error.no_object_selected"), U" ", I18n_translate("error.cannot_inspect"));
		if (theCurrentPraatApplication -> batch) {
			Melder_throw (I18n_translate("error.cannot_inspect_batch"));
		} else {
			WHERE (SELECTED) {
				autoDataEditor editor = DataEditor_create (ID_AND_FULL_NAME, OBJECT);
				praat_installEditor (editor.get(), IOBJECT);
				editor.releaseToUser();
			}
		}
	PRAAT_END
}

/********** The fixed menus. **********/

static GuiMenu praatMenu, editMenu, windowMenu, newMenu, readMenu, goodiesMenu, preferencesMenu, technicalMenu, applicationHelpMenu, helpMenu;

GuiMenu praat_objects_resolveMenu (conststring32 menu) {
	return
		str32equ (menu, U"Praat") || str32equ (menu, I18n_translate("menu.control")) ? praatMenu :
		#if cocoa
			str32equ (menu, U"Edit") || str32equ (menu, I18n_translate("menu.edit")) ? editMenu :
			str32equ (menu, U"Window") || str32equ (menu, I18n_translate("menu.window")) ? windowMenu :
		#endif
		str32equ (menu, U"New") || str32equ (menu, I18n_translate("menu.new")) || str32equ (menu, I18n_translate("menu.create")) ? newMenu :
		str32equ (menu, U"Open") || str32equ (menu, I18n_translate("menu.open")) || str32equ (menu, I18n_translate("menu.read")) ? readMenu :
		str32equ (menu, U"Help") || str32equ (menu, I18n_translate("menu.help")) ? helpMenu :
		str32equ (menu, U"Goodies") || str32equ (menu, I18n_translate("menu.goodies")) ? goodiesMenu :
		str32equ (menu, U"Settings") || str32equ (menu, I18n_translate("menu.settings")) || str32equ (menu, I18n_translate("menu.preferences")) ? preferencesMenu :
		str32equ (menu, U"Technical") || str32equ (menu, I18n_translate("menu.technical")) ? technicalMenu :
		#ifdef macintosh
			str32equ (menu, U"ApplicationHelp") || str32equ (menu, I18n_translate("menu.application_help")) ? applicationHelpMenu :
		#else
			str32equ (menu, U"ApplicationHelp") || str32equ (menu, I18n_translate("menu.application_help")) ? helpMenu :
		#endif
		newMenu;   // default
}

/********** Callbacks of the Praat menu. **********/

DIRECT (PRAAT__About) {
	PRAAT
		praat_showLogo ();
	PRAAT_END
}

DIRECT (PRAAT__newScript) {
	PRAAT
		autoScriptEditor scriptEditor = ScriptEditor_createFromText (nullptr, nullptr);
		scriptEditor.releaseToUser();
	PRAAT_END
}

DIRECT (PRAAT__newNotebook) {
	PRAAT
		autoNotebookEditor notebookEditor = NotebookEditor_createFromText (nullptr);
		notebookEditor.releaseToUser();
	PRAAT_END
}

DIRECT (PRAAT__openScript) {
	PRAAT
		autoScriptEditor scriptEditor = ScriptEditor_createFromText (nullptr, nullptr);
		TextEditor_showOpen (scriptEditor.get());
		scriptEditor.releaseToUser();
	PRAAT_END
}

DIRECT (PRAAT__openNotebook) {
	PRAAT
		autoNotebookEditor notebookEditor = NotebookEditor_createFromText (nullptr);
		TextEditor_showOpen (notebookEditor.get());
		notebookEditor.releaseToUser();
	PRAAT_END
}

static ButtonEditor theReferenceToTheOnlyButtonEditor;

static void cb_ButtonEditor_destruction (Editor /* editor */) {
	theReferenceToTheOnlyButtonEditor = nullptr;
}

DIRECT (PRAAT__editButtons) {
	PRAAT
		if (theReferenceToTheOnlyButtonEditor) {
			Editor_raise (theReferenceToTheOnlyButtonEditor);
		} else {
			autoButtonEditor editor = ButtonEditor_create ();
			Editor_setDestructionCallback (editor.get(), cb_ButtonEditor_destruction);
			theReferenceToTheOnlyButtonEditor = editor.get();
			editor.releaseToUser();
		}
	PRAAT_END
}

FORM (PRAAT__addMenuCommand, I18n_translate("form.add_menu_command"), I18n_translate("menu.add_menu_command")) {
	WORD (window, I18n_translate("form.window"), U"Objects")
	WORD (menu, I18n_translate("form.menu"), U"New")
	SENTENCE (command, I18n_translate("form.command"), U"Hallo...")
	SENTENCE (afterCommand, I18n_translate("form.after_command"), U"")
	INTEGER (depth, I18n_translate("form.depth"), U"0")
	INFILE (script, I18n_translate("form.script_file"), U"/u/miep/hallo.praat")
	OK
DO
	PRAAT
		praat_addMenuCommandScript (window, menu, command, afterCommand, depth, script);
	PRAAT_END
}

FORM (PRAAT__hideMenuCommand, I18n_translate("form.hide_menu_command"), I18n_translate("menu.hide_menu_command")) {
	WORD (window, I18n_translate("form.window"), U"Objects")
	WORD (menu, I18n_translate("form.menu"), U"New")
	SENTENCE (command, I18n_translate("form.command"), U"Hallo...")
	OK
DO
	PRAAT
		praat_hideMenuCommand (window, menu, command);
	PRAAT_END
}

FORM (PRAAT__showMenuCommand, I18n_translate("form.show_menu_command"), I18n_translate("menu.show_menu_command")) {
	WORD (window, I18n_translate("form.window"), U"Objects")
	WORD (menu, I18n_translate("form.menu"), U"New")
	SENTENCE (command, I18n_translate("form.command"), U"Hallo...")
	OK
DO
	PRAAT
		praat_showMenuCommand (window, menu, command);
	PRAAT_END
}

FORM (PRAAT__addAction, I18n_translate("form.add_action_command"), I18n_translate("menu.add_action_command")) {
	WORD (class1, I18n_translate("form.class_1"), U"Sound")
	INTEGER (number1, I18n_translate("form.number_1"), U"0")
	WORD (class2, I18n_translate("form.class_2"), U"")
	INTEGER (number2, I18n_translate("form.number_2"), U"0")
	WORD (class3, I18n_translate("form.class_3"), U"")
	INTEGER (number3, I18n_translate("form.number_3"), U"0")
	SENTENCE (command, I18n_translate("form.command"), U"Play reverse")
	SENTENCE (afterCommand, I18n_translate("form.after_command"), U"Play")
	INTEGER (depth, I18n_translate("form.depth"), U"0")
	INFILE (script, I18n_translate("form.script_file"), U"/u/miep/playReverse.praat")
	OK
DO
	PRAAT
		praat_addActionScript (class1, number1, class2, number2, class3, number3, command, afterCommand, depth, script);
	PRAAT_END
}

FORM (PRAAT__hideAction, I18n_translate("form.hide_action_command"), I18n_translate("menu.hide_action_command")) {
	WORD (class1, I18n_translate("form.class_1"), U"Sound")
	WORD (class2, I18n_translate("form.class_2"), U"")
	WORD (class3, I18n_translate("form.class_3"), U"")
	SENTENCE (command, I18n_translate("form.command"), U"Play")
	OK
DO
	PRAAT
		praat_hideAction_classNames (class1, class2, class3, command);
	PRAAT_END
}

FORM (PRAAT__showAction, I18n_translate("form.show_action_command"), I18n_translate("menu.show_action_command")) {
	WORD (class1, I18n_translate("form.class_1"), U"Sound")
	WORD (class2, I18n_translate("form.class_2"), U"")
	WORD (class3, I18n_translate("form.class_3"), U"")
	SENTENCE (command, I18n_translate("form.command"), U"Play")
	OK
DO
	PRAAT
		praat_showAction_classNames (class1, class2, class3, command);
	PRAAT_END
}

/********** Callbacks of the Settings menu. **********/

FORM (SETTINGS__TextReadingSettings, I18n_translate("form.text_reading_settings"), I18n_translate("menu.text_reading_settings")) {
	CHOICE_ENUM (kMelder_textInputEncoding, encodingOf8BitTextFiles,
			I18n_translate("form.encoding_8bit_text_files"), kMelder_textInputEncoding::DEFAULT)
OK
	SET_ENUM (encodingOf8BitTextFiles, kMelder_textInputEncoding, Melder_getInputEncoding ())
DO
	PREFS
		Melder_setInputEncoding (encodingOf8BitTextFiles);
	PREFS_END
}

FORM (SETTINGS__TextWritingSettings, I18n_translate("form.text_writing_settings"), I18n_translate("menu.text_writing_settings")) {
	CHOICE_ENUM (kMelder_textOutputEncoding, outputEncoding,
			I18n_translate("form.output_encoding"), kMelder_textOutputEncoding::DEFAULT)
OK
	SET_ENUM (outputEncoding, kMelder_textOutputEncoding, Melder_getOutputEncoding ())
DO
	PREFS
		Melder_setOutputEncoding (outputEncoding);
	PREFS_END
}

FORM (SETTINGS__CjkFontStyleSettings, I18n_translate("form.cjk_font_style_settings"), nullptr) {
	OPTIONMENU_ENUM (kGraphics_cjkFontStyle, cjkFontStyle,
			I18n_translate("form.cjk_font_style"), kGraphics_cjkFontStyle::DEFAULT)
OK
	SET_ENUM (cjkFontStyle, kGraphics_cjkFontStyle, theGraphicsCjkFontStyle)
DO
	PREFS
		theGraphicsCjkFontStyle = cjkFontStyle;
	PREFS_END
}

/********** Callbacks of the Goodies menu. **********/

FORM (INFO_NONE__praat_calculator, I18n_translate("form.calculator"), I18n_translate("menu.calculator")) {
	TEXTFIELD (expression, I18n_translate("form.type_formula"), U"5*5", 5)
	COMMENT (I18n_translate("form.note_special_functions"))
	COMMENT (I18n_translate("form.including_functions"))
	COMMENT (I18n_translate("form.for_details_help"))
	OK
DO
	INFO_NONE
		Formula_Result result;
		if (! interpreter) {
			autoInterpreter tempInterpreter = Interpreter_create ();
			Interpreter_anyExpression (tempInterpreter.get(), expression, & result);
		} else {
			Interpreter_anyExpression (interpreter, expression, & result);
		}
		switch (result. expressionType) {
			case kFormula_EXPRESSION_TYPE_NUMERIC:
				Melder_information (result. numericResult);
			break; case kFormula_EXPRESSION_TYPE_STRING:
				Melder_information (result. stringResult.get());
			break; case kFormula_EXPRESSION_TYPE_NUMERIC_VECTOR:
				Melder_information (constVECVU (result. numericVectorResult));
			break; case kFormula_EXPRESSION_TYPE_NUMERIC_MATRIX:
				Melder_information (constMATVU (result. numericMatrixResult));
			break; case kFormula_EXPRESSION_TYPE_STRING_ARRAY:
				Melder_information (result. stringArrayResult);
		}
	INFO_NONE_END
}

FORM (INFO_reportDifferenceOfTwoProportions, I18n_translate("form.report_difference"), I18n_translate("menu.report_difference")) {
	INTEGER (a_int, I18n_translate("form.left_row_1"), U"71")
	INTEGER (b_int, I18n_translate("form.right_row_1"), U"39")
	INTEGER (c_int, I18n_translate("form.left_row_2"), U"93")
	INTEGER (d_int, I18n_translate("form.right_row_2"), U"27")
	OK
DO
	INFO_NONE
		double a = a_int, b = b_int, c = c_int, d = d_int;
		double n = a + b + c + d;
		if (a < 0 || b < 0 || c < 0 || d < 0)
			Melder_throw (I18n_translate("error.numbers_not_negative"));
		if (a + b <= 0 || c + d <= 0)
			Melder_throw (I18n_translate("error.row_totals_positive"));
		if (a + c <= 0 || b + d <= 0)
			Melder_throw (I18n_translate("error.column_totals_positive"));
		MelderInfo_open ();
		MelderInfo_writeLine (I18n_translate("info.observed_row_1"), U"    ", Melder_iround (a), U"    ", Melder_iround (b));
		MelderInfo_writeLine (I18n_translate("info.observed_row_2"), U"    ", Melder_iround (c), U"    ", Melder_iround (d));
		double aexp = (a + b) * (a + c) / n;
		double bexp = (a + b) * (b + d) / n;
		double cexp = (a + c) * (c + d) / n;
		double dexp = (b + d) * (c + d) / n;
		MelderInfo_writeLine (U"");
		MelderInfo_writeLine (I18n_translate("info.expected_row_1"), U"    ", aexp, U"    ", bexp);
		MelderInfo_writeLine (I18n_translate("info.expected_row_2"), U"    ", cexp, U"    ", dexp);
		/*
			Continuity correction:
			bring the observed numbers closer to the expected numbers by 0.5 (if possible).
		*/
		Melder_moveCloserToBy (& a, aexp, 0.5);
		Melder_moveCloserToBy (& b, bexp, 0.5);
		Melder_moveCloserToBy (& c, cexp, 0.5);
		Melder_moveCloserToBy (& d, dexp, 0.5);
		MelderInfo_writeLine (U"");
		MelderInfo_writeLine (U"Corrected observed row 1 =    ", a, U"    ", b);
		MelderInfo_writeLine (U"Corrected observed row 2 =    ", c, U"    ", d);

		n = a + b + c + d;
		double crossDifference = a * d - b * c;
		double x2 = n * crossDifference * crossDifference / (a + b) / (c + d) / (a + c) / (b + d);
		MelderInfo_writeLine (U"");
		MelderInfo_writeLine (U"Chi-square =    ", x2);
		MelderInfo_writeLine (U"Two-tailed p =    ", NUMchiSquareQ (x2, 1));
		MelderInfo_close ();
	INFO_NONE_END
}

FORM_SAVE (GRAPHICS_saveDemoWindowAsPdfFile, I18n_translate("form.save_demo_pdf"), nullptr, U"praatDemoWindow.pdf") {
	Demo_saveToPdfFile (file);
	END_NO_NEW_DATA
}

/********** Callbacks of the Technical menu. **********/

FORM (SETTINGS__debug, I18n_translate("form.debug"), nullptr) {
	COMMENT (U"If you switch Tracing on, Praat will write lots of detailed ")
	COMMENT (U"information about what goes on in Praat")
	{// scope
		structMelderFile file { };
		#ifdef UNIX
			MelderFolder_getFile (Melder_preferencesFolder(), U"tracing", & file);
		#else
			MelderFolder_getFile (Melder_preferencesFolder(), U"Tracing.txt", & file);
		#endif
		COMMENT (Melder_cat (U"to ", MelderFile_peekPath (& file), U"."))
	}
	BOOLEAN (tracing, I18n_translate("form.tracing"), false)
	COMMENT (U"Setting the following to anything other than zero")
	COMMENT (U"will alter the behaviour of Praat")
	COMMENT (U"in unpredictable ways.")
	INTEGER (debugOption, I18n_translate("form.debug_option"), U"0")
OK
	SET_BOOLEAN (tracing, Melder_isTracingGlobally)
	SET_INTEGER (debugOption, Melder_debug)
DO
	PREFS
		Melder_setTracing (tracing);
		Melder_debug = debugOption;
	PREFS_END
}

FORM (SETTINGS__DebugMultithreading, I18n_translate("form.debug_multithreading"), I18n_translate("menu.debug_multithreading")) {
	COMMENT (U"These settings determine how fast parallelized")
	COMMENT (U"procedures are performed on your computer.")
	BOOLEAN (useMultithreading, I18n_translate("form.use_multithreading"), true)
	INTEGER (maximumNumberOfConcurrentThreads, I18n_translate("form.maximum_number_of_threads"), U"0 (= automatic)")
	INTEGER (minimumNumberOfElementsPerThread, I18n_translate("form.minimum_frames_per_thread"), U"0 (= automatic)")
	BOOLEAN (traceThreads, I18n_translate("form.trace_threads"), false)
	OK
DO
	PREFS
		MelderThread_debugMultithreading (useMultithreading, maximumNumberOfConcurrentThreads,
				minimumNumberOfElementsPerThread, traceThreads);
	PREFS_END
}

DIRECT (INFO_NONE__listReadableTypesOfObjects) {
	INFO_NONE
		Thing_listReadableClasses ();
	INFO_NONE_END
}

FORM (INFO_praat_library_createC, I18n_translate("form.create_c_interface"), nullptr) {
	BOOLEAN (isInHeader, I18n_translate("form.is_in_header"), true)
	BOOLEAN (includeCreateAPI, I18n_translate("form.include_create_api"), true)
	BOOLEAN (includeReadAPI, I18n_translate("form.include_read_api"), true)
	BOOLEAN (includeSaveAPI, I18n_translate("form.include_save_api"), true)
	BOOLEAN (includeQueryAPI, I18n_translate("form.include_query_api"), true)
	BOOLEAN (includeModifyAPI, I18n_translate("form.include_modify_api"), true)
	BOOLEAN (includeToAPI, I18n_translate("form.include_to_api"), true)
	BOOLEAN (includeRecordAPI, I18n_translate("form.include_record_api"), true)
	BOOLEAN (includePlayAPI, I18n_translate("form.include_play_api"), true)
	BOOLEAN (includeDrawAPI, I18n_translate("form.include_draw_api"), true)
	BOOLEAN (includeHelpAPI, I18n_translate("form.include_help_api"), false)
	BOOLEAN (includeWindowAPI, I18n_translate("form.include_window_api"), false)
	BOOLEAN (includeDemoAPI, I18n_translate("form.include_demo_api"), false)
	OK
DO
	praat_library_createC (isInHeader, includeCreateAPI, includeReadAPI, includeSaveAPI,
		includeQueryAPI, includeModifyAPI, includeToAPI, includeRecordAPI, includePlayAPI,
		includeDrawAPI, includeHelpAPI, includeWindowAPI, includeDemoAPI);
	END_NO_NEW_DATA
}

DIRECT (INFO_NONE__reportSystemProperties) {
	INFO_NONE
		praat_reportSystemProperties ();
	INFO_NONE_END
}

DIRECT (INFO_NONE__reportAppProperties) {
	INFO_NONE
		praat_reportAppProperties ();
	INFO_NONE_END
}

DIRECT (INFO_NONE__reportGraphicalProperties) {
	INFO_NONE
		praat_reportGraphicalProperties ();
	INFO_NONE_END
}

DIRECT (INFO_NONE__reportIntegerProperties) {
	INFO_NONE
		praat_reportIntegerProperties ();
	INFO_NONE_END
}

DIRECT (INFO_NONE__reportFloatingPointProperties) {
	INFO_NONE
		praat_reportFloatingPointProperties ();
	INFO_NONE_END
}

DIRECT (INFO_NONE__reportMemoryUse) {
	INFO_NONE
		praat_reportMemoryUse ();
	INFO_NONE_END
}

DIRECT (INFO_NONE__reportTextProperties) {
	INFO_NONE
		praat_reportTextProperties ();
	INFO_NONE_END
}

DIRECT (INFO_NONE__reportFontProperties) {
	INFO_NONE
		praat_reportFontProperties ();
	INFO_NONE_END
}

/********** Callbacks of the Open menu. **********/

/*
	Note: readFromFile should not call praat_updateSelection(),
	because praat_updateSelection() should be called after all files have been read,
	not just the current one.
*/
static void readFromFile (MelderFile file) {
	autoDaata object = Data_readFromFile (file);
	if (! object)
		return;   // this can happen with Picture_readFromPraatPictureFile (file recognized, but no data)
	if (Thing_isa (object.get(), classManPages) && ! Melder_batch) {
		ManPages manPages = (ManPages) object.get();
		manPages -> commandsWithExternalSideEffectsAreAllowed = false;
		ManPage firstPage = manPages -> pages.at [1];
		autoManual manual = Manual_create (firstPage -> title.get(), nullptr,
				(ManPages) object.releaseToAmbiguousOwner(), true, false);
		if (manPages -> executable)
			Melder_warning (U"These manual pages contain links to executable scripts.\n"
				"Only navigate these pages if you trust their author!");
		manual.releaseToUser();
		return;
	}
	if (Thing_isa (object.get(), classScript) && ! Melder_batch) {
		autoScriptEditor editor = ScriptEditor_createFromScript_canBeNull (nullptr, object.static_cast_move <structScript>());
		if (! editor) {
			(void) 0;   // the script was already open, and the user has been notified of that
		} else {
			editor.releaseToUser();
		}
		return;
	}
	if (Thing_isa (object.get(), classNotebook) && ! Melder_batch) {
		autoNotebookEditor editor = NotebookEditor_createFromNotebook_canBeNull ((Notebook) object.get());
		if (! editor) {
			(void) 0;   // the script was already open, and the user has been notified of that
		} else {
			editor.releaseToUser();
		}
		return;
	}
	praat_newWithFile (object.move(), file, MelderFile_name (file));
}

FORM_READ (READMANY_Data_readFromFile, I18n_translate("form.read_from_file"), 0, true) {
	readFromFile (file);
	if (interpreter)
		interpreter -> returnType = kInterpreter_ReturnType::OBJECT_;
	END_WITH_NEW_DATA   // this calls praat_updateSelection(), after reading a single file; see also cb_openDocument
}

/********** Callbacks of the Save menu. **********/

FORM_SAVE (SAVE_Data_writeToTextFile, I18n_translate("form.save_as_text"), nullptr, nullptr) {
	if (theCurrentPraatObjects -> totalSelection == 1) {
		LOOP {
			iam_LOOP (Daata);
			Data_writeToTextFile (me, file);
		}
	} else {
		autoCollection set = praat_getSelectedObjects ();
		Data_writeToTextFile (set.get(), file);
	}
	END_NO_NEW_DATA
}

FORM_SAVE (SAVE_Data_writeToShortTextFile, I18n_translate("form.save_as_short_text"), nullptr, nullptr) {
	if (theCurrentPraatObjects -> totalSelection == 1) {
		LOOP {
			iam_LOOP (Daata);
			Data_writeToShortTextFile (me, file);
		}
	} else {
		autoCollection set = praat_getSelectedObjects ();
		Data_writeToShortTextFile (set.get(), file);
	}
	END_NO_NEW_DATA
}

FORM_SAVE (SAVE_Data_writeToBinaryFile, I18n_translate("form.save_as_binary"), nullptr, nullptr) {
	if (theCurrentPraatObjects -> totalSelection == 1) {
		LOOP {
			iam_LOOP (Daata);
			Data_writeToBinaryFile (me, file);
		}
	} else {
		autoCollection set = praat_getSelectedObjects ();
		Data_writeToBinaryFile (set.get(), file);
	}
	END_NO_NEW_DATA
}

FORM (PRAAT_ManPages_saveToHtmlFolder, I18n_translate("form.save_manual_to_html"), nullptr) {
	FOLDER (folder, I18n_translate("form.folder"), U"")
OK
	LOOP {
		iam_LOOP (ManPages);
		SET_STRING (folder, MelderFolder_peekPath (& my rootDirectory))
	}
DO
	LOOP {
		iam_LOOP (ManPages);
		ManPages_writeAllToHtmlDir (me, nullptr, folder);
	}
	END_NO_NEW_DATA
}

DIRECT (WINDOW_ManPages_view) {
	LOOP {
		iam_LOOP (ManPages);
		ManPage firstPage = my pages.at [1];
		autoManual manual = Manual_create (firstPage -> title.get(), nullptr, me, false, false);
		if (my executable)
			Melder_warning (U"These manual pages contain links to executable scripts.\n"
				"Only navigate these pages if you trust their author!");
		praat_installEditor (manual.get(), IOBJECT);
		manual.releaseToUser();
	}
	END_NO_NEW_DATA
}

/********** Callbacks of the Help menu. **********/

FORM (PRAAT__SearchManual, I18n_translate("form.search_manual"), I18n_translate("menu.search_manual")) {
	TEXTFIELD (query, U"Search for strings (separate with spaces)", U"", 3)
	OK
DO
	PRAAT
		if (theCurrentPraatApplication -> batch)
			Melder_throw (U"Cannot view a manual from batch.");
		autoManual manual = Manual_create (U"Intro", nullptr, theCurrentPraatApplication -> manPages, false, true);
		Manual_search (manual.get(), query);
		manual.releaseToUser();
	PRAAT_END
}

FORM (PRAAT__GoToManualPage, I18n_translate("form.go_to_manual_page"), nullptr) {
	LIST (goToPageNumber, U"Page", ManPages_getTitles (theCurrentPraatApplication -> manPages), 1)
	OK
DO
	PRAAT
		if (theCurrentPraatApplication -> batch)
			Melder_throw (U"Cannot view a manual from batch.");
		autoManual manual = Manual_create (U"Intro", nullptr, theCurrentPraatApplication -> manPages, false, true);
		HyperPage_goToPage_number (manual.get(), goToPageNumber);
		manual.releaseToUser();
	PRAAT_END
}

FORM (HELP_SaveManualToHtmlFolder, I18n_translate("form.save_manual_to_html"), nullptr) {
	FOLDER (folder, I18n_translate("form.folder"), U"")
OK
	structMelderFolder currentFolder { };
	Melder_getCurrentFolder (& currentFolder);
	SET_STRING (folder, MelderFolder_peekPath (& currentFolder))
DO
	ManPages_writeAllToHtmlDir (theCurrentPraatApplication -> manPages, nullptr, folder);
	END_NO_NEW_DATA
}

/********** Menu descriptions. **********/

void praat_show () {
	/*
		(De)sensitivize the fixed buttons as appropriate for the current selection.
	*/
	praat_sensitivizeFixedButtonCommand (I18n_translate("button.remove"), theCurrentPraatObjects -> totalSelection != 0);
	praat_sensitivizeFixedButtonCommand (I18n_translate("button.rename"), theCurrentPraatObjects -> totalSelection == 1);
	praat_sensitivizeFixedButtonCommand (I18n_translate("button.copy"), theCurrentPraatObjects -> totalSelection == 1);
	praat_sensitivizeFixedButtonCommand (I18n_translate("button.info"), theCurrentPraatObjects -> totalSelection == 1);
	praat_sensitivizeFixedButtonCommand (I18n_translate("button.inspect"), theCurrentPraatObjects -> totalSelection != 0);
	praat_actions_show ();
	if (theCurrentPraatApplication == & theForegroundPraatApplication && theReferenceToTheOnlyButtonEditor)
		Editor_dataChanged (theReferenceToTheOnlyButtonEditor, nullptr);
}

/********** Menu descriptions. **********/

void praat_addFixedButtons (GuiWindow window) {
	praat_addFixedButtonCommand (window, I18n_translate("button.rename"), MODIFY_Rename, 8, 70);
	praat_addFixedButtonCommand (window, I18n_translate("button.copy"), NEW1_Copy, 98, 70);
	praat_addFixedButtonCommand (window, I18n_translate("button.inspect"),
			PRAAT__Inspect, 8, 40);
	praat_addFixedButtonCommand (window, I18n_translate("button.info"), INFO_Info, 98, 40);
	praat_addFixedButtonCommand (window, I18n_translate("button.remove"), PRAAT_Remove, 8, 10);
}

static void searchProc () {
	PRAAT__SearchManual (nullptr, 0, nullptr, nullptr, nullptr, nullptr, false, nullptr, nullptr);
}

static MelderString itemTitle_about;

static autoDaata scriptRecognizer (integer nread, const char *header, MelderFile file) {
	conststring32 name = MelderFile_name (file);
	if (nread < 2)
		return autoDaata ();
	if ((header [0] == '#' && header [1] == '!')
		|| Melder_endsWith_caseAware (name, U".praat")
	    || Melder_endsWith_caseAware (name, U".html")
	) {
		return Script_createFromFile (file);
	}
	return autoDaata ();
}

static autoDaata notebookRecognizer (integer nread, const char * /* header */, MelderFile file) {
	conststring32 name = MelderFile_name (file);
	if (nread < 2)
		return autoDaata ();
	if (Melder_endsWith_caseAware (name, U".praatnb") || Melder_endsWith_caseAware (name, U".cpp"))
		return Notebook_createFromFile (file);
	return autoDaata ();
}

static void cb_openDocument (MelderFile file) {
	try {
		readFromFile (file);   // read a single file without calling praat_updateSelection()
	} catch (MelderError) {
		Melder_flushError ();
	}
}
static void cb_finishedOpeningDocuments () {
	try {
		praat_updateSelection ();   // this finally calls praat_updateSelection(), after each separate file has been read
	} catch (MelderError) {
		Melder_flushError ();
	}
}

#if cocoa
DIRECT (PRAAT__cut) {
	PRAAT
		NSWindow *window = [NSApp keyWindow];
		NSResponder *firstResponder = [window firstResponder];
		if ([firstResponder isKindOfClass: [NSTextView class]])
			[(NSTextView *) firstResponder cut: nil];
	PRAAT_END
}
DIRECT (PRAAT__copy) {
	PRAAT
		NSWindow *window = [NSApp keyWindow];
		NSResponder *firstResponder = [window firstResponder];
		if ([firstResponder isKindOfClass: [NSTextView class]])
			[(NSTextView *) firstResponder copy: nil];
	PRAAT_END
}
DIRECT (PRAAT__paste) {
	PRAAT
		trace (U"Pasting to app.");
		NSWindow *window = [NSApp keyWindow];
		NSResponder *firstResponder = [window firstResponder];
		if ([firstResponder isKindOfClass: [NSTextView class]])
			[(NSTextView *) firstResponder pasteAsPlainText: nil];
	PRAAT_END
}
DIRECT (PRAAT__minimize) {
	PRAAT
		[[NSApp keyWindow] performMiniaturize: nil];
	PRAAT_END
}
DIRECT (PRAAT__zoom) {
	PRAAT
		[[NSApp keyWindow] performZoom: nil];
	PRAAT_END
}
DIRECT (PRAAT__close) {
	PRAAT
		[[NSApp keyWindow] performClose: nil];
	PRAAT_END
}
#endif

void praat_addMenus (GuiWindow window) {
	Melder_setSearchProc (searchProc);

	Data_recognizeFileType (scriptRecognizer);
	Data_recognizeFileType (notebookRecognizer);

	/*
		Create the menu titles in the bar.
	*/
	if (! theCurrentPraatApplication -> batch) {
	#ifdef macintosh
		praatMenu = GuiMenu_createInWindow (nullptr, U"\024", 0);
		#if cocoa
			editMenu = GuiMenu_createInWindow (nullptr, I18n_translate("menu.edit"), 0);
			windowMenu = GuiMenu_createInWindow (nullptr, I18n_translate("menu.window"), 0);
		#endif
	#else
		praatMenu = GuiMenu_createInWindow (window, I18n_translate("menu.praat"), 0);
	#endif
	newMenu = GuiMenu_createInWindow (window, I18n_translate("menu.new"), 0);
	readMenu = GuiMenu_createInWindow (window, I18n_translate("menu.open"), 0);
	
	// Create i18n menu (before Save menu)
	I18n_addToMenuBar (window);
	
	praat_actions_createWriteMenu (window);
		
		#ifdef macintosh
			applicationHelpMenu = GuiMenu_createInWindow (nullptr, I18n_translate("menu.help"), 0);
		#endif
		helpMenu = GuiMenu_createInWindow (window, I18n_translate("menu.help"), 0);
		
		// Set menu references for i18n updates
		I18n_setMenuReferences(window, newMenu, readMenu, helpMenu);
	}
	
	MelderString_append (& itemTitle_about, I18n_translate("info.about"), U" ", Melder_upperCaseAppName());
	praat_addMenuCommand (U"Objects", U"Praat", itemTitle_about.string, nullptr, GuiMenu_UNHIDABLE,
			PRAAT__About);
	#ifdef macintosh
		#if cocoa
			/*
				HACK: give the following commands weird names,
				because otherwise they may be called from a script.
				(we add three alt-spaces)
			*/
			praat_addMenuCommand (U"Objects", U"Edit", I18n_translate("menu.cut"), nullptr, GuiMenu_UNHIDABLE | 'X' | GuiMenu_NO_API,
					PRAAT__cut);
			praat_addMenuCommand (U"Objects", U"Edit", I18n_translate("menu.copy"), nullptr, GuiMenu_UNHIDABLE | 'C' | GuiMenu_NO_API,
					PRAAT__copy);
			praat_addMenuCommand (U"Objects", U"Edit", I18n_translate("menu.paste"), nullptr, GuiMenu_UNHIDABLE | 'V' | GuiMenu_NO_API,
					PRAAT__paste);
			praat_addMenuCommand (U"Objects", U"Window", I18n_translate("menu.minimize"), nullptr, GuiMenu_UNHIDABLE | GuiMenu_NO_API,
					PRAAT__minimize);
			praat_addMenuCommand (U"Objects", U"Window", I18n_translate("menu.zoom"), nullptr, GuiMenu_UNHIDABLE | GuiMenu_NO_API,
					PRAAT__zoom);
			praat_addMenuCommand (U"Objects", U"Window", I18n_translate("menu.close"), nullptr, 'W' | GuiMenu_NO_API,
					PRAAT__close);
		#endif
	#endif
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.separator_script"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.new_script_dialog"), nullptr, GuiMenu_NO_API,
			PRAAT__newScript);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.new_notebook_dialog"), nullptr, GuiMenu_NO_API,
			PRAAT__newNotebook);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.open_script_dialog"), nullptr, GuiMenu_NO_API,
			PRAAT__openScript);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.open_notebook_dialog"), nullptr, GuiMenu_NO_API,
			PRAAT__openNotebook);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.separator_buttons"), nullptr, 0, nullptr);
	// Temporarily disable all i18n macros to debug startup issue
	/*
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.add_menu_command"), nullptr, GuiMenu_HIDDEN | GuiMenu_NO_API,
		PRAAT__addMenuCommand);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.hide_menu_command"), nullptr, GuiMenu_HIDDEN | GuiMenu_NO_API,
		PRAAT__hideMenuCommand);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.show_menu_command"), nullptr, GuiMenu_HIDDEN | GuiMenu_NO_API,
		PRAAT__showMenuCommand);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.add_action_command"), nullptr, GuiMenu_HIDDEN | GuiMenu_NO_API,
		PRAAT__addAction);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.hide_action_command"), nullptr, GuiMenu_HIDDEN | GuiMenu_NO_API,
		PRAAT__hideAction);
	praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.show_action_command"), nullptr, GuiMenu_HIDDEN | GuiMenu_NO_API,
		PRAAT__showAction);
	*/

	GuiMenuItem menuItem = praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.goodies"), nullptr, GuiMenu_UNHIDABLE, nullptr);
	goodiesMenu = menuItem ? menuItem -> d_menu : nullptr;
	praat_addMenuCommand (U"Objects", U"Goodies", I18n_translate("menu.calculator"),
			nullptr, 'U', INFO_NONE__praat_calculator);
	praat_addMenuCommand (U"Objects", U"Goodies", I18n_translate("menu.report_difference"),
			nullptr, 0, INFO_reportDifferenceOfTwoProportions);
	praat_addMenuCommand (U"Objects", U"Goodies", I18n_translate("menu.separator_demo_window"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Goodies", I18n_translate("menu.save_demo_pdf"), nullptr, 0, GRAPHICS_saveDemoWindowAsPdfFile);

	menuItem = praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.settings"), nullptr, GuiMenu_UNHIDABLE, nullptr);
	preferencesMenu = menuItem ? menuItem -> d_menu : nullptr;
	praat_addMenuCommand (U"Objects", U"Settings", I18n_translate("menu.buttons"),
			nullptr, GuiMenu_UNHIDABLE, PRAAT__editButtons);
	praat_addMenuCommand (U"Objects", U"Settings", I18n_translate("menu.separator_encoding_prefs"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Settings", I18n_translate("menu.text_reading_settings"),
			nullptr, 0, SETTINGS__TextReadingSettings);   // alternative GuiMenu_DEPRECATED_2023
	praat_addMenuCommand (U"Objects", U"Settings", I18n_translate("menu.text_writing_settings"),
			nullptr, 0, SETTINGS__TextWritingSettings);   // alternative GuiMenu_DEPRECATED_2023
	praat_addMenuCommand (U"Objects", U"Settings", I18n_translate("menu.cjk_font_style_settings"),
			nullptr, 0, SETTINGS__CjkFontStyleSettings);   // alternative GuiMenu_DEPRECATED_2023

	menuItem = praat_addMenuCommand (U"Objects", U"Praat", I18n_translate("menu.technical"), nullptr, GuiMenu_UNHIDABLE, nullptr);
	technicalMenu = menuItem ? menuItem -> d_menu : nullptr;
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.report_memory_use"),
			nullptr, 0, INFO_NONE__reportMemoryUse);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.report_integer_properties"),
			nullptr, 0, INFO_NONE__reportIntegerProperties);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.report_floating_point_properties"),
			nullptr, 0, INFO_NONE__reportFloatingPointProperties);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.report_system_properties"),
			nullptr, 0, INFO_NONE__reportSystemProperties);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.report_app_properties"),
			nullptr, 0, INFO_NONE__reportAppProperties);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.report_graphical_properties"),
			nullptr, 0, INFO_NONE__reportGraphicalProperties);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.report_text_properties"),
			nullptr, 0, INFO_NONE__reportTextProperties);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.report_font_properties"),
			nullptr, 0, INFO_NONE__reportFontProperties);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.debug"),
			nullptr, 0, SETTINGS__debug);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.debug_multithreading"),
			nullptr, 0, SETTINGS__DebugMultithreading);

	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.separator_api"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.list_readable_types"),
			nullptr, 0, INFO_NONE__listReadableTypesOfObjects);
	praat_addMenuCommand (U"Objects", U"Technical", I18n_translate("menu.create_c_interface"),
			nullptr, 0, INFO_praat_library_createC);

	praat_addMenuCommand (U"Objects", U"Open", I18n_translate("menu.read_from_file"), nullptr, GuiMenu_ATTRACTIVE | 'O', READMANY_Data_readFromFile);

	praat_addAction1 (classDaata, 0, Melder_cat (I18n_translate("menu.save_as_text_file"), U" || ", I18n_translate("menu.write_to_text_file")),
			nullptr, 0, SAVE_Data_writeToTextFile);   // alternative GuiMenu_DEPRECATED_2011
	praat_addAction1 (classDaata, 0, Melder_cat (I18n_translate("menu.save_as_short_text_file"), U" || ", I18n_translate("menu.write_to_short_text_file")),
			nullptr, 0, SAVE_Data_writeToShortTextFile);   // alternative GuiMenu_DEPRECATED_2011
	praat_addAction1 (classDaata, 0, Melder_cat (I18n_translate("menu.save_as_binary_file"), U" || ", I18n_translate("menu.write_to_binary_file")),
			nullptr, 0, SAVE_Data_writeToBinaryFile);   // alternative GuiMenu_DEPRECATED_2011

	praat_addAction1 (classManPages, 1, Melder_cat (I18n_translate("menu.save_to_html_folder"), U" || ", I18n_translate("menu.save_to_html_directory")),
			nullptr, 0, PRAAT_ManPages_saveToHtmlFolder);   // alternative GuiMenu_DEPRECATED_2020
	praat_addAction1 (classManPages, 1, I18n_translate("menu.view_manual"),
			nullptr, 0, WINDOW_ManPages_view);
}

void praat_addMenus2 () {
	praat_addMenuCommand (U"Objects", U"ApplicationHelp", I18n_translate("menu.separator_manual"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"ApplicationHelp", I18n_translate("menu.go_to_manual_page"),
			nullptr, 0, PRAAT__GoToManualPage);
	praat_addMenuCommand (U"Objects", U"ApplicationHelp", I18n_translate("menu.save_manual_to_html"),
			nullptr, GuiMenu_HIDDEN, HELP_SaveManualToHtmlFolder);
	praat_addMenuCommand (U"Objects", U"ApplicationHelp", I18n_translate("menu.search_manual"),
			nullptr, 'M' | GuiMenu_NO_API, PRAAT__SearchManual);
	praat_addMenuCommand (U"Objects", U"ApplicationHelp", itemTitle_about.string,
			nullptr, GuiMenu_UNHIDABLE, PRAAT__About);

	#if defined (macintosh)
		Gui_setOpenDocumentCallback (cb_openDocument, cb_finishedOpeningDocuments);
	#endif
}

/* End of file praat_objectMenus.cpp */
