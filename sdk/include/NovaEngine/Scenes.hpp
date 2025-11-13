#pragma once

/**
 * @file Scenes.hpp
 * @brief Fichier d'inclusion principal pour le système de scènes
 *
 * Ce fichier inclut tous les composants du système de scènes de NovaEngine.
 * Incluez simplement ce fichier pour avoir accès à toutes les fonctionnalités de scènes.
 *
 * @example
 * #include "NovaEngine/Scenes.hpp"
 *
 * using namespace NovaEngine::Scene;
 *
 * // Créer un gestionnaire de scènes
 * SceneManager manager;
 *
 * // Créer des scènes
 * auto exterior = std::make_shared<ExteriorScene>("City");
 * auto house = std::make_shared<InteriorScene>("PlayerHouse", "house");
 *
 * // Ajouter les scènes
 * manager.addScene(exterior, true);
 * manager.addScene(house, false);
 *
 * // Changer de scène
 * manager.changeScene("City");
 *
 * // Mettre à jour
 * manager.update(deltaTime);
 */

#include "Scene/Scene.hpp"
#include "Scene/InteriorScene.hpp"
#include "Scene/ExteriorScene.hpp"
#include "Scene/SceneManager.hpp"
