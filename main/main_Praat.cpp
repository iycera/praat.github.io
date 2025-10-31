/* main_Praat.cpp
 *
 * Copyright (C) 1992-2008,2010-2021,2023-2025 Paul Boersma
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

#include "../sys/praat.h"
#include "main_Praat.h"
#include "../sys/i18n_simple.h"

static void logo (Graphics graphics) {
	Graphics_setWindow (graphics, 0.0, 1.0, 0.0, 1.0);
	Graphics_setGrey (graphics, 0.95);
	Graphics_fillRectangle (graphics, 0.0, 1.0, 0.0, 1.0);
	Graphics_setTextAlignment (graphics, Graphics_CENTRE, Graphics_HALF);
	Graphics_setFont (graphics, kGraphics_font::TIMES);
	Graphics_setFontSize (graphics, 45.0);
	Graphics_setColour (graphics, Melder_MAROON);
	Graphics_text (graphics, 0.370, 0.89, U"P");
	Graphics_text (graphics, 0.433, 0.89, U"\\s{R}");
	Graphics_text (graphics, 0.495, 0.89, U"\\s{A}");
	Graphics_text (graphics, 0.560, 0.89, U"\\s{A}");
	Graphics_text (graphics, 0.613, 0.89, U"\\s{T}");
	// i18n corner mark (small-sized, top-right)
	Graphics_setTextAlignment (graphics, Graphics_RIGHT, Graphics_TOP);	
	Graphics_setFontSize (graphics, 20.0);
	Graphics_text (graphics, 0.720, 0.911, U"\\s{I18N}");
	// restore defaults for subsequent text
	Graphics_setFontSize (graphics, 16.0);
	Graphics_setTextAlignment (graphics, Graphics_CENTRE, Graphics_HALF);
	Graphics_text (graphics, 0.5, 0.77, I18n_translate("app.tagline"));
	// version text
	Graphics_setFontSize (graphics, 12.0);
	Graphics_setFont (graphics, kGraphics_font::PALATINO);
	Graphics_text (graphics, 0.5, 0.62, Melder_cat (I18n_translate("form.version"), U" ", Melder_appVersionSTR(),
			U" (", Melder_appMonthSTR(), U" ", Melder_appDay(), U", ", Melder_appYear(), U")"));
	Graphics_setFontSize (graphics, 12.0);
	[[maybe_unused]] constexpr bool isArm64 =
		#if defined (__aarch64__) || defined (_M_ARM64_)
			true;
		#else
			false;
		#endif
	const conststring32 builtFor =
		#if defined (macintosh)
			isArm64 ? U"ARM64 macOS" : U"Intel64 macOS";
		#elif defined (_WIN32)
			isArm64 ? U"ARM64 Windows" : sizeof (void *) == 4 ? U"Intel32 Windows" : U"Intel64 Windows";
		#elif defined (__s390x__)
			U"s390x Linux";
		#elif defined (raspberrypi)
			U"ARMV7 Raspberry Pi";
		#elif defined (chrome)
			isArm64 ? U"ARM64 Chromebook" : U"Intel64 Chromebook";
		#elif defined (linux)
			isArm64 ? U"ARM64 Linux" : U"Intel64 Linux";
		#else
			U"";
			#error Unknown OS type.
		#endif
	Graphics_text (graphics, 0.5, 0.54, Melder_cat (U"built for ", builtFor));
	Graphics_setColour (graphics, Melder_BLACK);
	Graphics_setFont (graphics, kGraphics_font::HELVETICA);
	Graphics_setFontSize (graphics, 10.0);
	Graphics_text (graphics, 0.5, 0.40, Melder_cat (U"Copyright © 1992–", Melder_appYear(), U" by Paul Boersma and David Weenink"));
	Graphics_setFontSize (graphics, 10.0);
	Graphics_setTextAlignment (graphics, Graphics_RIGHT, Graphics_HALF);
	Graphics_text (graphics, 0.28, 0.32, U"Download sites:");
	Graphics_text (graphics, 0.28, 0.20, U"Source code:");
	Graphics_text (graphics, 0.28, 0.14, U"i18n repo:");
	Graphics_text (graphics, 0.28, 0.08, U"Praat Users List:");
	Graphics_setTextAlignment (graphics, Graphics_LEFT, Graphics_HALF);
	Graphics_setFont (graphics, kGraphics_font::COURIER);
	Graphics_text (graphics, 0.30, 0.32, U"https://praat.org");
	Graphics_text (graphics, 0.30, 0.26, U"https://www.fon.hum.uva.nl/praat");
	Graphics_text (graphics, 0.30, 0.20, U"https://github.com/praat/praat.github.io");
	Graphics_text (graphics, 0.30, 0.14, U"https://github.com/iycera/praat.i18n");
	Graphics_text (graphics, 0.30, 0.08, U"https://groups.io/g/Praat-Users-List");
}

int main (int argc, char *argv []) {
	try {
		//TRACE
		praat_setLogo (130.0, 100.0, logo);
		MelderStopwatch stopwatch;
		praat_init (U"" stringize (PRAAT_NAME),
			U"" stringize (PRAAT_VERSION_STR), PRAAT_VERSION_NUM,
			PRAAT_YEAR, PRAAT_MONTH, PRAAT_DAY,
			U"paul.boersma", U"uva.nl",
			argc, argv
		);
		trace (stopwatch());
		INCLUDE_LIBRARY (praat_uvafon_init)
		trace (stopwatch());
		praat_run ();
		trace (stopwatch());
	} catch (MelderError) {
		Melder_flushError (U"This error message percolated all the way to the top.");   // an attempt to catch Apache errors
	}
	return 0;   // obligatory (because on Windows `main` is just a normal function called from our WinMain)
}

/* End of file main_Praat.cpp */
