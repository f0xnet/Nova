# NovaEngine

Moteur de jeu 2D écrit en C++17, construit sur SFML 2.6 via une couche d'abstraction backend swappable.

## Architecture

- **ECS** — Entity-Component-System avec 11 composants et 7 systèmes built-in
- **Backend abstrait** — 7 interfaces (Window, Input, Graphics, Audio, Font, Viewport, Resource) implémentées en SFML
- **Post-processing** — pipeline ping-pong (SSAO, Bloom, ColorGrading, DynamicLighting, CRT)
- **UI déclaratif** — composants chargés depuis JSON via `UILoader`
- **SceneGraph + JourneySystem** — voyages NPC multi-scènes avec pathfinding

## Build

**Linux** (cross-compile vers Windows via MinGW) :
```bash
bash compile_client.sh
```

**Windows** :
```bat
client__compile.bat
```

## Lancement

```bash
cd client/bin/Release
./Nova.exe
```

Les chemins sont relatifs au cwd — le binaire doit être lancé depuis `client/bin/Release/`.

## Documentation interne

Voir `CLAUDE.md` pour la référence technique complète (architecture, ECS, backend, pipeline, UI, gotchas, cookbook).

