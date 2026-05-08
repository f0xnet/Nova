# NovaEngine — Guide de Scripting Lua

Guide complet de l'API Lua de NovaEngine. Tous les modules listés ici sont des **globaux
auto-chargés** : aucun `require` n'est nécessaire, ils sont disponibles dans n'importe
quel script dès le premier frame.

---

## Table des Matières

1. [Architecture des scripts](#1-architecture-des-scripts)
2. [Named Handlers — le cycle de vie d'un script](#2-named-handlers--le-cycle-de-vie-dun-script)
3. [EventBus — bus d'événements](#3-eventbus--bus-dévénements)
4. [Timer — délais et répétitions](#4-timer--délais-et-répétitions)
5. [Scheduler — coroutines asynchrones](#5-scheduler--coroutines-asynchrones)
6. [Scene — gestion des scènes](#6-scene--gestion-des-scènes)
7. [World — entités dans la scène](#7-world--entités-dans-la-scène)
8. [Spatial — grille de proximité](#8-spatial--grille-de-proximité)
9. [Camera — contrôle de la caméra](#9-camera--contrôle-de-la-caméra)
10. [Stats — valeurs d'acteur](#10-stats--valeurs-dacteur)
11. [Effect — effets de statut](#11-effect--effets-de-statut)
12. [Cooldown — gestion des temps de recharge](#12-cooldown--gestion-des-temps-de-recharge)
13. [Anim — animations](#13-anim--animations)
14. [Tween — interpolations fluides](#14-tween--interpolations-fluides)
15. [Sound — audio](#15-sound--audio)
16. [UI — interface utilisateur](#16-ui--interface-utilisateur)
17. [Particles — système de particules](#17-particles--système-de-particules)
18. [Projectile — tirs et projectiles](#18-projectile--tirs-et-projectiles)
19. [Trigger — zones de déclenchement](#19-trigger--zones-de-déclenchement)
20. [Pool — pooling d'entités](#20-pool--pooling-dentités)
21. [Nav — navigation et pathfinding](#21-nav--navigation-et-pathfinding)
22. [SceneFX — effets visuels plein-écran](#22-scenefx--effets-visuels-plein-écran)
23. [Sequence — cutscènes et séquenceurs](#23-sequence--cutscènes-et-séquenceurs)
24. [StateMachine — machines à états finis](#24-statemachine--machines-à-états-finis)
25. [Data — chargement de données JSON](#25-data--chargement-de-données-json)
26. [Flag — drapeaux booléens](#26-flag--drapeaux-booléens)
27. [Persist — données persistantes](#27-persist--données-persistantes)
28. [Game — fonctions globales du jeu](#28-game--fonctions-globales-du-jeu)
29. [InputEx — saisie frame-parfaite](#29-inputex--saisie-frame-parfaite)
30. [Notify — notifications et textes flottants](#30-notify--notifications-et-textes-flottants)
31. [Patterns avancés](#31-patterns-avancés)
32. [Garanties de nettoyage automatique](#32-garanties-de-nettoyage-automatique)

---

## 1. Architecture des scripts

### Types de scripts

Il existe deux grandes catégories de scripts dans NovaEngine :

**Scripts d'entité** — attachés à un `ScriptComponent` sur une entité ECS. Ils reçoivent
un objet `entity` en paramètre de leurs handlers et leur cycle de vie est lié à l'entité.

**Scripts de scène** — chargés directement par la scène (ex: `intro.lua`, `player.lua`).
Ils s'exécutent dans le contexte global de la scène.

### Globals disponibles

| Global | Description |
|--------|-------------|
| `EventBus` | Bus d'événements partagé |
| `Timer` | Timers one-shot et répétitifs |
| `Scheduler` | Coroutines asynchrones |
| `Scene` | Gestion des scènes |
| `World` | Recherche et manipulation d'entités |
| `Spatial` | Grille spatiale de proximité |
| `Camera` | Contrôle de la caméra |
| `Stats` | Valeurs d'acteur par entité |
| `Effect` | Effets de statut temporaires |
| `Cooldown` | Temps de recharge par action |
| `Anim` | Contrôleur d'animation |
| `Tween` | Interpolations fluides |
| `Sound` | Lecture audio |
| `UI` | Interface utilisateur |
| `Particles` | Système de particules 2D |
| `Projectile` | Tirs poolés |
| `Trigger` | Zones de déclenchement |
| `Pool` | Pooling d'entités |
| `Nav` | Navigation par waypoints |
| `SceneFX` | Effets visuels plein-écran |
| `Sequence` | Séquenceur de cutscènes |
| `StateMachine` | Machines à états finis |
| `Data` | Chargement de fichiers JSON |
| `Flag` | Drapeaux booléens |
| `Persist` | Données persistantes |
| `Game` | Fonctions globales du jeu |
| `InputEx` | Détection frame-parfaite des touches |
| `Notify` | Notifications écran |
| `Input` | Saisie clavier/souris (C++) |
| `Registry` | Registre d'entités (C++) |
| `Log` | Journalisation |
| `Debug` | Dessin de débogage |
| `Viewport` | Informations d'affichage |
| `Resources` | Chargement de ressources |
| `Color` | Couleurs RGBA |

---

## 2. Named Handlers — le cycle de vie d'un script

Les scripts d'entité peuvent définir des fonctions spéciales (**named handlers**) qui sont
automatiquement câblées par le moteur. Vous n'avez pas à les enregistrer manuellement.

### Fonctions disponibles

```lua
-- Appelé une seule fois à l'initialisation de l'entité
function OnInit(entity)
    Stats.init(entity.id, { Health = 100, Speed = 120 })
    Anim.play(entity, "idle", true)
end

-- Appelé chaque frame (éviter les opérations lourdes ici)
function OnUpdate(entity, dt)
    if Input.isKeyPressed("Z") then
        movePlayer(entity, dt)
    end
end

-- Appelé à la destruction de l'entité
function OnDestroy(entity)
    Particles.emit(entity:getPosition().x, entity:getPosition().y, {
        count = 20, speed = 100
    })
end

-- Appelé quand l'entité reçoit un message via entity:sendMessage()
function OnMessage(entity, msg, data)
    if msg == "takeDamage" then
        local hp = Stats.mod(entity.id, "Health", -data.amount)
        if hp <= 0 then entity:sendMessage("die") end
    end
end

-- Appelé quand une touche est enfoncée
function OnKeyDown(entity, key)
    if key == "Space" then jump(entity) end
end

-- Appelé quand une touche est relâchée
function OnKeyUp(entity, key)
    if key == "Space" then landAnimation(entity) end
end

-- Appelé lors d'une collision (nécessite un ColliderComponent)
function OnCollision(entity, other)
    if other:hasTag("enemy") then
        Stats.mod(entity.id, "Health", -10)
    end
end

-- Appelé quand le joueur active l'entité (ActivatorComponent)
function OnActivate(entity)
    EventBus.emit("dialogue_start", { speaker = "npc_guard", text = "Halte !" })
end

-- Appelé à la fin d'une animation non-loop
function OnAnimEnd(entity, animName)
    if animName == "attack" then
        Anim.play(entity, "idle", true)
    end
end
```

### Nettoyage automatique

Quand une entité est détruite (`Registry.destroyEntity(id)`), le moteur émet
`entity_destroyed` **avant** la suppression, ce qui déclenche automatiquement :
- Désinscription de tous les named handlers de EventBus
- Suppression de l'environnement Lua de l'entité
- `Stats.clear(id)`, `Effect.clear(id)`, `Cooldown.resetAll(id)`, `Anim.clearCallbacks(id)`

Vous n'avez pas à gérer ce nettoyage manuellement.

### Envoyer des messages entre entités

```lua
-- Depuis n'importe quel script :
local boss = World.findByTag("boss")
if boss then
    boss:sendMessage("takeDamage", { amount = 50, type = "fire" })
end

-- Le script du boss reçoit dans OnMessage :
function OnMessage(entity, msg, data)
    if msg == "takeDamage" then
        -- data.amount = 50, data.type = "fire"
    end
end
```

---

## 3. EventBus — bus d'événements

L'EventBus est le système de communication central. Tous les modules l'utilisent pour
notifier les changements d'état.

### API complète

```lua
-- S'abonner à un événement (handler persistant)
EventBus.on("player_died", function(data)
    Log.info("Le joueur est mort à " .. data.x .. ", " .. data.y)
end)

-- S'abonner une seule fois (auto-désinscription après le premier appel)
EventBus.once("boss_defeated", function(data)
    Scene.transition("victory_screen")
end)

-- Se désabonner (nécessite une référence à la fonction)
local function onDamage(data) end
EventBus.on("damage", onDamage)
EventBus.off("damage", onDamage)

-- Émettre un événement
EventBus.emit("player_died", { x = 100, y = 200 })
EventBus.emit("simple_event")  -- data est optionnel

-- Supprimer tous les handlers d'un événement
EventBus.clear("damage")

-- Supprimer tous les handlers de tous les événements
EventBus.clear()
```

### Scene.listen — handlers à portée de scène

Pour les scripts de scène, préférez `Scene.listen()` à `EventBus.on()`. Les handlers
enregistrés via `Scene.listen()` sont automatiquement retirés au changement de scène.

```lua
-- ❌ Risque de fuite — le handler survit au changement de scène
EventBus.on("enemy_killed", function(data) updateKillCount(data) end)

-- ✅ Nettoyage automatique — retiré quand la scène change
Scene.listen("enemy_killed", function(data) updateKillCount(data) end)
```

### Événements système émis par les modules

| Événement | Données | Émetteur |
|-----------|---------|---------|
| `scene_changing` | `{ from, to }` | Scene |
| `scene_changed` | `{ name }` | Scene |
| `entity_destroyed` | `{ entityId }` | Registry |
| `stat_changed` | `{ entityId, stat, value, previous }` | Stats |
| `stat_zeroed` | `{ entityId, stat }` | Stats |
| `effect_applied` | `{ entityId, effectId }` | Effect |
| `effect_expired` | `{ entityId, effectId }` | Effect |
| `effect_removed` | `{ entityId, effectId }` | Effect |
| `cooldown_ready` | `{ entityId, action }` | Cooldown |
| `trigger_enter` | `{ id, entityId }` | Trigger |
| `trigger_exit` | `{ id, entityId }` | Trigger |
| `flag_set` | `{ name }` | Flag |
| `flag_unset` | `{ name }` | Flag |
| `ui_notification` | `{ text, duration, color, size }` | Notify |
| `floating_text` | `{ entityId, text, … }` | Notify |
| `dialogue_start` | `{ speaker, text }` | Sequence |
| `dialogue_end` | `{ speaker }` | Dialogue system |

---

## 4. Timer — délais et répétitions

Le module Timer permet d'exécuter des fonctions après un délai ou à intervalles réguliers.
Tous les timers sont automatiquement liés à la scène courante et annulés au changement de scène.

### Fonctions de base

```lua
-- Exécuter une fonction une seule fois après 2 secondes
Timer.after(2.0, function()
    Log.info("2 secondes écoulées !")
end)

-- Exécuter une fonction toutes les 5 secondes
local id = Timer.every(5.0, function()
    spawnEnemy()
end)

-- Annuler un timer spécifique
Timer.cancel(id)
```

### Gestion des scopes

```lua
-- Auto-scopé à Scene.current() — annulé automatiquement au changement de scène
Timer.after(3.0, fn)

-- Scopé explicitement à une entité — utile dans un script d'entité
Timer.after(1.0, function()
    Anim.play(entity, "idle", true)
end, tostring(entity.id))

-- Annuler tous les timers d'un scope
Timer.cancelScope("dungeon_01")

-- Annuler TOUS les timers (reset global — rarement nécessaire)
Timer.cancelAll()
```

### Exemple : ennemi qui tire en rafale

```lua
function OnInit(entity)
    -- Tire une rafale de 3 projectiles toutes les 4 secondes
    Timer.every(4.0, function()
        for i = 1, 3 do
            Timer.after((i-1) * 0.2, function()
                Projectile.fire(entity:getPosition().x, entity:getPosition().y, {
                    angle = 270, speed = 300, lifetime = 2.0
                })
            end)
        end
    end, tostring(entity.id))
end
```

---

## 5. Scheduler — coroutines asynchrones

Le Scheduler permet d'écrire des séquences d'actions avec des pauses, sans callbacks
imbriqués. Il utilise les coroutines Lua.

### Syntaxe de base

```lua
Scheduler.start(function()
    Log.info("Début")
    Scheduler.wait(2.0)      -- suspend 2 secondes
    Log.info("Après 2s")
    Scheduler.wait(1.0)      -- suspend 1 seconde
    Log.info("Après 3s au total")
end)
```

### Avec un scope explicite

```lua
-- Scopé à une entité — annulé automatiquement si l'entité est détruite
Scheduler.start(function()
    Anim.play(entity, "attack", false)
    Scheduler.wait(0.5)
    Stats.mod(target.id, "Health", -damage)
    Scheduler.wait(0.3)
    Anim.play(entity, "idle", true)
end, tostring(entity.id))
```

### Exemple : dialogue cinématique

```lua
Scheduler.start(function()
    -- Désactive le contrôle du joueur
    Flag.set("player_locked")
    
    -- Déplace la caméra vers le PNJ
    Camera.moveTo(npc:getPosition().x, npc:getPosition().y, 1.5, "easeInOut")
    Scheduler.wait(1.5)
    
    -- Démarre le dialogue
    EventBus.emit("dialogue_start", { speaker = "elder", text = "L'ennemi approche..." })
    Scheduler.wait(3.0)
    
    -- Retour au joueur
    Camera.follow(Game.getPlayer())
    Flag.unset("player_locked")
end)
```

### Annulation

```lua
-- Annuler toutes les coroutines de la scène courante
Scheduler.cancelScope(Scene.current())

-- Annuler toutes les coroutines (reset global)
Scheduler.clear()
```

---

## 6. Scene — gestion des scènes

### Charger et activer une scène

```lua
-- Chargement depuis un fichier JSON
Scene.load("data/scenes/dungeon_01.scene", "dungeon_01")

-- Activation immédiate (sans transition)
Scene.setActive("dungeon_01")

-- Transition avec fondu au noir (recommandé)
Scene.transition("dungeon_01")

-- Transition personnalisée
Scene.transition("dungeon_01", {
    fade     = true,        -- fondu au noir (défaut: true)
    duration = 0.6,         -- durée de chaque demi-fondu en secondes
    onBeforeChange = function()
        -- Appelé pendant le noir, avant le changement de scène
        UI.removeUI("hud")
        Sound.stopMusic()
    end
})
```

### Navigation et état

```lua
-- Nom de la scène courante
local nom = Scene.current()     -- ex: "dungeon_01"

-- Alias utile pour tagger des timers manuellement
Timer.after(1.0, fn, Scene.scope())

-- Vérifier si une scène est chargée
if Scene.has("dungeon_02") then
    Scene.transition("dungeon_02")
end

-- Nombre de scènes chargées en mémoire
Log.info("Scènes en mémoire : " .. Scene.count())

-- Décharger une scène
Scene.unload("dungeon_01")

-- Re-activer la scène courante (force un rechargement des scripts)
Scene.reload()
```

### Réagir aux changements de scène

```lua
-- Depuis n'importe quel script — attention, cet handler survit au changement de scène !
EventBus.on("scene_changed", function(data)
    Log.info("Scène active : " .. data.name)
end)

-- Depuis un script de scène — utiliser Scene.listen pour auto-cleanup
Scene.listen("scene_changing", function(data)
    Log.info("Transition de " .. data.from .. " vers " .. data.to)
end)
```

### Pathfinding via WaypointGraph

```lua
-- Calcule un chemin entre deux positions (via le WaypointGraph de la scène active)
local path = Scene.findPath(100, 200, 500, 300)
-- path = { {x=100,y=200}, {x=200,y=250}, ..., {x=500,y=300} }

Nav.moveAlongPath(entity, path, 80)
```

---

## 7. World — entités dans la scène

World est le module principal pour trouver et manipuler des entités par tag.

### Recherche d'entités

```lua
-- Trouver la première entité avec un tag
local player = World.findByTag("player")
if player then
    Log.info("Joueur trouvé : " .. player.id)
end

-- Trouver toutes les entités avec un tag
local enemies = World.findAllByTag("enemy")
Log.info(#enemies .. " ennemis en vie")

-- Itérer sur les entités (stop si callback retourne false)
World.forEach("enemy", function(enemy)
    Stats.mod(enemy.id, "Health", -5)
    return true  -- continuer
end)

-- Compter les entités
local count = World.count("coin")
```

### Entité la plus proche

```lua
local player = World.findByTag("player")
local pos    = player:getPosition()

-- Trouver l'ennemi le plus proche
local nearest, dist = World.nearest(player, "enemy")
if nearest and dist < 150 then
    nearest:sendMessage("attack", { target = player.id })
end
```

### Positions et distances

```lua
-- Obtenir la position d'une entité
local pos = World.getPosition(entity)   -- { x = 200, y = 300 }
local x, y = pos.x, pos.y

-- Téléporter une entité
World.setPosition(entity, 400, 200)

-- Distance entre deux entités
local d = World.distance(player, boss)
if d < 100 then
    Log.info("Corps à corps !")
end
```

### Création et destruction

```lua
-- Créer une entité (par type défini dans les definitions JSON)
local coin = World.spawn("CoinEntity")
World.setPosition(coin, 250, 180)

-- Détruire une entité (émet entity_destroyed puis la retire du registre)
World.destroy(coin.id)
```

### Raycast

```lua
-- Lancer un rayon et détecter la première entité touchée
local hit = World.raycast(player.x, player.y, target.x, target.y, "enemy")
if hit then
    Log.info("Touché : entité " .. hit.entity.id)
    Log.info("Point d'impact : " .. hit.x .. ", " .. hit.y)
    Log.info("Distance : " .. hit.distance)
end
```

---

## 8. Spatial — grille de proximité

Le module Spatial fournit des requêtes de proximité optimisées via une grille de hachage
spatiale. La grille est reconstruite une fois par frame — les requêtes sont O(cellules),
pas O(n entités).

### Requêtes de base

```lua
-- Toutes les entités dans un rayon
local nearby = Spatial.findInRadius(x, y, 96, "enemy")
for _, e in ipairs(nearby) do
    Stats.mod(e.id, "Health", -blast_damage)
end

-- Toutes les entités dans un rectangle
local inZone = Spatial.findInRect(200, 150, 300, 200, "item")

-- Entité la plus proche (sans tag = toutes)
local nearest, dist = Spatial.nearest(x, y, "npc")
if nearest and dist < 120 then
    Nav.moveTo(entity, nearest, 80)
end

-- Compter les entités (nil = toutes)
Log.info("Ennemis total : " .. Spatial.count("enemy"))
```

### Configuration

```lua
-- Taille de cellule (modifier avant le premier frame)
Spatial.CELL_SIZE = 128   -- défaut: 64
```

> **Préférer Spatial à World pour les requêtes de proximité répétées dans la même frame.**
> `World.nearest` itère toutes les entités ; `Spatial.nearest` examine seulement les
> cellules voisines.

---

## 9. Camera — contrôle de la caméra

### Suivre une entité

```lua
-- Suivre le joueur
Camera.follow(Game.getPlayer())

-- Arrêter le suivi
Camera.unfollow()
```

### Déplacements

```lua
-- Téléporter la caméra (désactive le follow)
Camera.setPosition(500, 300)

-- Obtenir la position courante
local pos = Camera.getPosition()   -- { x, y }

-- Déplacement fluide (utilise Tween en interne)
Camera.moveTo(500, 300, 2.0, "easeInOut")
```

### Zoom

```lua
-- Zoom instantané (1.0 = normal, 2.0 = x2, 0.5 = dézoom)
Camera.setZoom(1.5)
local scale = Camera.getZoom()

-- Zoom progressif
Camera.zoomTo(2.0, 1.0, "easeOut")
```

### Effets

```lua
-- Tremblement d'écran (ex: explosion)
Camera.shake(0.4, 8)   -- durée 0.4s, force 8 pixels

-- Retour aux paramètres par défaut
Camera.reset()
```

### Chemin cinématique

```lua
-- Déplace la caméra le long d'un chemin de waypoints
Camera.path(
    { {x=100, y=100}, {x=400, y=200}, {x=600, y=300} },
    200,           -- vitesse px/s
    "easeInOut",   -- easing
    function()     -- callback de fin
        Camera.follow(Game.getPlayer())
    end
)
```

### Exemple complet : intro de boss

```lua
Scheduler.start(function()
    local player = Game.getPlayer()
    local boss   = World.findByTag("boss")
    
    -- Désactive l'input
    Flag.set("cutscene_active")
    
    -- Zoom et déplace vers le boss
    Camera.zoomTo(1.8, 1.5, "easeInOut")
    Camera.moveTo(boss:getPosition().x, boss:getPosition().y, 2.0, "easeInOut")
    Scheduler.wait(2.0)
    
    -- Tremblement dramatique
    Camera.shake(0.5, 12)
    Sound.play("data/sounds/boss_roar.ogg")
    Scheduler.wait(1.5)
    
    -- Retour au joueur
    Camera.moveTo(player:getPosition().x, player:getPosition().y, 1.5, "easeInOut")
    Camera.zoomTo(1.0, 1.5, "easeOut")
    Scheduler.wait(1.5)
    
    Camera.follow(player)
    Flag.unset("cutscene_active")
end)
```

---

## 10. Stats — valeurs d'acteur

Stats est le système de valeurs d'acteur numérique (Health, Stamina, Mana, Level, etc.)
inspiré de `GetActorValue` / `ModActorValue` de Papyrus.

### Lecture et écriture

```lua
-- Définir une valeur
Stats.set(entity.id, "Health",  100)
Stats.set(entity.id, "MaxHealth", 100)
Stats.set(entity.id, "Speed",   80)

-- Lire une valeur (avec valeur par défaut)
local hp    = Stats.get(entity.id, "Health", 0)
local speed = Stats.get(entity.id, "Speed",  60)

-- Vérifier si une stat existe
if Stats.has(entity.id, "Mana") then
    Stats.mod(entity.id, "Mana", -10)
end
```

### Modifications

```lua
-- Modifier par delta, retourne la nouvelle valeur
local newHp = Stats.mod(entity.id, "Health", -15)
if newHp <= 0 then
    entity:sendMessage("die")
end

-- Modifier avec borne min/max
local newSp = Stats.modClamped(entity.id, "Stamina", -20, 0, 100)
-- Résultat garanti dans [0, 100]
```

### Initialisation et suppression

```lua
-- Initialise sans écraser les valeurs déjà définies
-- Utile pour appliquer des définitions JSON sans perdre les états sauvegardés
Stats.init(entity.id, {
    Health    = 100,
    MaxHealth = 100,
    Stamina   = 50,
    Level     = 1,
})

-- Supprimer une stat
Stats.remove(entity.id, "TempBuff")

-- Obtenir toutes les stats d'une entité
local all = Stats.getAll(entity.id)
for stat, val in pairs(all) do
    Log.info(stat .. " = " .. val)
end

-- Supprimer toutes les stats (appelé auto à entity_destroyed)
Stats.clear(entity.id)
```

### Réagir aux changements

```lua
EventBus.on("stat_changed", function(data)
    if data.stat == "Health" then
        UI.setText("hud_hp", "HP: " .. data.value)
    end
end)

-- Quand une stat atteint 0
EventBus.on("stat_zeroed", function(data)
    if data.stat == "Health" then
        World.destroy(data.entityId)
    end
end)
```

### Exemple complet : ennemi avec stats

```lua
function OnInit(entity)
    Stats.init(entity.id, { Health = 60, Speed = 70, Armor = 5 })
end

function OnMessage(entity, msg, data)
    if msg == "takeDamage" then
        local armor  = Stats.get(entity.id, "Armor", 0)
        local actual = math.max(1, data.amount - armor)
        local hp     = Stats.mod(entity.id, "Health", -actual)
        
        Notify.floatingText(entity, "-" .. actual, { color = "red" })
        
        if hp <= 0 then
            Sound.play("data/sounds/enemy_death.ogg")
            Particles.emit(entity:getPosition().x, entity:getPosition().y, {
                count = 15, speed = 80, color = Color.new(200, 0, 0)
            })
            World.destroy(entity.id)
        end
    end
end
```

---

## 11. Effect — effets de statut

Le module Effect gère les effets temporaires (poison, ralentissement, bouclier, feu, etc.)
avec des callbacks d'application, de tick et d'expiration.

### Appliquer un effet

```lua
-- Poison : 3 dégâts/seconde pendant 5 secondes
Effect.apply(entity.id, "poisoned", {
    duration = 5.0,
    onApply  = function(eid)
        Anim.play(World.findById(eid), "poisoned", true)
        Notify.floatingText(World.findById(eid), "Empoisonné !", { color = "#55ff00" })
    end,
    onTick   = function(eid, dt)
        Stats.mod(eid, "Health", -3 * dt)
    end,
    onExpire = function(eid)
        Anim.play(World.findById(eid), "idle", true)
    end,
})

-- Bouclier permanent (duration = -1)
Effect.apply(entity.id, "shielded", {
    duration = -1,
    onApply  = function(eid) Stats.set(eid, "Armor", 20) end,
    onExpire = function(eid) Stats.set(eid, "Armor", 0)  end,
})

-- Ralentissement — empilable (stackable = true)
Effect.apply(entity.id, "slowed", {
    duration  = 3.0,
    stackable = true,
    onApply   = function(eid) Stats.mod(eid, "Speed", -30) end,
    onExpire  = function(eid) Stats.mod(eid, "Speed",  30) end,
})
```

### Gestion des effets

```lua
-- Vérifier si un effet est actif
if Effect.has(entity.id, "poisoned") then
    Log.info("L'entité est empoisonnée !")
end

-- Temps restant
local rem = Effect.getRemaining(entity.id, "poisoned")
Log.info(string.format("Poison : %.1fs restant", rem))

-- Retirer un effet spécifique
Effect.remove(entity.id, "shielded")

-- Retirer tous les effets (onExpire est appelé pour chacun)
Effect.clear(entity.id)
```

### Réagir aux événements d'effet

```lua
EventBus.on("effect_applied", function(data)
    Log.info("Effet '" .. data.effectId .. "' sur " .. data.entityId)
end)

EventBus.on("effect_expired", function(data)
    if data.effectId == "invincible" then
        -- L'entité redevient vulnérable
    end
end)
```

---

## 12. Cooldown — gestion des temps de recharge

### Usage de base

```lua
function OnUpdate(entity, dt)
    -- Attaque toutes les 1.5 secondes
    if Input.isKeyPressed("Space") then
        if Cooldown.use(entity.id, "attack", 1.5) then
            performAttack(entity)
        end
    end
end
```

`Cooldown.use()` vérifie si le cooldown est prêt ET le déclenche en une seule opération.
Retourne `true` si l'action peut s'exécuter, `false` sinon.

### API complète

```lua
-- Vérifier sans déclencher
if Cooldown.isReady(entity.id, "dash") then
    -- Le dash est disponible
end

-- Déclencher sans vérifier (utile pour les CD déclenchés par le jeu, pas le joueur)
Cooldown.start(entity.id, "respawn", 10.0)

-- Temps restant en secondes
local rem = Cooldown.getRemaining(entity.id, "heal")
UI.setText("cd_heal", string.format("%.1f", rem))

-- Progression 0..1 (0 = vient de commencer, 1 = prêt)
local prog = Cooldown.getProgress(entity.id, "ultimate", 30.0)
UI.setSlider("cd_ult_bar", prog)

-- Annuler un cooldown (remet à "prêt" immédiatement)
Cooldown.reset(entity.id, "attack")

-- Annuler tous les cooldowns d'une entité
Cooldown.resetAll(entity.id)
```

### Réagir à la fin d'un cooldown

```lua
EventBus.on("cooldown_ready", function(data)
    if data.entityId == player.id and data.action == "ultimate" then
        Notify.show("Ultime disponible !", { color = "gold" })
    end
end)
```

### Exemple : soin avec affichage HUD

```lua
function OnKeyDown(entity, key)
    if key == "H" then
        if Cooldown.use(entity.id, "heal", 10.0) then
            local healed = Stats.modClamped(entity.id, "Health", 30, 0, 100)
            Notify.floatingText(entity, "+" .. 30 .. " HP", { color = "#00ff88" })
            Sound.play("data/sounds/heal.ogg")
        else
            local rem = Cooldown.getRemaining(entity.id, "heal")
            Notify.show(string.format("Soin en CD : %.1fs", rem), { color = "gray" })
        end
    end
end
```

---

## 13. Anim — animations

### Contrôle de base

```lua
-- Jouer une animation (loop = true par défaut)
Anim.play(entity, "run", true)
Anim.play(entity, "attack", false)   -- non-loop

-- Arrêter l'animation
Anim.stop(entity)

-- Vitesse de lecture (1.0 = normale, 0.5 = ralenti, 2.0 = accéléré)
Anim.setSpeed(entity, 0.5)

-- Nom de l'animation courante
local name = Anim.current(entity)   -- ex: "run"
```

### Callbacks

```lua
-- Callback quand une animation non-loop se termine
Anim.onFinish(entity, function()
    Anim.play(entity, "idle", true)
end)

-- Callback au frame N (one-shot — se déclenche une seule fois)
Anim.onFrame(entity, 3, function()
    -- Frame 3 de l'animation : déclencher l'impact
    Sound.play("data/sounds/sword_swing.ogg")
    local pos = World.nearest(entity, "enemy")
    if pos then
        pos:sendMessage("takeDamage", { amount = 20 })
    end
end)

-- Supprimer tous les callbacks d'une entité
Anim.clearCallbacks(entity)
```

### Exemple : cycle attaque

```lua
local _isAttacking = false

function OnKeyDown(entity, key)
    if key == "Space" and not _isAttacking then
        if Cooldown.use(entity.id, "attack", 0.8) then
            _isAttacking = true
            Anim.play(entity, "attack", false)
            
            -- Impact au frame 4
            Anim.onFrame(entity, 4, function()
                local hit = World.nearest(entity, "enemy")
                if hit and World.distance(entity, hit) < 80 then
                    hit:sendMessage("takeDamage", { amount = 25 })
                end
            end)
            
            -- Retour à idle à la fin
            Anim.onFinish(entity, function()
                _isAttacking = false
                Anim.play(entity, "idle", true)
            end)
        end
    end
end
```

---

## 14. Tween — interpolations fluides

Le module Tween interpole des valeurs dans le temps avec des courbes d'accélération.

### Fonctions disponibles

| Easing | Effet |
|--------|-------|
| `"linear"` | Vitesse constante |
| `"easeIn"` | Démarre lentement, accélère |
| `"easeOut"` | Démarre vite, décélère |
| `"easeInOut"` | Démarre et finit lentement |
| `"easeInCubic"` | Easing cubique en entrée |
| `"easeOutCubic"` | Easing cubique en sortie |
| `"bounce"` | Rebond à l'arrivée |

### Tween scalaire

```lua
-- Interpole une valeur de 0 à 255 en 1 seconde
local id = Tween.new(0, 255, 1.0, "easeOut",
    function(v)
        -- v évolue de 0 à 255
        entity:getSprite().color.a = math.floor(v)
    end,
    function()
        -- Fin de l'interpolation
        Log.info("Fondu terminé")
    end
)

-- Annuler un tween
Tween.cancel(id)
```

### Tween vectoriel

```lua
-- Déplace une entité de sa position courante à (400, 200) en 2 secondes
local pos = entity:getPosition()
Tween.newVec2(
    { x = pos.x, y = pos.y },
    { x = 400,   y = 200   },
    2.0, "easeInOut",
    function(v)
        World.setPosition(entity, v.x, v.y)
    end
)
```

### Exemple : fade-in d'un sprite

```lua
function OnInit(entity)
    -- L'entité apparaît progressivement en 0.5 secondes
    entity:getSprite().color.a = 0
    Tween.new(0, 255, 0.5, "easeOut", function(v)
        entity:getSprite().color.a = math.floor(v)
    end)
end
```

---

## 15. Sound — audio

### Effets sonores

```lua
-- Jouer un son (volume 1.0, pitch 1.0, loop false par défaut)
Sound.play("data/sounds/sword.ogg")
Sound.play("data/sounds/hit.ogg", 0.8)              -- volume 80%
Sound.play("data/sounds/explosion.ogg", 1.0, 1.2)   -- pitch +20%
Sound.play("data/sounds/rain.ogg", 0.5, 1.0, true)  -- en boucle

-- Arrêter un son
Sound.stop("data/sounds/rain.ogg")

-- Arrêter tous les effets sonores
Sound.stopAll()
```

### Musique

```lua
-- Jouer une musique (une seule active à la fois)
Sound.playMusic("data/music/dungeon.ogg", true)   -- en boucle
Sound.playMusic("data/music/fanfare.ogg", false)  -- une seule fois

-- Contrôles
Sound.pauseMusic()
Sound.resumeMusic()
Sound.stopMusic()

-- Volume
Sound.setMusicVolume(0.7)   -- 70%
local vol = Sound.getMusicVolume()

-- Fondu progressif vers un nouveau volume
Sound.fadeMusic(0.0, 2.0, function()
    Sound.stopMusic()
    Sound.playMusic("data/music/boss.ogg", true)
    Sound.fadeMusic(1.0, 1.0)
end)
```

### Préchargement

```lua
-- Précharger des sons au début d'une scène pour éviter les latences
Sound.preload("data/sounds/sword.ogg")
Sound.preload("data/sounds/hit.ogg")
Sound.preloadMusic("data/music/dungeon.ogg")
```

### Volume global

```lua
Sound.setVolume(0.8)       -- SFX à 80%
Sound.setMusicVolume(0.6)  -- Musique à 60%
```

### Exemple : transition musicale entre zones

```lua
Scene.listen("scene_changed", function(data)
    if data.name == "dungeon_boss" then
        Sound.fadeMusic(0.0, 1.5, function()
            Sound.playMusic("data/music/boss_theme.ogg", true)
            Sound.fadeMusic(1.0, 1.0)
        end)
    end
end)
```

---

## 16. UI — interface utilisateur

### Réagir aux actions UI

```lua
-- Callback quand un bouton est cliqué (par ID de composant)
UI.onClick("btn_play", function()
    Scene.transition("game_level_01")
end)

-- Callback pour une action nommée (définie dans le JSON UI)
UI.onAction("menu_quit", function(value, id)
    Log.info("Quitter demandé depuis " .. id)
end)

-- Callback générique par événement UI
UI.on("button_hover", function(data)
    Sound.play("data/sounds/ui_hover.ogg")
end)
```

### Lecture et écriture de valeurs

```lua
-- Lire la valeur d'un composant (TextInput, Slider, Button)
local name = UI.getValue("input_playername")
local vol  = UI.getValue("slider_volume")

-- Définir la valeur d'un Slider ou le texte d'un Button
UI.setValue("slider_volume", 0.75)

-- Lire/écrire le texte d'un composant Text
local txt = UI.getText("label_score")
UI.setText("label_score", "Score : " .. score)
```

### Visibilité et layout

```lua
-- Afficher/masquer un composant
UI.show("btn_resume")
UI.hide("btn_resume")
UI.setVisible("btn_resume", flag)

-- Charger un layout UI depuis un fichier JSON
UI.load("data/ui/hud.json")

-- Supprimer un layout
UI.removeUI("hud")

-- Supprimer un groupe de composants
UI.removeGroup("inventory_panel")

-- Supprimer toute l'UI
UI.clear()
```

### Named handler OnUIAction

Dans un script d'entité, vous pouvez recevoir les actions UI directement :

```lua
function OnUIAction(entity, action, value, componentId)
    if action == "attack" then
        performAttack(entity)
    end
end
```

### Exemple : menu principal

```lua
-- menu.lua (script de scène)
UI.load("data/ui/main_menu.json")

UI.onClick("btn_new_game", function()
    SceneFX.transition(function()
        Scene.load("data/scenes/intro.scene", "intro")
        Scene.setActive("intro")
    end, 0.5)
end)

UI.onClick("btn_options", function()
    UI.show("panel_options")
    UI.hide("panel_main")
end)

UI.onClick("btn_back_options", function()
    UI.hide("panel_options")
    UI.show("panel_main")
end)

UI.onAction("slider_volume_sfx", function(value)
    Sound.setVolume(value)
end)

UI.onAction("slider_volume_music", function(value)
    Sound.setMusicVolume(value)
end)
```

---

## 17. Particles — système de particules 2D

### Burst one-shot

```lua
-- Explosion
Particles.emit(x, y, {
    count    = 30,
    speed    = 200,
    spread   = 180,   -- 180° = hémisphère complet
    angle    = 270,   -- vers le haut (0=droite, 90=bas, 270=haut)
    lifetime = 0.8,
    color    = Color.new(255, 120, 0),
    colorEnd = Color.new(80, 0, 0, 0),
    size     = 6,
    sizeEnd  = 1,
    gravity  = 60,    -- gravité px/s²
})

-- Étincelles
Particles.emit(x, y, {
    count = 12, speed = 150, spread = 60, angle = 0,
    lifetime = 0.4, size = 3, sizeEnd = 0,
    color = Color.new(255, 220, 50)
})
```

### Émetteur continu

```lua
-- Crée un émetteur lié à une position
local trail = Particles.emitter(entity:getPosition().x, entity:getPosition().y, {
    rate     = 0.03,   -- secondes entre spawns
    count    = 1,
    speed    = 20,
    spread   = 360,    -- toutes directions
    lifetime = 0.5,
    size     = 4,
    sizeEnd  = 0,
    color    = Color.new(80, 120, 255)
})

-- Mise à jour de la position dans OnUpdate
function OnUpdate(entity, dt)
    local pos = entity:getPosition()
    Particles.move(trail, pos.x, pos.y)
end

-- Arrêter l'émetteur
Particles.stop(trail)
```

### Gestion globale

```lua
-- Nombre de particules actives
Log.info("Particules : " .. Particles.count())

-- Supprimer toutes les particules et émetteurs
Particles.clear()
```

> **Note** : `Particles.clear()` est appelé automatiquement au changement de scène.

### Paramètres de particules

| Paramètre | Type | Défaut | Description |
|-----------|------|--------|-------------|
| `count` | int | 12 | Particules par burst |
| `rate` | number | 0.04 | Secondes entre spawns (émetteur) |
| `lifetime` | number | 1.0 | Durée de vie (secondes) |
| `speed` | number | 80 | Vitesse max (px/s) |
| `spread` | number | 180 | Demi-angle de dispersion (degrés) |
| `angle` | number | 270 | Direction centrale (0=droite, 270=haut) |
| `gravity` | number | 0 | Accélération verticale (px/s²) |
| `size` | number | 5 | Taille initiale (px) |
| `sizeEnd` | number | 0 | Taille finale (px) |
| `color` | Color | Blanc | Couleur initiale |
| `colorEnd` | Color | alpha→0 | Couleur finale |

---

## 18. Projectile — tirs et projectiles

Le module Projectile utilise Pool en interne pour réutiliser les entités de tir.

### Initialisation du pool

```lua
-- À appeler une fois (ex: OnInit de l'entité tireur, ou script de scène)
Projectile.init({
    poolID     = "arrow",     -- identifiant du pool
    count      = 16,          -- capacité du pool
    templateFn = function()
        return World.spawn("ArrowEntity")
    end
})
```

### Tirer un projectile

```lua
-- Tir par angle et vitesse
local proj = Projectile.fire(player.x, player.y, {
    poolID   = "arrow",
    angle    = 0,       -- 0=droite, 90=bas, 180=gauche, 270=haut
    speed    = 400,
    lifetime = 2.0,
    onHit = function(projEntity, hitEntityId)
        Sound.play("data/sounds/arrow_hit.ogg")
        local pos = World.getPosition(projEntity)
        Particles.emit(pos.x, pos.y, { count = 8, speed = 60 })
        hitEntityId:sendMessage("takeDamage", { amount = 20 })
        Projectile.release(projEntity)
    end,
    onExpire = function(projEntity)
        -- Projectile hors-champ, sans impact
    end
})

-- Tir par vecteur vitesse (prioritaire sur angle+speed)
Projectile.fire(x, y, {
    poolID = "bullet",
    vx = 300, vy = -150,    -- direction diagonale
    lifetime = 1.5,
    onHit = function(proj, hitId)
        Projectile.release(proj)
    end
})
```

### Gestion

```lua
-- Nombre de projectiles en vol
Log.info("Projectiles actifs : " .. Projectile.activeCount())

-- Libérer manuellement avant expiration
Projectile.release(entity)

-- Détruire un pool
Projectile.clearPool("arrow")

-- Tout nettoyer (appelé auto au changement de scène)
Projectile.clearAll()
```

---

## 19. Trigger — zones de déclenchement

### Créer une zone

```lua
-- Zone simple avec callbacks
Trigger.create("lava_zone", { x = 200, y = 400, w = 100, h = 50 }, {
    tags    = { "player" },      -- filtre par tag
    onEnter = function(eid)
        Sound.play("data/sounds/sizzle.ogg")
        Effect.apply(eid, "burning", { duration = 3.0,
            onTick = function(e, dt) Stats.mod(e, "Health", -10 * dt) end
        })
    end,
    onStay  = function(eid, dt)
        -- Appelé chaque frame où l'entité est dans la zone
    end,
    onExit  = function(eid)
        Effect.remove(eid, "burning")
    end,
})

-- Zone à déclenchement unique
Trigger.create("boss_trigger", { x = 512, y = 256, w = 64, h = 64 }, {
    once    = true,
    tags    = { "player" },
    onEnter = function(eid)
        EventBus.emit("boss_fight_start", {})
    end,
})

-- Zone avec filtre personnalisé
Trigger.create("key_door", { x = 300, y = 100, w = 32, h = 64 }, {
    filter  = function(eid)
        -- N'accepte que les joueurs qui ont la clé
        return Flag.get("has_key")
    end,
    onEnter = function(eid)
        Flag.unset("has_key")
        Scene.transition("next_room")
    end,
})
```

### Gestion des zones

```lua
-- Activer/désactiver sans supprimer
Trigger.enable("boss_trigger")
Trigger.disable("boss_trigger")

-- Supprimer une zone
Trigger.remove("lava_zone")

-- Supprimer toutes les zones
Trigger.clear()
```

### Réagir aux événements trigger

```lua
EventBus.on("trigger_enter", function(data)
    Log.info("Entité " .. data.entityId .. " est entrée dans " .. data.id)
end)
```

---

## 20. Pool — pooling d'entités

Le pooling évite la création/destruction répétée d'entités (balles, ennemis, effets…).

### Créer et utiliser un pool

```lua
-- Créer un pool de 64 entités "Bullet"
Pool.create("bullet", function()
    return World.spawn("BulletEntity")
end, 64)

-- Acquérir une entité du pool
local bullet = Pool.acquire("bullet")
if bullet then
    -- L'entité est activée automatiquement (entity:enable())
    World.setPosition(bullet, shooter.x, shooter.y)
    -- Configurer la vitesse, etc.
end

-- Libérer dans le pool (l'entité est désactivée automatiquement)
Pool.release("bullet", bullet)
```

### Inspection

```lua
-- Nombre d'entités disponibles
Log.info("Balles disponibles : " .. Pool.available("bullet"))

-- Taille totale du pool
Log.info("Capacité totale : " .. Pool.size("bullet"))
```

### Nettoyage

```lua
-- Détruire toutes les entités d'un pool
Pool.clear("bullet")

-- Détruire tous les pools
Pool.clearAll()
```

> **Note** : `entity:enable()` active `SpriteComponent.visible` et `ColliderComponent.enabled`.
> `entity:disable()` les désactive. Ces méthodes sont fournies automatiquement aux entités poolées.

---

## 21. Nav — navigation et pathfinding

### Navigation directe

```lua
-- Déplacer une entité vers une position
Nav.moveTo(entity, { x = 400, y = 300 }, 80, function()
    Log.info("Arrivé !")
end)

-- Déplacer vers une autre entité
Nav.moveTo(npc, Game.getPlayer(), 60, function()
    npc:sendMessage("startDialogue")
end)
```

### Pathfinding via WaypointGraph

```lua
-- Calculer un chemin puis le suivre
local path = Nav.findPath(npc, { x = 600, y = 200 })
Nav.moveAlongPath(npc, path, 100, function()
    Log.info("Chemin complété")
end)

-- Ou en une seule étape via Scene.findPath
local path2 = Scene.findPath(npc:getPosition().x, npc:getPosition().y, 600, 200)
Nav.moveAlongPath(npc, path2, 100)
```

### Contrôle de navigation

```lua
-- Arrêter immédiatement
Nav.stop(entity)

-- Vérifier si en mouvement
if Nav.isMoving(npc) then
    -- Le NPC est en train de se déplacer
end
```

### Exemple : NPC en patrouille

```lua
local waypoints = { {x=100,y=200}, {x=300,y=200}, {x=300,y=400}, {x=100,y=400} }
local _wpIndex  = 1

local function patrol(entity)
    local wp = waypoints[_wpIndex]
    Nav.moveTo(entity, wp, 60, function()
        _wpIndex = (_wpIndex % #waypoints) + 1
        Timer.after(1.0, function() patrol(entity) end)
    end)
end

function OnInit(entity)
    patrol(entity)
end

function OnDestroy(entity)
    Nav.stop(entity)
end
```

---

## 22. SceneFX — effets visuels plein-écran

SceneFX dessine des overlays plein-écran via DebugDraw (au-dessus de tout, UI incluse).

### Fondus

```lua
-- Fondu vers le noir (transparent → noir)
SceneFX.fadeIn(0.5, function()
    Log.info("Écran noir")
end)

-- Fondu depuis le noir (noir → transparent)
SceneFX.fadeOut(0.5, function()
    Log.info("Retour à la normale")
end)
```

### Transition complète

```lua
-- fadeIn → callback → fadeOut
SceneFX.transition(function()
    -- Exécuté quand l'écran est noir
    Scene.setActive("dungeon_02")
    UI.removeUI("hud_old")
    UI.load("data/ui/hud.json")
end, 0.6)
```

### Flash

```lua
-- Flash blanc rapide (ex: coup de foudre)
SceneFX.flash(Color.new(255, 255, 255, 200), 0.15)

-- Flash rouge (dégât critique)
SceneFX.flash(Color.new(255, 0, 0, 150), 0.2)
```

### Vérifier l'état

```lua
if SceneFX.isActive() then
    -- Un effet est en cours, éviter d'en lancer un autre
end
```

---

## 23. Sequence — cutscènes et séquenceurs

Le module Sequence permet d'écrire des cutscènes déclaratives, étape par étape.

### Syntaxe de base

```lua
local seq = Sequence.new()

seq:call(function()
    Flag.set("cutscene_active")
    Camera.unfollow()
end)

seq:tween(1.5, function(t)
    -- t évolue de 0.0 à 1.0 sur 1.5 secondes
    Camera.setZoom(1.0 + t * 0.8)   -- zoom de 1.0 à 1.8
end)

seq:wait(0.5)

seq:dialogue("elder", "L'ennemi approche des murs...")

seq:emit("cutscene_boss_reveal", {})

seq:wait(1.0)

seq:call(function()
    Camera.follow(Game.getPlayer())
    Flag.unset("cutscene_active")
end)

seq:play(function()
    Log.info("Cutscène terminée")
end)
```

### Toutes les étapes disponibles

```lua
seq:wait(seconds)            -- pause
seq:call(fn)                 -- appel synchrone
seq:tween(seconds, fn)       -- fn(t) chaque frame, t = 0..1
seq:emit(event, data)        -- EventBus.emit
seq:dialogue(speakerId, text)-- attend dialogue_end
seq:repeat_(n, fn)           -- répète fn(i) n fois immédiatement
```

### Contrôle

```lua
local seq = Sequence.new()
    :call(function() ... end)
    :wait(1.0)

local handle = seq:play()    -- retourne la séquence

-- Arrêter en cours de route
seq:stop()
```

> **Note** : Toutes les séquences actives sont arrêtées automatiquement au changement de scène.

---

## 24. StateMachine — machines à états finis

### Créer une FSM

```lua
local sm = StateMachine.new("idle")

sm:addState("idle",
    function(entity)        -- onEnter
        Anim.play(entity, "idle", true)
    end,
    function(entity, dt)    -- onUpdate
        local player = Game.getPlayer()
        if player and World.distance(entity, player) < 150 then
            sm:transition("chase")
        end
    end,
    function(entity)        -- onExit
        -- (rien)
    end
)

sm:addState("chase",
    function(entity)
        Anim.play(entity, "run", true)
    end,
    function(entity, dt)
        local player = Game.getPlayer()
        if not player then
            sm:transition("idle")
            return
        end
        local dist = World.distance(entity, player)
        if dist > 200 then
            sm:transition("idle")
        elseif dist < 40 then
            sm:transition("attack")
        else
            Nav.moveTo(entity, player, 80)
        end
    end,
    function(entity) Nav.stop(entity) end
)

sm:addState("attack",
    function(entity)
        Anim.play(entity, "attack", false)
        Anim.onFinish(entity, function()
            sm:transition("chase")
        end)
    end,
    function(entity, dt)
        local player = Game.getPlayer()
        if player and World.distance(entity, player) < 40 then
            if Cooldown.use(entity.id, "atk", 1.0) then
                player:sendMessage("takeDamage", { amount = 15 })
            end
        end
    end,
    nil
)

-- Appel dans OnUpdate :
function OnUpdate(entity, dt)
    sm:update(entity, dt)
end

-- Récupérer l'état courant
Log.info("État : " .. sm:getState())
```

### Chaînage

```lua
-- addState est chaînable
local sm = StateMachine.new("patrol")
    :addState("patrol", onEnterPatrol, onUpdatePatrol, onExitPatrol)
    :addState("alert",  onEnterAlert,  onUpdateAlert,  nil)
    :addState("flee",   onEnterFlee,   onUpdateFlee,   nil)
```

---

## 25. Data — chargement de données JSON

### Charger des définitions

```lua
-- items.json : { "sword_iron": { "damage": 15, "weight": 3, "rarity": "common" }, ... }
Data.load("items",   "data/definitions/items.json")
Data.load("enemies", "data/definitions/enemies.json")
Data.load("levels",  "data/definitions/levels.json")
```

### Accéder aux données

```lua
-- Accès direct à une entrée
local sword  = Data.get("items", "sword_iron")
local damage = sword.damage     -- 15

-- Accès à toute la table
local allItems = Data.getAll("items")

-- Filtre : obtenir tous les items de rareté "rare"
local rares = Data.query("items", function(item)
    return item.rarity == "rare"
end)
```

### Gestion du cache

```lua
-- Vérifier si chargé
if not Data.isLoaded("items") then
    Data.load("items", "data/definitions/items.json")
end

-- Modifier en mémoire (pas sur disque)
Data.set("items", "sword_iron", { damage = 20 })

-- Recharger depuis le disque
Data.reload("items")

-- Libérer le cache
Data.unload("items")
```

### Exemple : initialiser un ennemi depuis JSON

```lua
-- enemies.json : { "goblin": { "stats": { "Health": 40, "Speed": 90 }, "anim": "goblin_walk" } }

function OnInit(entity)
    local tag = entity:getTag() or "goblin"
    local def = Data.get("enemies", tag)
    if def then
        Stats.init(entity.id, def.stats)
        Anim.play(entity, def.anim, true)
    end
end
```

---

## 26. Flag — drapeaux booléens

Les flags sont des booléens nommés globaux — équivalent des `GlobalVariable` de Papyrus.
Idéals pour les états de jeu simples : portes ouvertes, boss vaincus, quêtes déclenchées.

### Utilisation de base

```lua
-- Activer un flag
Flag.set("boss_defeated")
Flag.set("bridge_repaired")

-- Désactiver
Flag.unset("bridge_repaired")

-- Inverser
Flag.toggle("lamp_on")

-- Lire
if Flag.get("boss_defeated") then
    Scene.transition("credits")
end

-- Alias sémantique
if Flag.require("has_key") then
    openDoor()
end
```

### Gestion globale

```lua
-- Tous les flags actifs
local all = Flag.getAll()
for name, _ in pairs(all) do
    Log.info("Flag actif : " .. name)
end

-- Réinitialiser tous les flags
Flag.clearAll()

-- Supprimer un flag spécifique
Flag.clear("tmp_cutscene_done")
```

### Réagir aux changements

```lua
EventBus.on("flag_set", function(data)
    if data.name == "all_crystals_collected" then
        unlockBossRoom()
    end
end)

EventBus.on("flag_unset", function(data)
    Log.info("Flag retiré : " .. data.name)
end)
```

---

## 27. Persist — données persistantes

Persist stocke des données qui survivent aux changements de scène et aux redémarrages.
Les données sont sauvegardées dans `data/saves/persist.lua`.

### Types supportés

`string`, `number`, `boolean` (pas de tables imbriquées complexes)

### Lire et écrire

```lua
-- Écrire
Persist.set("playerGold",   250)
Persist.set("bossDefeated", true)
Persist.set("playerName",   "Héros")

-- Lire (avec valeur par défaut)
local gold    = Persist.get("playerGold",   0)
local defeated = Persist.get("bossDefeated", false)
local name    = Persist.get("playerName",   "Inconnu")
```

### Gestion

```lua
-- Supprimer une clé
Persist.delete("tmpFlag")

-- Tout effacer
Persist.clear()

-- Toutes les données en mémoire
local all = Persist.getAll()

-- Forcer la sauvegarde
Persist.save()

-- Forcer le rechargement depuis le disque
Persist.load()
```

### Exemple : sauvegarder la progression

```lua
-- À la mort du boss
EventBus.on("boss_defeated", function(data)
    Persist.set("boss_" .. data.bossId .. "_defeated", true)
    Persist.set("totalKills", Persist.get("totalKills", 0) + 1)
    Persist.save()
end)

-- Au chargement de la scène, restaurer l'état
function checkBossState()
    if Persist.get("boss_dragon_defeated", false) then
        -- Le boss est déjà mort, ne pas le respawner
        World.destroy(World.findByTag("boss_dragon").id)
    end
end
```

---

## 28. Game — fonctions globales du jeu

### Accès au joueur

```lua
local player = Game.getPlayer()   -- entité taguée "player", ou nil
if player then
    Log.info("Joueur à " .. player:getPosition().x)
end
```

### Temps

```lua
-- Temps total écoulé depuis le démarrage (secondes)
local t = Game.getTime()

-- Dernier delta-time (approximatif)
local dt = Game.getDeltaTime()
```

### Timescale (pause et ralenti)

```lua
-- Pause complète
Game.pause()
Game.isPaused()    -- true
Game.resume()

-- Ralenti (ex: bullet time)
Game.setTimescale(0.3)

-- Accéléré (ex: fast-forward)
Game.setTimescale(2.0)

-- Normal
Game.setTimescale(1.0)

-- Lire le timescale courant
local ts = Game.getTimescale()
```

### Notifications UI

```lua
-- Émet "ui_notification" { text } — l'UI peut s'y brancher
Game.notification("Quête complétée !")
```

---

## 29. InputEx — saisie frame-parfaite

InputEx complète `Input.isKeyPressed()` (qui retourne `true` tant que la touche est tenue)
avec des détections d'événements qui ne se déclenchent qu'**une seule frame**.

### Différences entre les APIs d'input

| API | Comportement |
|-----|-------------|
| `Input.isKeyPressed("E")` | `true` tant que la touche est enfoncée |
| `InputEx.isKeyJustPressed("E")` | `true` uniquement le premier frame |
| `InputEx.isKeyJustReleased("E")` | `true` uniquement au relâchement |
| `function OnKeyDown(entity, key)` | Named handler, déclenché une fois |

### Utilisation

```lua
function OnUpdate(entity, dt)
    -- Saut au moment de l'appui (pas maintenu)
    if InputEx.isKeyJustPressed("Space") then
        jump(entity)
    end
    
    -- Fin de charge à l'appui de la touche
    if InputEx.isKeyJustReleased("Space") then
        releaseCharge(entity)
    end
    
    -- Interagir
    if InputEx.isKeyJustPressed("E") then
        interact(entity)
    end
end
```

### Souris

```lua
function OnUpdate(entity, dt)
    if InputEx.isMouseJustPressed("left") then
        local mx, my = Input.getMousePosition()
        fireToward(entity, mx, my)
    end
    
    if InputEx.isMouseJustReleased("right") then
        cancelAiming(entity)
    end
end
```

### Inspecter les touches pressées ce frame

```lua
local keys = InputEx.getJustPressedKeys()
for _, key in ipairs(keys) do
    Log.info("Touche pressée : " .. key)
end
```

---

## 30. Notify — notifications et textes flottants

### Notifications écran

```lua
-- Notification simple en haut de l'écran
Notify.show("Quête complétée !")

-- Notification colorée avec durée personnalisée
Notify.show("Vie critique !", { color = "red",  duration = 2.0, size = 1.3 })
Notify.show("Niveau supérieur !", { color = "gold", duration = 4.0 })
Notify.show("Objet trouvé : Épée de Feu", { color = "#ff6600" })

-- Effacer la notification courante
Notify.clear()
```

### Textes flottants sur les entités

```lua
-- Texte qui monte au-dessus d'une entité
Notify.floatingText(entity, "-25", { color = "red" })
Notify.floatingText(entity, "+50 XP", { color = "#aaffaa", duration = 1.5 })
Notify.floatingText(entity, "CRITIQUE !", { color = "gold", size = 1.5 })
```

### Brancher l'UI sur les notifications

L'UI doit écouter les événements émis par Notify :

```lua
-- Dans le script HUD
EventBus.on("ui_notification", function(data)
    UI.setText("hud_notif_text", data.text)
    UI.show("hud_notif")
    Timer.after(data.duration, function()
        UI.hide("hud_notif")
    end)
end)

EventBus.on("floating_text", function(data)
    -- Créer un label temporaire à la position de l'entité
    -- ou utiliser le système de particules texte de l'UI
end)
```

---

## 31. Patterns avancés

### Pattern : script d'entité complet (ennemi RPG)

```lua
-- scripts/enemy_goblin.lua
local sm = StateMachine.new("idle")

-- ─ États FSM ────────────────────────────────────────────────────────────────

sm:addState("idle",
    function(e) Anim.play(e, "idle", true) end,
    function(e, dt)
        local player = Game.getPlayer()
        if player and World.distance(e, player) < 200 then
            sm:transition("chase")
        end
    end,
    nil
)

sm:addState("chase",
    function(e) Anim.play(e, "run", true) end,
    function(e, dt)
        local player = Game.getPlayer()
        if not player then sm:transition("idle"); return end
        local d = World.distance(e, player)
        if     d > 250 then sm:transition("idle")
        elseif d < 50  then sm:transition("attack")
        else Nav.moveTo(e, player, Stats.get(e.id, "Speed", 80))
        end
    end,
    function(e) Nav.stop(e) end
)

sm:addState("attack",
    function(e)
        Anim.play(e, "attack", false)
        Anim.onFrame(e, 4, function()
            local player = Game.getPlayer()
            if player and World.distance(e, player) < 60 then
                player:sendMessage("takeDamage", {
                    amount = Stats.get(e.id, "Damage", 10)
                })
            end
        end)
        Anim.onFinish(e, function() sm:transition("chase") end)
    end,
    function(e, dt) end,
    nil
)

-- ─ Named Handlers ────────────────────────────────────────────────────────────

function OnInit(entity)
    local tag = entity:getTag() or "goblin"
    local def = Data.get("enemies", tag)
    Stats.init(entity.id, def and def.stats or { Health=40, Speed=80, Damage=8 })
end

function OnUpdate(entity, dt)
    sm:update(entity, dt)
end

function OnMessage(entity, msg, data)
    if msg == "takeDamage" then
        local armor = Stats.get(entity.id, "Armor", 0)
        local dmg   = math.max(1, data.amount - armor)
        local hp    = Stats.mod(entity.id, "Health", -dmg)
        
        Notify.floatingText(entity, "-" .. dmg, { color = "red" })
        Camera.shake(0.1, 2)
        
        if hp <= 0 then
            die(entity)
        end
    elseif msg == "stun" then
        sm:transition("idle")
        Effect.apply(entity.id, "stunned", {
            duration = data.duration or 2.0,
            onApply  = function(eid) Anim.play(entity, "stun", true) end,
            onExpire = function(eid) sm:transition("chase") end,
        })
    end
end

function die(entity)
    Sound.play("data/sounds/enemy_death.ogg")
    local pos = entity:getPosition()
    Particles.emit(pos.x, pos.y, {
        count=20, speed=100, spread=360, lifetime=0.8,
        color=Color.new(180,30,30), colorEnd=Color.new(80,0,0,0)
    })
    EventBus.emit("enemy_killed", { entityId = entity.id, tag = entity:getTag() })
    World.destroy(entity.id)
end
```

### Pattern : script de scène avec nettoyage

```lua
-- scripts/dungeon_01.lua
-- Toujours utiliser Scene.listen() pour les scripts de scène !

-- Musique
Sound.playMusic("data/music/dungeon.ogg", true)

-- Spawner d'ennemis toutes les 30s
Timer.every(30.0, function()
    if World.count("enemy") < 10 then
        local e = World.spawn("GoblinEntity")
        World.setPosition(e, math.random(100, 700), math.random(100, 500))
    end
end)

-- Réagir aux kills
Scene.listen("enemy_killed", function(data)
    local kills = (Persist.get("dungeon01_kills", 0)) + 1
    Persist.set("dungeon01_kills", kills)
    if kills >= 20 then
        EventBus.emit("dungeon_cleared", {})
    end
end)

-- Transition vers le boss
Scene.listen("dungeon_cleared", function()
    Sound.fadeMusic(0, 1.0, function()
        Scene.transition("boss_room", {
            fade = true, duration = 0.8,
            onBeforeChange = function()
                UI.removeUI("hud")
            end
        })
    end)
end)

-- Trigger de checkpoint
Trigger.create("checkpoint_mid", { x=400, y=300, w=64, h=64 }, {
    tags = { "player" }, once = true,
    onEnter = function(eid)
        Persist.set("last_checkpoint", "dungeon01_mid")
        Notify.show("Checkpoint !", { color = "gold" })
    end
})
```

### Pattern : système de quêtes simple

```lua
-- scripts/quest_manager.lua
local QUESTS = {
    slay_goblins = {
        title = "Fléau des Gobelins",
        required = 10,
        progress = 0,
        reward = { gold = 50, xp = 100 }
    }
}

Scene.listen("enemy_killed", function(data)
    local q = QUESTS.slay_goblins
    if data.tag == "goblin" and q.progress < q.required then
        q.progress = q.progress + 1
        Notify.show(q.title .. " : " .. q.progress .. "/" .. q.required)
        
        if q.progress >= q.required then
            completeQuest("slay_goblins")
        end
    end
end)

function completeQuest(id)
    local q = QUESTS[id]
    Flag.set("quest_" .. id .. "_complete")
    Stats.mod(Game.getPlayer().id, "Gold", q.reward.gold)
    Stats.mod(Game.getPlayer().id, "XP",   q.reward.xp)
    Notify.show("Quête complétée : " .. q.title .. " !", { color = "gold", duration = 4 })
    EventBus.emit("quest_completed", { id = id, title = q.title })
end
```

---

## 32. Garanties de nettoyage automatique

Un des avantages clés de NovaEngine est que les ressources Lua sont nettoyées
automatiquement dans les cas courants. Voici la liste complète des nettoyages garantis.

### Au changement de scène (`scene_changed`)

| Module | Action |
|--------|--------|
| Timer | Annule tous les timers scopés à la scène sortante |
| Scheduler | Annule toutes les coroutines scopées à la scène sortante |
| `Scene.listen()` handlers | Désinscription automatique de EventBus |
| Particles | `Particles.clear()` — supprime toutes les particules |
| Projectile | `Projectile.clearAll()` — libère les entités et handlers |
| Sequence | Arrête toutes les séquences actives |

### À la destruction d'une entité (`entity_destroyed`)

L'événement est émis **avant** la suppression du registre, permettant aux handlers
d'accéder encore à l'état de l'entité.

| Module | Action |
|--------|--------|
| Named handlers | Désinscription de tous les handlers EventBus câblés par `__wireNamedHandlers` |
| ScriptRegistry | Suppression de l'environnement Lua de l'entité |
| Stats | `Stats.clear(id)` |
| Effect | `Effect.clear(id)` (appelle `onExpire` pour chaque effet actif) |
| Cooldown | `Cooldown.resetAll(id)` |
| Anim | `Anim.clearCallbacks(id)` |

### Ce que vous devez gérer manuellement

- Les handlers enregistrés via `EventBus.on()` dans un script de scène (préférez `Scene.listen()`)
- Les timers avec un scope explicite non lié à la scène (ex: `Timer.after(1, fn, "global")`)
- Le nettoyage de pools créés manuellement via `Pool.create()` (appelez `Pool.clear()` dans le nettoyage de scène)
- Les zones `Trigger` créées dans une scène (appelez `Trigger.clear()` au changement)

### Règle d'or

> Utilisez `Scene.listen()` à la place de `EventBus.on()` dans les scripts de scène.
> Utilisez les named handlers pour les scripts d'entités — ils sont auto-nettoyés.
> `Timer.after()` et `Scheduler.start()` sans scope sont automatiquement liés à la scène courante.
