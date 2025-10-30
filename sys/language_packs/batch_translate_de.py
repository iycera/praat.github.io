#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import re

def batch_translate_german():
    """批量翻译德语语言包中的常见未翻译条目"""
    
    # 读取德语文件
    with open('de-DE.json', 'r', encoding='utf-8-sig') as f:
        de_data = json.load(f)
    
    # 需要保留英文的键
    keep_english = {
        'menu.help', 'menu.save_as_text_file', 'menu.write_to_text_file', 
        'menu.save_as_short_text_file', 'menu.write_to_short_text_file', 
        'menu.save_as_binary_file', 'menu.write_to_binary_file'
    }
    
    # 常见翻译映射
    translations = {
        # 基本菜单项
        "menu.praat": "Praat",  # 保持英文
        "menu.zoom": "Zoom",  # 保持英文
        "menu.run": "Ausführen",
        "menu.search": "Suchen",
        "menu.font": "Schriftart",
        "menu.convert": "Konvertieren",
        
        # 分隔符
        "menu.separator_script": "-- Skript --",
        "menu.separator_actions": "-- Aktionen --",
        "menu.separator_api": "-- API --",
        "menu.separator_manual": "-- Handbuch --",
        "menu.separator_save": "-- Speichern --",
        "menu.separator_praat_picture_file": "-- Praat-Bilddatei --",
        "menu.separator_print": "-- Drucken --",
        "menu.separator_text": "-- Text --",
        "menu.separator_marks_every": "-- Markierungen alle --",
        "menu.separator_one_mark": "-- eine Markierung --",
        "menu.separator_marks": "-- Markierungen --",
        "menu.separator_axes": "-- Achsen --",
        "menu.separator_line": "-- Linie --",
        "menu.separator_function": "-- Funktion --",
        "menu.separator_rectangle": "-- Rechteck --",
        "menu.separator_arc": "-- Bogen --",
        "menu.separator_picture": "-- Bild --",
        "menu.separator_text_measure": "-- Text messen --",
        "menu.separator_select": "-- Auswählen --",
        "menu.separator_line_width": "-- Linienbreite --",
        "menu.separator_colour": "-- Farbe --",
        "menu.separator_pen": "-- Stift --",
        "menu.separator_erase": "-- Löschen --",
        "menu.separator_paint": "-- Malen --",
        "menu.separator_move": "-- Bewegen --",
        "menu.separator_rotate": "-- Drehen --",
        "menu.separator_scale": "-- Skalieren --",
        "menu.separator_zoom": "-- Zoom --",
        "menu.separator_view": "-- Ansicht --",
        "menu.separator_help": "-- Hilfe --",
        
        # 帮助相关
        "menu.help_history": "-- Hilfe-Verlauf --",
        "menu.help_add": "-- Hilfe hinzufügen --",
        "menu.line": "-- Zeile --",
        
        # 窗口和对话框
        "menu.demo_window": "Demo-Fenster",
        "menu.about_script_editor": "Über Skript-Editor",
        "menu.about_notebook_editor": "Über Notizbuch-Editor",
        "menu.new_script_dialog": "Neues Praat-Skript...",
        "menu.new_notebook_dialog": "Neues Praat-Notizbuch...",
        "menu.open_script_dialog": "Praat-Skript öffnen...",
        "menu.open_notebook_dialog": "Praat-Notizbuch öffnen...",
        
        # 文件操作
        "menu.save_as_pdf": "Als PDF-Datei speichern...",
        "menu.save_as_300dpi_png": "Als 300-dpi PNG-Datei speichern...",
        "menu.save_as_600dpi_png": "Als 600-dpi PNG-Datei speichern...",
        "menu.save_as_eps": "Als EPS-Datei speichern",
        "menu.save_as_eps_file": "Als EPS-Datei speichern...",
        "menu.save_as_fontless_eps_xipa": "Als schriftartenlose EPS-Datei speichern (XIPA)...",
        "menu.save_as_fontless_eps_silipa": "Als schriftartenlose EPS-Datei speichern (SILIPA)...",
        "menu.save_as_windows_metafile": "Als Windows-Metadatei speichern...",
        "menu.save_as_praat_picture_file": "Als Praat-Bilddatei speichern...",
        "menu.read_from_praat_picture_file": "Aus Praat-Bilddatei lesen...",
        
        # 文本相关
        "menu.text_left": "Text links...",
        "menu.text_right": "Text rechts...",
        "menu.text_top": "Text oben...",
        "menu.text_bottom": "Text unten...",
        "menu.text": "Text...",
        "menu.text_special": "Text speziell...",
        
        # 标记相关
        "menu.logarithmic_marks": "Logarithmische Markierungen",
        "menu.logarithmic_marks_left": "Logarithmische Markierungen links...",
        "menu.logarithmic_marks_right": "Logarithmische Markierungen rechts...",
        "menu.logarithmic_marks_bottom": "Logarithmische Markierungen unten...",
        "menu.logarithmic_marks_top": "Logarithmische Markierungen oben...",
        "menu.one_logarithmic_mark": "Eine logarithmische Markierung",
        "menu.one_logarithmic_mark_left": "Eine logarithmische Markierung links...",
        "menu.one_logarithmic_mark_right": "Eine logarithmische Markierung rechts...",
        "menu.one_logarithmic_mark_bottom": "Eine logarithmische Markierung unten...",
        "menu.one_logarithmic_mark_top": "Eine logarithmische Markierung oben...",
        "menu.one_mark_left": "Eine Markierung links...",
        "menu.one_mark_right": "Eine Markierung rechts...",
        "menu.one_mark_bottom": "Eine Markierung unten...",
        "menu.one_mark_top": "Eine Markierung oben...",
        
        # 轴相关
        "menu.axes": "Achsen...",
        "menu.axes_left": "Achsen links...",
        "menu.axes_right": "Achsen rechts...",
        "menu.axes_bottom": "Achsen unten...",
        "menu.axes_top": "Achsen oben...",
        
        # 图形相关
        "menu.rectangle": "Rechteck...",
        "menu.arc": "Bogen...",
        "menu.picture": "Bild...",
        "menu.picture_info": "Bild-Info",
        "menu.select_picture": "Bild auswählen",
        "menu.pen": "Stift",
        "menu.line_width": "Linienbreite...",
        "menu.colour": "Farbe...",
        "menu.erase": "Löschen...",
        "menu.paint": "Malen...",
        "menu.move": "Bewegen...",
        "menu.rotate": "Drehen...",
        "menu.scale": "Skalieren...",
        "menu.zoom_in": "Hineinzoomen",
        "menu.zoom_out": "Herauszoomen",
        "menu.zoom_to_selection": "Auf Auswahl zoomen",
        "menu.zoom_back": "Zurück zoomen",
        "menu.show_all": "Alles anzeigen",
        "menu.view": "Ansicht",
        
        # 选择相关
        "menu.select": "Auswählen...",
        "menu.select_all": "Alles auswählen",
        "menu.select_none": "Nichts auswählen",
        "menu.select_inner_viewport": "Inneren Ansichtsbereich auswählen...",
        "menu.select_outer_viewport": "Äußeren Ansichtsbereich auswählen...",
        
        # 鼠标相关
        "menu.mouse_selects_inner_viewport": "Maus wählt inneren Ansichtsbereich",
        "menu.mouse_selects_outer_viewport": "Maus wählt äußeren Ansichtsbereich",
        "menu.viewport_text": "Ansichtsbereich-Text...",
        "menu.separator_viewport_drawing": "-- Ansichtsbereich zeichnen --",
        
        # 其他常见项
        "menu.expand_include_files": "Include-Dateien erweitern",
        "menu.run_selection": "Auswahl ausführen",
        "menu.scripting_tutorial": "Skripting-Tutorial",
        "menu.scripting_examples": "Skripting-Beispiele",
        "menu.praat_script": "Praat-Skript",
        "menu.formulas_tutorial": "Formeln-Tutorial",
        "menu.functions": "Funktionen",
        "menu.history_mechanism": "Der Verlauf-Mechanismus",
        "menu.initialization_scripts": "Initialisierungsskripte",
        "menu.adding_to_fixed_menu": "Zu einem festen Menü hinzufügen",
        "menu.adding_to_dynamic_menu": "Zu einem dynamischen Menü hinzufügen",
        "menu.find": "Suchen...",
        "menu.find_again": "Erneut suchen",
        "menu.replace": "Ersetzen...",
        "menu.replace_again": "Erneut ersetzen",
        "menu.use_selection_for_find": "Auswahl für Suche verwenden",
        "menu.where_am_i": "Wo bin ich?",
        "menu.go_to_line": "Zu Zeile gehen...",
        "menu.convert_to_c_string": "In C-String konvertieren",
        "menu.run_chunk": "Chunk ausführen",
        "menu.margins": "Ränder",
        "menu.world": "Welt",
        "menu.page_setup": "Seiteneinrichtung...",
        "menu.print": "Drucken...",
        "menu.draw_inner_box": "Inneren Rahmen zeichnen",
    }
    
    # 应用翻译
    updated_count = 0
    for key, translation in translations.items():
        if key in de_data and key not in keep_english:
            de_data[key] = translation
            updated_count += 1
    
    # 保存文件
    with open('de-DE.json', 'w', encoding='utf-8') as f:
        json.dump(de_data, f, ensure_ascii=False, indent=2)
    
    print(f"已更新 {updated_count} 个条目")

if __name__ == "__main__":
    batch_translate_german()
