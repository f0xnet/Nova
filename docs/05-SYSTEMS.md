# Systèmes ECS - NovaEngine

Les systèmes contiennent toute la **logique** du moteur. Ils opèrent sur les entités ayant certains composants.

## Systèmes built-in (7 systèmes)

### 1. RenderSystem

**Rôle** : Rendu de tous les sprites

**Composants requis** : `TransformComponent` + `SpriteComponent`

```cpp
class RenderSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"TransformComponent", "SpriteComponent"});

        // Tri par z-order (lower = behind)
        std::sort(entities.begin(), entities.end(), [](Entity* a, Entity* b) {
            auto* spriteA = a->getComponent<SpriteComponent>();
            auto* spriteB = b->getComponent<SpriteComponent>();
            return spriteA->zOrder < spriteB->zOrder;
        });

        // Rendu
        for (Entity* entity : entities) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* sprite = entity->getComponent<SpriteComponent>();

            if (!sprite->visible || sprite->textureHandle == INVALID_HANDLE) continue;

            // Préparer données sprite
            SpriteData spriteData;
            spriteData.texture = sprite->textureHandle;
            spriteData.position = transform->position;
            spriteData.size = sprite->size;
            spriteData.rotation = transform->rotation;
            spriteData.scale = transform->scale;
            spriteData.origin = transform->origin;
            spriteData.textureRect = sprite->textureRect;
            spriteData.color = sprite->tint;
            spriteData.blendMode = sprite->blendMode;

            // Shader custom (optionnel)
            auto* shaderComp = entity->getComponent<ShaderComponent>();
            if (shaderComp && shaderComp->enabled && shaderComp->shader != INVALID_HANDLE) {
                spriteData.shader = shaderComp->shader;
            }

            // Dessiner
            GRAPHICS().drawSprite(spriteData);
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "SpriteComponent"};
    }
};
```

**Ordre de rendu** : Contrôlé par `zOrder` (0 = arrière-plan, 100 = premier plan)

---

### 2. AnimationSystem

**Rôle** : Met à jour les animations frame-based

**Composants requis** : `SpriteComponent` + `AnimationComponent`

```cpp
class AnimationSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"SpriteComponent", "AnimationComponent"});

        for (Entity* entity : entities) {
            auto* sprite = entity->getComponent<SpriteComponent>();
            auto* anim = entity->getComponent<AnimationComponent>();

            if (!anim->playing || anim->frames.empty()) continue;

            // Avancer temps d'animation
            anim->currentTime += deltaTime;

            // Frame suivante ?
            if (anim->currentTime >= anim->frameDuration) {
                anim->currentTime = 0.0f;
                anim->currentFrame++;

                // Boucle ou fin
                if (anim->currentFrame >= anim->frames.size()) {
                    if (anim->loop) {
                        anim->currentFrame = 0;
                    } else {
                        anim->currentFrame = static_cast<u32>(anim->frames.size()) - 1;
                        anim->playing = false;
                    }
                }

                // Mettre à jour sprite texture rect
                if (anim->currentFrame < anim->frames.size()) {
                    sprite->textureRect = anim->frames[anim->currentFrame];
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"SpriteComponent", "AnimationComponent"};
    }
};
```

**Fonctionnement** :
1. Avance `currentTime` par `deltaTime`
2. Quand `currentTime >= frameDuration`, passe à la frame suivante
3. Met à jour `sprite->textureRect` avec nouvelle frame
4. Gère loop/fin animation

---

### 3. LightSystem

**Rôle** : Rendu simple des lumières (visualisation)

**Composants requis** : `TransformComponent` + `LightComponent`

```cpp
class LightSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"TransformComponent", "LightComponent"});

        for (Entity* entity : entities) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* light = entity->getComponent<LightComponent>();

            if (!light->enabled) continue;

            // Visualisation simple (cercle coloré) pour Point lights
            if (light->type == LightComponent::LightType::Point) {
                RectData lightRect;
                lightRect.position = transform->position - Vec2f{light->radius, light->radius};
                lightRect.size = Vec2f{light->radius * 2.0f, light->radius * 2.0f};

                // Couleur semi-transparente
                Color lightColor = light->color;
                lightColor.a = static_cast<u8>(light->intensity * 50.0f);
                lightRect.fillColor = lightColor;
                lightRect.outlineThickness = 0.0f;

                GRAPHICS().drawRect(lightRect);
            }
            // Directional et Spot nécessitent shader-based rendering
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "LightComponent"};
    }
};
```

**Note** : Ce système fait une visualisation basique. Le vrai éclairage dynamique est géré par `DynamicLightingEffect` qui collecte les lumières et les applique via shader.

---

### 4. AudioSystem

**Rôle** : Joue les sons des entités

**Composants requis** : `AudioComponent`

```cpp
class AudioSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"AudioComponent"});

        for (Entity* entity : entities) {
            auto* audio = entity->getComponent<AudioComponent>();

            // Jouer au démarrage
            if (audio->playOnStart && !audio->playing && audio->soundHandle != INVALID_HANDLE) {
                AUDIO().playSound(audio->soundHandle, audio->volume, 1.0f, audio->loop);
                audio->playing = true;
                LOG_DEBUG("Entity {}: Playing sound (handle: {})", entity->getID(), audio->soundHandle);
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"AudioComponent"};
    }
};
```

**Utilisation** :
- Sons d'ambiance : `playOnStart=true`, `loop=true`
- Effets sonores ponctuels : Contrôlés manuellement via code

---

### 5. PhysicsSystem

**Rôle** : Détection de collision AABB basique

**Composants requis** : `TransformComponent` + `ColliderComponent`

```cpp
class PhysicsSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto entities = registry.getEntitiesWith({"TransformComponent", "ColliderComponent"});

        // Vérifier collisions entre toutes paires
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                Entity* entityA = entities[i];
                Entity* entityB = entities[j];

                auto* transformA = entityA->getComponent<TransformComponent>();
                auto* colliderA = entityA->getComponent<ColliderComponent>();
                auto* transformB = entityB->getComponent<TransformComponent>();
                auto* colliderB = entityB->getComponent<ColliderComponent>();

                if (!colliderA->enabled || !colliderB->enabled) continue;

                // Box-Box collision seulement (simplifié)
                if (colliderA->type == ColliderComponent::ColliderType::Box &&
                    colliderB->type == ColliderComponent::ColliderType::Box) {

                    Vec2f posA = transformA->position + colliderA->offset;
                    Vec2f posB = transformB->position + colliderB->offset;

                    Rect boundsA{posA.x, posA.y, colliderA->size.x, colliderA->size.y};
                    Rect boundsB{posB.x, posB.y, colliderB->size.x, colliderB->size.y};

                    // Test intersection AABB
                    if (boundsA.left < boundsB.left + boundsB.width &&
                        boundsA.left + boundsA.width > boundsB.left &&
                        boundsA.top < boundsB.top + boundsB.height &&
                        boundsA.top + boundsA.height > boundsB.top) {

                        LOG_DEBUG("Collision entre {} et {}", entityA->getID(), entityB->getID());
                        // Ici: fire événements de collision, résoudre physique, etc.
                    }
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "ColliderComponent"};
    }
};
```

**Limitation** : Système basique, sans résolution de physique avancée. Pour un vrai moteur physique, utiliser Box2D ou similaire.

---

### 6. ActivatorSystem

**Rôle** : Gère les zones de déclenchement (triggers)

**Composants requis** : `TransformComponent` + `ActivatorComponent`

**Fonctionnement** :
1. Trouve toutes entités avec `ActivatorComponent`
2. Trouve toutes entités avec `TagComponent` (déclencheurs potentiels)
3. Pour chaque activateur :
   - Vérifie si entité avec `targetTag` est dans la zone
   - Active/désactive selon le type (Proximity/Manual/Automatic)
   - Gère cooldowns
   - Fire événements

```cpp
class ActivatorSystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        auto activators = registry.getEntitiesWith({"TransformComponent", "ActivatorComponent"});
        auto potentialTriggers = registry.getEntitiesWith({"TransformComponent", "TagComponent"});

        // Mettre à jour cooldowns
        for (Entity* activatorEntity : activators) {
            auto* activator = activatorEntity->getComponent<ActivatorComponent>();

            if (activator->currentCooldown > 0.0f) {
                activator->currentCooldown -= deltaTime;
                if (activator->currentCooldown <= 0.0f) {
                    activator->currentCooldown = 0.0f;
                }
            }
        }

        // Vérifier activations
        for (Entity* activatorEntity : activators) {
            auto* activatorTransform = activatorEntity->getComponent<TransformComponent>();
            auto* activator = activatorEntity->getComponent<ActivatorComponent>();

            if (activator->currentCooldown > 0.0f) continue;

            Vec2f activatorPos = activatorTransform->position + activator->offset;
            bool wasActive = activator->isActive;
            bool entityInZone = false;

            // Vérifier chaque entité tagguée
            for (Entity* triggerEntity : potentialTriggers) {
                auto* tag = triggerEntity->getComponent<TagComponent>();

                if (tag->tag != activator->targetTag) continue;

                auto* triggerTransform = triggerEntity->getComponent<TransformComponent>();
                Vec2f triggerPos = triggerTransform->position;

                // Test collision zone
                bool inZone = false;

                if (activator->shape == ActivatorComponent::ActivatorShape::Box) {
                    Rect activatorRect{
                        activatorPos.x - activator->size.x * 0.5f,
                        activatorPos.y - activator->size.y * 0.5f,
                        activator->size.x,
                        activator->size.y
                    };
                    inZone = activatorRect.contains(triggerPos);
                }
                else if (activator->shape == ActivatorComponent::ActivatorShape::Circle) {
                    f32 dx = triggerPos.x - activatorPos.x;
                    f32 dy = triggerPos.y - activatorPos.y;
                    f32 distSquared = dx * dx + dy * dy;
                    inZone = distSquared <= (activator->radius * activator->radius);
                }

                if (inZone) {
                    entityInZone = true;
                    break;
                }
            }

            // Logique activation selon type
            if (activator->type == ActivatorComponent::ActivatorType::Proximity) {
                if (entityInZone && !wasActive) {
                    activateActivator(activatorEntity, activator);
                }
                else if (!entityInZone && wasActive) {
                    deactivateActivator(activatorEntity, activator);
                }
            }
            else if (activator->type == ActivatorComponent::ActivatorType::Automatic) {
                if (entityInZone) {
                    if (!wasActive) {
                        activateActivator(activatorEntity, activator);
                    }
                }
                else if (wasActive) {
                    deactivateActivator(activatorEntity, activator);
                }
            }
            else if (activator->type == ActivatorComponent::ActivatorType::Manual) {
                // Nécessite input explicite (géré par game code)
                if (entityInZone && !wasActive) {
                    LOG_DEBUG("Entity {} peut être activée manuellement (E)", activatorEntity->getID());
                }
            }

            // Debug visuel
            if (activator->showDebugZone) {
                // Dessiner zone (code omis pour brièveté)
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "ActivatorComponent"};
    }

private:
    void activateActivator(Entity* entity, ActivatorComponent* activator) {
        activator->isActive = true;
        LOG_INFO("Activator {} activé! Action: '{}'", entity->getID(), activator->actionID);

        if (!activator->onActivateEvent.empty()) {
            LOG_DEBUG("Firing event: {}", activator->onActivateEvent);
            // Fire event via EventDispatcher
        }

        // Cooldown
        if (!activator->canReactivate) {
            activator->currentCooldown = -1.0f;  // Disable permanent
        }
        else if (activator->cooldownTime > 0.0f) {
            activator->currentCooldown = activator->cooldownTime;
        }
    }

    void deactivateActivator(Entity* entity, ActivatorComponent* activator) {
        activator->isActive = false;
        LOG_DEBUG("Activator {} désactivé", entity->getID());

        if (!activator->onDeactivateEvent.empty()) {
            LOG_DEBUG("Firing event: {}", activator->onDeactivateEvent);
        }
    }
};
```

**Types d'activateurs** :
- **Proximity** : Active une fois à l'entrée, désactive à la sortie
- **Manual** : Nécessite action utilisateur (touche E par exemple)
- **Automatic** : Active continuellement tant que dans zone

---

### 7. JourneySystem

**Rôle** : Gère les voyages multi-scènes des NPCs

**Composants requis** : `TransformComponent` + `SceneTransitionComponent` + `JourneyComponent`

**Fonctionnement détaillé** :

```cpp
class JourneySystem : public System {
private:
    class SceneGraph* m_sceneGraph;

    struct PendingTransfer {
        u64 entityID;
        std::string fromScene, toScene;
        Vec2f targetPosition;
        int nextSceneIndex;
        std::vector<std::string> remainingPath;
        Vec2f finalDestination;
        std::string finalScene;
    };

    std::vector<PendingTransfer> m_pendingTransfers;

public:
    explicit JourneySystem(SceneGraph* sceneGraph) : m_sceneGraph(sceneGraph) {}

    void update(float deltaTime, EntityRegistry& registry) override {
        auto travelers = registry.getEntitiesWith({
            "TransformComponent",
            "SceneTransitionComponent",
            "JourneyComponent"
        });

        for (Entity* entity : travelers) {
            auto* transform = entity->getComponent<TransformComponent>();
            auto* transition = entity->getComponent<SceneTransitionComponent>();
            auto* journey = entity->getComponent<JourneyComponent>();

            if (!journey->isOnJourney) continue;

            // Suivre waypoints locaux si disponibles
            if (!journey->localWaypointPath.empty() &&
                journey->currentLocalWaypointIndex < (int)journey->localWaypointPath.size()) {

                Vec2f currentWaypoint = journey->localWaypointPath[journey->currentLocalWaypointIndex];
                Vec2f toWaypoint = currentWaypoint - transform->position;
                f32 distSquared = toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y;

                if (distSquared < 25.0f) {  // <5 pixels
                    journey->currentLocalWaypointIndex++;

                    if (journey->currentLocalWaypointIndex >= (int)journey->localWaypointPath.size()) {
                        journey->reachedCurrentDestination = true;
                        LOG_DEBUG("NPC {} atteint destination via waypoints", entity->getID());
                    } else {
                        LOG_DEBUG("NPC {} atteint waypoint {}/{}",
                                 entity->getID(),
                                 journey->currentLocalWaypointIndex,
                                 journey->localWaypointPath.size());
                    }
                }
            } else {
                // Navigation directe (pas de waypoints)
                Vec2f toDestination = journey->currentDestination - transform->position;
                f32 distSquared = toDestination.x * toDestination.x + toDestination.y * toDestination.y;

                if (distSquared < 25.0f) {
                    journey->reachedCurrentDestination = true;
                }
            }

            // Gérer transition de scène
            if (journey->reachedCurrentDestination) {
                if (journey->currentSceneIndex + 1 < (int)journey->scenePath.size()) {
                    // Préparer transition vers scène suivante
                    std::string currentScene = journey->scenePath[journey->currentSceneIndex];
                    std::string nextScene = journey->scenePath[journey->currentSceneIndex + 1];

                    if (m_sceneGraph) {
                        const auto* connection = m_sceneGraph->getConnection(currentScene, nextScene);

                        if (connection) {
                            LOG_INFO("NPC {} transition: '{}' -> '{}'",
                                    entity->getID(), currentScene, nextScene);

                            // Enregistrer transfer
                            PendingTransfer transfer;
                            transfer.entityID = entity->getID();
                            transfer.fromScene = currentScene;
                            transfer.toScene = nextScene;
                            transfer.targetPosition = connection->entryPortalPos;
                            transfer.nextSceneIndex = journey->currentSceneIndex + 1;
                            transfer.remainingPath = journey->scenePath;
                            transfer.finalDestination = journey->finalDestinationPos;
                            transfer.finalScene = journey->finalDestinationScene;
                            m_pendingTransfers.push_back(transfer);

                            // Marquer transition
                            transition->targetScene = nextScene;
                            transition->targetPosition = connection->entryPortalPos;
                            transition->isTransitioning = true;
                        }
                    }
                } else {
                    // FIN du voyage
                    LOG_INFO("NPC {} a terminé son voyage vers '{}'",
                            entity->getID(), journey->finalDestinationScene);
                    journey->isOnJourney = false;
                    journey->scenePath.clear();
                }
            }
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"TransformComponent", "SceneTransitionComponent", "JourneyComponent"};
    }

    // Démarrer un voyage
    bool startJourney(Entity* entity,
                     const std::string& currentScene,
                     const std::string& targetScene,
                     const Vec2f& targetPosition) {
        auto* journey = entity->getComponent<JourneyComponent>();
        auto* transition = entity->getComponent<SceneTransitionComponent>();

        if (!journey || !transition) return false;
        if (!m_sceneGraph) return false;

        // Trouver chemin inter-scènes
        journey->scenePath = m_sceneGraph->findPath(currentScene, targetScene);

        if (journey->scenePath.empty()) {
            LOG_ERROR("No path from '{}' to '{}'", currentScene, targetScene);
            return false;
        }

        LOG_INFO("NPC {} démarre voyage: {} -> {} ({} scènes)",
                entity->getID(), currentScene, targetScene, journey->scenePath.size());

        journey->currentSceneIndex = 0;
        journey->isOnJourney = true;
        journey->finalDestinationScene = targetScene;
        journey->finalDestinationPos = targetPosition;
        journey->reachedCurrentDestination = false;

        // Première destination
        if (journey->scenePath.size() > 1) {
            const auto* connection = m_sceneGraph->getConnection(
                journey->scenePath[0],
                journey->scenePath[1]
            );
            if (connection) {
                journey->currentDestination = connection->exitPortalPos;
            }
        } else {
            journey->currentDestination = targetPosition;
        }

        return true;
    }

    // Calculer chemin de waypoints local (implémenté dans Scene.hpp)
    void calculateLocalWaypointPath(Entity* entity, class Scene* currentScene);
};
```

**Caractéristiques** :
- **Pathfinding inter-scènes** : Via SceneGraph
- **Pathfinding intra-scène** : Via WaypointGraph + waypoints
- **Personnalité NPC** : `preferredPathTags` (ex: ["main_road"], ["shortcut"])
- **Transitions physiques** : NPC traverse réellement chaque scène intermédiaire

---

## Ordre d'exécution des systèmes

Dans `Scene::Scene()`, les systèmes sont créés dans cet ordre :

```cpp
m_systems.push_back(std::make_unique<AnimationSystem>());     // 1. Animation
m_systems.push_back(std::make_unique<PhysicsSystem>());       // 2. Physique
m_systems.push_back(std::make_unique<ActivatorSystem>());     // 3. Triggers
m_systems.push_back(std::make_unique<AudioSystem>());         // 4. Audio
m_systems.push_back(std::make_unique<LightSystem>());         // 5. Lumières
m_systems.push_back(std::make_unique<RenderSystem>());        // 6. Rendu (dernier!)
```

**Pourquoi cet ordre ?**
- Animation avant rendu (met à jour textureRect)
- Physique avant triggers (vérifie collisions)
- Audio pendant update (peut être déclenché par triggers)
- Rendu en dernier (affiche état final)

---

## Créer un système custom

```cpp
// 1. Hériter de System
class InventorySystem : public System {
public:
    void update(float deltaTime, EntityRegistry& registry) override {
        // Query entités avec inventaire
        auto entities = registry.getEntitiesWith({"InventoryComponent"});

        for (Entity* entity : entities) {
            auto* inventory = entity->getComponent<InventoryComponent>();

            // Logique inventaire
            // - Vérifier poids total
            // - Appliquer effets items
            // - Gérer expiration items
            // etc.
        }
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return {"InventoryComponent"};
    }
};

// 2. Ajouter à la scène (dans Scene.cpp ou custom Scene)
m_systems.push_back(std::make_unique<InventorySystem>());
```

---

**Prochaine section** : [Gestion des Scènes](06-SCENE-MANAGEMENT.md)
