#pragma once

/**
 * @file ECS.hpp
 * @brief Main header for the Nova ECS (Entity Component System)
 *
 * Include this file to get access to the complete ECS system.
 */

// Core ECS classes
#include "Component.hpp"
#include "Entity.hpp"
#include "System.hpp"
#include "EntityRegistry.hpp"

// Built-in components
#include "Components.hpp"

// Built-in systems
#include "Systems.hpp"

// Scene management
#include "DefinitionManager.hpp"
#include "Scene.hpp"
#include "SceneManager.hpp"

/**
 * @namespace NovaEngine
 * @brief The main namespace for the Nova Engine
 *
 * ## ECS Architecture
 *
 * The Nova ECS follows a clean Entity-Component-System pattern:
 *
 * - **Entity**: A unique ID with a collection of components
 * - **Component**: Pure data (no logic) attached to entities
 * - **System**: Logic that operates on entities with specific components
 *
 * ## Two-Tier JSON System
 *
 * **Tier 1: Entity Definitions** (loaded once at startup)
 * - `assets/data/definitions/Sprites.json`
 * - `assets/data/definitions/Lights.json`
 * - `assets/data/definitions/Animations.json`
 * - `assets/data/definitions/Audio.json`
 *
 * These files contain the complete definition of each entity type
 * (texture paths, colors, animation frames, etc.)
 *
 * **Tier 2: Scene Files** (loaded on demand)
 * - `assets/data/scenes/interior_house_1.json`
 * - `assets/data/scenes/exterior_forest_1.json`
 *
 * These files only contain entity placement data (which entity, position, rotation, scale).
 * They reference definitions by ID.
 *
 * ## Usage Example
 *
 * ```cpp
 * #include <NovaEngine/ECS/ECS.hpp>
 *
 * class Game : public Application {
 *     NovaEngine::SceneManager m_sceneManager;
 *
 *     void onInitialize() override {
 *         // Initialize scene manager (loads definitions)
 *         m_sceneManager.initialize();
 *
 *         // Load scenes
 *         m_sceneManager.loadScene("assets/data/scenes/house.json", "house");
 *         m_sceneManager.setActiveScene("house");
 *     }
 *
 *     void onUpdate(float deltaTime) override {
 *         m_sceneManager.update(deltaTime);
 *     }
 *
 *     void onRender() override {
 *         m_sceneManager.render();
 *     }
 * };
 * ```
 */
