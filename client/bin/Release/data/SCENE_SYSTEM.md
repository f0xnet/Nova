# NovaEngine - Scene System Guide

## Structure des fichiers

```
data/
├── sprites/           # Fichiers images (.png)
│   ├── player.png
│   ├── box.png
│   └── wall.png
├── definitions/       # Définitions de sprites réutilisables
│   └── Sprites.json
└── scenes/           # Fichiers de scènes
    └── test.json
```

## Format simplifié

### 1. Définitions de sprites (data/definitions/Sprites.json)

**Format minimal** :
```json
{
  "sprites": [
    {
      "id": "player",
      "texture": "data/sprites/player.png",
      "width": 64,
      "height": 64
    }
  ]
}
```

**Champs disponibles** :
- `id` (requis) : Identifiant unique du sprite
- `texture` (requis) : Chemin vers l'image PNG
- `width`, `height` (recommandés) : Taille du sprite en pixels logiques
- `scale` (optionnel) : Échelle uniforme (ex: 2.0 = double taille)
- `zOrder` (optionnel) : Ordre de rendu (-10 = fond, 10 = premier plan)
- `textureRect` (optionnel) : Pour tilesheets `[x, y, width, height]`
- `origin` (optionnel) : Point d'ancrage `[x, y]`, par défaut = centre

**Anciens formats supportés** (rétro-compatibilité) :
```json
{
  "size": [64, 64],       // Au lieu de width/height
  "scale": [1.0, 1.0]     // Au lieu de scale uniforme
}
```

### 2. Scènes (data/scenes/test.json)

**Format minimal** :
```json
{
  "name": "Test Scene",
  "backgroundColor": [30, 30, 40, 255],
  "entities": [
    {
      "type": "sprite",
      "spriteID": "player",
      "x": 1920,
      "y": 1080
    }
  ]
}
```

**Champs d'entité** :
- `type` (requis) : "sprite", "light", "audio", etc.
- `spriteID` (requis pour type sprite) : ID depuis Sprites.json
- `x`, `y` (requis) : Position en pixels logiques
- `scale` (optionnel) : Override l'échelle de la définition
- `rotation` (optionnel) : Rotation en degrés
- `zOrder` (optionnel) : Override le zOrder de la définition

**Ancien format supporté** :
```json
{
  "position": [1920, 1080]  // Au lieu de x, y séparés
}
```

## Résolution logique

Le jeu utilise une résolution logique (définie dans config/engine.ini : `nativeWidth` x `nativeHeight`).

- Par défaut : 3840 x 2160 (4K)
- Les positions/tailles sont en "pixels logiques"
- Le moteur scale automatiquement vers la résolution réelle de la fenêtre
- Le letterboxing préserve l'aspect ratio 16:9 sur tous les écrans

**Exemple** :
```
Position logique : (1920, 1080) = centre de l'écran 3840x2160
Sur écran 1920x1080 → (960, 540) en pixels réels
Sur écran 3840x2160 → (1920, 1080) en pixels réels
```

## Charger des textures

1. **Placer l'image** dans `data/sprites/`
2. **Créer la définition** dans `data/definitions/Sprites.json`
3. **Utiliser dans une scène** via `spriteID`

**Exemple complet** :

```bash
# 1. Copier l'image
cp player.png client/bin/Release/data/sprites/
```

```json
// 2. Ajouter dans Sprites.json
{
  "id": "player",
  "texture": "data/sprites/player.png",
  "width": 114,
  "height": 225
}
```

```json
// 3. Utiliser dans test.json
{
  "type": "sprite",
  "spriteID": "player",
  "x": 1920,
  "y": 1080
}
```

## Astuces

- **Centre de l'écran** : `x: 1920, y: 1080` (pour 3840x2160)
- **Scale 2x** : `"scale": 2.0` double la taille
- **Premier plan** : `"zOrder": 100`
- **Arrière-plan** : `"zOrder": -100`
- **Origin personnalisée** : `"origin": [0, 0]` = coin haut-gauche
