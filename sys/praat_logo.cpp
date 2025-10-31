/* praat_logo.cpp
 *
 * Copyright (C) 1996-2024 Paul Boersma
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
#include "i18n_simple.h"
#include "Picture.h"

static void logo_defaultDraw (Graphics g) {
	Graphics_setColour (g, Melder_MAGENTA);
	Graphics_fillRectangle (g, 0.0, 1.0, 0.0, 1.0);
	Graphics_setGrey (g, 0.5);
	Graphics_fillRectangle (g, 0.05, 0.95, 0.1, 0.9);
	Graphics_setTextAlignment (g, Graphics_CENTRE, Graphics_HALF);
	Graphics_setColour (g, Melder_YELLOW);
	Graphics_setFont (g, kGraphics_font::TIMES);
	Graphics_setFontSize (g, 24);
	Graphics_setFontStyle (g, Graphics_ITALIC);
	Graphics_setUnderscoreIsSubscript (g, false);   // because program names may contain underscores
	Graphics_text (g, 0.5, 0.6, Melder_upperCaseAppName());
	Graphics_setFontStyle (g, 0);
	Graphics_setFontSize (g, 12);
	Graphics_text (g, 0.5, 0.25, I18n_translate ("form.built_on_praat_shell"));

	// Draw link texts
	Graphics_setTextAlignment (g, Graphics_LEFT, Graphics_HALF);
	Graphics_setColour (g, Melder_BLUE);
	Graphics_setFont (g, kGraphics_font::COURIER);
	Graphics_setFontSize (g, 10.0);
	Graphics_text (g, 0.30, 0.32, U"https://praat.org");
	Graphics_text (g, 0.30, 0.26, U"https://www.fon.hum.uva.nl/praat");
	Graphics_text (g, 0.30, 0.20, U"https://github.com/praat/praat.github.io");
	Graphics_text (g, 0.30, 0.14, U"https://github.com/iycera/praat.i18n");
	Graphics_text (g, 0.30, 0.08, U"https://groups.io/g/Praat-Users-List");
}

static struct {
	double width_mm, height_mm;
	void (*draw) (Graphics g);
	GuiDialog dia;
	GuiForm form;
	GuiDrawingArea drawingArea;
	autoGraphics graphics;
} theLogo = { 90, 40, logo_defaultDraw };

void praat_setLogo (double width_mm, double height_mm, void (*draw) (Graphics g)) {
	theLogo.width_mm = width_mm;
	theLogo.height_mm = height_mm;
	theLogo.draw = draw;
}

static void gui_drawingarea_cb_expose (Thing /* me */, GuiDrawingArea_ExposeEvent /* event */) {
	theLogo.draw (theLogo.graphics.get());
}

// 简单的可点击链接支持：检测点击位置，命中则打开浏览器访问对应网址；否则按原行为关闭窗口
#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

static int g_logoWidthPx = 0;
static int g_logoHeightPx = 0;
static int g_hoverIndex = 0; // 1..4 for links

static void openUrl (const char32 *url32) {
#if defined(_WIN32)
	const wchar_t* urlW = reinterpret_cast<const wchar_t*>(Melder_peek32toW (url32));
	// 首先弹窗让用户选择是否在浏览器打开链接（左侧为“取消”，右侧为“确认”）
	wchar_t buf[1024];
	_snwprintf(buf, 1024, L"是否在您的默认浏览器打开：\n%ls", urlW);
	int ret = MessageBoxW(NULL, buf, L"打开链接，请确认", MB_YESNO | MB_DEFBUTTON2);
	if (ret != IDYES) return;
	HINSTANCE result = ShellExecuteW (NULL, L"open", urlW, NULL, NULL, SW_SHOWNORMAL);
	if ((INT_PTR) result <= 32) {
		wchar_t buf[1024];
		_snwprintf(buf, 1024, L"ShellExecuteW 失败: code=%ld, GetLastError=%lu\n将尝试回退方案...", (long)(INT_PTR)result, GetLastError());
		MessageBoxW(NULL, buf, L"Praat Debug", MB_OK | MB_ICONWARNING);
		// 回退：使用 cmd /c start 打开默认浏览器
		wchar_t cmd[1024];
		_snwprintf(cmd, 1024, L"cmd /c start \"\" \"%ls\"", urlW);
		STARTUPINFOW si{}; si.cb = sizeof(si);
		PROCESS_INFORMATION pi{};
		BOOL ok = CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		if (ok) {
			MessageBoxW(NULL, L"CreateProcessW 成功(回退方案)", L"Praat Debug", MB_OK | MB_ICONINFORMATION);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		} else {
			wchar_t buf2[512];
			_snwprintf(buf2, 512, L"CreateProcessW 失败, GetLastError=%lu", GetLastError());
			MessageBoxW(NULL, buf2, L"Praat Debug", MB_OK | MB_ICONERROR);
		}
	}
#elif defined(macintosh)
	system (Melder_peek32to8 (Melder_cat (U"open ", url32)));
#else
	system (Melder_peek32to8 (Melder_cat (U"xdg-open ", url32, U" &")));
#endif
}

static bool hit (int x, int y, double xNormLeft, double yNorm, int widthPx, int heightPx) {
	const int paddingX = (int) (0.01 * widthPx);
	const int halfHeight = 12; // 像素容差
	const int left = (int) (xNormLeft * widthPx) - paddingX;
	const int right = (int) (0.95 * widthPx);
	// 注意：Graphics 文本的 y 以底部为 0、向上为正；
	// 而鼠标事件 y 以顶部为 0、向下为正，需要翻转。
	const int cy = (int) ((1.0 - yNorm) * heightPx);
	return (y > cy - halfHeight && y < cy + halfHeight && x > left && x < right);
}

static void drawUnderlineForIndex (int index) {
	if (! theLogo.graphics) return;
	Graphics graphics = theLogo.graphics.get();
	Graphics_setColour (graphics, Melder_BLUE);
	Graphics_setFont (graphics, kGraphics_font::COURIER);
	Graphics_setFontSize (graphics, 10.0);
	double xLeft = 0.0, y = 0.0; const char32 *text = U"";
	if (index == 1) { xLeft = 0.30; y = 0.32; text = U"https://praat.org"; }
	else if (index == 2) { xLeft = 0.30; y = 0.26; text = U"https://www.fon.hum.uva.nl/praat"; }
	else if (index == 3) { xLeft = 0.30; y = 0.20; text = U"https://github.com/praat/praat.github.io"; }
	else if (index == 4) { xLeft = 0.30; y = 0.08; text = U"https://groups.io/g/Praat-Users-List"; }
	else if (index == 5) { xLeft = 0.30; y = 0.14; text = U"https://github.com/iycera/praat.i18n"; }
	else return;
	Graphics_setTextAlignment (graphics, Graphics_LEFT, Graphics_HALF);
	const double width = Graphics_textWidth (graphics, text);
	const double yUnderline = y - 0.012;
	Graphics_line (graphics, xLeft, yUnderline, xLeft + width, yUnderline);
}


static void gui_drawingarea_cb_mouse (Thing /* me */, GuiDrawingArea_MouseEvent event) {
	if ((event -> phase == structGuiDrawingArea_MouseEvent::Phase::CLICK ||
		 event -> phase == structGuiDrawingArea_MouseEvent::Phase::DROP) && g_logoWidthPx > 0) {
		const int x = event -> x, y = event -> y;
		g_hoverIndex = 0;
#if defined(_WIN32)
		SetCursor (LoadCursor (NULL, IDC_ARROW));
#endif
		// 四个链接的规范化位置（与 main_Praat.cpp 中一致）
		if (hit (x, y, 0.30, 0.32, g_logoWidthPx, g_logoHeightPx)) { g_hoverIndex = 1; }
		else if (hit (x, y, 0.30, 0.26, g_logoWidthPx, g_logoHeightPx)) { g_hoverIndex = 2; }
		else if (hit (x, y, 0.30, 0.20, g_logoWidthPx, g_logoHeightPx)) { g_hoverIndex = 3; }
		else if (hit (x, y, 0.30, 0.14, g_logoWidthPx, g_logoHeightPx)) { g_hoverIndex = 5; }
		else if (hit (x, y, 0.30, 0.08, g_logoWidthPx, g_logoHeightPx)) { g_hoverIndex = 4; }
		if (g_hoverIndex) {
#if defined(_WIN32)
			SetCursor (LoadCursor (NULL, IDC_HAND));
#endif
			drawUnderlineForIndex (g_hoverIndex);
			if (event -> phase == structGuiDrawingArea_MouseEvent::Phase::CLICK)
				return; // 等待鼠标释放再打开
			if (event -> phase == structGuiDrawingArea_MouseEvent::Phase::DROP) {
				if (g_hoverIndex == 1) openUrl (U"https://praat.org");
				else if (g_hoverIndex == 2) openUrl (U"https://www.fon.hum.uva.nl/praat");
				else if (g_hoverIndex == 3) openUrl (U"https://github.com/praat/praat.github.io");
				else if (g_hoverIndex == 4) openUrl (U"https://groups.io/g/Praat-Users-List");
				else if (g_hoverIndex == 5) openUrl (U"https://github.com/iycera/praat.i18n");
				return;
			}
		}
	}
	GuiThing_hide (theLogo.form);
}

static void gui_cb_goAway (Thing /* boss */) {
 	GuiThing_hide (theLogo.form);
}

void praat_showLogo () {
	if (theCurrentPraatApplication -> batch || ! theLogo.draw)
		return;
	if (! theLogo.dia) {
		int width  = theLogo.width_mm  / 25.4 * Gui_getResolution (nullptr);
		int height = theLogo.height_mm / 25.4 * Gui_getResolution (nullptr);
		g_logoWidthPx = width; g_logoHeightPx = height;
		theLogo.dia = GuiDialog_create (theCurrentPraatApplication -> topShell, 100, 100, width, height,
				I18n_translate ("form.about"), gui_cb_goAway, nullptr, GuiDialog_Modality::MODELESS);
		theLogo.form = theLogo.dia;
		theLogo.drawingArea = GuiDrawingArea_createShown (theLogo.form, 0, width, 0, height,
				gui_drawingarea_cb_expose, gui_drawingarea_cb_mouse, nullptr, nullptr, nullptr, nullptr, 0);
		/*
			Note about ordering the following three statements (2021-01-20).
			On some platforms (e.g. on macOS 10.10, but not 10.15), showing entails immediate drawing.
			So the Graphics has to exist before that.
			(It is possible that the *native* graphics *context* is created later.)
		*/
		theLogo.graphics = Graphics_create_xmdrawingarea (theLogo.drawingArea);
		GuiThing_show (theLogo.form);
		GuiThing_show (theLogo.dia);
	} else {
		GuiThing_show (theLogo.form);
		GuiThing_show (theLogo.dia);
	}
}

/* End of file praat_logo.cpp */
