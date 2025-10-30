/* praat_picture.cpp
 *
 * Copyright (C) 1992-2025 Paul Boersma
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

#include "praatP.h"
#include "i18n_simple.h"
#include "praatM.h"
#include "Picture.h"
#include "Printer.h"
#include "machine.h"
#include "Formula.h"
#include "site.h"

#include "GuiP.h"
#include "DemoEditor.h"

static bool praat_mouseSelectsInnerViewport;

void praat_picture_prefs () {
	Preferences_addEnum (U"Picture.font", & theCurrentPraatPicture -> font, kGraphics_font, kGraphics_font::DEFAULT);
	Preferences_addDouble (U"Picture.fontSize", & theCurrentPraatPicture -> fontSize, 10.0);
	Preferences_addBool (U"Picture.mouseSelectsInnerViewport", & praat_mouseSelectsInnerViewport, false);
}

/***** static variable *****/

static autoPicture praat_picture;

/********** CALLBACKS OF THE PICTURE MENUS **********/

/***** "Font" MENU: font part *****/

static GuiMenuItem praatButton_fonts [1 + (int) kGraphics_font::MAX];

static void updateFontMenu () {
	if (! theCurrentPraatApplication -> batch) {
		Melder_clip ((int) kGraphics_font::MIN, (int *) & theCurrentPraatPicture -> font, (int) kGraphics_font::MAX);
		for (int i = (int) kGraphics_font::MIN; i <= (int) kGraphics_font::MAX; i ++)
			GuiMenuItem_check (praatButton_fonts [i], (int) theCurrentPraatPicture -> font == i);
	}
}
DIRECT (GRAPHICS_NONE__Times) {
	GRAPHICS_NONE
		Graphics_setFont (GRAPHICS, theCurrentPraatPicture -> font = kGraphics_font::TIMES);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateFontMenu ();
}
DIRECT (GRAPHICS_NONE__Helvetica) {
	GRAPHICS_NONE
		Graphics_setFont (GRAPHICS, theCurrentPraatPicture -> font = kGraphics_font::HELVETICA);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateFontMenu ();
}
DIRECT (GRAPHICS_NONE__Palatino) {
	GRAPHICS_NONE
		Graphics_setFont (GRAPHICS, theCurrentPraatPicture -> font = kGraphics_font::PALATINO);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateFontMenu ();
}
DIRECT (GRAPHICS_NONE__Courier) {
	GRAPHICS_NONE
		Graphics_setFont (GRAPHICS, theCurrentPraatPicture -> font = kGraphics_font::COURIER);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateFontMenu ();
}

/***** "Font" MENU: size part *****/

static GuiMenuItem praatButton_10, praatButton_12, praatButton_14, praatButton_18, praatButton_24;

static void updateSizeMenu () {
	if (! theCurrentPraatApplication -> batch) {
		GuiMenuItem_check (praatButton_10, theCurrentPraatPicture -> fontSize == 10.0);
		GuiMenuItem_check (praatButton_12, theCurrentPraatPicture -> fontSize == 12.0);
		GuiMenuItem_check (praatButton_14, theCurrentPraatPicture -> fontSize == 14.0);
		GuiMenuItem_check (praatButton_18, theCurrentPraatPicture -> fontSize == 18.0);
		GuiMenuItem_check (praatButton_24, theCurrentPraatPicture -> fontSize == 24.0);
	}
}
DIRECT (GRAPHICS_10) {
	GRAPHICS_NONE
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize = 10.0);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateSizeMenu ();
}
DIRECT (GRAPHICS_12) {
	GRAPHICS_NONE
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize = 12.0);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateSizeMenu ();
}
DIRECT (GRAPHICS_14) {
	GRAPHICS_NONE
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize = 14.0);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateSizeMenu ();
}
DIRECT (GRAPHICS_18) {
	GRAPHICS_NONE
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize = 18.0);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateSizeMenu ();
}
DIRECT (GRAPHICS_24) {
	GRAPHICS_NONE
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize = 24.0);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateSizeMenu ();
}
FORM (GRAPHICS_Font_size, I18n_translate ("form.praat_picture_font_size"), I18n_translate ("form.font_menu")) {
	POSITIVE (fontSize, I18n_translate ("form.font_size_points"), U"10")
OK
	SET_REAL (fontSize, (integer) theCurrentPraatPicture -> fontSize);
DO
	GRAPHICS_NONE
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize = fontSize);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updateSizeMenu ();
}

/*static void setFontSize_keepInnerViewport (int fontSize) {
	double xmargin = praat_size * 4.2 / 72.0, ymargin = praat_size * 2.8 / 72.0;
	if (xmargin > 0.4 * (x2NDC - x1NDC)) xmargin = 0.4 * (x2NDC - x1NDC);
	if (ymargin > 0.4 * (y2NDC - y1NDC)) ymargin = 0.4 * (y2NDC - y1NDC);
	x1NDC += xmargin;
	x2NDC -= xmargin;
	y1NDC += ymargin;
	y2NDC -= ymargin;
	xmargin = fontSize * 4.2 / 72.0, ymargin = fontSize * 2.8 / 72.0;
	if (xmargin > 2 * (x2NDC - x1NDC)) xmargin = 2 * (x2NDC - x1NDC);
	if (ymargin > 2 * (y2NDC - y1NDC)) ymargin = 2 * (y2NDC - y1NDC);
	x1NDC -= xmargin;
	x2NDC += xmargin;
	y1NDC -= ymargin;
	y2NDC += ymargin;
	{
		autoPraatPictureOpen picture;
		Graphics_setFontSize (GRAPHICS, praat_size = fontSize);
	}
	Picture_setSelection (praat_picture, x1NDC, x2NDC, y1NDC, y2NDC);
	updateSizeMenu ();
}*/

/***** "Select" MENU *****/

static GuiMenuItem praatButton_innerViewport, praatButton_outerViewport;

static void updateViewportMenu () {
	if (! theCurrentPraatApplication -> batch) {
		GuiMenuItem_check (praatButton_innerViewport, praat_mouseSelectsInnerViewport ? 1 : 0);
		GuiMenuItem_check (praatButton_outerViewport, praat_mouseSelectsInnerViewport ? 0 : 1);
	}
}

DIRECT (GRAPHICS_MouseSelectsInnerViewport) {
	if (theCurrentPraatPicture != & theForegroundPraatPicture)
		Melder_throw (I18n_translate ("error.mouse_commands_not_available_inside_pictures"));
	{// scope
		autoPraatPictureOpen picture;
		Picture_setMouseSelectsInnerViewport (praat_picture.get(), praat_mouseSelectsInnerViewport = true);
	}
	updateViewportMenu ();
	END_NO_NEW_DATA
}

DIRECT (GRAPHICS_MouseSelectsOuterViewport) {
	if (theCurrentPraatPicture != & theForegroundPraatPicture)
		Melder_throw (I18n_translate ("error.mouse_commands_not_available_inside_pictures"));
	{// scope
		autoPraatPictureOpen picture;
		Picture_setMouseSelectsInnerViewport (praat_picture.get(), praat_mouseSelectsInnerViewport = false);
	}
	updateViewportMenu ();
	END_NO_NEW_DATA
}

FORM (GRAPHICS_SelectInnerViewport, I18n_translate ("form.praat_picture_select_inner_viewport"), I18n_translate ("form.select_inner_viewport")) {
	COMMENT (I18n_translate ("comment.viewport_is_selected_rectangle"))
	COMMENT (U"It is where your next drawing will appear.")
	COMMENT (U"The rectangle you select here will not include the margins.")
	COMMENT (U"")
	REAL (left, U"left Horizontal range (inches)", U"0.0")
	REAL (right, U"right Horizontal range (inches)", U"6.0")
	REAL (top, U"left Vertical range (inches)", U"0.0")
	REAL (bottom, U"right Vertical range (inches)", U"6.0")
OK
	double xmargin = theCurrentPraatPicture -> fontSize * 4.2 / 72.0, ymargin = theCurrentPraatPicture -> fontSize * 2.8 / 72.0;
	Melder_clipRight (& ymargin, 0.4 * (theCurrentPraatPicture -> y2NDC - theCurrentPraatPicture -> y1NDC));
	Melder_clipRight (& xmargin, 0.4 * (theCurrentPraatPicture -> x2NDC - theCurrentPraatPicture -> x1NDC));
	SET_REAL (left, theCurrentPraatPicture -> x1NDC + xmargin)
	SET_REAL (right, theCurrentPraatPicture -> x2NDC - xmargin)
	SET_REAL (top, Picture_HEIGHT - theCurrentPraatPicture -> y2NDC + ymargin)
	SET_REAL (bottom, Picture_HEIGHT - theCurrentPraatPicture -> y1NDC - ymargin)
DO
	//Melder_require (praat_commandsWithExternalSideEffectsAreAllowed (), U"Viewport commands are not available inside manuals.");
	double xmargin = theCurrentPraatPicture -> fontSize * 4.2 / 72.0, ymargin = theCurrentPraatPicture -> fontSize * 2.8 / 72.0;
	trace (U"1: xmargin ", xmargin, U" ymargin ", ymargin);
	if (theCurrentPraatPicture != & theForegroundPraatPicture) {
		integer x1DC, x2DC, y1DC, y2DC;
		Graphics_inqWsViewport (GRAPHICS, & x1DC, & x2DC, & y1DC, & y2DC);
		double x1wNDC, x2wNDC, y1wNDC, y2wNDC;
		Graphics_inqWsWindow (GRAPHICS, & x1wNDC, & x2wNDC, & y1wNDC, & y2wNDC);
		const double wDC = (x2DC - x1DC) / (x2wNDC - x1wNDC);
		const double hDC = integer_abs (y2DC - y1DC) / (y2wNDC - y1wNDC);
		xmargin *= Graphics_getResolution (GRAPHICS) / wDC;
		ymargin *= Graphics_getResolution (GRAPHICS) / hDC;
	}
	Melder_clipRight (& xmargin, 2.0 * (right - left));
	Melder_clipRight (& ymargin, 2.0 * (bottom - top));
	trace (U"2: xmargin ", xmargin, U" ymargin ", ymargin);
	if (left == right)
		Melder_throw (U"The left and right edges of the viewport cannot be equal.\nPlease change the horizontal range.");
	Melder_sort (& left, & right);
	if (top == bottom)
		Melder_throw (U"The top and bottom edges of the viewport cannot be equal.\nPlease change the vertical range.");
	theCurrentPraatPicture -> x1NDC = left - xmargin;
	theCurrentPraatPicture -> x2NDC = right + xmargin;
	if (theCurrentPraatPicture == & theForegroundPraatPicture) {
		Melder_sort (& top, & bottom);
		theCurrentPraatPicture -> y1NDC = (Picture_HEIGHT - bottom) - ymargin;
		theCurrentPraatPicture -> y2NDC = (Picture_HEIGHT - top) + ymargin;
		Picture_setSelection (praat_picture.get(), theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC,
				theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_updateWs (GRAPHICS);
	} else if (theCurrentPraatObjects != & theForegroundPraatObjects) {   // in manual?
		Melder_sort (& top, & bottom);
		double x1wNDC, x2wNDC, y1wNDC, y2wNDC;
		Graphics_inqWsWindow (GRAPHICS, & x1wNDC, & x2wNDC, & y1wNDC, & y2wNDC);
		const double height_NDC = y2wNDC - y1wNDC;
		theCurrentPraatPicture -> y1NDC = height_NDC - bottom - ymargin;
		theCurrentPraatPicture -> y2NDC = height_NDC - top + ymargin;
	} else {
		Melder_sort (& bottom, & top);
		theCurrentPraatPicture -> y1NDC = bottom - ymargin;
		theCurrentPraatPicture -> y2NDC = top + ymargin;
		Graphics_setViewport (GRAPHICS,
			theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC,
			theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC
		);   // to ensure that Demo_x() updates
	}
	trace (U"3:"
		U" x1NDC ", theCurrentPraatPicture -> x1NDC,
		U" x2NDC ", theCurrentPraatPicture -> x2NDC,
		U" y1NDC ", theCurrentPraatPicture -> y1NDC,
		U" y2NDC ", theCurrentPraatPicture -> y2NDC
	);
	END_NO_NEW_DATA
}

FORM (GRAPHICS_SelectOuterViewport, I18n_translate("form.praat_picture_select_outer_viewport"), I18n_translate("form.select_outer_viewport")) {
	COMMENT (I18n_translate ("comment.viewport_is_selected_rectangle"))
	COMMENT (U"It is where your next drawing will appear.")
	COMMENT (U"The rectangle you select here will include the margins.")
	COMMENT (U"")
	REAL (left, U"left Horizontal range (inches)", U"0.0")
	REAL (right, U"right Horizontal range (inches)", U"6.0")
	REAL (top, U"left Vertical range (inches)", U"0.0")
	REAL (bottom, U"right Vertical range (inches)", U"6.0")
OK
	SET_REAL (left, theCurrentPraatPicture -> x1NDC)
	SET_REAL (right, theCurrentPraatPicture -> x2NDC)
	SET_REAL (top, Picture_HEIGHT - theCurrentPraatPicture -> y2NDC)
	SET_REAL (bottom, Picture_HEIGHT - theCurrentPraatPicture -> y1NDC)
DO
	//Melder_require (praat_commandsWithExternalSideEffectsAreAllowed (), U"Viewport commands are not available inside manuals.");
	if (left == right)
		Melder_throw (U"The left and right edges of the viewport cannot be equal.\nPlease change the horizontal range.");
	Melder_sort (& left, & right);
	if (top == bottom)
		Melder_throw (U"The top and bottom edges of the viewport cannot be equal.\nPlease change the vertical range.");
	theCurrentPraatPicture -> x1NDC = left;
	theCurrentPraatPicture -> x2NDC = right;
	if (theCurrentPraatPicture == & theForegroundPraatPicture) {
		Melder_sort (& top, & bottom);
		theCurrentPraatPicture -> y1NDC = Picture_HEIGHT - bottom;
		theCurrentPraatPicture -> y2NDC = Picture_HEIGHT - top;
		Picture_setSelection (praat_picture.get(), theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC,
				theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_updateWs (GRAPHICS);
	} else if (theCurrentPraatObjects != & theForegroundPraatObjects) {   // in manual?
		Melder_sort (& top, & bottom);
		double x1wNDC, x2wNDC, y1wNDC, y2wNDC;
		Graphics_inqWsWindow (GRAPHICS, & x1wNDC, & x2wNDC, & y1wNDC, & y2wNDC);
		const double height_NDC = y2wNDC - y1wNDC;
		theCurrentPraatPicture -> y1NDC = height_NDC - bottom;
		theCurrentPraatPicture -> y2NDC = height_NDC - top;
	} else {
		Melder_sort (& bottom, & top);
		theCurrentPraatPicture -> y1NDC = bottom;
		theCurrentPraatPicture -> y2NDC = top;
		Graphics_setViewport (GRAPHICS,
			theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC,
			theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC
		);   // to ensure that Demo_x() updates
	}
	END_NO_NEW_DATA
}

FORM (GRAPHICS_ViewportText, I18n_translate("form.praat_picture_viewport_text"), I18n_translate("form.viewport_text")) {
	CHOICEx (horizontalAlignment, I18n_translate("form.horizontal_alignment_picture"), 2, 0)
		OPTION (U"left")
		OPTION (U"centre")
		OPTION (U"right")
	CHOICEx (verticalAlignment, I18n_translate("form.vertical_alignment_picture"), 2, 0)
		OPTION (U"bottom")
		OPTION (U"half")
		OPTION (U"top")
	REAL (rotation, I18n_translate("form.rotation_degrees_picture"), U"0")
	TEXTFIELD (text, I18n_translate("form.text"), U"", 3)
OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	autoPraatPictureOpen picture;
	Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	Graphics_setWindow (GRAPHICS, 0, 1, 0, 1);
	Graphics_setTextAlignment (GRAPHICS, (kGraphics_horizontalAlignment) horizontalAlignment, verticalAlignment);
	Graphics_setTextRotation (GRAPHICS, rotation);
	Graphics_text (GRAPHICS, horizontalAlignment == 0 ? 0.0 : horizontalAlignment == 1 ? 0.5 : 1.0,
			verticalAlignment == 0 ? 0.0 : verticalAlignment == 1 ? 0.5 : 1.0, text);
	Graphics_setTextRotation (GRAPHICS, 0.0);
	Graphics_setWindow (GRAPHICS, x1WC, x2WC, y1WC, y2WC);
	END_NO_NEW_DATA
}

/***** "Pen" MENU *****/

static GuiMenuItem praatButton_lines [4];
static GuiMenuItem praatButton_black, praatButton_white, praatButton_red, praatButton_green, praatButton_blue,
	praatButton_yellow, praatButton_cyan, praatButton_magenta, praatButton_maroon, praatButton_lime, praatButton_navy,
	praatButton_teal, praatButton_purple, praatButton_olive, praatButton_pink, praatButton_silver, praatButton_grey;

static void updatePenMenu () {
	if (! theCurrentPraatApplication -> batch) {
		for (int i = Graphics_DRAWN; i <= Graphics_DASHED; i ++)
			GuiMenuItem_check (praatButton_lines [i], theCurrentPraatPicture -> lineType == i);
		GuiMenuItem_check (praatButton_black   , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_BLACK));
		GuiMenuItem_check (praatButton_white   , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_WHITE));
		GuiMenuItem_check (praatButton_red     , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_RED));
		GuiMenuItem_check (praatButton_green   , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_GREEN));
		GuiMenuItem_check (praatButton_blue    , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_BLUE));
		GuiMenuItem_check (praatButton_yellow  , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_YELLOW));
		GuiMenuItem_check (praatButton_cyan    , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_CYAN));
		GuiMenuItem_check (praatButton_magenta , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_MAGENTA));
		GuiMenuItem_check (praatButton_maroon  , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_MAROON));
		GuiMenuItem_check (praatButton_lime    , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_LIME));
		GuiMenuItem_check (praatButton_navy    , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_NAVY));
		GuiMenuItem_check (praatButton_teal    , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_TEAL));
		GuiMenuItem_check (praatButton_purple  , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_PURPLE));
		GuiMenuItem_check (praatButton_olive   , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_OLIVE));
		GuiMenuItem_check (praatButton_pink    , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_PINK));
		GuiMenuItem_check (praatButton_silver  , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_SILVER));
		GuiMenuItem_check (praatButton_grey    , MelderColour_equal (theCurrentPraatPicture -> colour, Melder_GREY));
	}
}

DIRECT (GRAPHICS_Solid_line) {
	GRAPHICS_NONE
		Graphics_setLineType (GRAPHICS, theCurrentPraatPicture -> lineType = Graphics_DRAWN);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updatePenMenu ();
}
DIRECT (GRAPHICS_Dotted_line) {
	GRAPHICS_NONE
		Graphics_setLineType (GRAPHICS, theCurrentPraatPicture -> lineType = Graphics_DOTTED);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updatePenMenu ();
}
DIRECT (GRAPHICS_Dashed_line) {
	GRAPHICS_NONE
		Graphics_setLineType (GRAPHICS, theCurrentPraatPicture -> lineType = Graphics_DASHED);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updatePenMenu ();
}
DIRECT (GRAPHICS_Dashed_dotted_line) {
	GRAPHICS_NONE
		Graphics_setLineType (GRAPHICS, theCurrentPraatPicture -> lineType = Graphics_DASHED_DOTTED);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updatePenMenu ();
}

FORM (GRAPHICS_Line_width, I18n_translate("form.praat_picture_line_width"), nullptr) {
	POSITIVE (lineWidth, I18n_translate("form.line_width"), U"1.0")
OK
	SET_REAL (lineWidth, theCurrentPraatPicture -> lineWidth)
DO
	GRAPHICS_NONE
		Graphics_setLineWidth (GRAPHICS, theCurrentPraatPicture -> lineWidth = lineWidth);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_Arrow_size, I18n_translate("form.praat_picture_arrow_size"), nullptr) {
	POSITIVE (arrowSize, I18n_translate("form.arrow_size"), U"1.0")
OK
	SET_REAL (arrowSize, theCurrentPraatPicture -> arrowSize)
DO
	GRAPHICS_NONE
		Graphics_setArrowSize (GRAPHICS, theCurrentPraatPicture -> arrowSize = arrowSize);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_Speckle_size, I18n_translate("form.praat_picture_speckle_size"), nullptr) {
	COMMENT (U"Here you determine the diameter (in millimetres)")
	COMMENT (U"of the dots that are drawn by \"speckle\" commands.")
	POSITIVE (speckleSize, I18n_translate("form.speckle_size_mm"), U"1.0")
OK
	SET_REAL (speckleSize, theCurrentPraatPicture -> speckleSize)
DO
	GRAPHICS_NONE
		Graphics_setSpeckleSize (GRAPHICS, theCurrentPraatPicture -> speckleSize = speckleSize);
	GRAPHICS_NONE_END
}

static void setColour (MelderColour colour) {
	{// scope
		autoPraatPictureOpen picture;
		Graphics_setColour (GRAPHICS, colour);
	}
	theCurrentPraatPicture -> colour = colour;
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updatePenMenu ();
}
DIRECT (GRAPHICS_Black)   { setColour (Melder_BLACK);   END_NO_NEW_DATA }
DIRECT (GRAPHICS_White)   { setColour (Melder_WHITE);   END_NO_NEW_DATA }
DIRECT (GRAPHICS_Red)     { setColour (Melder_RED);     END_NO_NEW_DATA }
DIRECT (GRAPHICS_Green)   { setColour (Melder_GREEN);   END_NO_NEW_DATA }
DIRECT (GRAPHICS_Blue)    { setColour (Melder_BLUE);    END_NO_NEW_DATA }
DIRECT (GRAPHICS_Yellow)  { setColour (Melder_YELLOW);  END_NO_NEW_DATA }
DIRECT (GRAPHICS_Cyan)    { setColour (Melder_CYAN);    END_NO_NEW_DATA }
DIRECT (GRAPHICS_Magenta) { setColour (Melder_MAGENTA); END_NO_NEW_DATA }
DIRECT (GRAPHICS_Maroon)  { setColour (Melder_MAROON);  END_NO_NEW_DATA }
DIRECT (GRAPHICS_Lime)    { setColour (Melder_LIME);    END_NO_NEW_DATA }
DIRECT (GRAPHICS_Navy)    { setColour (Melder_NAVY);    END_NO_NEW_DATA }
DIRECT (GRAPHICS_Teal)    { setColour (Melder_TEAL);    END_NO_NEW_DATA }
DIRECT (GRAPHICS_Purple)  { setColour (Melder_PURPLE);  END_NO_NEW_DATA }
DIRECT (GRAPHICS_Olive)   { setColour (Melder_OLIVE);   END_NO_NEW_DATA }
DIRECT (GRAPHICS_Pink)    { setColour (Melder_PINK);    END_NO_NEW_DATA }
DIRECT (GRAPHICS_Silver)  { setColour (Melder_SILVER);  END_NO_NEW_DATA }
DIRECT (GRAPHICS_Grey)    { setColour (Melder_GREY);    END_NO_NEW_DATA }

FORM (GRAPHICS_Colour, I18n_translate("form.praat_picture_colour"), nullptr) {
	COLOUR (colour, I18n_translate("form.colour_0_1"), U"0.0")
OK
DO
	GRAPHICS_NONE
		Graphics_setColour (GRAPHICS, theCurrentPraatPicture -> colour = colour);
	GRAPHICS_NONE_END
	if (theCurrentPraatPicture == & theForegroundPraatPicture)
		updatePenMenu ();
}

/***** "File" MENU *****/

FORM_READ (GRAPHICS_Picture_readFromPraatPictureFile, I18n_translate("form.read_picture_from_praat_picture_file"), nullptr, false) {
	Picture_readFromPraatPictureFile (praat_picture.get(), file);
	END_NO_NEW_DATA
}

FORM_SAVE (GRAPHICS_Picture_writeToEpsFile, I18n_translate("form.save_picture_as_eps_file"), nullptr, U"praat.eps") {
	Picture_writeToEpsFile (praat_picture.get(), file, true, false);
	END_NO_NEW_DATA
}

FORM_SAVE (GRAPHICS_Picture_writeToFontlessEpsFile_xipa, I18n_translate("form.save_as_fontless_eps_file"), nullptr, U"praat.eps") {
	Picture_writeToEpsFile (praat_picture.get(), file, false, false);
	END_NO_NEW_DATA
}

FORM_SAVE (GRAPHICS_Picture_writeToFontlessEpsFile_silipa, I18n_translate("form.save_as_fontless_eps_file"), nullptr, U"praat.eps") {
	Picture_writeToEpsFile (praat_picture.get(), file, false, true);
	END_NO_NEW_DATA
}

FORM_SAVE (GRAPHICS_Picture_writeToPdfFile, I18n_translate("form.save_as_pdf_file"), nullptr, U"praat.pdf") {
	if (theCurrentPraatPicture == & theForegroundPraatPicture) {
		Picture_writeToPdfFile (praat_picture.get(), file);
	} else {
		try {
			//autoPraatPictureOpen picture;
			autoGraphics graphics = Graphics_create_pdffile (file, 300, undefined, 10.24, undefined, 7.68);
			Graphics_play (GRAPHICS, graphics.get());
		} catch (MelderError) {
			Melder_throw (U"Picture not written to PDF file ", file, U".");
		}
	}
	END_NO_NEW_DATA
}

FORM_SAVE (GRAPHICS_Picture_writeToPngFile_300, I18n_translate("form.save_as_png_file"), nullptr, U"praat.png") {
	if (theCurrentPraatPicture == & theForegroundPraatPicture) {
		Picture_writeToPngFile_300 (praat_picture.get(), file);
	} else {
		try {
			autoGraphics graphics = Graphics_create_pngfile (file, 300, 0.0, 10.24, 0.0, 7.68);
			Graphics_play (GRAPHICS, graphics.get());
		} catch (MelderError) {
			Melder_throw (U"Picture not written to PNG file ", file, U".");
		}
	}
	END_NO_NEW_DATA
}

FORM_SAVE (GRAPHICS_Picture_writeToPngFile_600, I18n_translate("form.save_as_png_file"), nullptr, U"praat.png") {
	if (theCurrentPraatPicture == & theForegroundPraatPicture) {
		Picture_writeToPngFile_600 (praat_picture.get(), file);
	} else {
		try {
			autoGraphics graphics = Graphics_create_pngfile (file, 600, 0.0, 10.24, 0.0, 7.68);
			Graphics_play (GRAPHICS, graphics.get());
		} catch (MelderError) {
			Melder_throw (U"Picture not written to PNG file ", file, U".");
		}
	}
	END_NO_NEW_DATA
}


FORM_SAVE (GRAPHICS_Picture_writeToPraatPictureFile, I18n_translate("form.save_as_praat_picture_file"), nullptr, U"praat.prapic") {
	Picture_writeToPraatPictureFile (praat_picture.get(), file);
	END_NO_NEW_DATA
}

#ifdef macintosh
DIRECT (GRAPHICS_Page_setup) {
	Printer_pageSetup ();
	END_NO_NEW_DATA
}
#endif

FORM (GRAPHICS_PostScript_settings, I18n_translate("form.postscript_settings"), I18n_translate("form.postscript_settings_dialog")) {
	#if defined (_WIN32)
		BOOLEAN (allowDirectPostscript, U"Allow direct PostScript", true);
	#endif
	CHOICE_ENUM (kGraphicsPostscript_spots, greyResolution,
			U"Grey resolution", kGraphicsPostscript_spots::DEFAULT)
	#if defined (UNIX)
		CHOICE_ENUM (kGraphicsPostscript_paperSize, paperSize,
				U"Paper size", kGraphicsPostscript_paperSize::DEFAULT)
		CHOICE_ENUM (kGraphicsPostscript_orientation, orientation,
				U"Orientation", kGraphicsPostscript_orientation::DEFAULT)
		POSITIVE (magnification, I18n_translate("form.magnification_picture"), U"1.0");
		#if defined (linux)
			TEXTFIELD (printCommand, I18n_translate("form.print_command_picture"), U"lpr %s", 4)
		#else
			TEXTFIELD (printCommand, I18n_translate("form.print_command_picture"), U"lp -c %s", 4)
		#endif
	#endif
	CHOICE_ENUM (kGraphicsPostscript_fontChoiceStrategy, fontChoiceStrategy,
			U"Font choice strategy", kGraphicsPostscript_fontChoiceStrategy::DEFAULT)
OK
	#if defined (_WIN32)
		SET_BOOLEAN (allowDirectPostscript, thePrinter. allowDirectPostScript)
	#endif
	SET_ENUM (greyResolution, kGraphicsPostscript_spots, thePrinter. spots)
	#if defined (UNIX)
		SET_ENUM (paperSize, kGraphicsPostscript_paperSize, thePrinter. paperSize)
		SET_ENUM (orientation, kGraphicsPostscript_orientation, thePrinter. orientation)
		SET_REAL (magnification, thePrinter. magnification)
		SET_STRING (printCommand, Site_getPrintCommand ())
	#endif
	SET_ENUM (fontChoiceStrategy, kGraphicsPostscript_fontChoiceStrategy, thePrinter. fontChoiceStrategy)
DO
	INFO_NONE
		#if defined (_WIN32)
			thePrinter. allowDirectPostScript = allowDirectPostscript;
		#endif
		thePrinter. spots = greyResolution;
		#if defined (UNIX)
			thePrinter. paperSize = paperSize;
			if (thePrinter. paperSize == kGraphicsPostscript_paperSize::A3) {
				thePrinter. paperWidth = 842 * thePrinter. resolution / 72;
				thePrinter. paperHeight = 1191 * thePrinter. resolution / 72;
			} else if (thePrinter. paperSize == kGraphicsPostscript_paperSize::US_LETTER) {
				thePrinter. paperWidth = 612 * thePrinter. resolution / 72;
				thePrinter. paperHeight = 792 * thePrinter. resolution / 72;
			} else {
				thePrinter. paperWidth = 595 * thePrinter. resolution / 72;
				thePrinter. paperHeight = 842 * thePrinter. resolution / 72;
			}
			thePrinter. orientation = orientation;
			thePrinter. magnification = magnification;
			Site_setPrintCommand (printCommand);
		#endif
		thePrinter. fontChoiceStrategy = fontChoiceStrategy;
	INFO_NONE_END
}

DIRECT (GRAPHICS_Print) {
	Picture_print (praat_picture.get());
	END_NO_NEW_DATA
}

#ifdef _WIN32
FORM_SAVE (GRAPHICS_Picture_writeToWindowsMetafile, I18n_translate("form.save_as_windows_metafile"), nullptr, U"praat.emf") {
	Picture_writeToWindowsMetafile (praat_picture.get(), file);
	END_NO_NEW_DATA
}

#endif

#if defined (_WIN32) || defined (macintosh)
	DIRECT (GRAPHICS_Copy_picture_to_clipboard) {
		Picture_copyToClipboard (praat_picture.get());
	END_NO_NEW_DATA
}
#endif

/***** "Edit" MENU *****/

DIRECT (GRAPHICS_Undo) {
	Graphics_undoGroup (GRAPHICS);
	Graphics_updateWs (GRAPHICS);
	END_NO_NEW_DATA
}

DIRECT (GRAPHICS_Erase_all) {
	//TRACE
	if (theCurrentPraatPicture == & theForegroundPraatPicture) {
		trace(1);
		Picture_erase (praat_picture.get());   // this kills the recording
	} else {
		trace(2);
		Graphics_clearRecording (GRAPHICS);
		if (Demo_hasGraphics (GRAPHICS)) {
			trace (3);
			Graphics_clearWs (GRAPHICS);
		}
	}
	END_NO_NEW_DATA
}

/***** "World" MENU *****/

FORM (GRAPHICS_Text, I18n_translate("form.praat_picture_text"), I18n_translate("form.text_dialog")) {
	REAL (horizontalPosition, I18n_translate("form.horizontal_position"), U"0.0")
	OPTIONMENU_ENUM (kGraphics_horizontalAlignment, horizontalAlignment,
			U"Horizontal alignment", kGraphics_horizontalAlignment::LEFT)
	REAL (verticalPosition, I18n_translate("form.vertical_position"), U"0.0")
	OPTIONMENUx (verticalAlignment, U"Vertical alignment", 2, 0)
		OPTION (U"bottom")
		OPTION (U"half")
		OPTION (U"top")
	TEXTFIELD (text, I18n_translate("form.text"), U"", 4)
	OK
DO
	GRAPHICS_NONE
		Graphics_setTextAlignment (GRAPHICS, horizontalAlignment, verticalAlignment);
		Graphics_setInner (GRAPHICS);
		Graphics_text (GRAPHICS, horizontalPosition, verticalPosition, text);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_TextSpecial, I18n_translate("form.praat_picture_text_special"), nullptr) {
	REAL (horizontalPosition, I18n_translate("form.horizontal_position"), U"0.0")
	OPTIONMENU_ENUM (kGraphics_horizontalAlignment, horizontalAlignment,
			U"Horizontal alignment", kGraphics_horizontalAlignment::LEFT)
	REAL (verticalPosition, I18n_translate("form.vertical_position"), U"0.0")
	OPTIONMENUx (verticalAlignment, U"Vertical alignment", 2, 0)
		OPTION (U"bottom")
		OPTION (U"half")
		OPTION (U"top")
	OPTIONMENU_ENUM (kGraphics_font, font, U"Font", kGraphics_font::DEFAULT)
	POSITIVE (fontSize, I18n_translate("form.font_size_picture"), U"10")
	SENTENCE (rotation, U"Rotation (degrees or dx;dy)", U"0")
	TEXTFIELD (text, I18n_translate("form.text"), U"", 4)
OK
DO
	kGraphics_font currentFont = Graphics_inqFont (GRAPHICS);
	const double currentSize = Graphics_inqFontSize (GRAPHICS);
	GRAPHICS_NONE
		Graphics_setTextAlignment (GRAPHICS, horizontalAlignment, verticalAlignment);
		Graphics_setInner (GRAPHICS);
		Graphics_setFont (GRAPHICS, (kGraphics_font) font);
		Graphics_setFontSize (GRAPHICS, fontSize);
		const char32 *semicolon = str32chr (rotation, ';');
		if (semicolon) {
			conststring32 dx = rotation, dy = semicolon + 1;
			Graphics_setTextRotation_vector (GRAPHICS, Melder_atof (dx), Melder_atof (dy));
		} else {
			Graphics_setTextRotation (GRAPHICS, Melder_atof (rotation));
		}
		Graphics_text (GRAPHICS, horizontalPosition, verticalPosition, text);
		Graphics_setFont (GRAPHICS, currentFont);
		Graphics_setFontSize (GRAPHICS, currentSize);
		Graphics_setTextRotation (GRAPHICS, 0.0);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_RectangleText_MaximalFit, I18n_translate("form.praat_picture_rectangle_text_maximal_fit"), nullptr) {
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	OPTIONMENU_ENUM (kGraphics_horizontalAlignment, horizontalAlignment,
			U"Horizontal alignment", kGraphics_horizontalAlignment::LEFT)
	REAL (minimumHorizontalMargin_in_textHeights, U"Minimal horizontal margin (in text heights)", U"0.10")
	REAL (minimumHorizontalMargin_mm, U"Minimal horizontal margin (mm)", U"0.5")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OPTIONMENUx (verticalAlignment, U"Vertical alignment", 2, 0)
		OPTION (U"bottom")
		OPTION (U"half")
		OPTION (U"top")
	REAL (minimumVerticalMargin_in_textHeights, U"Minimal vertical margin (in text heights)", U"0.18")
	REAL (minimumVerticalMargin_mm, U"Minimal vertical margin (mm)", U"0.25")
	TEXTFIELD (text, I18n_translate("form.text"), U"", 4)
	OK
DO
	GRAPHICS_NONE
		Graphics_setTextAlignment (GRAPHICS, horizontalAlignment, verticalAlignment);
		Graphics_setInner (GRAPHICS);
		Graphics_rectangleText_maximalFit (GRAPHICS, fromX, toX, minimumHorizontalMargin_in_textHeights, minimumHorizontalMargin_mm,
				fromY, toY, minimumVerticalMargin_in_textHeights, minimumVerticalMargin_mm, text);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_RectangleText_WrapAndTruncate, I18n_translate("form.praat_picture_rectangle_text_wrap_and_truncate"), nullptr) {
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	OPTIONMENU_ENUM (kGraphics_horizontalAlignment, horizontalAlignment,
			U"Horizontal alignment", kGraphics_horizontalAlignment::LEFT)
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OPTIONMENUx (verticalAlignment, U"Vertical alignment", 2, 0)
		OPTION (U"bottom")
		OPTION (U"half")
		OPTION (U"top")
	TEXTFIELD (text, I18n_translate("form.text"), U"", 4)
	OK
DO
	GRAPHICS_NONE
		Graphics_setTextAlignment (GRAPHICS, horizontalAlignment, verticalAlignment);
		Graphics_setInner (GRAPHICS);
		Graphics_rectangleText_wrapAndTruncate (GRAPHICS, fromX, toX, fromY, toY, text);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawLine, I18n_translate("form.praat_picture_draw_line"), nullptr) {
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_line (GRAPHICS, fromX, fromY, toX, toY);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawArrow, I18n_translate("form.praat_picture_draw_arrow"), nullptr) {
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_arrow (GRAPHICS, fromX, fromY, toX, toY);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawDoubleArrow, I18n_translate("form.praat_picture_draw_double_arrow"), nullptr) {
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_doubleArrow (GRAPHICS, fromX, fromY, toX, toY);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

Thing_define (PraatPictureFunction, Daata) {
	// new data:
	public:
		double xmin, xmax, dx, x1;
		integer nx;
	// overridden methods:
		virtual bool v_hasGetXmin () const { return true; }   virtual double v_getXmin ()           const { return xmin; }
		virtual bool v_hasGetXmax () const { return true; }   virtual double v_getXmax ()           const { return xmax; }
		virtual bool v_hasGetNx   () const { return true; }   virtual double v_getNx   ()           const { return nx; }
		virtual bool v_hasGetDx   () const { return true; }   virtual double v_getDx   ()           const { return dx; }
		virtual bool v_hasGetX    () const { return true; }   virtual double v_getX    (integer ix) const { return x1 + (ix - 1) * dx; }
};
Thing_implement (PraatPictureFunction, Daata, 0);

FORM (GRAPHICS_DrawFunction, I18n_translate("form.praat_picture_draw_function"), nullptr) {
	COMMENT (U"This command assumes that the x and y axes")
	COMMENT (U"have been set by a Draw command or by \"Axes...\".")
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, U"To x", U"0.0 (= all)")
	NATURAL (numberOfHorizontalSteps, U"Number of horizontal steps", U"1000")
	FORMULA (formula, U"Formula", U"x^2 - x^4")
	OK
DO
	if (numberOfHorizontalSteps < 2)
		return;
	double x1WC, x2WC, y1WC, y2WC;
	Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	if (fromX == toX) {
		fromX = x1WC;
		toX = x2WC;
	}
	autoVEC y = raw_VEC (numberOfHorizontalSteps);
	autoPraatPictureFunction function = Thing_new (PraatPictureFunction);
	function -> xmin = x1WC;
	function -> xmax = x2WC;
	function -> nx = numberOfHorizontalSteps;
	function -> x1 = fromX;
	function -> dx = (toX - fromX) / (numberOfHorizontalSteps - 1);
	Formula_compile (interpreter, function.get(), formula, kFormula_EXPRESSION_TYPE_NUMERIC, true);
	Formula_Result result;
	for (integer i = 1; i <= numberOfHorizontalSteps; i ++) {
		Formula_run (1, i, & result);
		y [i] = result. numericResult;
	}
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_function (GRAPHICS, & y [0], 1, numberOfHorizontalSteps, fromX, toX);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawRectangle, I18n_translate("form.praat_picture_draw_rectangle"), nullptr) {
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_rectangle (GRAPHICS, fromX, toX, fromY, toY);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_PaintRectangle, I18n_translate("form.praat_picture_paint_rectangle"), nullptr) {
	COLOUR (colour, I18n_translate("form.colour_0_1"), U"0.5")
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_setColour (GRAPHICS, colour);
		Graphics_fillRectangle (GRAPHICS, fromX, toX, fromY, toY);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawRoundedRectangle, I18n_translate("form.praat_picture_draw_rounded_rectangle"), nullptr) {
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	POSITIVE (radius, U"Radius (mm)", U"3.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_roundedRectangle (GRAPHICS, fromX, toX, fromY, toY, radius);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_PaintRoundedRectangle, I18n_translate("form.praat_picture_paint_rounded_rectangle"), nullptr) {
	COLOUR (colour, I18n_translate("form.colour_0_1"), U"0.5")
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	POSITIVE (radius, U"Radius (mm)", U"3.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_setColour (GRAPHICS, colour);
		Graphics_fillRoundedRectangle (GRAPHICS, fromX, toX, fromY, toY, radius);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawArc, I18n_translate("form.praat_picture_draw_arc"), nullptr) {
	REAL (centreX, U"Centre x", U"0.0")
	REAL (centreY, U"Centre y", U"0.0")
	POSITIVE (radius, U"Radius (along x)", U"1.0")
	REAL (fromAngle, U"From angle (degrees)", U"0.0")
	REAL (toAngle, U"To angle (degrees)", U"90.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_arc (GRAPHICS, centreX, centreY, radius, fromAngle, toAngle);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawEllipse, I18n_translate("form.praat_picture_draw_ellipse"), nullptr) {
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_ellipse (GRAPHICS, fromX, toX, fromY, toY);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_PaintEllipse, I18n_translate("form.praat_picture_paint_ellipse"), nullptr) {
	COLOUR (colour, I18n_translate("form.colour_0_1"), U"0.5")
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_setColour (GRAPHICS, colour);
		Graphics_fillEllipse (GRAPHICS, fromX, toX, fromY, toY);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawCircle, I18n_translate("form.praat_picture_draw_circle"), nullptr) {
	REAL (centreX, U"Centre x", U"0.0")
	REAL (centreY, U"Centre y", U"0.0")
	POSITIVE (radius, U"Radius (along x)", U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_circle (GRAPHICS, centreX, centreY, radius);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_PaintCircle, I18n_translate("form.praat_picture_paint_circle"), nullptr) {
	COLOUR (colour, I18n_translate("form.colour_0_1"), U"0.5")
	REAL (centreX, U"Centre x", U"0.0")
	REAL (centreY, U"Centre y", U"0.0")
	POSITIVE (radius, U"Radius (along x)", U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_setColour (GRAPHICS, colour);
		Graphics_fillCircle (GRAPHICS, centreX, centreY, radius);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_DrawCircle_mm, I18n_translate("form.praat_picture_draw_circle_mm"), nullptr) {
	REAL (centreX, U"Centre x", U"0.0")
	REAL (centreY, U"Centre y", U"0.0")
	POSITIVE (diameter, U"Diameter (mm)", U"5.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_circle_mm (GRAPHICS, centreX, centreY, diameter);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_PaintCircle_mm, I18n_translate("form.praat_picture_paint_circle_mm"), nullptr) {
	COLOUR (colour, I18n_translate("form.colour_0_1"), U"0.5")
	REAL (centreX, U"Centre x", U"0.0")
	REAL (centreY, U"Centre y", U"0.0")
	POSITIVE (diameter, U"Diameter (mm)", U"5.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_setColour (GRAPHICS, colour);
		Graphics_fillCircle_mm (GRAPHICS, centreX, centreY, diameter);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_InsertPictureFromFile, I18n_translate("form.insert_picture_from_file"), I18n_translate("form.insert_picture_from_file_dialog")) {
	INFILE (fileName, U"File name", U"~/Desktop/paul.jpg")
	REAL (fromX, I18n_translate("form.from_x"), U"0.0")
	REAL (toX, I18n_translate("form.to_x"), U"1.0")
	REAL (fromY, I18n_translate("form.from_y"), U"0.0")
	REAL (toY, I18n_translate("form.to_y"), U"1.0")
	OK
DO
	GRAPHICS_NONE
		Graphics_setInner (GRAPHICS);
		Graphics_imageFromFile_embedded (GRAPHICS, fileName, fromX, toX, fromY, toY);
		Graphics_unsetInner (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_Axes, I18n_translate("form.axes"), I18n_translate("form.axes_dialog")) {
	REAL (left, U"left Left and right", U"0.0")
	REAL (right, U"right Left and right", U"1.0")
	REAL (bottom, U"left Bottom and top", U"0.0")
	REAL (top, U"right Bottom and top", U"1.0")
OK
	double x1WC, x2WC, y1WC, y2WC;
	Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	SET_REAL (left, x1WC)
	SET_REAL (right, x2WC)
	SET_REAL (bottom, y1WC)
	SET_REAL (top, y2WC)
DO
	if (left == right)
		Melder_throw (U"Left and right should not be equal.");
	if (top == bottom)
		Melder_throw (U"Top and bottom should not be equal.");
	GRAPHICS_NONE
		Graphics_setWindow (GRAPHICS, left, right, bottom, top);
	GRAPHICS_NONE_END
}

// MARK: Margins

DIRECT (GRAPHICS_DrawInnerBox) {
	GRAPHICS_NONE
		Graphics_drawInnerBox (GRAPHICS);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_TextLeft, I18n_translate("form.text_left"), I18n_translate("form.text_left_right_top_bottom_dialog")) {
	BOOLEAN (farr, U"Far", true)
	TEXTFIELD (text, I18n_translate("form.text"), U"", 3)
	OK
DO
	GRAPHICS_NONE
		Graphics_textLeft (GRAPHICS, farr, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_TextRight, I18n_translate("form.text_right"), I18n_translate("form.text_left_right_top_bottom_dialog")) {
	BOOLEAN (farr, U"Far", true)
	TEXTFIELD (text, I18n_translate("form.text"), U"", 3)
	OK
DO
	GRAPHICS_NONE
		Graphics_textRight (GRAPHICS, farr, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_TextTop, I18n_translate("form.text_top"), I18n_translate("form.text_left_right_top_bottom_dialog")) {
	BOOLEAN (farr, U"Far", true)
	TEXTFIELD (text, I18n_translate("form.text"), U"", 3)
	OK
DO
	GRAPHICS_NONE
		Graphics_textTop (GRAPHICS, farr, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_TextBottom, I18n_translate("form.text_bottom"), I18n_translate("form.text_left_right_top_bottom_dialog")) {
	BOOLEAN (farr, U"Far", true)
	TEXTFIELD (text, I18n_translate("form.text"), U"", 3)
	OK
DO
	GRAPHICS_NONE
		Graphics_textBottom (GRAPHICS, farr, text);
	GRAPHICS_NONE_END
}

#define FIELDS_MARKS_EVERY  \
	POSITIVE (units, U"Units", U"1.0") \
	POSITIVE (distance, U"Distance", U"0.1") \
	BOOLEAN (writeNumbers, U"Write numbers", true) \
	BOOLEAN (drawTicks, U"Draw ticks", true) \
	BOOLEAN (drawDottedLines, U"Draw dotted lines", true)

FORM (GRAPHICS_MarksLeftEvery, I18n_translate("form.marks_left_every"), I18n_translate("form.marks_left_right_top_bottom_every_dialog")) {
	FIELDS_MARKS_EVERY
	OK
DO
	GRAPHICS_NONE
		Graphics_marksLeftEvery (GRAPHICS, units, distance, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_MarksRightEvery, I18n_translate("form.marks_right_every"), I18n_translate("form.marks_left_right_top_bottom_every_dialog")) {
	FIELDS_MARKS_EVERY
	OK
DO
	GRAPHICS_NONE
		Graphics_marksRightEvery (GRAPHICS, units, distance, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_MarksBottomEvery, I18n_translate("form.marks_bottom_every"), I18n_translate("form.marks_left_right_top_bottom_every_dialog")) {
	FIELDS_MARKS_EVERY
	OK
DO
	GRAPHICS_NONE
		Graphics_marksBottomEvery (GRAPHICS, units, distance, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_MarksTopEvery, I18n_translate("form.marks_top_every"), I18n_translate("form.marks_left_right_top_bottom_every_dialog")) {
	FIELDS_MARKS_EVERY
	OK
DO
	GRAPHICS_NONE
		Graphics_marksTopEvery (GRAPHICS, units, distance, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

#define FIELDS_MARKS  \
	NATURAL (numberOfMarks, U"Number of marks", U"6") \
	BOOLEAN (writeNumbers, U"Write numbers", true) \
	BOOLEAN (drawTicks, U"Draw ticks", true) \
	BOOLEAN (drawDottedLines, U"Draw dotted lines", true)

FORM (GRAPHICS_MarksLeft, I18n_translate("form.marks_left"), I18n_translate("form.marks_left_right_top_bottom_dialog")) {
	FIELDS_MARKS
	OK
DO
	GRAPHICS_NONE
		if (numberOfMarks < 2)
			Melder_throw (U"The number of marks should be at least 2.");
		Graphics_marksLeft (GRAPHICS, numberOfMarks, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_MarksRight, I18n_translate("form.marks_right"), I18n_translate("form.marks_left_right_top_bottom_dialog")) {
	FIELDS_MARKS
	OK
DO
	GRAPHICS_NONE
		if (numberOfMarks < 2)
			Melder_throw (U"The number of marks should be at least 2.");
		Graphics_marksRight (GRAPHICS, numberOfMarks, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_MarksBottom, I18n_translate("form.marks_bottom"), I18n_translate("form.marks_left_right_top_bottom_dialog")) {
	FIELDS_MARKS
	OK
DO
	GRAPHICS_NONE
		if (numberOfMarks < 2)
			Melder_throw (U"The number of marks should be at least 2.");
		Graphics_marksBottom (GRAPHICS, numberOfMarks, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_MarksTop, I18n_translate("form.marks_top"), I18n_translate("form.marks_left_right_top_bottom_dialog")) {
	FIELDS_MARKS
	OK
DO
	GRAPHICS_NONE
		if (numberOfMarks < 2)
			Melder_throw (U"The number of marks should be at least 2.");
		Graphics_marksTop (GRAPHICS, numberOfMarks, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

#define FIELDS_MARKS_LOGARITHMIC  \
	NATURAL (marksPerDecade, U"Marks per decade", U"3") \
	BOOLEAN (writeNumbers, U"Write numbers", true) \
	BOOLEAN (drawTicks, U"Draw ticks", true) \
	BOOLEAN (drawDottedLines, U"Draw dotted lines", true)

FORM (GRAPHICS_LogarithmicMarksLeft, I18n_translate("form.logarithmic_marks_left"), I18n_translate("form.logarithmic_marks_left_right_top_bottom_dialog")) {
	FIELDS_MARKS_LOGARITHMIC
	OK
DO
	GRAPHICS_NONE
		Graphics_marksLeftLogarithmic (GRAPHICS, marksPerDecade, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_LogarithmicMarksRight, I18n_translate("form.logarithmic_marks_right"), I18n_translate("form.logarithmic_marks_left_right_top_bottom_dialog")) {
	FIELDS_MARKS_LOGARITHMIC
	OK
DO
	GRAPHICS_NONE
		Graphics_marksRightLogarithmic (GRAPHICS, marksPerDecade, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_LogarithmicMarksBottom, I18n_translate("form.logarithmic_marks_bottom"), I18n_translate("form.logarithmic_marks_left_right_top_bottom_dialog")) {
	FIELDS_MARKS_LOGARITHMIC
	OK
DO
	GRAPHICS_NONE
		Graphics_marksBottomLogarithmic (GRAPHICS, marksPerDecade, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_LogarithmicMarksTop, I18n_translate("form.logarithmic_marks_top"), I18n_translate("form.logarithmic_marks_left_right_top_bottom_dialog")) {
	FIELDS_MARKS_LOGARITHMIC
	OK
DO
	GRAPHICS_NONE
		Graphics_marksTopLogarithmic (GRAPHICS, marksPerDecade, writeNumbers, drawTicks, drawDottedLines);
	GRAPHICS_NONE_END
}

static void sortBoundingBox (double *x1WC, double *x2WC, double *y1WC, double *y2WC) {
	Melder_sort (x1WC, x2WC);
	Melder_sort (y1WC, y2WC);
}

FORM (GRAPHICS_OneMarkLeft, I18n_translate("form.one_mark_left"), I18n_translate("form.one_mark_left_right_top_bottom_dialog")) {
	REAL (position, U"Position", U"0.0")
	BOOLEAN (writeNumber, U"Write number", true)
	BOOLEAN (drawTick, U"Draw tick", true)
	BOOLEAN (drawDottedLine, U"Draw dotted line", true)
	TEXTFIELD (text, U"Draw text", U"", 3)
	OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	{// scope
		autoPraatPictureOpen picture;
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	}
	sortBoundingBox (& x1WC, & x2WC, & y1WC, & y2WC);
	const double dy = 0.2 * (y2WC - y1WC);
	if (position < y1WC - dy || position > y2WC + dy)
		Melder_throw (U"\"Position\" must be between ", y1WC, U" and ", y2WC, U".");
	GRAPHICS_NONE
		Graphics_markLeft (GRAPHICS, position, writeNumber, drawTick, drawDottedLine, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_OneMarkRight, I18n_translate("form.one_mark_right"), I18n_translate("form.one_mark_left_right_top_bottom_dialog")) {
	REAL (position, U"Position", U"0.0")
	BOOLEAN (writeNumber, U"Write number", true)
	BOOLEAN (drawTick, U"Draw tick", true)
	BOOLEAN (drawDottedLine, U"Draw dotted line", true)
	TEXTFIELD (text, U"Draw text", U"", 3)
	OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	{// scope
		autoPraatPictureOpen picture;
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	}
	sortBoundingBox (& x1WC, & x2WC, & y1WC, & y2WC);
	const double dy = 0.2 * (y2WC - y1WC);
	if (position < y1WC - dy || position > y2WC + dy)
		Melder_throw (U"\"Position\" must be between ", y1WC, U" and ", y2WC, U".");
	GRAPHICS_NONE
		Graphics_markRight (GRAPHICS, position, writeNumber, drawTick, drawDottedLine, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_OneMarkTop, I18n_translate("form.one_mark_top"), I18n_translate("form.one_mark_left_right_top_bottom_dialog")) {
	REAL (position, U"Position", U"0.0")
	BOOLEAN (writeNumber, U"Write number", true)
	BOOLEAN (drawTick, U"Draw tick", true)
	BOOLEAN (drawDottedLine, U"Draw dotted line", true)
	TEXTFIELD (text, U"Draw text", U"", 3)
	OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	{// scope
		autoPraatPictureOpen picture;   // WHY?
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	}
	sortBoundingBox (& x1WC, & x2WC, & y1WC, & y2WC);
	const double dx = 0.2 * (x2WC - x1WC);
	if (position < x1WC - dx || position > x2WC + dx)
		Melder_throw (U"\"Position\" must be between ", x1WC, U" and ", x2WC, U".");
	GRAPHICS_NONE
		Graphics_markTop (GRAPHICS, position, writeNumber, drawTick, drawDottedLine, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_OneMarkBottom, I18n_translate("form.one_mark_bottom"), I18n_translate("form.one_mark_left_right_top_bottom_dialog")) {
	REAL (position, U"Position", U"0.0")
	BOOLEAN (writeNumber, U"Write number", true)
	BOOLEAN (drawTick, U"Draw tick", true)
	BOOLEAN (drawDottedLine, U"Draw dotted line", true)
	TEXTFIELD (text, U"Draw text", U"", 3)
	OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	{// scope
		autoPraatPictureOpen picture;
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	}
	sortBoundingBox (& x1WC, & x2WC, & y1WC, & y2WC);
	const double dx = 0.2 * (x2WC - x1WC);
	if (position < x1WC - dx || position > x2WC + dx)
		Melder_throw (U"\"Position\" must be between ", x1WC, U" and ", x2WC, U".");
	GRAPHICS_NONE
		Graphics_markBottom (GRAPHICS, position, writeNumber, drawTick, drawDottedLine, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_OneLogarithmicMarkLeft, I18n_translate("form.one_logarithmic_mark_left"), I18n_translate("form.one_logarithmic_mark_left_right_top_bottom_dialog")) {
	REAL (position, U"Position", U"1.0")
	BOOLEAN (writeNumber, U"Write number", 1)
	BOOLEAN (drawTick, U"Draw tick", 1)
	BOOLEAN (drawDottedLine, U"Draw dotted line", 1)
	TEXTFIELD (text, U"Draw text", U"", 3)
	OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	{// scope
		autoPraatPictureOpen picture;
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	}
	sortBoundingBox (& x1WC, & x2WC, & y1WC, & y2WC);
	const double dy = 0.2 * (y2WC - y1WC);
	if (position < pow (10, y1WC - dy) || position > pow (10, y2WC + dy))
		Melder_throw (U"\"Position\" must be between ", pow (10, y1WC), U" and ", pow (10, y2WC), U".");
	GRAPHICS_NONE
		Graphics_markLeftLogarithmic (GRAPHICS, position, writeNumber, drawTick, drawDottedLine, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_OneLogarithmicMarkRight, I18n_translate("form.one_logarithmic_mark_right"), I18n_translate("form.one_logarithmic_mark_left_right_top_bottom_dialog")) {
	REAL (position, U"Position", U"1.0")
	BOOLEAN (writeNumber, U"Write number", 1)
	BOOLEAN (drawTick, U"Draw tick", 1)
	BOOLEAN (drawDottedLine, U"Draw dotted line", 1)
	TEXTFIELD (text, U"Draw text", U"", 3)
	OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	{// scope
		autoPraatPictureOpen picture;
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	}
	sortBoundingBox (& x1WC, & x2WC, & y1WC, & y2WC);
	const double dy = 0.2 * (y2WC - y1WC);
	if (position < pow (10, y1WC - dy) || position > pow (10, y2WC + dy))
		Melder_throw (U"\"Position\" must be between ", pow (10, y1WC), U" and ", pow (10, y2WC), U".");
	GRAPHICS_NONE
		Graphics_markRightLogarithmic (GRAPHICS, position, writeNumber, drawTick, drawDottedLine, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_OneLogarithmicMarkTop, I18n_translate("form.one_logarithmic_mark_top"), I18n_translate("form.one_logarithmic_mark_left_right_top_bottom_dialog")) {
	REAL (position, U"Position", U"1.0")
	BOOLEAN (writeNumber, U"Write number", 1)
	BOOLEAN (drawTick, U"Draw tick", 1)
	BOOLEAN (drawDottedLine, U"Draw dotted line", 1)
	TEXTFIELD (text, U"Draw text", U"", 3)
	OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	{// scope
		autoPraatPictureOpen picture;
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	}
	sortBoundingBox (& x1WC, & x2WC, & y1WC, & y2WC);
	const double dx = 0.2 * (x2WC - x1WC);
	if (position < pow (10, x1WC - dx) || position > pow (10, x2WC + dx))
		Melder_throw (U"\"Position\" must be between ", pow (10, x1WC), U" and ", pow (10, x2WC), U".");
	GRAPHICS_NONE
		Graphics_markTopLogarithmic (GRAPHICS, position, writeNumber, drawTick, drawDottedLine, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_OneLogarithmicMarkBottom, I18n_translate("form.one_logarithmic_mark_bottom"), I18n_translate("form.one_logarithmic_mark_left_right_top_bottom_dialog")) {
	REAL (position, U"Position", U"1.0")
	BOOLEAN (writeNumber, U"Write number", 1)
	BOOLEAN (drawTick, U"Draw tick", 1)
	BOOLEAN (drawDottedLine, U"Draw dotted line", 1)
	TEXTFIELD (text, U"Draw text", U"", 3)
	OK
DO
	double x1WC, x2WC, y1WC, y2WC;
	{// scope
		autoPraatPictureOpen picture;
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	}
	sortBoundingBox (& x1WC, & x2WC, & y1WC, & y2WC);
	const double dx = 0.2 * (x2WC - x1WC);
	if (position < pow (10, x1WC - dx) || position > pow (10, x2WC + dx))
		Melder_throw (U"\"Position\" must be between ", pow (10, x1WC), U" and ", pow (10, x2WC), U".");
	GRAPHICS_NONE
		Graphics_markBottomLogarithmic (GRAPHICS, position, writeNumber, drawTick, drawDottedLine, text);
	GRAPHICS_NONE_END
}

FORM (GRAPHICS_HorizontalMmToWorldCoordinates, I18n_translate("form.compute_horizontal_distance_in_world_coordinates"), nullptr) {
	REAL (distance, U"Distance (mm)", U"10.0")
	OK
DO
	QUERY_GRAPHICS_FOR_REAL
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize);
		Graphics_setViewport (GRAPHICS, theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC,
				theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_setInner (GRAPHICS);
		const double result = Graphics_dxMMtoWC (GRAPHICS, distance);
		Graphics_unsetInner (GRAPHICS);
	QUERY_GRAPHICS_FOR_REAL_END (U" (world coordinates)");
}

FORM (GRAPHICS_HorizontalWorldCoordinatesToMm, I18n_translate("form.compute_horizontal_distance_in_millimetres"), nullptr) {
	REAL (distance, U"Distance (wc)", U"0.1")
	OK
DO
	QUERY_GRAPHICS_FOR_REAL   // TODO: do we need autoPraatPictureOpen for any of these?
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize);
		Graphics_setViewport (GRAPHICS, theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC,
				theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_setInner (GRAPHICS);
		const double result = Graphics_dxWCtoMM (GRAPHICS, distance);
		Graphics_unsetInner (GRAPHICS);
	QUERY_GRAPHICS_FOR_REAL_END (U" mm")
}

FORM (GRAPHICS_VerticalMmToWorldCoordinates, I18n_translate("form.compute_vertical_distance_world_coordinates"), nullptr) {
	REAL (distance, U"Distance (mm)", U"10.0")
	OK
DO
	QUERY_GRAPHICS_FOR_REAL
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize);
		Graphics_setViewport (GRAPHICS, theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC, theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_setInner (GRAPHICS);
		const double result = Graphics_dyMMtoWC (GRAPHICS, distance);
		Graphics_unsetInner (GRAPHICS);
	QUERY_GRAPHICS_FOR_REAL_END (U" (world coordinates)")
}

FORM (GRAPHICS_VerticalWorldCoordinatesToMm, I18n_translate("form.compute_vertical_distance_millimetres"), nullptr) {
	REAL (distance, U"Distance (wc)", U"1.0")
	OK
DO
	QUERY_GRAPHICS_FOR_REAL
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize);
		Graphics_setViewport (GRAPHICS, theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC, theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_setInner (GRAPHICS);
		const double result = Graphics_dyWCtoMM (GRAPHICS, distance);
		Graphics_unsetInner (GRAPHICS);
	QUERY_GRAPHICS_FOR_REAL_END (U" mm")
}

FORM (GRAPHICS_TextWidth_worldCoordinates, I18n_translate("form.text_width_world_coordinates"), nullptr) {
	TEXTFIELD (text, U"Text", U"Hello world", 3)
	OK
DO
	QUERY_GRAPHICS_FOR_REAL
		Graphics_setInner (GRAPHICS);
		const double result = Graphics_textWidth (GRAPHICS, text);
		Graphics_unsetInner (GRAPHICS);
	QUERY_GRAPHICS_FOR_REAL_END (U" (world coordinates)")
}

FORM (GRAPHICS_TextWidth_mm, I18n_translate("form.text_width_millimetres"), nullptr) {
	TEXTFIELD (text, U"Text", U"Hello world", 3)
	OK
DO
	QUERY_GRAPHICS_FOR_REAL
		Graphics_setFont (GRAPHICS, theCurrentPraatPicture -> font);
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize);
		Graphics_setViewport (GRAPHICS, theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC, theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_setInner (GRAPHICS);
		const double result = Graphics_dxWCtoMM (GRAPHICS, Graphics_textWidth (GRAPHICS, text));
		Graphics_unsetInner (GRAPHICS);
	QUERY_GRAPHICS_FOR_REAL_END (U" mm")
}

FORM (GRAPHICS_PostScriptTextWidth_worldCoordinates, I18n_translate("form.postscript_text_width_world_coordinates"), nullptr) {
	CHOICEx (phoneticFont, U"Phonetic font", 1, 0)
		OPTION (U"XIPA")
		OPTION (U"SILIPA")
	TEXTFIELD (text, U"Text", U"Hello world", 3)
	OK
DO
	QUERY_GRAPHICS_FOR_REAL
		Graphics_setFont (GRAPHICS, theCurrentPraatPicture -> font);
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize);
		Graphics_setViewport (GRAPHICS, theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC, theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_setInner (GRAPHICS);
		const double result = Graphics_textWidth_ps (GRAPHICS, text, phoneticFont);
		Graphics_unsetInner (GRAPHICS);
	QUERY_GRAPHICS_FOR_REAL_END (U" (world coordinates)")
}


FORM (GRAPHICS_PostScriptTextWidth_mm, I18n_translate("form.postscript_text_width_millimetres"), nullptr) {
	CHOICEx (phoneticFont, U"Phonetic font", 1, 0)
		OPTION (U"XIPA")
		OPTION (U"SILIPA")
	TEXTFIELD (text, U"Text", U"Hello world", 3)
	OK
DO
	QUERY_GRAPHICS_FOR_REAL
		Graphics_setFont (GRAPHICS, theCurrentPraatPicture -> font);
		Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize);
		Graphics_setViewport (GRAPHICS, theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC, theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
		Graphics_setInner (GRAPHICS);
		const double result = Graphics_textWidth_ps_mm (GRAPHICS, text, phoneticFont);
		Graphics_unsetInner (GRAPHICS);
	QUERY_GRAPHICS_FOR_REAL_END (U" mm")
}

DIRECT (HELP_PraatIntro_picture) { HELP (U"Intro") }
DIRECT (HELP_SearchManual_Picture) { Melder_search (); END_WITH_NEW_DATA }
DIRECT (HELP_PictureWindowHelp) { HELP (U"Picture window") }
DIRECT (HELP_AboutSpecialSymbols) { HELP (U"Special symbols") }
DIRECT (HELP_AboutTextStyles) { HELP (U"Text styles") }
DIRECT (HELP_PhoneticSymbols) { HELP (U"Phonetic symbols") }
DIRECT (GRAPHICS_Picture_settings_report) {
	INFO_NONE
		MelderInfo_open ();
		const conststring32 units = ( theCurrentPraatPicture == & theForegroundPraatPicture ? U" inches" : U"" );
		MelderInfo_writeLine (U"Outer viewport left: ", theCurrentPraatPicture -> x1NDC, units);
		MelderInfo_writeLine (U"Outer viewport right: ", theCurrentPraatPicture -> x2NDC, units);
		MelderInfo_writeLine (U"Outer viewport top: ",
			( theCurrentPraatPicture != & theForegroundPraatPicture ?
			  theCurrentPraatPicture -> y1NDC :
			  Picture_HEIGHT - theCurrentPraatPicture -> y2NDC) ,
			units
		);
		MelderInfo_writeLine (U"Outer viewport bottom: ",
			( theCurrentPraatPicture != & theForegroundPraatPicture ?
			  theCurrentPraatPicture -> y2NDC :
			  Picture_HEIGHT - theCurrentPraatPicture -> y1NDC ),
			units
		);
		MelderInfo_writeLine (U"Font size: ", theCurrentPraatPicture -> fontSize, U" points");
		double xmargin = theCurrentPraatPicture -> fontSize * 4.2 / 72.0;
		double ymargin = theCurrentPraatPicture -> fontSize * 2.8 / 72.0;
		if (theCurrentPraatPicture != & theForegroundPraatPicture) {
			integer x1DC, x2DC, y1DC, y2DC;
			Graphics_inqWsViewport (GRAPHICS, & x1DC, & x2DC, & y1DC, & y2DC);
			double x1wNDC, x2wNDC, y1wNDC, y2wNDC;
			Graphics_inqWsWindow (GRAPHICS, & x1wNDC, & x2wNDC, & y1wNDC, & y2wNDC);
			double wDC = (x2DC - x1DC) / (x2wNDC - x1wNDC);
			double hDC = integer_abs (y2DC - y1DC) / (y2wNDC - y1wNDC);
			xmargin *= Graphics_getResolution (GRAPHICS) / wDC;
			ymargin *= Graphics_getResolution (GRAPHICS) / hDC;
		}
		Melder_clipRight (& ymargin, 0.4 * (theCurrentPraatPicture -> y2NDC - theCurrentPraatPicture -> y1NDC));
		Melder_clipRight (& xmargin, 0.4 * (theCurrentPraatPicture -> x2NDC - theCurrentPraatPicture -> x1NDC));
		MelderInfo_writeLine (U"Inner viewport left: ", theCurrentPraatPicture -> x1NDC + xmargin, units);
		MelderInfo_writeLine (U"Inner viewport right: ", theCurrentPraatPicture -> x2NDC - xmargin, units);
		MelderInfo_writeLine (U"Inner viewport top: ",
			( theCurrentPraatPicture != & theForegroundPraatPicture ?
			  theCurrentPraatPicture -> y1NDC + ymargin :
			  Picture_HEIGHT - theCurrentPraatPicture -> y2NDC + ymargin ),
			units
		);
		MelderInfo_writeLine (U"Inner viewport bottom: ",
			( theCurrentPraatPicture != & theForegroundPraatPicture ?
			  theCurrentPraatPicture -> y2NDC - ymargin :
			  Picture_HEIGHT - theCurrentPraatPicture -> y1NDC - ymargin ),
			units
		);
		MelderInfo_writeLine (U"Font: ", kGraphics_font_getText (theCurrentPraatPicture -> font));
		MelderInfo_writeLine (U"Line type: ",
			theCurrentPraatPicture -> lineType == Graphics_DRAWN ? U"Solid" :
			theCurrentPraatPicture -> lineType == Graphics_DOTTED ? U"Dotted" :
			theCurrentPraatPicture -> lineType == Graphics_DASHED ? U"Dashed" :
			theCurrentPraatPicture -> lineType == Graphics_DASHED_DOTTED ? U"Dashed-dotted" :
			U"(unknown)"
		);
		MelderInfo_writeLine (U"Line width: ", theCurrentPraatPicture -> lineWidth);
		MelderInfo_writeLine (U"Arrow size: ", theCurrentPraatPicture -> arrowSize);
		MelderInfo_writeLine (U"Speckle size: ", theCurrentPraatPicture -> speckleSize);
		MelderInfo_writeLine (U"Colour: ", MelderColour_name (theCurrentPraatPicture -> colour));
		MelderInfo_writeLine (U"Red: ", theCurrentPraatPicture -> colour. red);
		MelderInfo_writeLine (U"Green: ", theCurrentPraatPicture -> colour. green);
		MelderInfo_writeLine (U"Blue: ", theCurrentPraatPicture -> colour. blue);
		double x1WC, x2WC, y1WC, y2WC;
		Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
		MelderInfo_writeLine (U"Axis left: ", x1WC);
		MelderInfo_writeLine (U"Axis right: ", x2WC);
		MelderInfo_writeLine (U"Axis bottom: ", y1WC);
		MelderInfo_writeLine (U"Axis top: ", y2WC);
		MelderInfo_close ();
	INFO_NONE_END
}

/**********   **********/

static void cb_selectionChanged (Picture p, void * /* closure */,
	double selx1, double selx2, double sely1, double sely2)
	/* The user selected a new viewport in the picture window. */
{
	Melder_assert (p == praat_picture.get());
	theCurrentPraatPicture -> x1NDC = selx1;
	theCurrentPraatPicture -> x2NDC = selx2;
	theCurrentPraatPicture -> y1NDC = sely1;
	theCurrentPraatPicture -> y2NDC = sely2;
	if (praat_mouseSelectsInnerViewport) {
		const double fontSize = Graphics_inqFontSize (GRAPHICS);
		const double xmargin = Melder_clippedRight (fontSize * 4.2 / 72.0, 0.4 * (theCurrentPraatPicture -> x2NDC - theCurrentPraatPicture -> x1NDC));
		const double ymargin = Melder_clippedRight (fontSize * 2.8 / 72.0, 0.4 * (theCurrentPraatPicture -> y2NDC - theCurrentPraatPicture -> y1NDC));
		UiHistory_write (U"\nSelect inner viewport: ");
		UiHistory_write (Melder_single (theCurrentPraatPicture -> x1NDC + xmargin));
		UiHistory_write (U", ");
		UiHistory_write (Melder_single (theCurrentPraatPicture -> x2NDC - xmargin));
		UiHistory_write (U", ");
		UiHistory_write (Melder_single (Picture_HEIGHT - theCurrentPraatPicture -> y2NDC + ymargin));
		UiHistory_write (U", ");
		UiHistory_write (Melder_single (Picture_HEIGHT - theCurrentPraatPicture -> y1NDC - ymargin));
	} else {
		UiHistory_write (U"\nSelect outer viewport: ");
		UiHistory_write (Melder_single (theCurrentPraatPicture -> x1NDC));
		UiHistory_write (U", ");
		UiHistory_write (Melder_single (theCurrentPraatPicture -> x2NDC));
		UiHistory_write (U", ");
		UiHistory_write (Melder_single (Picture_HEIGHT - theCurrentPraatPicture -> y2NDC));
		UiHistory_write (U", ");
		UiHistory_write (Melder_single (Picture_HEIGHT - theCurrentPraatPicture -> y1NDC));
	}
}

/***** Public functions. *****/

static GuiWindow thePictureWindow;

static GuiMenu fileMenu, editMenu, marginsMenu, worldMenu, selectMenu, fontMenu, penMenu, helpMenu;

GuiMenu praat_picture_resolveMenu (conststring32 menu) {
	return
		str32equ (menu, U"File") || str32equ (menu, I18n_translate("menu.file")) ? fileMenu :
		str32equ (menu, U"Edit") || str32equ (menu, I18n_translate("menu.edit")) ? editMenu :
		str32equ (menu, U"Margins") || str32equ (menu, I18n_translate("menu.margins")) ? marginsMenu :
		str32equ (menu, U"World") || str32equ (menu, I18n_translate("menu.world")) ? worldMenu :
		str32equ (menu, U"Select") || str32equ (menu, I18n_translate("menu.select_picture")) ? selectMenu :
		str32equ (menu, U"Font") || str32equ (menu, I18n_translate("menu.font")) ? fontMenu :
		str32equ (menu, U"Pen") || str32equ (menu, I18n_translate("menu.pen")) ? penMenu :
		str32equ (menu, U"Help") ? helpMenu :
		editMenu;   // default
}

void praat_picture_exit () {
	praat_picture. reset();
}

void praat_picture_open () {
	Graphics_markGroup (GRAPHICS);   // we start a group of graphics output here
	if (theCurrentPraatPicture == & theForegroundPraatPicture && ! theCurrentPraatApplication -> batch) {
		#if gtk
			gtk_window_present (GTK_WINDOW (thePictureWindow -> d_gtkWindow));
		#elif motif
			XtMapWidget (thePictureWindow -> d_xmShell);
			XMapRaised (XtDisplay (thePictureWindow -> d_xmShell), XtWindow (thePictureWindow -> d_xmShell));
		#elif cocoa
			GuiThing_show (thePictureWindow);
		#endif
	}
	/*
		Obsolete comment (noticed 2023-04-05):
		Foregoing drawing routines may have changed some of the output attributes
		that can be set by the user.
		Make sure that they have the right values before every drawing.
		This is especially necessary after an 'erase picture':
		the output attributes that were set by the user before the 'erase'
		must be recorded before copying to a PostScript file.
	*/
	Graphics_setFont (GRAPHICS, theCurrentPraatPicture -> font);
	Graphics_setFontSize (GRAPHICS, theCurrentPraatPicture -> fontSize);
	Graphics_setLineType (GRAPHICS, theCurrentPraatPicture -> lineType);
	Graphics_setLineWidth (GRAPHICS, theCurrentPraatPicture -> lineWidth);
	Graphics_setArrowSize (GRAPHICS, theCurrentPraatPicture -> arrowSize);
	Graphics_setSpeckleSize (GRAPHICS, theCurrentPraatPicture -> speckleSize);
	Graphics_setColour (GRAPHICS, theCurrentPraatPicture -> colour);

	Graphics_setViewport (GRAPHICS, theCurrentPraatPicture -> x1NDC, theCurrentPraatPicture -> x2NDC,
			theCurrentPraatPicture -> y1NDC, theCurrentPraatPicture -> y2NDC);
	/* The following will dump the axes to the PostScript file after Erase all. BUG: should be somewhere else. */
	double x1WC, x2WC, y1WC, y2WC;
	Graphics_inqWindow (GRAPHICS, & x1WC, & x2WC, & y1WC, & y2WC);
	Graphics_setWindow (GRAPHICS, x1WC, x2WC, y1WC, y2WC);
}

void praat_picture_close () {
	if (theCurrentPraatPicture != & theForegroundPraatPicture)
		return;
	if (! theCurrentPraatApplication -> batch)
		Graphics_updateWs (GRAPHICS);
}

Graphics praat_picture_datagui_open (bool eraseFirst) {
	if (eraseFirst)
		Picture_erase (praat_picture.get());
	praat_picture_open ();
	return GRAPHICS;
}

void praat_picture_datagui_close () {
	praat_picture_close ();
}

static autoDaata pictureRecognizer (integer nread, const char *header, MelderFile file) {
	if (nread < 2)
		return autoDaata ();
	if (strnequ (header, "PraatPicture", 12)) {
		Picture_readFromPraatPictureFile (praat_picture.get(), file);
		return Thing_new (Daata);   // a dummy
	}
	return autoDaata ();
}

void praat_picture_init (bool showPictureWindowAtStartUp) {
	GuiScrolledWindow scrollWindow;
	GuiDrawingArea drawingArea = nullptr;
	int margin, width, height, resolution, x, y;
	theCurrentPraatPicture -> lineType = Graphics_DRAWN;
	theCurrentPraatPicture -> colour = Melder_BLACK;
	theCurrentPraatPicture -> lineWidth = 1.0;
	theCurrentPraatPicture -> arrowSize = 1.0;
	theCurrentPraatPicture -> speckleSize = 1.0;
	theCurrentPraatPicture -> x1NDC = 0.0;
	theCurrentPraatPicture -> x2NDC = 6.0;
	theCurrentPraatPicture -> y1NDC = Picture_HEIGHT - 4.0;
	theCurrentPraatPicture -> y2NDC = Picture_HEIGHT;

	Data_recognizeFileType (pictureRecognizer);

	if (! theCurrentPraatApplication -> batch) {
		double screenX, screenY, screenWidth, screenHeight;
		Gui_getWindowPositioningBounds (& screenX, & screenY, & screenWidth, & screenHeight);
		resolution = Gui_getResolution (nullptr);
		#if defined (macintosh)
			margin = 2;
			width = 6 * resolution + 20;
			height = Machine_getMenuBarBottom () + 9 * resolution + 24;
			x = screenX + screenWidth - width - 14;
			y = screenY + 0;
			width += margin * 2;
		#elif defined (_WIN32)
			margin = 2;
			width = 6 * resolution + 22;
			height = 9 * resolution + 24;
			x = screenX + screenWidth - width - 17;
			y = screenY + 0;
		#else
			margin = 0;
			width = 6 * resolution + 30;
			height = width * 3 / 2 + Machine_getTitleBarHeight ();
			x = screenX + screenWidth - width - 10;
			y = screenY + 0;
			width += margin * 2;
		#endif
		thePictureWindow = GuiWindow_create (x, y, width, height, 400, 200, Melder_cat (Melder_upperCaseAppName(), U" Picture"), nullptr, nullptr, 0);
		GuiWindow_addMenuBar (thePictureWindow);
	}
	if (! theCurrentPraatApplication -> batch) {
		fileMenu =    GuiMenu_createInWindow (thePictureWindow, I18n_translate("menu.file"), 0);
		editMenu =    GuiMenu_createInWindow (thePictureWindow, I18n_translate("menu.edit"), 0);
		marginsMenu = GuiMenu_createInWindow (thePictureWindow, I18n_translate("menu.margins"), 0);
		worldMenu =   GuiMenu_createInWindow (thePictureWindow, I18n_translate("menu.world"), 0);
		selectMenu =  GuiMenu_createInWindow (thePictureWindow, I18n_translate("menu.select_picture"), 0);
		penMenu =     GuiMenu_createInWindow (thePictureWindow, I18n_translate("menu.pen"), 0);
		fontMenu =    GuiMenu_createInWindow (thePictureWindow, I18n_translate("menu.font"), 0);
		helpMenu =    GuiMenu_createInWindow (thePictureWindow, U"Help", 0);
	}

	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.picture_info"),
			nullptr, 0, GRAPHICS_Picture_settings_report);   // alternative GuiMenu_DEPRECATED_2007
	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.separator_save"), nullptr, 0, nullptr);
	#if defined (macintosh) || defined (UNIX)
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_pdf"),
				nullptr, 'S', GRAPHICS_Picture_writeToPdfFile);   // alternative GuiMenu_DEPRECATED_2011
	#endif
	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_300dpi_png"),
			nullptr, 0, GRAPHICS_Picture_writeToPngFile_300);
	#if defined (_WIN32)
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_600dpi_png"),
				nullptr, 'S', GRAPHICS_Picture_writeToPngFile_600);
	#endif
	#if defined (macintosh) || defined (UNIX)
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_600dpi_png"),
				nullptr, 0, GRAPHICS_Picture_writeToPngFile_600);
	#endif
	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_eps"), nullptr, 0, nullptr);
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.postscript_settings"),
				nullptr, GuiMenu_DEPTH_1 | GuiMenu_NO_API, GRAPHICS_PostScript_settings);
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_eps_file"),
				nullptr, 1, GRAPHICS_Picture_writeToEpsFile);   // alternative GuiMenu_DEPRECATED_2011
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_fontless_eps_xipa"),
				nullptr, 1, GRAPHICS_Picture_writeToFontlessEpsFile_xipa);   // alternative GuiMenu_DEPRECATED_2011
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_fontless_eps_silipa"),
				nullptr, 1, GRAPHICS_Picture_writeToFontlessEpsFile_silipa);   // alternative GuiMenu_DEPRECATED_2011
	#ifdef _WIN32
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_windows_metafile"),
				nullptr, 0, GRAPHICS_Picture_writeToWindowsMetafile);   // alternative GuiMenu_DEPRECATED_2011
	#endif
	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.separator_praat_picture_file"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.read_from_praat_picture_file"),
			nullptr, 0, GRAPHICS_Picture_readFromPraatPictureFile);
	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.save_as_praat_picture_file"),
			nullptr, 0, GRAPHICS_Picture_writeToPraatPictureFile);   // alternative GuiMenu_DEPRECATED_2011
	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.separator_print"), nullptr, 0, nullptr);
	#if defined (macintosh)
		praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.page_setup"),
				nullptr, GuiMenu_NO_API, GRAPHICS_Page_setup);
	#endif
	praat_addMenuCommand (U"Picture", U"File", I18n_translate("menu.print"), nullptr, 'P' | GuiMenu_NO_API, GRAPHICS_Print);

	praat_addMenuCommand (U"Picture", U"Edit", I18n_translate("menu.undo"), nullptr, 'Z' | GuiMenu_NO_API, GRAPHICS_Undo);
	#if defined (macintosh) || defined (_WIN32)
		praat_addMenuCommand (U"Picture", U"Edit", I18n_translate("menu.separator_clipboard"), nullptr, 0, nullptr);
		praat_addMenuCommand (U"Picture", U"Edit", I18n_translate("menu.copy_to_clipboard"), nullptr, 'C' | GuiMenu_NO_API, GRAPHICS_Copy_picture_to_clipboard);
	#endif
	praat_addMenuCommand (U"Picture", U"Edit", I18n_translate("menu.separator_erase"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Edit", I18n_translate("menu.erase_all"), nullptr, 'E', GRAPHICS_Erase_all);

	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.draw_inner_box"), nullptr, 0, GRAPHICS_DrawInnerBox);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.separator_text"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.text_left"), nullptr, 0, GRAPHICS_TextLeft);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.text_right"), nullptr, 0, GRAPHICS_TextRight);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.text_top"), nullptr, 0, GRAPHICS_TextTop);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.text_bottom"), nullptr, 0, GRAPHICS_TextBottom);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.separator_marks_every"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks_left_every"), nullptr, 0, GRAPHICS_MarksLeftEvery);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks_right_every"), nullptr, 0, GRAPHICS_MarksRightEvery);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks_bottom_every"), nullptr, 0, GRAPHICS_MarksBottomEvery);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks_top_every"), nullptr, 0, GRAPHICS_MarksTopEvery);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.separator_one_mark"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_mark_left"), nullptr, 0, GRAPHICS_OneMarkLeft);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_mark_right"), nullptr, 0, GRAPHICS_OneMarkRight);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_mark_bottom"), nullptr, 0, GRAPHICS_OneMarkBottom);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_mark_top"), nullptr, 0, GRAPHICS_OneMarkTop);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.separator_marks"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks_left"), nullptr, 1, GRAPHICS_MarksLeft);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks_right"), nullptr, 1, GRAPHICS_MarksRight);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks_bottom"), nullptr, 1, GRAPHICS_MarksBottom);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.marks_top"), nullptr, 1, GRAPHICS_MarksTop);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.logarithmic_marks"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.logarithmic_marks_left"), nullptr, 1, GRAPHICS_LogarithmicMarksLeft);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.logarithmic_marks_right"), nullptr, 1, GRAPHICS_LogarithmicMarksRight);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.logarithmic_marks_bottom"), nullptr, 1, GRAPHICS_LogarithmicMarksBottom);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.logarithmic_marks_top"), nullptr, 1, GRAPHICS_LogarithmicMarksTop);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_logarithmic_mark"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_logarithmic_mark_left"), nullptr, 1, GRAPHICS_OneLogarithmicMarkLeft);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_logarithmic_mark_right"), nullptr, 1, GRAPHICS_OneLogarithmicMarkRight);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_logarithmic_mark_bottom"), nullptr, 1, GRAPHICS_OneLogarithmicMarkBottom);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.one_logarithmic_mark_top"), nullptr, 1, GRAPHICS_OneLogarithmicMarkTop);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.separator_axes"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Margins", I18n_translate("menu.axes"), nullptr, 0, GRAPHICS_Axes);

	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.text"), nullptr, 0, GRAPHICS_Text);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.text_special"), nullptr, 0, GRAPHICS_TextSpecial);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.rectangle_text_maximal_fit"), nullptr, 0, GRAPHICS_RectangleText_MaximalFit);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.rectangle_text_wrap_and_truncate"), nullptr, 0, GRAPHICS_RectangleText_WrapAndTruncate);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.separator_line"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_line"), nullptr, 0, GRAPHICS_DrawLine);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_arrow"), nullptr, 0, GRAPHICS_DrawArrow);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_two_way_arrow"), nullptr, 0, GRAPHICS_DrawDoubleArrow);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.separator_function"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_function"), nullptr, 0, GRAPHICS_DrawFunction);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.separator_rectangle"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_rectangle"), nullptr, 0, GRAPHICS_DrawRectangle);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.paint_rectangle"), nullptr, 0, GRAPHICS_PaintRectangle);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_rounded_rectangle"), nullptr, 0, GRAPHICS_DrawRoundedRectangle);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.paint_rounded_rectangle"), nullptr, 0, GRAPHICS_PaintRoundedRectangle);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.separator_arc"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_arc"), nullptr, 0, GRAPHICS_DrawArc);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_ellipse"), nullptr, 0, GRAPHICS_DrawEllipse);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.paint_ellipse"), nullptr, 0, GRAPHICS_PaintEllipse);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_circle"), nullptr, 0, GRAPHICS_DrawCircle);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.paint_circle"), nullptr, 0, GRAPHICS_PaintCircle);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.draw_circle_mm"), nullptr, 0, GRAPHICS_DrawCircle_mm);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.paint_circle_mm"), nullptr, 0, GRAPHICS_PaintCircle_mm);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.separator_picture"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.insert_picture_from_file"), nullptr, 0, GRAPHICS_InsertPictureFromFile);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.separator_axes_world"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.axes_world"), nullptr, 0, GRAPHICS_Axes);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.measure"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.horizontal_mm_to_world_coordinates"),
			nullptr, 1, GRAPHICS_HorizontalMmToWorldCoordinates);   // alternative GuiMenu_DEPRECATED_2016
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.horizontal_world_coordinates_to_mm"),
			nullptr, 1, GRAPHICS_HorizontalWorldCoordinatesToMm);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.vertical_mm_to_world_coordinates"),
			nullptr, 1, GRAPHICS_VerticalMmToWorldCoordinates);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.vertical_world_coordinates_to_mm"),
			nullptr, 1, GRAPHICS_VerticalWorldCoordinatesToMm);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.separator_text_measure"), nullptr, 1, nullptr);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.text_width_world_coordinates"),
			nullptr, 1, GRAPHICS_TextWidth_worldCoordinates);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.text_width_mm"),
			nullptr, 1, GRAPHICS_TextWidth_mm);
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.postscript_text_width_world_coordinates"),
			nullptr, 1, GRAPHICS_PostScriptTextWidth_worldCoordinates);   // alternative GuiMenu_DEPRECATED_2016
	praat_addMenuCommand (U"Picture", U"World", I18n_translate("menu.postscript_text_width_mm"),
			nullptr, 1, GRAPHICS_PostScriptTextWidth_mm);

	praatButton_innerViewport = praat_addMenuCommand (U"Picture", U"Select",
			I18n_translate("menu.mouse_selects_inner_viewport"), nullptr, GuiMenu_RADIO_FIRST | GuiMenu_NO_API, GRAPHICS_MouseSelectsInnerViewport);
	praatButton_outerViewport = praat_addMenuCommand (U"Picture", U"Select",
			I18n_translate("menu.mouse_selects_outer_viewport"), nullptr, GuiMenu_RADIO_NEXT | GuiMenu_NO_API, GRAPHICS_MouseSelectsOuterViewport);
	praat_addMenuCommand (U"Picture", U"Select", I18n_translate("menu.separator_select"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Select", I18n_translate("menu.select_inner_viewport"),
			nullptr, 0, GRAPHICS_SelectInnerViewport);
	praat_addMenuCommand (U"Picture", U"Select", I18n_translate("menu.select_outer_viewport"),
			nullptr, 0, GRAPHICS_SelectOuterViewport);
	praat_addMenuCommand (U"Picture", U"Select", I18n_translate("menu.separator_viewport_drawing"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Select", I18n_translate("menu.viewport_text"),
			nullptr, 0, GRAPHICS_ViewportText);

	praatButton_lines [Graphics_DRAWN] = praat_addMenuCommand (U"Picture", U"Pen",
			I18n_translate("menu.solid_line"), nullptr, GuiMenu_RADIO_FIRST, GRAPHICS_Solid_line);
	praat_addMenuCommand (U"Picture", U"Pen",
			I18n_translate("menu.plain_line"), nullptr, GuiMenu_RADIO_NEXT | GuiMenu_DEPRECATED_2006, GRAPHICS_Solid_line);
	praatButton_lines [Graphics_DOTTED] = praat_addMenuCommand (U"Picture", U"Pen",
			I18n_translate("menu.dotted_line"), nullptr, GuiMenu_RADIO_NEXT, GRAPHICS_Dotted_line);
	praatButton_lines [Graphics_DASHED] = praat_addMenuCommand (U"Picture", U"Pen",
			I18n_translate("menu.dashed_line"), nullptr, GuiMenu_RADIO_NEXT, GRAPHICS_Dashed_line);
	praatButton_lines [Graphics_DASHED_DOTTED] = praat_addMenuCommand (U"Picture", U"Pen",
			I18n_translate("menu.dashed_dotted_line"), nullptr, GuiMenu_RADIO_NEXT, GRAPHICS_Dashed_dotted_line);
	praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.separator_line_width"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.line_width"), nullptr, 0, GRAPHICS_Line_width);
	praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.arrow_size"), nullptr, 0, GRAPHICS_Arrow_size);
	praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.speckle_size"), nullptr, 0, GRAPHICS_Speckle_size);
	praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.separator_colour"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.colour"), nullptr, 0, GRAPHICS_Colour);
	praatButton_black   = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.black"),   nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Black);
	praatButton_white   = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.white"),   nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_White);
	praatButton_red     = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.red"),     nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Red);
	praatButton_green   = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.green"),   nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Green);
	praatButton_blue    = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.blue"),    nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Blue);
	praatButton_yellow  = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.yellow"),  nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Yellow);
	praatButton_cyan    = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.cyan"),    nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Cyan);
	praatButton_magenta = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.magenta"), nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Magenta);
	praatButton_maroon  = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.maroon"),  nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Maroon);
	praatButton_lime    = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.lime"),    nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Lime);
	praatButton_navy    = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.navy"),    nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Navy);
	praatButton_teal    = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.teal"),    nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Teal);
	praatButton_purple  = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.purple"),  nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Purple);
	praatButton_olive   = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.olive"),   nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Olive);
	praatButton_pink    = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.pink"),    nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Pink);
	praatButton_silver  = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.silver"),  nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Silver);
	praatButton_grey    = praat_addMenuCommand (U"Picture", U"Pen", I18n_translate("menu.grey"),    nullptr, GuiMenu_CHECKBUTTON, GRAPHICS_Grey);

	praat_addMenuCommand (U"Picture", U"Font", I18n_translate("menu.font_size"), nullptr, 0, GRAPHICS_Font_size);
	praatButton_10 = praat_addMenuCommand (U"Picture", U"Font", I18n_translate("menu.font_10"), nullptr, GuiMenu_CHECKBUTTON | GuiMenu_NO_API, GRAPHICS_10);
	praatButton_12 = praat_addMenuCommand (U"Picture", U"Font", I18n_translate("menu.font_12"), nullptr, GuiMenu_CHECKBUTTON | GuiMenu_NO_API, GRAPHICS_12);
	praatButton_14 = praat_addMenuCommand (U"Picture", U"Font", I18n_translate("menu.font_14"), nullptr, GuiMenu_CHECKBUTTON | GuiMenu_NO_API, GRAPHICS_14);
	praatButton_18 = praat_addMenuCommand (U"Picture", U"Font", I18n_translate("menu.font_18"), nullptr, GuiMenu_CHECKBUTTON | GuiMenu_NO_API, GRAPHICS_18);
	praatButton_24 = praat_addMenuCommand (U"Picture", U"Font", I18n_translate("menu.font_24"), nullptr, GuiMenu_CHECKBUTTON | GuiMenu_NO_API, GRAPHICS_24);
	praat_addMenuCommand (U"Picture", U"Font", I18n_translate("menu.separator_font"), nullptr, 0, nullptr);
	praatButton_fonts [(int) kGraphics_font::TIMES] = praat_addMenuCommand (U"Picture", U"Font",
			I18n_translate("menu.times"), nullptr, GuiMenu_RADIO_FIRST, GRAPHICS_NONE__Times);
	praatButton_fonts [(int) kGraphics_font::HELVETICA] = praat_addMenuCommand (U"Picture", U"Font",
			I18n_translate("menu.helvetica"), nullptr, GuiMenu_RADIO_NEXT, GRAPHICS_NONE__Helvetica);
	praatButton_fonts [(int) kGraphics_font::PALATINO] = praat_addMenuCommand (U"Picture", U"Font",
			I18n_translate("menu.palatino"), nullptr, GuiMenu_RADIO_NEXT, GRAPHICS_NONE__Palatino);
	praatButton_fonts [(int) kGraphics_font::COURIER] = praat_addMenuCommand (U"Picture", U"Font",
			I18n_translate("menu.courier"), nullptr, GuiMenu_RADIO_NEXT, GRAPHICS_NONE__Courier);

	praat_addMenuCommand (U"Picture", U"Help", I18n_translate("menu.praat_intro"), nullptr, 0, HELP_PraatIntro_picture);
	praat_addMenuCommand (U"Picture", U"Help", I18n_translate("menu.picture_window_help"), nullptr, '?', HELP_PictureWindowHelp);
	praat_addMenuCommand (U"Picture", U"Help", I18n_translate("menu.separator_text_formatting_help"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Help", I18n_translate("menu.about_special_symbols"), nullptr, 0, HELP_AboutSpecialSymbols);
	praat_addMenuCommand (U"Picture", U"Help", I18n_translate("menu.about_text_styles"), nullptr, 0, HELP_AboutTextStyles);
	praat_addMenuCommand (U"Picture", U"Help", I18n_translate("menu.phonetic_symbols"), nullptr, 0, HELP_PhoneticSymbols);
	praat_addMenuCommand (U"Picture", U"Help", I18n_translate("menu.separator_manual"), nullptr, 0, nullptr);
	praat_addMenuCommand (U"Picture", U"Help",
		Melder_cat (I18n_translate("menu.search_manual_prefix"), U" ", Melder_upperCaseAppName(), I18n_translate("menu.search_manual_suffix")),
		nullptr, 'M', HELP_SearchManual_Picture);

	if (! theCurrentPraatApplication -> batch) {
		width = resolution * Picture_WIDTH;
		height = resolution * Picture_HEIGHT;
		scrollWindow = GuiScrolledWindow_createShown (thePictureWindow, margin, 0, Machine_getMenuBarBottom () + margin, 0, 1, 1, 0);
		drawingArea = GuiDrawingArea_createShown (scrollWindow, width, height,
				nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0);
		GuiThing_show (thePictureWindow);
	}

	// TODO: Paul: deze moet VOOR de update functies anders krijgen die void_me 0x0
	praat_picture = Picture_create (drawingArea, ! theCurrentPraatApplication -> batch);
	// READ THIS!
	Picture_setSelectionChangedCallback (praat_picture.get(), cb_selectionChanged, nullptr);
	theCurrentPraatPicture -> graphics = static_cast<Graphics> (Picture_peekGraphics (praat_picture.get()));

	updatePenMenu ();
	updateFontMenu ();
	updateSizeMenu ();
	updateViewportMenu ();

	if (! theCurrentPraatApplication -> batch && ! showPictureWindowAtStartUp) {
		#if gtk
			gtk_widget_hide (GTK_WIDGET (thePictureWindow -> d_gtkWindow));
		#elif motif
			ShowWindow (thePictureWindow -> d_xmShell -> window, SW_HIDE);
		#elif cocoa
			GuiThing_hide (thePictureWindow);
		#endif
	}
}

void praat_picture_prefsChanged () {
	updateFontMenu ();
	updateSizeMenu ();
	updateViewportMenu ();
	Graphics_setFontSize (theCurrentPraatPicture -> graphics, theCurrentPraatPicture -> fontSize);   // so that the thickness of the selection rectangle is correct
	Picture_setMouseSelectsInnerViewport (praat_picture.get(), praat_mouseSelectsInnerViewport);
}

/* End of file praat_picture.cpp */
