#pragma once

/**
 * @file ECS.hpp
 * @brief Fichier d'inclusion principal pour le système ECS (Entity Component System)
 *
 * Ce fichier inclut tous les composants du système ECS de NovaEngine.
 * Incluez simplement ce fichier pour avoir accès à toutes les fonctionnalités ECS.
 *
 * @example
 * #include "NovaEngine/ECS.hpp"
 *
 * using namespace NovaEngine::ECS;
 *
 * // Créer un World
 * World world;
 *
 * // Créer une entité
 * Entity entity = world.createEntity("Player");
 *
 * // Ajouter des composants
 * entity.addComponent<TransformComponent>();
 *
 * // Ajouter un système
 * world.addSystem<MovementSystem>();
 *
 * // Mettre à jour
 * world.update(deltaTime);
 */

#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "ECS/System.hpp"
#include "ECS/World.hpp"
