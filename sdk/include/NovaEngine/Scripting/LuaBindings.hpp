#pragma once

#include "../Backend/BackendManager.hpp"
#include "../Core/Logger.hpp"
#include "../ECS/Entity.hpp"
#include "../ECS/Components.hpp"
#include "../ECS/ComponentFactory.hpp"
#include "../ECS/EntityRegistry.hpp"
#include "../ECS/SceneManager.hpp"
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>

namespace NovaEngine {

// ============================================================================
// LuaBindings
//
// Enregistre tous les types et fonctions du moteur dans un sol::state.
// Appeler registerAll() une fois à l'init, puis registerSceneManager()
// quand le SceneManager est disponible.
//
// Globals Lua exposés après registerAll() :
//   Vec2f, Color                           — types mathématiques
//   TransformComponent, SpriteComponent,   — composants (R/W)
//   TagComponent, AudioComponent,
//   ColliderComponent, LightComponent
//   Entity                                 — entité avec méthodes
//   EntityRegistry                         — type registry (global "Registry"
//                                            mis à jour chaque frame par ScriptSystem)
//   Resources.loadTexture/Sound/Music/Font — chargement de ressources
//   Audio.playSound/stopSound/playMusic…   — lecture audio
//   Input.isKeyPressed/isMouseButtonPressed/getMousePosition
//   Log.info/warn/error/debug, print
//
// Après registerSceneManager() :
//   Scene.load/unload/setActive/hasScene   — gestion de scènes
// ============================================================================
class LuaBindings {
public:
    // Enregistre tout sauf SceneManager (qui nécessite une instance).
    static void registerAll(sol::state& lua) {
        registerMath(lua);
        registerComponents(lua);
        registerEntity(lua);
        registerEntityRegistry(lua);
        registerResources(lua);
        registerInput(lua);
        registerLog(lua);
    }

    // Appeler une fois le SceneManager disponible.
    // Expose le global "Scene" en Lua.
    static void registerSceneManager(sol::state& lua, SceneManager& sm) {
        lua.new_usertype<SceneManager>("SceneManager",
            "load",      [](SceneManager& s, const std::string& path, const std::string& name) {
                return s.loadScene(path, name);
            },
            "unload",    [](SceneManager& s, const std::string& name) {
                s.unloadScene(name);
            },
            "setActive", [](SceneManager& s, const std::string& name) {
                s.setActiveScene(name);
            },
            "hasScene",  [](SceneManager& s, const std::string& name) {
                return s.hasScene(name);
            },
            "sceneCount", [](SceneManager& s) { return s.getSceneCount(); }
        );
        lua["Scene"] = &sm;
    }

private:
    // -------------------------------------------------------------------------
    // Vec2f, Color
    // -------------------------------------------------------------------------
    static void registerMath(sol::state& lua) {
        lua.new_usertype<Vec2f>("Vec2f",
            sol::constructors<Vec2f(), Vec2f(f32, f32)>(),
            "x", &Vec2f::x,
            "y", &Vec2f::y,
            sol::meta_function::addition,
                [](const Vec2f& a, const Vec2f& b) { return a + b; },
            sol::meta_function::subtraction,
                [](const Vec2f& a, const Vec2f& b) { return a - b; },
            sol::meta_function::multiplication,
                sol::overload(
                    [](const Vec2f& v, f32 s) { return v * s; },
                    [](f32 s, const Vec2f& v) { return v * s; }),
            sol::meta_function::to_string,
                [](const Vec2f& v) {
                    return "Vec2f(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
                }
        );

        lua.new_usertype<Color>("Color",
            sol::constructors<Color(), Color(u8, u8, u8), Color(u8, u8, u8, u8)>(),
            "r", &Color::r,
            "g", &Color::g,
            "b", &Color::b,
            "a", &Color::a
        );
        auto colorType = lua["Color"];
        colorType["Black"]       = Color::Black;
        colorType["White"]       = Color::White;
        colorType["Red"]         = Color::Red;
        colorType["Green"]       = Color::Green;
        colorType["Blue"]        = Color::Blue;
        colorType["Yellow"]      = Color::Yellow;
        colorType["Transparent"] = Color::Transparent;
    }

    // -------------------------------------------------------------------------
    // Composants ECS — tous les champs sont R/W depuis Lua
    // -------------------------------------------------------------------------
    static void registerComponents(sol::state& lua) {
        lua.new_usertype<TransformComponent>("TransformComponent",
            "position", &TransformComponent::position,
            "rotation", &TransformComponent::rotation,
            "scale",    &TransformComponent::scale,
            "origin",   &TransformComponent::origin
        );

        lua.new_usertype<SpriteComponent>("SpriteComponent",
            "textureID",    &SpriteComponent::textureID,
            "textureHandle",&SpriteComponent::textureHandle,
            "zOrder",       &SpriteComponent::zOrder,
            "visible",      &SpriteComponent::visible,
            "tint",         &SpriteComponent::tint,
            "size",         &SpriteComponent::size
        );

        lua.new_usertype<TagComponent>("TagComponent",
            "tag", &TagComponent::tag
        );

        lua.new_usertype<AudioComponent>("AudioComponent",
            "soundHandle", &AudioComponent::soundHandle,
            "playOnStart", &AudioComponent::playOnStart,
            "loop",        &AudioComponent::loop,
            "volume",      &AudioComponent::volume,
            "pitch",       &AudioComponent::pitch,
            "playing",     &AudioComponent::playing
        );

        lua.new_usertype<ColliderComponent>("ColliderComponent",
            "isTrigger", &ColliderComponent::isTrigger,
            "enabled",   &ColliderComponent::enabled,
            "size",      &ColliderComponent::size,
            "radius",    &ColliderComponent::radius,
            "offset",    &ColliderComponent::offset
        );

        lua.new_usertype<LightComponent>("LightComponent",
            "radius",    &LightComponent::radius,
            "intensity", &LightComponent::intensity,
            "enabled",   &LightComponent::enabled,
            "color",     &LightComponent::color
        );
    }

    // -------------------------------------------------------------------------
    // Entity — getters typés + addComponent/removeComponent par nom
    //
    // Usage Lua :
    //   entity:addComponent("TransformComponent")
    //   local t = entity:getTransform()
    //   t.position.x = 100
    //   entity:removeComponent("SpriteComponent")
    // -------------------------------------------------------------------------
    static void registerEntity(sol::state& lua) {
        lua.new_usertype<Entity>("Entity",
            "id",              sol::property(&Entity::getID),

            // Getters typés (retournent nil si absent)
            "getTransform",    [](Entity& e) { return e.getComponent<TransformComponent>(); },
            "getSprite",       [](Entity& e) { return e.getComponent<SpriteComponent>(); },
            "getTag",          [](Entity& e) { return e.getComponent<TagComponent>(); },
            "getAudio",        [](Entity& e) { return e.getComponent<AudioComponent>(); },
            "getCollider",     [](Entity& e) { return e.getComponent<ColliderComponent>(); },
            "getLight",        [](Entity& e) { return e.getComponent<LightComponent>(); },

            // Opérations génériques par type ID (string)
            "hasComponent",    [](Entity& e, const std::string& typeID) {
                return e.hasComponent(typeID);
            },
            "addComponent",    [](Entity& e, const std::string& typeID) -> Component* {
                auto comp = ComponentFactory::get().create(typeID);
                if (!comp) {
                    LOG_WARN("[Lua] addComponent: type inconnu '{}'", typeID);
                    return nullptr;
                }
                return e.addComponent(std::move(comp));
            },
            "removeComponent", [](Entity& e, const std::string& typeID) {
                e.removeComponent(typeID);
            }
        );
    }

    // -------------------------------------------------------------------------
    // EntityRegistry — création / destruction / lookup d'entités
    //
    // Le global "Registry" est mis à jour chaque frame par ScriptSystem::update().
    //
    // Usage Lua :
    //   local e = Registry:createEntity()
    //   e:addComponent("TransformComponent")
    //   Registry:destroyEntity(e.id)
    //   local existing = Registry:getEntity(42)
    // -------------------------------------------------------------------------
    static void registerEntityRegistry(sol::state& lua) {
        lua.new_usertype<EntityRegistry>("EntityRegistry",
            "createEntity",   [](EntityRegistry& r) { return r.createEntity(); },
            "destroyEntity",  [](EntityRegistry& r, u64 id) {
                r.destroyEntity(id);
                r.invalidateQueryCache();
            },
            "getEntity",      [](EntityRegistry& r, u64 id) { return r.getEntity(id); },
            "getAllEntities",  [](EntityRegistry& r) { return r.getAllEntities(); },
            "entityCount",    [](EntityRegistry& r) { return (int)r.getEntityCount(); }
        );
        // Le global "Registry" est positionné dans ScriptSystem::update().
    }

    // -------------------------------------------------------------------------
    // Resources — chargement de textures, sons, musiques, polices
    // Audio     — lecture, pause, volume
    //
    // Usage Lua :
    //   local tex = Resources.loadTexture("data/images/hero.png")
    //   sprite.textureHandle = tex
    //   local snd = Resources.loadSound("data/audio/jump.wav")
    //   Audio.playSound(snd, 1.0, 1.0, false)
    //   Audio.playMusic(Resources.loadMusic("data/music/town.ogg"))
    // -------------------------------------------------------------------------
    static void registerResources(sol::state& lua) {
        sol::table res = lua.create_table();
        res["loadTexture"] = [](const std::string& p) -> TextureHandle { return RESOURCES().loadTexture(p); };
        res["loadFont"]    = [](const std::string& p) -> FontHandle    { return RESOURCES().loadFont(p);    };
        res["loadSound"]   = [](const std::string& p) -> SoundHandle   { return RESOURCES().loadSound(p);   };
        res["loadMusic"]   = [](const std::string& p) -> MusicHandle   { return RESOURCES().loadMusic(p);   };
        res["unloadTexture"] = [](TextureHandle h) { RESOURCES().unloadTexture(h); };
        res["unloadSound"]   = [](SoundHandle h)   { RESOURCES().unloadSound(h);   };
        res["unloadMusic"]   = [](MusicHandle h)   { RESOURCES().unloadMusic(h);   };
        lua["Resources"] = res;

        sol::table audio = lua.create_table();
        audio["playSound"]    = [](SoundHandle h, float vol, float pitch, bool loop) {
            AUDIO().playSound(h, vol, pitch, loop);
        };
        audio["stopSound"]    = [](SoundHandle h) { AUDIO().stopSound(h);    };
        audio["stopAll"]      = []()              { AUDIO().stopAllSounds(); };
        audio["playMusic"]    = [](MusicHandle h, bool loop) { AUDIO().playMusic(h, loop); };
        audio["stopMusic"]    = []() { AUDIO().stopMusic();   };
        audio["pauseMusic"]   = []() { AUDIO().pauseMusic();  };
        audio["resumeMusic"]  = []() { AUDIO().resumeMusic(); };
        audio["setVolume"]       = [](float v) { AUDIO().setSoundVolume(v);  };
        audio["setMusicVolume"]  = [](float v) { AUDIO().setMusicVolume(v);  };
        lua["Audio"] = audio;
    }

    // -------------------------------------------------------------------------
    // Input — clavier et souris par nom de touche (string)
    //
    // Usage Lua :
    //   if Input.isKeyPressed("W") then ... end
    //   if Input.isMouseButtonPressed("left") then ... end
    //   local pos = Input.getMousePosition()  -- Vec2f
    // -------------------------------------------------------------------------
    static void registerInput(sol::state& lua) {
        sol::table input = lua.create_table();

        input["isKeyPressed"] = [](const std::string& keyName) -> bool {
            static const std::unordered_map<std::string, KeyCode> s_map = {
                {"A", KeyCode::A}, {"B", KeyCode::B}, {"C", KeyCode::C}, {"D", KeyCode::D},
                {"E", KeyCode::E}, {"F", KeyCode::F}, {"G", KeyCode::G}, {"H", KeyCode::H},
                {"I", KeyCode::I}, {"J", KeyCode::J}, {"K", KeyCode::K}, {"L", KeyCode::L},
                {"M", KeyCode::M}, {"N", KeyCode::N}, {"O", KeyCode::O}, {"P", KeyCode::P},
                {"Q", KeyCode::Q}, {"R", KeyCode::R}, {"S", KeyCode::S}, {"T", KeyCode::T},
                {"U", KeyCode::U}, {"V", KeyCode::V}, {"W", KeyCode::W}, {"X", KeyCode::X},
                {"Y", KeyCode::Y}, {"Z", KeyCode::Z},
                {"0", KeyCode::Num0}, {"1", KeyCode::Num1}, {"2", KeyCode::Num2},
                {"3", KeyCode::Num3}, {"4", KeyCode::Num4}, {"5", KeyCode::Num5},
                {"6", KeyCode::Num6}, {"7", KeyCode::Num7}, {"8", KeyCode::Num8},
                {"9", KeyCode::Num9},
                {"Escape",   KeyCode::Escape},   {"Space",     KeyCode::Space},
                {"Enter",    KeyCode::Enter},    {"Backspace", KeyCode::Backspace},
                {"Tab",      KeyCode::Tab},
                {"Left",     KeyCode::Left},     {"Right",     KeyCode::Right},
                {"Up",       KeyCode::Up},       {"Down",      KeyCode::Down},
                {"LShift",   KeyCode::LShift},   {"RShift",    KeyCode::RShift},
                {"LControl", KeyCode::LControl}, {"RControl",  KeyCode::RControl},
                {"LAlt",     KeyCode::LAlt},     {"RAlt",      KeyCode::RAlt},
            };
            auto it = s_map.find(keyName);
            return (it != s_map.end()) && INPUT().isKeyPressed(it->second);
        };

        input["isMouseButtonPressed"] = [](const std::string& btn) -> bool {
            if (btn == "left")   return INPUT().isMouseButtonPressed(MouseButton::Left);
            if (btn == "right")  return INPUT().isMouseButtonPressed(MouseButton::Right);
            if (btn == "middle") return INPUT().isMouseButtonPressed(MouseButton::Middle);
            return false;
        };

        input["getMousePosition"] = []() -> Vec2f {
            auto pos = INPUT().getMousePosition();
            return Vec2f(static_cast<f32>(pos.x), static_cast<f32>(pos.y));
        };

        lua["Input"] = input;
    }

    // -------------------------------------------------------------------------
    // Log — redirige print et Log.* vers le logger moteur
    // -------------------------------------------------------------------------
    static void registerLog(sol::state& lua) {
        sol::table log = lua.create_table();
        log["info"]  = [](const std::string& msg) { LOG_INFO("[Lua] {}", msg);  };
        log["warn"]  = [](const std::string& msg) { LOG_WARN("[Lua] {}", msg);  };
        log["error"] = [](const std::string& msg) { LOG_ERROR("[Lua] {}", msg); };
        log["debug"] = [](const std::string& msg) { LOG_DEBUG("[Lua] {}", msg); };
        lua["Log"]   = log;
        lua["print"] = [](const std::string& msg) { LOG_INFO("[Lua] {}", msg);  };
    }
};

} // namespace NovaEngine
