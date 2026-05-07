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
#include <cmath>

namespace NovaEngine {

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
    // -------------------------------------------------------------------------
    // Shared key map — used by registerInput() and ScriptSystem input bridge
    // -------------------------------------------------------------------------
    static const std::unordered_map<std::string, KeyCode>& getKeyMap() {
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
        return s_map;
    }

    // Enregistre tout sauf SceneManager (qui nécessite une instance).
    static void registerAll(sol::state& lua) {
        registerMath(lua);
        registerComponents(lua);
        registerEntity(lua);
        registerEntityRegistry(lua);
        registerResources(lua);
        registerInput(lua);
        registerViewport(lua);
        registerLog(lua);
    }

    // Expose le global "Scene" en Lua.
    static void registerSceneManager(sol::state& lua, SceneManager& sm) {
        lua.new_usertype<SceneManager>("SceneManager",
            "load",          [](SceneManager& s, const std::string& path, const std::string& name) {
                return s.loadScene(path, name);
            },
            "unload",        [](SceneManager& s, const std::string& name) {
                s.unloadScene(name);
            },
            "setActive",     [](SceneManager& s, const std::string& name) {
                s.setActiveScene(name);
            },
            "hasScene",      [](SceneManager& s, const std::string& name) {
                return s.hasScene(name);
            },
            "sceneCount",    [](SceneManager& s) { return s.getSceneCount(); },
            "getActiveName", [](SceneManager& s) { return s.getActiveSceneName(); }
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
    // Composants ECS — tous les champs R/W depuis Lua
    // -------------------------------------------------------------------------
    static void registerComponents(sol::state& lua) {
        lua.new_usertype<TransformComponent>("TransformComponent",
            "position", &TransformComponent::position,
            "rotation", &TransformComponent::rotation,
            "scale",    &TransformComponent::scale,
            "origin",   &TransformComponent::origin
        );

        lua.new_usertype<SpriteComponent>("SpriteComponent",
            "textureID",     &SpriteComponent::textureID,
            "textureHandle", &SpriteComponent::textureHandle,
            "zOrder",        &SpriteComponent::zOrder,
            "visible",       &SpriteComponent::visible,
            "tint",          &SpriteComponent::tint,
            "size",          &SpriteComponent::size
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

        lua.new_usertype<AnimationComponent>("AnimationComponent",
            "animationID",   &AnimationComponent::animationID,
            "frameDuration", &AnimationComponent::frameDuration,
            "currentFrame",  &AnimationComponent::currentFrame,
            "loop",          &AnimationComponent::loop,
            "playing",       &AnimationComponent::playing
        );

        lua.new_usertype<ShaderComponent>("ShaderComponent",
            "enabled", &ShaderComponent::enabled,
            "shader",  &ShaderComponent::shader
        );
    }

    // -------------------------------------------------------------------------
    // Entity — getters typés, raccourcis position/distance, messages, composants
    //
    // Usage Lua :
    //   local pos = entity:getPosition()         -- Vec2f {x, y}
    //   entity:setPosition(100, 200)
    //   local d = entity:getDistance(other)      -- pixels
    //   if entity:isNearby(other, 150) then ...
    //   entity:sendMessage("takeDamage", {amount=10})
    //   local t = entity:getTransform()
    //   local anim = entity:getAnimation()
    // -------------------------------------------------------------------------
    static void registerEntity(sol::state& lua) {
        lua.new_usertype<Entity>("Entity",
            "id",           sol::property(&Entity::getID),

            // --- Raccourcis lecture directe ---
            // entity.tag  →  lit le tag sans passer par entity:getTag().tag
            "tag",          sol::property([](Entity& e) -> std::string {
                auto* t = e.getComponent<TagComponent>();
                return t ? t->tag : "";
            }),

            // --- Stats — raccourcis vers le module Stats Lua ---
            // Évite d'écrire Stats.mod(entity.id, ...) dans les scripts attachés à entity
            "modStat",  [](sol::this_state ts, Entity& e,
                           const std::string& stat, double delta) -> double {
                sol::state_view lua(ts);
                sol::protected_function fn = lua["Stats"]["mod"];
                if (!fn.valid()) return 0.0;
                auto r = fn(e.getID(), stat, delta);
                return (r.valid() && r.get_type(0) == sol::type::number)
                    ? r.get<double>(0) : 0.0;
            },
            "getStat",  [](sol::this_state ts, Entity& e,
                           const std::string& stat, sol::object def) -> sol::object {
                sol::state_view lua(ts);
                sol::protected_function fn = lua["Stats"]["get"];
                if (!fn.valid()) return def;
                auto r = fn(e.getID(), stat, def);
                return r.valid() ? r.get<sol::object>(0) : def;
            },
            "setStat",  [](sol::this_state ts, Entity& e,
                           const std::string& stat, double value) {
                sol::state_view lua(ts);
                sol::protected_function fn = lua["Stats"]["set"];
                if (fn.valid()) fn(e.getID(), stat, value);
            },
            "hasStat",  [](sol::this_state ts, Entity& e,
                           const std::string& stat) -> bool {
                sol::state_view lua(ts);
                sol::protected_function fn = lua["Stats"]["has"];
                if (!fn.valid()) return false;
                auto r = fn(e.getID(), stat);
                return r.valid() && r.get<bool>(0);
            },

            // --- Visibilité / activation ---
            // Désactive sprite + collider (entité invisible et sans collision)
            "enable",       [](Entity& e) {
                auto* s = e.getComponent<SpriteComponent>();
                if (s) s->visible = true;
                auto* c = e.getComponent<ColliderComponent>();
                if (c) c->enabled = true;
            },
            "disable",      [](Entity& e) {
                auto* s = e.getComponent<SpriteComponent>();
                if (s) s->visible = false;
                auto* c = e.getComponent<ColliderComponent>();
                if (c) c->enabled = false;
            },
            "isEnabled",    [](Entity& e) -> bool {
                auto* s = e.getComponent<SpriteComponent>();
                return s ? s->visible : true;
            },

            // --- Raccourcis position (évite entity:getTransform().position) ---
            "getPosition",  [](Entity& e) -> Vec2f {
                auto* t = e.getComponent<TransformComponent>();
                return t ? t->position : Vec2f{0.0f, 0.0f};
            },
            "setPosition",  [](Entity& e, f32 x, f32 y) {
                auto* t = e.getComponent<TransformComponent>();
                if (t) { t->position.x = x; t->position.y = y; }
            },

            // --- Distance / proximité ---
            "getDistance",  [](Entity& e, Entity& other) -> f32 {
                auto* t1 = e.getComponent<TransformComponent>();
                auto* t2 = other.getComponent<TransformComponent>();
                if (!t1 || !t2) return 0.0f;
                f32 dx = t1->position.x - t2->position.x;
                f32 dy = t1->position.y - t2->position.y;
                return std::sqrt(dx * dx + dy * dy);
            },
            "isNearby",     [](Entity& e, Entity& other, f32 radius) -> bool {
                auto* t1 = e.getComponent<TransformComponent>();
                auto* t2 = other.getComponent<TransformComponent>();
                if (!t1 || !t2) return false;
                f32 dx = t1->position.x - t2->position.x;
                f32 dy = t1->position.y - t2->position.y;
                return (dx * dx + dy * dy) <= (radius * radius);
            },

            // --- Message inter-scripts → déclenche OnMessage(msg, payload) ---
            "sendMessage",  [](sol::this_state ts, Entity& e,
                               const std::string& msg, sol::object payload) {
                sol::state_view lua(ts);
                sol::object bus = lua["EventBus"];
                if (!bus.valid() || bus.get_type() != sol::type::table) return;
                sol::protected_function fn = lua["EventBus"]["emit"];
                if (!fn.valid()) return;
                auto data       = lua.create_table();
                data["msg"]     = msg;
                data["payload"] = payload;
                fn("script_message_" + std::to_string(e.getID()), data);
            },

            // --- Getters composants typés (retournent nil si absent) ---
            "getTransform", [](Entity& e) { return e.getComponent<TransformComponent>(); },
            "getSprite",    [](Entity& e) { return e.getComponent<SpriteComponent>();    },
            "getTag",       [](Entity& e) { return e.getComponent<TagComponent>();       },
            "getAudio",     [](Entity& e) { return e.getComponent<AudioComponent>();     },
            "getCollider",  [](Entity& e) { return e.getComponent<ColliderComponent>();  },
            "getLight",     [](Entity& e) { return e.getComponent<LightComponent>();     },
            "getAnimation", [](Entity& e) { return e.getComponent<AnimationComponent>(); },
            "getShader",    [](Entity& e) { return e.getComponent<ShaderComponent>();    },

            // --- Opérations génériques par type ID (string) ---
            "hasComponent",    [](Entity& e, const std::string& typeID) {
                return e.hasComponent(typeID);
            },
            "addComponent",    [](sol::this_state ts, Entity& e, const std::string& typeID) -> Component* {
                auto comp = ComponentFactory::get().create(typeID);
                if (!comp) {
                    LOG_WARN("[Lua] addComponent: type inconnu '{}'", typeID);
                    return nullptr;
                }
                auto* result = e.addComponent(std::move(comp));
                sol::state_view lua(ts);
                sol::object regObj = lua["Registry"];
                if (regObj.valid() && regObj.get_type() == sol::type::userdata)
                    regObj.as<EntityRegistry*>()->invalidateQueryCache();
                return result;
            },
            "removeComponent", [](sol::this_state ts, Entity& e, const std::string& typeID) {
                e.removeComponent(typeID);
                sol::state_view lua(ts);
                sol::object regObj = lua["Registry"];
                if (regObj.valid() && regObj.get_type() == sol::type::userdata)
                    regObj.as<EntityRegistry*>()->invalidateQueryCache();
            }
        );
    }

    // -------------------------------------------------------------------------
    // EntityRegistry — création / destruction / lookup d'entités
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
    }

    // -------------------------------------------------------------------------
    // Resources — chargement de textures, sons, musiques, polices
    // Audio     — lecture, pause, volume
    // -------------------------------------------------------------------------
    static void registerResources(sol::state& lua) {
        sol::table res = lua.create_table();
        res["loadTexture"]   = [](const std::string& p) -> TextureHandle { return RESOURCES().loadTexture(p); };
        res["loadFont"]      = [](const std::string& p) -> FontHandle    { return RESOURCES().loadFont(p);    };
        res["loadSound"]     = [](const std::string& p) -> SoundHandle   { return RESOURCES().loadSound(p);   };
        res["loadMusic"]     = [](const std::string& p) -> MusicHandle   { return RESOURCES().loadMusic(p);   };
        res["unloadTexture"] = [](TextureHandle h) { RESOURCES().unloadTexture(h); };
        res["unloadSound"]   = [](SoundHandle h)   { RESOURCES().unloadSound(h);   };
        res["unloadMusic"]   = [](MusicHandle h)   { RESOURCES().unloadMusic(h);   };
        lua["Resources"] = res;

        sol::table audio = lua.create_table();
        audio["playSound"]       = [](SoundHandle h, float vol, float pitch, bool loop) {
            AUDIO().playSound(h, vol, pitch, loop);
        };
        audio["stopSound"]       = [](SoundHandle h) { AUDIO().stopSound(h);    };
        audio["stopAll"]         = []()              { AUDIO().stopAllSounds(); };
        audio["playMusic"]       = [](MusicHandle h, bool loop) { AUDIO().playMusic(h, loop); };
        audio["stopMusic"]       = []() { AUDIO().stopMusic();   };
        audio["pauseMusic"]      = []() { AUDIO().pauseMusic();  };
        audio["resumeMusic"]     = []() { AUDIO().resumeMusic(); };
        audio["setVolume"]       = [](float v) { AUDIO().setSoundVolume(v); };
        audio["setMusicVolume"]  = [](float v) { AUDIO().setMusicVolume(v); };
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
            const auto& map = getKeyMap();
            auto it = map.find(keyName);
            return (it != map.end()) && INPUT().isKeyPressed(it->second);
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
    // Viewport — accès bas niveau à la caméra moteur
    // Utilisé par nova/camera.lua pour Camera.follow / Camera.shake / Camera.setZoom
    //
    // Usage Lua (préférer le module Camera pour les API haut niveau) :
    //   Viewport.setCenter(x, y)
    //   local pos = Viewport.getCenter()   -- Vec2f
    //   Viewport.setSize(w, h)
    //   local sz = Viewport.getSize()      -- Vec2f
    //   Viewport.setRotation(angle)
    // -------------------------------------------------------------------------
    static void registerViewport(sol::state& lua) {
        sol::table vp = lua.create_table();
        vp["getCenter"]   = []() -> Vec2f { return VIEWPORT().getViewCenter(); };
        vp["setCenter"]   = [](f32 x, f32 y) { VIEWPORT().setViewCenter(Vec2f{x, y}); };
        vp["getSize"]     = []() -> Vec2f { return VIEWPORT().getViewSize(); };
        vp["setSize"]     = [](f32 w, f32 h) { VIEWPORT().setViewSize(Vec2f{w, h}); };
        vp["getRotation"] = []() -> f32 {
            return VIEWPORT().getView().rotation;
        };
        vp["setRotation"] = [](f32 r) {
            auto v = VIEWPORT().getView();
            v.rotation = r;
            VIEWPORT().setView(v);
        };
        lua["Viewport"] = vp;
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
