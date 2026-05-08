#pragma once

#include "../Backend/Core/BackendTypes.hpp"
#include <sol/forward.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace NovaEngine {

class SceneManager;

// ============================================================================
// LuaBindings
//
// Enregistre tous les types et fonctions du moteur dans un sol::state.
// Appeler registerAll() une fois à l'init, puis registerSceneManager()
// quand le SceneManager est disponible.
//
// Globals Lua exposés après registerAll() :
//   Vec2f, Color                                     — types mathématiques
//   TransformComponent, SpriteComponent,              — composants (R/W)
//   TagComponent, AudioComponent, ColliderComponent,
//   LightComponent, AnimationComponent, ShaderComponent
//   Entity                                           — entité avec méthodes
//   EntityRegistry                                   — type registry
//   Resources.loadTexture/Sound/Music/Font            — ressources
//   Audio.playSound/stopSound/playMusic…              — audio
//   Input.isKeyPressed/isMouseButtonPressed/getMousePosition
//   Log.info/warn/error/debug, print
//
// Après registerSceneManager() :
//   Scene.load/unload/setActive/hasScene             — scènes
// ============================================================================
class LuaBindings {
public:
    // Shared key map — used by registerInput() and ScriptSystem input bridge
    static const std::unordered_map<std::string, KeyCode>& getKeyMap();

    // Enregistre tout sauf SceneManager (qui nécessite une instance).
    static void registerAll(sol::state& lua);

    // Expose le global "Scene" en Lua.
    static void registerSceneManager(sol::state& lua, SceneManager& sm);

private:
    static FontHandle& s_debugFont();
    static void registerMath(sol::state& lua);
    static void registerComponents(sol::state& lua);
    static void registerEntity(sol::state& lua);
    static void registerEntityRegistry(sol::state& lua);
    static void registerResources(sol::state& lua);
    static void registerInput(sol::state& lua);
    static void registerViewport(sol::state& lua);
    static void registerLog(sol::state& lua);
    static void registerDebugDraw(sol::state& lua);
    static sol::object jsonToLua(const nlohmann::json& j, sol::state_view& lua);
};

} // namespace NovaEngine
