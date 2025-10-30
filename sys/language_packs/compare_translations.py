#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import sys

def compare_translations(en_file, target_file, language_name):
    """比较英语和目标语言的翻译完成度"""
    
    # 读取英语文件
    with open(en_file, 'r', encoding='utf-8-sig') as f:
        en_data = json.load(f)
    
    # 读取目标语言文件
    with open(target_file, 'r', encoding='utf-8-sig') as f:
        target_data = json.load(f)
    
    # 需要保留英文的键
    keep_english = {
        'menu.help', 'menu.save_as_text_file', 'menu.write_to_text_file', 
        'menu.save_as_short_text_file', 'menu.write_to_short_text_file', 
        'menu.save_as_binary_file', 'menu.write_to_binary_file'
    }
    
    # 找出未翻译的条目
    untranslated = []
    partial_translated = []
    
    for key in en_data:
        if key in target_data:
            en_value = en_data[key]
            target_value = target_data[key]
            
            # 如果目标语言值和英语值相同，且不在保留列表中
            if en_value == target_value and key not in keep_english:
                untranslated.append((key, en_value))
            # 检查是否只是部分翻译（比如只翻译了部分单词）
            elif key not in keep_english and len(target_value) < len(en_value) * 0.5:
                partial_translated.append((key, en_value, target_value))
    
    print(f"\n=== {language_name} 翻译检查结果 ===")
    print(f"总条目数: {len(en_data)}")
    print(f"完全未翻译: {len(untranslated)}")
    print(f"部分翻译: {len(partial_translated)}")
    print(f"翻译完成度: {((len(en_data) - len(untranslated) - len(partial_translated)) / len(en_data) * 100):.1f}%")
    
    if untranslated:
        print(f"\n前20个完全未翻译的条目:")
        for i, (key, value) in enumerate(untranslated[:20]):
            print(f"{i+1:3d}. {key}: \"{value}\"")
        if len(untranslated) > 20:
            print(f"... 还有 {len(untranslated) - 20} 个未翻译的条目")
    
    if partial_translated:
        print(f"\n前10个部分翻译的条目:")
        for i, (key, en_val, target_val) in enumerate(partial_translated[:10]):
            print(f"{i+1:3d}. {key}: EN=\"{en_val}\" -> {language_name}=\"{target_val}\"")
        if len(partial_translated) > 10:
            print(f"... 还有 {len(partial_translated) - 10} 个部分翻译的条目")
    
    return untranslated, partial_translated

if __name__ == "__main__":
    languages = [
        ("en-US.json", "de-DE.json", "德语"),
        ("en-US.json", "es-ES.json", "西班牙语"),
        ("en-US.json", "fr-FR.json", "法语"),
        ("en-US.json", "it-IT.json", "意大利语"),
        ("en-US.json", "ja-JP.json", "日语"),
        ("en-US.json", "ko-KR.json", "韩语"),
        ("en-US.json", "pt-PT.json", "葡萄牙语"),
        ("en-US.json", "ru-RU.json", "俄语"),
    ]
    
    for en_file, target_file, lang_name in languages:
        try:
            compare_translations(en_file, target_file, lang_name)
        except Exception as e:
            print(f"检查 {lang_name} 时出错: {e}")
