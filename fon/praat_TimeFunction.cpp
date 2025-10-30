/* praat_TimeFunction.cpp
 *
 * Copyright (C) 1992-2017,2022,2023 Paul Boersma
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

#include "praat_TimeFunction.h"
#include "../sys/i18n_simple.h"

// MARK: - TIMEFUNCTION

// MARK: Query

DIRECT (REAL_TimeFunction_getStartTime) {
	QUERY_ONE_FOR_REAL (Function)
		double result = my xmin;
	QUERY_ONE_FOR_REAL_END (U" seconds")
}

DIRECT (REAL_TimeFunction_getEndTime) {
	QUERY_ONE_FOR_REAL (Function)
		double result = my xmax;
	QUERY_ONE_FOR_REAL_END (U" seconds")
}

DIRECT (REAL_TimeFunction_getTotalDuration) {
	QUERY_ONE_FOR_REAL (Function)
		double result = my xmax - my xmin;
	QUERY_ONE_FOR_REAL_END (U" seconds")
}

// MARK: Modify

FORM (MODIFY_TimeFunction_shiftTimesBy, I18n_translate("form.shift_times_by"), nullptr) {
	REAL (shift, I18n_translate("form.shift_s"), U"0.5")
	OK
DO
	MODIFY_EACH (Function)
		Function_shiftXBy (me, shift);
	MODIFY_EACH_END
}

FORM (MODIFY_TimeFunction_shiftTimesTo, I18n_translate("form.shift_times_to"), nullptr) {
	CHOICE (shift, I18n_translate("form.shift"), 1)
		OPTION (I18n_translate("form.start_time"))
		OPTION (I18n_translate("form.centre_time"))
		OPTION (I18n_translate("form.end_time"))
	REAL (toTime, I18n_translate("form.to_time_s"), U"0.0")
	OK
DO
	MODIFY_EACH (Function)
		Function_shiftXTo (me, shift == 1 ? my xmin : shift == 2 ? 0.5 * (my xmin + my xmax) : my xmax, toTime);
	MODIFY_EACH_END
}

DIRECT (MODIFY_TimeFunction_shiftToZero) {
	MODIFY_EACH (Function)
		Function_shiftXTo (me, my xmin, 0.0);
	MODIFY_EACH_END
}

FORM (MODIFY_TimeFunction_scaleTimesBy, I18n_translate("form.scale_times_by"), nullptr) {
	POSITIVE (factor, I18n_translate("form.factor"), U"2.0")
	OK
DO
	MODIFY_EACH (Function)
		Function_scaleXBy (me, factor);
	MODIFY_EACH_END
}

FORM (MODIFY_TimeFunction_scaleTimesTo, I18n_translate("form.scale_times_to"), nullptr) {
	REAL (newStartTime, I18n_translate("form.new_start_time_s"), U"0.0")
	REAL (newEndTime, I18n_translate("form.new_end_time_s"), U"1.0")
	OK
DO
	if (newStartTime >= newEndTime) Melder_throw (I18n_translate("error.new_end_time_should_be_greater_than_new_start_time"));
	MODIFY_EACH (Function)
		Function_scaleXTo (me, newStartTime, newEndTime);
	MODIFY_EACH_END
}

// MARK: - buttons

void praat_TimeFunction_query_init (ClassInfo klas) {
	praat_addAction1 (klas, 1, I18n_translate("menu.query_time_domain"), nullptr, 1, nullptr);
	praat_addAction1 (klas, 1, I18n_translate("menu.get_start_time"), nullptr, 2, REAL_TimeFunction_getStartTime);
	praat_addAction1 (klas, 1, I18n_translate("menu.get_end_time"), nullptr, 2, REAL_TimeFunction_getEndTime);
	praat_addAction1 (klas, 1, I18n_translate("menu.get_total_duration"), nullptr, 2, REAL_TimeFunction_getTotalDuration);
}

void praat_TimeFunction_modify_init (ClassInfo klas) {
	praat_addAction1 (klas, 0, I18n_translate("menu.modify_times"), nullptr, 1, nullptr);
	praat_addAction1 (klas, 0, I18n_translate("menu.shift_times_by"), nullptr, 2, MODIFY_TimeFunction_shiftTimesBy);
	praat_addAction1 (klas, 0, I18n_translate("menu.shift_times_to"), nullptr, 2, MODIFY_TimeFunction_shiftTimesTo);
	praat_addAction1 (klas, 0,   U"Shift to zero", nullptr, GuiMenu_DEPTH_2 | GuiMenu_DEPRECATED_2008, MODIFY_TimeFunction_shiftToZero);
			// replace with "Shift times to..."
	praat_addAction1 (klas, 0, I18n_translate("menu.scale_times_by"), nullptr, 2, MODIFY_TimeFunction_scaleTimesBy);
	praat_addAction1 (klas, 0, I18n_translate("menu.scale_times_to"), nullptr, 2, MODIFY_TimeFunction_scaleTimesTo);
			// alternatives COMPATIBILITY <= 2008
}

/* End of file praat_TimeFunction.cpp */
