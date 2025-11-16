#!/usr/bin/env python3
"""
Script pour créer automatiquement les sprites de test pour Simple RPG Example.

Usage:
    python create_sprites.py

Crée 3 sprites PNG 64x64 :
- player.png (bleu clair)
- npc_merchant.png (vert)
- npc_innkeeper.png (orange)
"""

import os
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False
    print("WARNING: PIL/Pillow not installed. Install with: pip install Pillow")
    print("Falling back to basic sprite generation...")

def create_sprite_basic(width, height, color, filepath):
    """Crée un sprite carré simple avec une couleur unie (sans PIL)."""
    # PNG basique avec couleur unie
    # Format PNG minimal : signature + IHDR + IDAT + IEND

    # Pour simplifier, on crée juste un fichier texte qui explique comment créer le sprite
    # Car créer un PNG valide sans bibliothèque est complexe

    print(f"⚠️  Cannot create {filepath} without PIL")
    print(f"   Please install Pillow: pip install Pillow")
    return False

def create_sprite_with_pil(width, height, color, text, filepath):
    """Crée un sprite avec PIL/Pillow."""
    # Créer une image avec transparence
    img = Image.new('RGBA', (width, height), color + (255,))

    # Ajouter un contour noir
    draw = ImageDraw.Draw(img)
    draw.rectangle([(0, 0), (width-1, height-1)], outline=(0, 0, 0, 255), width=2)

    # Ajouter du texte au centre
    try:
        font = ImageFont.truetype("arial.ttf", 12)
    except:
        font = ImageFont.load_default()

    # Centrer le texte
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    x = (width - text_width) // 2
    y = (height - text_height) // 2

    draw.text((x, y), text, fill=(255, 255, 255, 255), font=font)

    # Sauvegarder
    img.save(filepath, 'PNG')
    print(f"✓ Created: {filepath}")
    return True

def main():
    # Déterminer le chemin des assets
    script_dir = Path(__file__).parent
    data_dir = script_dir / "assets" / "data"

    # Créer le dossier si nécessaire
    data_dir.mkdir(parents=True, exist_ok=True)

    print("=" * 50)
    print("Simple RPG - Sprite Generator")
    print("=" * 50)
    print()

    sprites = [
        {
            'filename': 'player.png',
            'color': (100, 200, 255),  # Bleu clair
            'text': 'P',
            'description': 'Player sprite (blue)'
        },
        {
            'filename': 'npc_merchant.png',
            'color': (50, 200, 100),   # Vert
            'text': 'M',
            'description': 'Merchant NPC (green)'
        },
        {
            'filename': 'npc_innkeeper.png',
            'color': (255, 150, 50),   # Orange
            'text': 'I',
            'description': 'Innkeeper NPC (orange)'
        }
    ]

    if not PIL_AVAILABLE:
        print("ERROR: Pillow library is not installed!")
        print()
        print("To install Pillow:")
        print("  pip install Pillow")
        print()
        print("Or create sprites manually (see CREATE_SPRITES.md)")
        return 1

    success_count = 0

    for sprite in sprites:
        filepath = data_dir / sprite['filename']
        print(f"Creating {sprite['description']}...")

        if create_sprite_with_pil(64, 64, sprite['color'], sprite['text'], filepath):
            success_count += 1
        else:
            print(f"  ✗ Failed to create {sprite['filename']}")

    print()
    print("=" * 50)

    if success_count == len(sprites):
        print(f"✓ SUCCESS: All {success_count} sprites created!")
        print()
        print("Sprites created in: assets/data/")
        print()
        print("Next steps:")
        print("  1. Copy a font file to assets/data/arial.ttf")
        print("  2. Run compile.bat to build the game")
        print("  3. Enjoy your Simple RPG!")
    else:
        print(f"⚠️  WARNING: Only {success_count}/{len(sprites)} sprites created")
        print()
        print("Please check errors above or create sprites manually")
        print("See CREATE_SPRITES.md for manual creation instructions")

    print("=" * 50)
    return 0 if success_count == len(sprites) else 1

if __name__ == "__main__":
    exit(main())
