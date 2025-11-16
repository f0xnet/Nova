# Simple RPG Example - NovaEngine

Un exemple de jeu RPG simple démontrant les fonctionnalités de base de NovaEngine.

## Fonctionnalités démontrées

- ✅ Navigation entre 2 scènes (Village et Taverne)
- ✅ 2 NPCs avec dialogues interactifs
- ✅ Système de dialogue simple
- ✅ Détection de proximité pour activation
- ✅ Système ECS (Entity Component System)
- ✅ Chargement de scènes depuis JSON
- ✅ Architecture deux niveaux (Definitions + Instances)

## Contrôles

- **WASD** : Déplacer le joueur
- **E** : Parler aux NPCs / Avancer le dialogue
- **ESC** : Quitter le jeu

## Structure du projet

```
simple_rpg/
├── main.cpp                    # Code source principal
├── compile.bat                 # Script de compilation Windows
├── README.md                   # Ce fichier
└── assets/
    ├── data/
    │   ├── definitions/
    │   │   └── Sprites.json    # Définitions des sprites NPCs
    │   ├── scenes/
    │   │   ├── village.json    # Scène du village
    │   │   └── taverne.json    # Scène de la taverne
    │   └── scenegraph.json     # Connexions entre scènes
    └── sprites/
        ├── player.png          # Sprite du joueur (À CRÉER)
        ├── npc_merchant.png    # Sprite du marchand (À CRÉER)
        └── npc_innkeeper.png   # Sprite du tavernier (À CRÉER)
```

## Avant de compiler

### 1. Créer les sprites

Vous devez créer 3 sprites PNG (64x64 pixels recommandé) :

**assets/data/player.png**
- Sprite du joueur
- Couleur suggérée : Bleu clair (100, 200, 255)
- Peut être un simple carré ou silhouette

**assets/data/npc_merchant.png**
- Sprite du marchand dans le village
- Couleur suggérée : Vert (50, 200, 100)
- Peut être un simple carré ou silhouette

**assets/data/npc_innkeeper.png**
- Sprite du tavernier dans la taverne
- Couleur suggérée : Orange (255, 150, 50)
- Peut être un simple carré ou silhouette

**Option rapide** : Créez des carrés de couleur 64x64 avec n'importe quel éditeur d'image (Paint, GIMP, Photoshop, etc.)

### 2. Créer la police Arial

Copiez une police TrueType dans :
```
assets/data/arial.ttf
```

Vous pouvez :
- Copier depuis `C:\Windows\Fonts\arial.ttf` (Windows)
- Utiliser n'importe quelle autre police .ttf et la renommer en `arial.ttf`

## Compilation

### Windows

1. Assurez-vous que MinGW est installé et dans le PATH
2. Double-cliquez sur `compile.bat`
3. Le jeu sera compilé dans `bin/Release/SimpleRPG.exe`

### Ligne de commande

```bash
compile.bat
```

## Exécution

Après compilation, le jeu sera dans `bin/Release/`.

Pour lancer :
```bash
cd bin/Release
SimpleRPG.exe
```

Ou utilisez l'option "Run the game now" à la fin de la compilation.

## Personnalisation

### Ajouter plus de dialogues

Éditez `assets/data/scenes/village.json` ou `taverne.json` et ajoutez des lignes dans le tableau `dialogueLines` :

```json
"dialogueLines": [
    "Ligne 1 du dialogue",
    "Ligne 2 du dialogue",
    "Ligne 3 du dialogue"
]
```

### Ajouter plus de NPCs

1. Créez un nouveau sprite dans `assets/data/definitions/Sprites.json`
2. Ajoutez une nouvelle entité dans la scène avec les composants :
   - `Transform` (position)
   - `Sprite` (référence à la définition)
   - `Dialogue` (nom et lignes de dialogue)

Exemple :
```json
{
  "id": 101,
  "components": {
    "Transform": {
      "position": [600, 400]
    },
    "Sprite": {
      "definitionID": "mon_nouveau_npc"
    },
    "Dialogue": {
      "npcName": "Guard Captain",
      "dialogueLines": [
        "Halt! State your business.",
        "The roads are dangerous these days."
      ]
    }
  }
}
```

### Modifier les couleurs de fond

Éditez `backgroundColor` dans les fichiers de scène (format RGBA) :

```json
"backgroundColor": [R, G, B, A]
```

Exemples :
- Jour : `[135, 206, 235, 255]` (bleu ciel)
- Nuit : `[20, 20, 40, 255]` (bleu foncé)
- Forêt : `[34, 139, 34, 255]` (vert forêt)
- Donjon : `[40, 40, 40, 255]` (gris sombre)

## Dépendances

Ce projet utilise :
- **NovaEngine SDK** (inclus dans le repo parent)
- **SFML 2.6** (statique, inclus dans `sdk/libs/`)
- **nlohmann/json** (header-only, inclus dans SDK)

## Notes techniques

### Architecture du code

- **DialogueComponent** : Composant personnalisé pour stocker les dialogues des NPCs
- **SimpleRPG::findNearestNPC()** : Détecte le NPC le plus proche dans un rayon de 80 pixels
- **SimpleRPG::drawDialogueBox()** : Dessine la boîte de dialogue avec coordonnées monde
- Les dialogues bloquent le mouvement du joueur pendant qu'ils sont actifs

### Système de dialogue

Le système fonctionne ainsi :
1. Chaque frame, on cherche le NPC le plus proche
2. Si un NPC est à portée (< 80px), on affiche "[ E ]" au-dessus
3. Quand le joueur appuie sur E près d'un NPC, le dialogue démarre
4. Chaque pression sur E fait avancer le dialogue
5. À la fin du dialogue, on revient au jeu normal

## Troubleshooting

### "Failed to load font"
- Vérifiez que `arial.ttf` existe dans `assets/data/`
- Vérifiez les permissions du fichier

### Sprites invisibles
- Vérifiez que les fichiers PNG existent dans `assets/data/`
- Vérifiez que les chemins dans `Sprites.json` sont corrects
- Les sprites doivent avoir de la transparence (canal alpha)

### Compilation échoue
- Vérifiez que MinGW est installé
- Vérifiez que le SDK NovaEngine est présent dans le repo parent
- Vérifiez les chemins dans `compile.bat`

## Améliorations possibles

Idées pour étendre cet exemple :

1. **Système d'inventaire** : Ajouter des objets que le joueur peut ramasser
2. **Combat simple** : Système de combat au tour par tour
3. **Quêtes** : Système de quêtes avec suivi
4. **Plus de scènes** : Maisons, boutiques, donjon
5. **Animations** : Animer les sprites des NPCs et du joueur
6. **Musique** : Ajouter musique de fond et effets sonores
7. **Sauvegarde** : Sauvegarder la progression du joueur

## Licence

Cet exemple est fourni comme démonstration de NovaEngine.
Libre d'utilisation et de modification.
