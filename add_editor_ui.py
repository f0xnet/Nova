#!/usr/bin/env python3
"""
Script to add Time of Day, Post-Processing, and Light Properties panels to editor UI
"""
import json
import sys

def add_ui_elements(json_path):
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # ===== ADD TOOLBAR BUTTONS =====

    # Time of Day button
    btn_time_of_day = {
        "ID": "btn_time_of_day",
        "groupID": "toolbar",
        "haveText": True,
        "text": "Time",
        "font": "C:/Windows/Fonts/arial.ttf",
        "fontSize": 24,
        "color": "0,0,0",
        "path": "data/ui/ui_assets/button.png",
        "path_hover": "data/ui/ui_assets/button_hover.png",
        "path_pressed": "data/ui/ui_assets/button_clicked.png",
        "x": 1660,
        "y": 20,
        "width": 140,
        "height": 60,
        "effect": "none",
        "action": "show_time_of_day",
        "value": "",
        "layer": 11,
        "isActive": True
    }

    # Post-Processing button
    btn_postprocess = {
        "ID": "btn_postprocess",
        "groupID": "toolbar",
        "haveText": True,
        "text": "Effects",
        "font": "C:/Windows/Fonts/arial.ttf",
        "fontSize": 24,
        "color": "0,0,0",
        "path": "data/ui/ui_assets/button.png",
        "path_hover": "data/ui/ui_assets/button_hover.png",
        "path_pressed": "data/ui/ui_assets/button_clicked.png",
        "x": 1820,
        "y": 20,
        "width": 160,
        "height": 60,
        "effect": "none",
        "action": "show_postprocess",
        "value": "",
        "layer": 11,
        "isActive": True
    }

    # Find where to insert (after btn_show_properties)
    insert_idx = None
    for i, btn in enumerate(data['buttons']):
        if btn['ID'] == 'btn_show_properties':
            insert_idx = i + 1
            break

    if insert_idx:
        data['buttons'].insert(insert_idx, btn_time_of_day)
        data['buttons'].insert(insert_idx + 1, btn_postprocess)
        print(f"✓ Added toolbar buttons at index {insert_idx}")

    # ===== ADD TIME OF DAY PANEL =====

    time_panel_images = [
        {
            "ID": "time_panel_bg_overlay",
            "groupID": "time_panel",
            "x": 0,
            "y": 0,
            "width": 3840,
            "height": 2160,
            "color": "0,0,0,128",
            "layer": 17,
            "isActive": False
        },
        {
            "ID": "time_panel_bg",
            "groupID": "time_panel",
            "image": "data/ui/ui_assets/editor_background.png",
            "x": 1320,
            "y": 600,
            "width": 1200,
            "height": 800,
            "color": "255,255,255",
            "layer": 18,
            "isActive": False
        }
    ]

    time_panel_texts = [
        {
            "ID": "time_panel_title",
            "groupID": "time_panel",
            "content": "Time of Day Settings",
            "x": 1370,
            "y": 640,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 36,
            "color": "220,220,220",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "time_label",
            "groupID": "time_panel",
            "content": "Current Time:",
            "x": 1370,
            "y": 730,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "200,200,200",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "time_value",
            "groupID": "time_panel",
            "content": "12:00 (Noon)",
            "x": 1700,
            "y": 730,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "100,255,100",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "ambient_label",
            "groupID": "time_panel",
            "content": "Ambient Darkness:",
            "x": 1370,
            "y": 900,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "200,200,200",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "ambient_value",
            "groupID": "time_panel",
            "content": "0.50",
            "x": 1800,
            "y": 900,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "100,255,100",
            "layer": 19,
            "isActive": False
        }
    ]

    time_panel_buttons = [
        # Close button
        {
            "ID": "btn_time_close",
            "groupID": "time_panel",
            "haveText": True,
            "text": "Close",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 2220,
            "y": 1260,
            "width": 240,
            "height": 80,
            "effect": "none",
            "action": "close_time_panel",
            "value": "",
            "layer": 19,
            "isActive": False
        },
        # Time presets
        {
            "ID": "btn_time_midnight",
            "groupID": "time_panel",
            "haveText": True,
            "text": "Midnight",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 24,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1370,
            "y": 800,
            "width": 180,
            "height": 60,
            "effect": "none",
            "action": "set_time",
            "value": "0.0",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "btn_time_dawn",
            "groupID": "time_panel",
            "haveText": True,
            "text": "Dawn",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 24,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1570,
            "y": 800,
            "width": 180,
            "height": 60,
            "effect": "none",
            "action": "set_time",
            "value": "0.25",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "btn_time_noon",
            "groupID": "time_panel",
            "haveText": True,
            "text": "Noon",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 24,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1770,
            "y": 800,
            "width": 180,
            "height": 60,
            "effect": "none",
            "action": "set_time",
            "value": "0.5",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "btn_time_dusk",
            "groupID": "time_panel",
            "haveText": True,
            "text": "Dusk",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 24,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1970,
            "y": 800,
            "width": 180,
            "height": 60,
            "effect": "none",
            "action": "set_time",
            "value": "0.75",
            "layer": 19,
            "isActive": False
        },
        # Ambient darkness controls
        {
            "ID": "btn_ambient_dec",
            "groupID": "time_panel",
            "haveText": True,
            "text": "-",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1900,
            "y": 890,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_ambient",
            "value": "-0.05",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "btn_ambient_inc",
            "groupID": "time_panel",
            "haveText": True,
            "text": "+",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 2000,
            "y": 890,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_ambient",
            "value": "+0.05",
            "layer": 19,
            "isActive": False
        }
    ]

    data['images'].extend(time_panel_images)
    data['text'].extend(time_panel_texts)
    data['buttons'].extend(time_panel_buttons)
    print(f"✓ Added Time of Day panel ({len(time_panel_images)} images, {len(time_panel_texts)} texts, {len(time_panel_buttons)} buttons)")

    # ===== ADD POST-PROCESSING PANEL =====

    pp_panel_images = [
        {
            "ID": "pp_panel_bg_overlay",
            "groupID": "pp_panel",
            "x": 0,
            "y": 0,
            "width": 3840,
            "height": 2160,
            "color": "0,0,0,128",
            "layer": 17,
            "isActive": False
        },
        {
            "ID": "pp_panel_bg",
            "groupID": "pp_panel",
            "image": "data/ui/ui_assets/editor_background.png",
            "x": 1220,
            "y": 400,
            "width": 1400,
            "height": 1200,
            "color": "255,255,255",
            "layer": 18,
            "isActive": False
        }
    ]

    pp_panel_texts = [
        {
            "ID": "pp_panel_title",
            "groupID": "pp_panel",
            "content": "Post-Processing Effects",
            "x": 1270,
            "y": 440,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 36,
            "color": "220,220,220",
            "layer": 19,
            "isActive": False
        },
        # Bloom
        {
            "ID": "pp_bloom_label",
            "groupID": "pp_panel",
            "content": "Bloom Intensity:",
            "x": 1270,
            "y": 540,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "200,200,200",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "pp_bloom_value",
            "groupID": "pp_panel",
            "content": "0.40",
            "x": 1700,
            "y": 540,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "100,255,100",
            "layer": 19,
            "isActive": False
        },
        # Saturation
        {
            "ID": "pp_saturation_label",
            "groupID": "pp_panel",
            "content": "Saturation:",
            "x": 1270,
            "y": 660,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "200,200,200",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "pp_saturation_value",
            "groupID": "pp_panel",
            "content": "1.30",
            "x": 1700,
            "y": 660,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "100,255,100",
            "layer": 19,
            "isActive": False
        },
        # Contrast
        {
            "ID": "pp_contrast_label",
            "groupID": "pp_panel",
            "content": "Contrast:",
            "x": 1270,
            "y": 780,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "200,200,200",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "pp_contrast_value",
            "groupID": "pp_panel",
            "content": "1.00",
            "x": 1700,
            "y": 780,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "100,255,100",
            "layer": 19,
            "isActive": False
        },
        # Brightness
        {
            "ID": "pp_brightness_label",
            "groupID": "pp_panel",
            "content": "Brightness:",
            "x": 1270,
            "y": 900,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "200,200,200",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "pp_brightness_value",
            "groupID": "pp_panel",
            "content": "0.00",
            "x": 1700,
            "y": 900,
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "100,255,100",
            "layer": 19,
            "isActive": False
        }
    ]

    pp_panel_buttons = [
        # Close button
        {
            "ID": "btn_pp_close",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "Close",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 28,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 2120,
            "y": 1420,
            "width": 240,
            "height": 80,
            "effect": "none",
            "action": "close_pp_panel",
            "value": "",
            "layer": 19,
            "isActive": False
        },
        # Bloom controls
        {
            "ID": "btn_bloom_dec",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "-",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1800,
            "y": 530,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_bloom",
            "value": "-0.1",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "btn_bloom_inc",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "+",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1900,
            "y": 530,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_bloom",
            "value": "+0.1",
            "layer": 19,
            "isActive": False
        },
        # Saturation controls
        {
            "ID": "btn_saturation_dec",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "-",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1800,
            "y": 650,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_saturation",
            "value": "-0.1",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "btn_saturation_inc",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "+",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1900,
            "y": 650,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_saturation",
            "value": "+0.1",
            "layer": 19,
            "isActive": False
        },
        # Contrast controls
        {
            "ID": "btn_contrast_dec",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "-",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1800,
            "y": 770,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_contrast",
            "value": "-0.1",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "btn_contrast_inc",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "+",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1900,
            "y": 770,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_contrast",
            "value": "+0.1",
            "layer": 19,
            "isActive": False
        },
        # Brightness controls
        {
            "ID": "btn_brightness_dec",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "-",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1800,
            "y": 890,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_brightness",
            "value": "-0.05",
            "layer": 19,
            "isActive": False
        },
        {
            "ID": "btn_brightness_inc",
            "groupID": "pp_panel",
            "haveText": True,
            "text": "+",
            "font": "C:/Windows/Fonts/arial.ttf",
            "fontSize": 32,
            "color": "0,0,0",
            "path": "data/ui/ui_assets/button.png",
            "path_hover": "data/ui/ui_assets/button_hover.png",
            "path_pressed": "data/ui/ui_assets/button_clicked.png",
            "x": 1900,
            "y": 890,
            "width": 80,
            "height": 60,
            "effect": "none",
            "action": "adjust_brightness",
            "value": "+0.05",
            "layer": 19,
            "isActive": False
        }
    ]

    data['images'].extend(pp_panel_images)
    data['text'].extend(pp_panel_texts)
    data['buttons'].extend(pp_panel_buttons)
    print(f"✓ Added Post-Processing panel ({len(pp_panel_images)} images, {len(pp_panel_texts)} texts, {len(pp_panel_buttons)} buttons)")

    # Save updated JSON
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

    print(f"\n✓✓✓ Successfully updated {json_path}")
    print(f"    Total images: {len(data['images'])}")
    print(f"    Total texts: {len(data['text'])}")
    print(f"    Total buttons: {len(data['buttons'])}")

if __name__ == '__main__':
    json_path = '/home/user/Nova/editor/assets/ui/editor_main.json'
    add_ui_elements(json_path)
