/**
 * SIMPLE RPG EXAMPLE - NovaEngine
 *
 * Ce jeu démontre :
 * - Navigation entre 2 scènes (village et taverne)
 * - NPCs activables avec la touche E
 * - Système de dialogue simple
 * - Utilisation du système ECS
 */

#include <NovaEngine/Core/Application.hpp>
#include <NovaEngine/ECS/SceneManager.hpp>
#include <NovaEngine/ECS/Components.hpp>
#include <memory>

using namespace NovaEngine;

// ============================================
// DIALOGUE COMPONENT
// ============================================

class DialogueComponent : public Component {
public:
    std::string npcName;
    std::vector<std::string> dialogueLines;
    int currentLine = 0;

    COMPONENT_TYPE_ID(DialogueComponent)

    void serialize(nlohmann::json& json) const override {
        json["npcName"] = npcName;
        json["dialogueLines"] = dialogueLines;
    }

    void deserialize(const nlohmann::json& json) override {
        npcName = json.value("npcName", "NPC");
        if (json.contains("dialogueLines")) {
            dialogueLines = json["dialogueLines"].get<std::vector<std::string>>();
        }
    }
};

// ============================================
// DIALOGUE SYSTEM
// ============================================

class SimpleRPG : public Application {
private:
    SceneManager m_sceneMgr;
    u64 m_playerID = 0;

    // Dialogue state
    bool m_dialogueActive = false;
    std::string m_currentNPCName;
    std::vector<std::string> m_currentDialogue;
    int m_currentDialogueLine = 0;

    FontHandle m_font;
    Entity* m_nearestNPC = nullptr;

public:
    SimpleRPG() {
        m_config.windowTitle = "Simple RPG Example - NovaEngine";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vSync = true;
        m_config.clearColor = Color(30, 30, 40);
    }

    bool onInitialize() override {
        LOG_INFO("=== Simple RPG Example Starting ===");

        // Charger la police
        m_font = RESOURCES().loadFont("data/arial.ttf");
        if (m_font == INVALID_HANDLE) {
            LOG_ERROR("Failed to load font!");
            return false;
        }

        // Initialiser le SceneManager avec les définitions
        if (!m_sceneMgr.initialize("data/definitions/", "data/scenegraph.json")) {
            LOG_ERROR("Failed to initialize SceneManager");
            return false;
        }

        // Charger les scènes
        if (!m_sceneMgr.loadScene("data/scenes/village.json", "village")) {
            LOG_ERROR("Failed to load village scene");
            return false;
        }

        if (!m_sceneMgr.loadScene("data/scenes/taverne.json", "taverne")) {
            LOG_ERROR("Failed to load taverne scene");
            return false;
        }

        // Activer la scène village
        m_sceneMgr.setActiveScene("village");

        // Créer le joueur
        Scene* village = m_sceneMgr.getActiveScene();
        Entity* player = village->getEntityRegistry().createEntity();
        m_playerID = player->getID();

        // Transform
        auto* transform = player->addComponent(std::make_unique<TransformComponent>());
        transform->position = Vec2f(640, 360);

        // Sprite
        auto* sprite = player->addComponent(std::make_unique<SpriteComponent>());
        sprite->textureHandle = RESOURCES().loadTexture("data/player.png");
        sprite->size = Vec2f(64, 64);
        sprite->color = Color(100, 200, 255); // Bleu clair pour le joueur

        // Tag
        auto* tag = player->addComponent(std::make_unique<TagComponent>());
        tag->tags.push_back("player");

        // ========================================
        // CRÉER NPC 1 - MERCHANT (VILLAGE)
        // ========================================
        Entity* merchant = village->getEntityRegistry().createEntity();

        auto* merchantTransform = merchant->addComponent(std::make_unique<TransformComponent>());
        merchantTransform->position = Vec2f(400, 300);

        auto* merchantSprite = merchant->addComponent(std::make_unique<SpriteComponent>());
        merchantSprite->textureHandle = RESOURCES().loadTexture("data/npc_merchant.png");
        merchantSprite->size = Vec2f(64, 64);
        merchantSprite->color = Color(50, 200, 100); // Vert pour le marchand

        auto* merchantDialogue = merchant->addComponent(std::make_unique<DialogueComponent>());
        merchantDialogue->npcName = "Marcus the Merchant";
        merchantDialogue->dialogueLines = {
            "Greetings, traveler! Welcome to our humble village.",
            "I sell the finest goods in all the land!",
            "Come back when you have more coin.",
            "The tavern is to the east if you need rest."
        };

        // ========================================
        // CRÉER NPC 2 - INNKEEPER (TAVERNE)
        // ========================================
        Scene* taverne = m_sceneMgr.getScene("taverne");
        Entity* innkeeper = taverne->getEntityRegistry().createEntity();

        auto* innkeeperTransform = innkeeper->addComponent(std::make_unique<TransformComponent>());
        innkeeperTransform->position = Vec2f(640, 360);

        auto* innkeeperSprite = innkeeper->addComponent(std::make_unique<SpriteComponent>());
        innkeeperSprite->textureHandle = RESOURCES().loadTexture("data/npc_innkeeper.png");
        innkeeperSprite->size = Vec2f(64, 64);
        innkeeperSprite->color = Color(255, 150, 50); // Orange pour le tavernier

        auto* innkeeperDialogue = innkeeper->addComponent(std::make_unique<DialogueComponent>());
        innkeeperDialogue->npcName = "Old Ben the Innkeeper";
        innkeeperDialogue->dialogueLines = {
            "Ah, a new face! Welcome to the Rusty Mug Tavern.",
            "We have the best ale this side of the mountains.",
            "Room and board is 5 gold pieces a night.",
            "Many adventurers have passed through these doors...",
            "Some never returned from the old ruins to the north."
        };

        LOG_INFO("Game initialized successfully!");
        LOG_INFO("Controls:");
        LOG_INFO("  WASD - Move");
        LOG_INFO("  E    - Talk to NPCs / Advance dialogue");
        LOG_INFO("  ESC  - Quit");

        return true;
    }

    void onUpdate(float deltaTime) override {
        // Mise à jour des scènes
        m_sceneMgr.update(deltaTime);

        Scene* scene = m_sceneMgr.getActiveScene();
        if (!scene) return;

        Entity* player = scene->getEntityRegistry().getEntity(m_playerID);
        if (!player) return;

        auto* playerTransform = player->getComponent<TransformComponent>();
        if (!playerTransform) return;

        // Contrôles du joueur (seulement si pas en dialogue)
        if (!m_dialogueActive) {
            Vec2f movement(0, 0);
            const float speed = 200.0f;

            if (INPUT().isKeyPressed(KeyCode::W)) movement.y -= 1;
            if (INPUT().isKeyPressed(KeyCode::S)) movement.y += 1;
            if (INPUT().isKeyPressed(KeyCode::A)) movement.x -= 1;
            if (INPUT().isKeyPressed(KeyCode::D)) movement.x += 1;

            if (movement.x != 0 || movement.y != 0) {
                float len = std::sqrt(movement.x * movement.x + movement.y * movement.y);
                movement /= len;
                playerTransform->position += movement * speed * deltaTime;
            }
        }

        // Trouver le NPC le plus proche
        m_nearestNPC = findNearestNPC(player, scene, 80.0f);

        // Caméra suit le joueur
        VIEWPORT().setViewCenter(playerTransform->position);
    }

    void onEvent(const Event& event) override {
        if (event.type != Event::Type::Input) return;

        const InputEvent& inputEvent = event.inputEvent;

        // Touche E pour parler / avancer le dialogue
        if (inputEvent.type == InputEventType::KeyPressed &&
            inputEvent.key.code == KeyCode::E) {

            if (m_dialogueActive) {
                // Avancer le dialogue
                m_currentDialogueLine++;

                if (m_currentDialogueLine >= m_currentDialogue.size()) {
                    // Fin du dialogue
                    m_dialogueActive = false;
                    m_currentDialogueLine = 0;
                    LOG_INFO("Dialogue ended");
                }
            } else if (m_nearestNPC) {
                // Commencer le dialogue avec le NPC
                startDialogue(m_nearestNPC);
            }
        }
    }

    void onRender() override {
        // Les systems dessinent automatiquement les sprites

        // Dessiner l'indicateur "E" au-dessus du NPC proche
        if (!m_dialogueActive && m_nearestNPC) {
            auto* npcTransform = m_nearestNPC->getComponent<TransformComponent>();
            if (npcTransform) {
                TextData indicator;
                indicator.text = "[ E ]";
                indicator.font = m_font;
                indicator.characterSize = 20;
                indicator.fillColor = Color::Yellow;
                indicator.position = Vec2f(
                    npcTransform->position.x - 20,
                    npcTransform->position.y - 50
                );
                GRAPHICS().drawText(indicator);
            }
        }

        // Dessiner la boîte de dialogue si active
        if (m_dialogueActive && m_currentDialogueLine < m_currentDialogue.size()) {
            drawDialogueBox();
        }
    }

    void onShutdown() override {
        LOG_INFO("Simple RPG shutting down");
    }

private:
    Entity* findNearestNPC(Entity* player, Scene* scene, float maxDistance) {
        auto* playerTransform = player->getComponent<TransformComponent>();
        if (!playerTransform) return nullptr;

        Entity* nearest = nullptr;
        float nearestDist = maxDistance;

        for (auto* entity : scene->getEntityRegistry().getEntities()) {
            // Skip le joueur lui-même
            if (entity->getID() == m_playerID) continue;

            // Vérifier si l'entité a un DialogueComponent
            auto* dialogue = entity->getComponent<DialogueComponent>();
            if (!dialogue) continue;

            auto* npcTransform = entity->getComponent<TransformComponent>();
            if (!npcTransform) continue;

            // Calculer la distance
            Vec2f diff = npcTransform->position - playerTransform->position;
            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

            if (dist < nearestDist) {
                nearestDist = dist;
                nearest = entity;
            }
        }

        return nearest;
    }

    void startDialogue(Entity* npc) {
        auto* dialogue = npc->getComponent<DialogueComponent>();
        if (!dialogue || dialogue->dialogueLines.empty()) return;

        m_dialogueActive = true;
        m_currentNPCName = dialogue->npcName;
        m_currentDialogue = dialogue->dialogueLines;
        m_currentDialogueLine = 0;

        LOG_INFO("Started dialogue with {}", m_currentNPCName);
    }

    void drawDialogueBox() {
        // Dimensions de la fenêtre
        Vec2f viewSize = VIEWPORT().getViewSize();
        Vec2f viewCenter = VIEWPORT().getViewCenter();

        // Boîte de dialogue en bas de l'écran (coordonnées monde)
        float boxWidth = 900.0f;
        float boxHeight = 150.0f;
        float boxX = viewCenter.x - boxWidth / 2;
        float boxY = viewCenter.y + viewSize.y / 2 - boxHeight - 50;

        // Fond de la boîte
        RectData box;
        box.position = Vec2f(boxX, boxY);
        box.size = Vec2f(boxWidth, boxHeight);
        box.fillColor = Color(0, 0, 0, 220);
        box.outlineColor = Color(200, 200, 200);
        box.outlineThickness = 3.0f;
        GRAPHICS().drawRect(box);

        // Nom du NPC
        TextData nameText;
        nameText.text = m_currentNPCName;
        nameText.font = m_font;
        nameText.characterSize = 24;
        nameText.fillColor = Color::Yellow;
        nameText.position = Vec2f(boxX + 20, boxY + 10);
        nameText.style = TextStyle::Bold;
        GRAPHICS().drawText(nameText);

        // Texte du dialogue
        TextData dialogueText;
        dialogueText.text = m_currentDialogue[m_currentDialogueLine];
        dialogueText.font = m_font;
        dialogueText.characterSize = 18;
        dialogueText.fillColor = Color::White;
        dialogueText.position = Vec2f(boxX + 20, boxY + 50);
        GRAPHICS().drawText(dialogueText);

        // Indicateur de continuation
        TextData continueText;
        continueText.text = "[ Press E to continue ]";
        continueText.font = m_font;
        continueText.characterSize = 14;
        continueText.fillColor = Color(150, 150, 150);
        continueText.position = Vec2f(boxX + boxWidth - 200, boxY + boxHeight - 30);
        GRAPHICS().drawText(continueText);
    }
};

// ============================================
// MAIN
// ============================================

int main() {
    SimpleRPG game;
    return game.run();
}
