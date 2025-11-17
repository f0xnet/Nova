# Guide de Création des Sprites

Ce document explique comment créer rapidement les sprites nécessaires pour Simple RPG.

## Option 1 : Utiliser le script Python (RECOMMANDÉ)

Un script Python est fourni pour générer automatiquement des sprites de test.

```bash
python create_sprites.py
```

Cela créera :
- `assets/data/player.png` (carré bleu 64x64)
- `assets/data/npc_merchant.png` (carré vert 64x64)
- `assets/data/npc_innkeeper.png` (carré orange 64x64)

## Option 2 : Créer manuellement avec Paint (Windows)

### 1. Ouvrir Paint
- Windows + R → `mspaint` → Enter

### 2. Configurer la taille
- Redimensionner → Pixels
- Largeur : 64
- Hauteur : 64
- Décocher "Conserver les proportions"

### 3. Créer player.png
1. Remplir avec du bleu clair (RGB: 100, 200, 255)
2. Enregistrer sous → Format PNG
3. Nom : `player.png`
4. Emplacement : `examples/simple_rpg/assets/data/`

### 4. Créer npc_merchant.png
1. Nouveau fichier 64x64
2. Remplir avec du vert (RGB: 50, 200, 100)
3. Enregistrer → `npc_merchant.png`

### 5. Créer npc_innkeeper.png
1. Nouveau fichier 64x64
2. Remplir avec de l'orange (RGB: 255, 150, 50)
3. Enregistrer → `npc_innkeeper.png`

## Option 3 : Créer avec GIMP (Gratuit, multi-plateforme)

### 1. Installer GIMP
Télécharger depuis : https://www.gimp.org/

### 2. Créer un sprite
1. Fichier → Nouvelle image
2. Taille : 64x64 pixels
3. Remplir avec la couleur désirée
4. Fichier → Exporter sous
5. Format : PNG
6. Options : Laisser par défaut

### Couleurs recommandées :
- **player.png** : Bleu clair #64C8FF (100, 200, 255)
- **npc_merchant.png** : Vert #32C864 (50, 200, 100)
- **npc_innkeeper.png** : Orange #FF9632 (255, 150, 50)

## Option 4 : Utiliser des sprites existants

Vous pouvez utiliser n'importe quels sprites 64x64 PNG que vous avez déjà.

Sites de sprites gratuits :
- OpenGameArt.org
- itch.io (section Game Assets)
- Kenney.nl (assets gratuits)

## Vérification

Une fois les sprites créés, vérifiez que vous avez :

```
examples/simple_rpg/assets/data/
├── player.png          ✓
├── npc_merchant.png    ✓
├── npc_innkeeper.png   ✓
└── arial.ttf           ✓
```

## Police Arial

### Windows
Copiez depuis : `C:\Windows\Fonts\arial.ttf`

Vers : `examples/simple_rpg/assets/data/arial.ttf`

### Autre police
N'importe quelle police .ttf peut être utilisée, renommez-la simplement en `arial.ttf`.

## Test rapide

Pour tester si vos sprites sont corrects :

1. Ouvrez les fichiers PNG dans un visualiseur d'images
2. Vérifiez qu'ils font 64x64 pixels
3. Vérifiez qu'ils ne sont pas corrompus

## Sprites avancés (optionnel)

Pour de meilleurs sprites, vous pouvez :

1. **Ajouter des détails** : Dessinez un personnage simple
2. **Utiliser la transparence** : Canal alpha pour les contours
3. **Animer** : Créer plusieurs frames (pour plus tard)
4. **Pixel art** : Utiliser Aseprite ou Pyxel Edit

### Logiciels recommandés pour pixel art :
- **Aseprite** (payant, ~20$) : https://www.aseprite.org/
- **Piskel** (gratuit, web) : https://www.piskelapp.com/
- **GraphicsGale** (gratuit) : https://graphicsgale.com/

## Troubleshooting

### "Failed to load texture"
- Vérifiez que les fichiers sont bien des PNG
- Vérifiez les permissions des fichiers
- Vérifiez que les noms correspondent exactement

### Sprites trop petits/grands à l'écran
- Modifiez la valeur `size` dans le code ou JSON
- Les sprites 64x64 devraient bien s'afficher par défaut

### Sprites pixelisés
- C'est normal pour les petits sprites
- Pour des sprites HD, utilisez 128x128 ou plus
