#pragma once

#include "../Backend/BackendManager.hpp"
#include "../Core/Logger.hpp"
#include "../ECS/Entity.hpp"
#include "../ECS/Components.hpp"
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>

namespace NovaEngine {

class LuaBindings {
public:
    static void registerAll(sol::state& lua) {
        registerMath(lua);
        registerComponents(lua);
        registerEntity(lua);
        registerInput(lua);
        registerLog(lua);
    }

private:
    // -------------------------------------------------------------------------
    // Math types: Vec2f, Color
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
        // Static color constants
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
    // Built-in ECS components
    // -------------------------------------------------------------------------
    static void registerComponents(sol::state& lua) {
        lua.new_usertype<TransformComponent>("TransformComponent",
            "position", &TransformComponent::position,
            "rotation", &TransformComponent::rotation,
            "scale",    &TransformComponent::scale,
            "origin",   &TransformComponent::origin
        );

        lua.new_usertype<SpriteComponent>("SpriteComponent",
            "textureID", &SpriteComponent::textureID,
            "zOrder",    &SpriteComponent::zOrder,
            "visible",   &SpriteComponent::visible,
            "tint",      &SpriteComponent::tint,
            "size",      &SpriteComponent::size
        );

        lua.new_usertype<TagComponent>("TagComponent",
            "tag", &TagComponent::tag
        );

        lua.new_usertype<AudioComponent>("AudioComponent",
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
    // Entity — typed component accessors
    // -------------------------------------------------------------------------
    static void registerEntity(sol::state& lua) {
        lua.new_usertype<Entity>("Entity",
            "id",           &Entity::getID,
            "getTransform", [](Entity& e) { return e.getComponent<TransformComponent>(); },
            "getSprite",    [](Entity& e) { return e.getComponent<SpriteComponent>(); },
            "getTag",       [](Entity& e) { return e.getComponent<TagComponent>(); },
            "getAudio",     [](Entity& e) { return e.getComponent<AudioComponent>(); },
            "getCollider",  [](Entity& e) { return e.getComponent<ColliderComponent>(); },
            "getLight",     [](Entity& e) { return e.getComponent<LightComponent>(); },
            "hasComponent", [](Entity& e, const std::string& typeID) {
                return e.hasComponent(typeID);
            }
        );
    }

    // -------------------------------------------------------------------------
    // Input — keyboard and mouse queries via string keys
    //
    // Usage in Lua:
    //   if Input.isKeyPressed("W") then ... end
    //   if Input.isMouseButtonPressed("left") then ... end
    //   local pos = Input.getMousePosition()  -- returns Vec2f
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
                {"Escape",    KeyCode::Escape},    {"Space",     KeyCode::Space},
                {"Enter",     KeyCode::Enter},     {"Backspace", KeyCode::Backspace},
                {"Tab",       KeyCode::Tab},
                {"Left",      KeyCode::Left},      {"Right",     KeyCode::Right},
                {"Up",        KeyCode::Up},         {"Down",      KeyCode::Down},
                {"LShift",    KeyCode::LShift},    {"RShift",    KeyCode::RShift},
                {"LControl",  KeyCode::LControl},  {"RControl",  KeyCode::RControl},
                {"LAlt",      KeyCode::LAlt},      {"RAlt",      KeyCode::RAlt},
            };
            auto it = s_map.find(keyName);
            if (it == s_map.end()) return false;
            return INPUT().isKeyPressed(it->second);
        };

        input["isMouseButtonPressed"] = [](const std::string& button) -> bool {
            if (button == "left")   return INPUT().isMouseButtonPressed(MouseButton::Left);
            if (button == "right")  return INPUT().isMouseButtonPressed(MouseButton::Right);
            if (button == "middle") return INPUT().isMouseButtonPressed(MouseButton::Middle);
            return false;
        };

        input["getMousePosition"] = []() -> Vec2f {
            auto pos = INPUT().getMousePosition();
            return Vec2f(static_cast<f32>(pos.x), static_cast<f32>(pos.y));
        };

        lua["Input"] = input;
    }

    // -------------------------------------------------------------------------
    // Log — maps print / Log.info|warn|error|debug → engine logger
    // -------------------------------------------------------------------------
    static void registerLog(sol::state& lua) {
        sol::table log = lua.create_table();
        log["info"]  = [](const std::string& msg) { LOG_INFO("[Lua] {}", msg); };
        log["warn"]  = [](const std::string& msg) { LOG_WARN("[Lua] {}", msg); };
        log["error"] = [](const std::string& msg) { LOG_ERROR("[Lua] {}", msg); };
        log["debug"] = [](const std::string& msg) { LOG_DEBUG("[Lua] {}", msg); };
        lua["Log"]   = log;

        // Override Lua's built-in print → engine logger
        lua["print"] = [](const std::string& msg) { LOG_INFO("[Lua] {}", msg); };
    }
};

} // namespace NovaEngine
