#pragma once

#include "../ECS/System.hpp"
#include "../ECS/EntityRegistry.hpp"
#include "../ECS/SceneManager.hpp"
#include "../ECS/ComponentFactory.hpp"
#include "../Core/Logger.hpp"
#include "ScriptComponent.hpp"
#include "LuaBindings.hpp"
#include <sol/sol.hpp>

namespace NovaEngine {

// ============================================================================
// ScriptSystem
//
// Owns the Lua VM (sol::state). On first update for each entity, loads and
// compiles its script file into a sandboxed environment, calls init(entity),
// then calls update(entity, dt) every frame.
//
// Usage:
//   scene.addSystem<ScriptSystem>();
//
// Script contract (Lua side):
//   function init(entity)   -- called once after load
//   function update(entity, dt)  -- called every frame
//
// Example script (assets/scripts/player.lua):
//   function init(entity)
//       Log.info("Player initialized, id=" .. entity.id)
//   end
//
//   function update(entity, dt)
//       local t = entity:getTransform()
//       local speed = 150
//       if Input.isKeyPressed("D") then t.position.x = t.position.x + speed * dt end
//       if Input.isKeyPressed("A") then t.position.x = t.position.x - speed * dt end
//       if Input.isKeyPressed("S") then t.position.y = t.position.y + speed * dt end
//       if Input.isKeyPressed("W") then t.position.y = t.position.y - speed * dt end
//   end
// ============================================================================
class ScriptSystem : public System {
public:
    // sm — SceneManager optionnel ; si fourni, expose le global "Scene" en Lua
    explicit ScriptSystem(SceneManager* sm = nullptr) : m_sceneManager(sm) {
        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table,
            sol::lib::io
        );
        LuaBindings::registerAll(m_lua);
        if (m_sceneManager) {
            LuaBindings::registerSceneManager(m_lua, *m_sceneManager);
        }

        // Register ScriptComponent so it can be created by name from the factory
        REGISTER_COMPONENT(ScriptComponent);

        LOG_DEBUG("ScriptSystem initialized (Lua {}.{}.{})",
            LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    }

    std::vector<ComponentTypeID> getRequiredComponents() const override {
        return { ScriptComponent::staticTypeID() };
    }

    void update(float deltaTime, EntityRegistry& registry) override {
        // Met à jour le global "Registry" pour que les scripts accèdent
        // toujours au registry de la scène active courante
        m_lua["Registry"] = &registry;

        auto entities = registry.getEntitiesWith(getRequiredComponents());
        for (auto* entity : entities) {
            auto* script = entity->getComponent<ScriptComponent>();
            if (!script || !script->enabled || script->errored) continue;

            if (!script->loaded) {
                loadScript(entity, script);
            }

            if (!script->errored && script->fnUpdate.valid()) {
                auto result = script->fnUpdate(entity, deltaTime);
                if (!result.valid()) {
                    sol::error err = result;
                    LOG_ERROR("[ScriptSystem] update error in '{}': {}",
                              script->scriptPath, err.what());
                    script->errored = true;
                }
            }
        }
    }

    // Returns the raw Lua state — use to register game-specific bindings.
    sol::state& getLua() { return m_lua; }

    // Reloads a single script (e.g. for hot-reload support later).
    void reloadScript(ScriptComponent* script) {
        if (!script) return;
        script->loaded  = false;
        script->errored = false;
    }

private:
    sol::state    m_lua;
    SceneManager* m_sceneManager = nullptr;

    void loadScript(Entity* entity, ScriptComponent* script) {
        if (script->scriptPath.empty()) {
            LOG_WARN("[ScriptSystem] Entity {} has ScriptComponent with empty scriptPath",
                     entity->getID());
            script->errored = true;
            return;
        }

        try {
            // Each entity gets its own sandboxed environment inheriting globals
            script->env = sol::environment(m_lua, sol::create, m_lua.globals());

            auto load_result = m_lua.script_file(script->scriptPath, script->env,
                                                  sol::script_pass_on_error);
            if (!load_result.valid()) {
                sol::error err = load_result;
                LOG_ERROR("[ScriptSystem] Failed to load '{}': {}",
                          script->scriptPath, err.what());
                script->errored = true;
                return;
            }

            script->fnInit   = script->env["init"];
            script->fnUpdate = script->env["update"];
            script->loaded   = true;

            LOG_DEBUG("[ScriptSystem] Loaded '{}'", script->scriptPath);

            if (script->fnInit.valid()) {
                auto result = script->fnInit(entity);
                if (!result.valid()) {
                    sol::error err = result;
                    LOG_ERROR("[ScriptSystem] init error in '{}': {}",
                              script->scriptPath, err.what());
                    script->errored = true;
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("[ScriptSystem] Exception loading '{}': {}",
                      script->scriptPath, e.what());
            script->errored = true;
        }
    }
};

} // namespace NovaEngine
