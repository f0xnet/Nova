#pragma once

#include "NovaEngine/Core/Types.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "System.hpp"
#include <vector>
#include <unordered_map>
#include <queue>
#include <typeindex>
#include <algorithm>

namespace NovaEngine {
namespace ECS {

    /**
     * @brief Structure de données pour une entité dans le World
     */
    struct EntityData {
        EntityHandle handle;
        bool active;
        String tag;
        std::unordered_map<std::type_index, ComponentPtr> components;
    };

    /**
     * @brief Le World est le conteneur principal du système ECS
     *
     * Il gère toutes les entités, composants et systèmes.
     * Il est responsable de la création, destruction et mise à jour de tous ces éléments.
     */
    class World {
    public:
        World();
        ~World();

        /**
         * @brief Crée une nouvelle entité
         * @param tag Tag optionnel pour l'entité
         * @return L'entité créée
         */
        Entity createEntity(const String& tag = "");

        /**
         * @brief Détruit une entité
         * @param entity L'entité à détruire
         */
        void destroyEntity(Entity entity);

        /**
         * @brief Détruit toutes les entités avec un tag donné
         * @param tag Le tag des entités à détruire
         */
        void destroyEntitiesWithTag(const String& tag);

        /**
         * @brief Vérifie si une entité existe et est valide
         */
        bool isEntityValid(EntityHandle handle) const;

        /**
         * @brief Récupère une entité par son handle
         */
        Entity getEntity(EntityHandle handle);

        /**
         * @brief Récupère toutes les entités avec un tag donné
         */
        std::vector<Entity> getEntitiesWithTag(const String& tag) const;

        /**
         * @brief Ajoute un composant à une entité
         */
        template<typename T, typename... Args>
        T& addComponent(EntityHandle handle, Args&&... args);

        /**
         * @brief Retire un composant d'une entité
         */
        template<typename T>
        void removeComponent(EntityHandle handle);

        /**
         * @brief Vérifie si une entité a un composant
         */
        template<typename T>
        bool hasComponent(EntityHandle handle) const;

        /**
         * @brief Récupère un composant d'une entité
         */
        template<typename T>
        T& getComponent(EntityHandle handle);

        /**
         * @brief Récupère un composant d'une entité (version const)
         */
        template<typename T>
        const T& getComponent(EntityHandle handle) const;

        /**
         * @brief Essaie de récupérer un composant
         */
        template<typename T>
        T* tryGetComponent(EntityHandle handle);

        /**
         * @brief Essaie de récupérer un composant (version const)
         */
        template<typename T>
        const T* tryGetComponent(EntityHandle handle) const;

        /**
         * @brief Ajoute un système au World
         * @tparam T Type du système
         * @tparam Args Arguments pour le constructeur du système
         * @return Référence au système créé
         */
        template<typename T, typename... Args>
        T& addSystem(Args&&... args);

        /**
         * @brief Retire un système du World
         */
        template<typename T>
        void removeSystem();

        /**
         * @brief Récupère un système
         */
        template<typename T>
        T* getSystem();

        /**
         * @brief Vérifie si un système existe
         */
        template<typename T>
        bool hasSystem() const;

        /**
         * @brief Met à jour tous les systèmes
         * @param deltaTime Temps écoulé depuis la dernière frame
         */
        void update(f32 deltaTime);

        /**
         * @brief Effectue le rendu via tous les systèmes
         */
        void render();

        /**
         * @brief Récupère toutes les entités qui possèdent certains composants
         */
        template<typename... ComponentTypes>
        std::vector<Entity> getEntitiesWithComponents() const;

        /**
         * @brief Récupère le nombre total d'entités
         */
        size_t getEntityCount() const;

        /**
         * @brief Récupère le nombre d'entités actives
         */
        size_t getActiveEntityCount() const;

        /**
         * @brief Vide le World (détruit toutes les entités et systèmes)
         */
        void clear();

        /**
         * @brief Active ou désactive une entité
         */
        void setEntityActive(EntityHandle handle, bool active);

        /**
         * @brief Vérifie si une entité est active
         */
        bool isEntityActive(EntityHandle handle) const;

        /**
         * @brief Définit le tag d'une entité
         */
        void setEntityTag(EntityHandle handle, const String& tag);

        /**
         * @brief Récupère le tag d'une entité
         */
        const String& getEntityTag(EntityHandle handle) const;

    private:
        /**
         * @brief Traite les entités en attente de destruction
         */
        void processPendingDestruction();

        /**
         * @brief Notifie les systèmes qu'une entité a été modifiée
         */
        void notifyEntityChanged(Entity entity);

        /**
         * @brief Vérifie si une entité correspond aux exigences d'un système
         */
        bool entityMatchesSystem(EntityHandle handle, System* system) const;

        // Données des entités
        std::unordered_map<EntityID, EntityData> m_entities;
        std::queue<EntityHandle> m_pendingDestruction;

        // Générateur d'ID
        EntityID m_nextEntityID;

        // Versions des entités (pour détecter les handles invalides)
        std::unordered_map<EntityID, EntityVersion> m_entityVersions;

        // Systèmes
        std::vector<SystemPtr> m_systems;
        std::unordered_map<std::type_index, System*> m_systemMap;

        // Index pour recherche rapide par tag
        std::unordered_map<String, std::vector<EntityID>> m_tagIndex;

        friend class Entity;
    };

    // ========================================
    // Implémentation des templates
    // ========================================

    template<typename T, typename... Args>
    T& World::addComponent(EntityHandle handle, Args&&... args) {
        auto it = m_entities.find(handle.id);
        if (it == m_entities.end() || it->second.handle.version != handle.version) {
            LOG_ERROR("Tried to add component to invalid entity: {}", handle.id);
            throw std::runtime_error("Invalid entity handle");
        }

        auto& entity = it->second;
        std::type_index typeIndex(typeid(T));

        if (entity.components.find(typeIndex) != entity.components.end()) {
            LOG_WARN("Component already exists on entity: {}", handle.id);
            return *static_cast<T*>(entity.components[typeIndex].get());
        }

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* componentPtr = component.get();

        component->m_owner = Entity(handle, this);
        entity.components[typeIndex] = std::move(component);

        componentPtr->onInit();
        if (componentPtr->isEnabled()) {
            componentPtr->onEnable();
        }

        notifyEntityChanged(Entity(handle, this));

        return *componentPtr;
    }

    template<typename T>
    void World::removeComponent(EntityHandle handle) {
        auto it = m_entities.find(handle.id);
        if (it == m_entities.end() || it->second.handle.version != handle.version) {
            return;
        }

        auto& entity = it->second;
        std::type_index typeIndex(typeid(T));

        auto compIt = entity.components.find(typeIndex);
        if (compIt != entity.components.end()) {
            compIt->second->onDestroy();
            entity.components.erase(compIt);
            notifyEntityChanged(Entity(handle, this));
        }
    }

    template<typename T>
    bool World::hasComponent(EntityHandle handle) const {
        auto it = m_entities.find(handle.id);
        if (it == m_entities.end() || it->second.handle.version != handle.version) {
            return false;
        }

        std::type_index typeIndex(typeid(T));
        return it->second.components.find(typeIndex) != it->second.components.end();
    }

    template<typename T>
    T& World::getComponent(EntityHandle handle) {
        auto it = m_entities.find(handle.id);
        if (it == m_entities.end() || it->second.handle.version != handle.version) {
            LOG_ERROR("Tried to get component from invalid entity: {}", handle.id);
            throw std::runtime_error("Invalid entity handle");
        }

        std::type_index typeIndex(typeid(T));
        auto compIt = it->second.components.find(typeIndex);

        if (compIt == it->second.components.end()) {
            LOG_ERROR("Component not found on entity: {}", handle.id);
            throw std::runtime_error("Component not found");
        }

        return *static_cast<T*>(compIt->second.get());
    }

    template<typename T>
    const T& World::getComponent(EntityHandle handle) const {
        auto it = m_entities.find(handle.id);
        if (it == m_entities.end() || it->second.handle.version != handle.version) {
            LOG_ERROR("Tried to get component from invalid entity: {}", handle.id);
            throw std::runtime_error("Invalid entity handle");
        }

        std::type_index typeIndex(typeid(T));
        auto compIt = it->second.components.find(typeIndex);

        if (compIt == it->second.components.end()) {
            LOG_ERROR("Component not found on entity: {}", handle.id);
            throw std::runtime_error("Component not found");
        }

        return *static_cast<const T*>(compIt->second.get());
    }

    template<typename T>
    T* World::tryGetComponent(EntityHandle handle) {
        auto it = m_entities.find(handle.id);
        if (it == m_entities.end() || it->second.handle.version != handle.version) {
            return nullptr;
        }

        std::type_index typeIndex(typeid(T));
        auto compIt = it->second.components.find(typeIndex);

        if (compIt == it->second.components.end()) {
            return nullptr;
        }

        return static_cast<T*>(compIt->second.get());
    }

    template<typename T>
    const T* World::tryGetComponent(EntityHandle handle) const {
        auto it = m_entities.find(handle.id);
        if (it == m_entities.end() || it->second.handle.version != handle.version) {
            return nullptr;
        }

        std::type_index typeIndex(typeid(T));
        auto compIt = it->second.components.find(typeIndex);

        if (compIt == it->second.components.end()) {
            return nullptr;
        }

        return static_cast<const T*>(compIt->second.get());
    }

    template<typename T, typename... Args>
    T& World::addSystem(Args&&... args) {
        std::type_index typeIndex(typeid(T));

        if (m_systemMap.find(typeIndex) != m_systemMap.end()) {
            LOG_WARN("System already exists");
            return *static_cast<T*>(m_systemMap[typeIndex]);
        }

        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* systemPtr = system.get();

        system->onInit(this);

        m_systems.push_back(std::move(system));
        m_systemMap[typeIndex] = systemPtr;

        // Trier les systèmes par priorité
        std::sort(m_systems.begin(), m_systems.end(),
            [](const SystemPtr& a, const SystemPtr& b) {
                return a->getPriority() < b->getPriority();
            });

        return *systemPtr;
    }

    template<typename T>
    void World::removeSystem() {
        std::type_index typeIndex(typeid(T));

        auto it = m_systemMap.find(typeIndex);
        if (it != m_systemMap.end()) {
            System* system = it->second;
            system->onDestroy();

            m_systems.erase(
                std::remove_if(m_systems.begin(), m_systems.end(),
                    [system](const SystemPtr& s) { return s.get() == system; }),
                m_systems.end()
            );

            m_systemMap.erase(it);
        }
    }

    template<typename T>
    T* World::getSystem() {
        std::type_index typeIndex(typeid(T));
        auto it = m_systemMap.find(typeIndex);

        if (it == m_systemMap.end()) {
            return nullptr;
        }

        return static_cast<T*>(it->second);
    }

    template<typename T>
    bool World::hasSystem() const {
        std::type_index typeIndex(typeid(T));
        return m_systemMap.find(typeIndex) != m_systemMap.end();
    }

    template<typename... ComponentTypes>
    std::vector<Entity> World::getEntitiesWithComponents() const {
        std::vector<Entity> result;

        for (const auto& [id, entityData] : m_entities) {
            if (!entityData.active) continue;

            bool hasAll = ((entityData.components.find(std::type_index(typeid(ComponentTypes)))
                           != entityData.components.end()) && ...);

            if (hasAll) {
                result.push_back(Entity(entityData.handle, const_cast<World*>(this)));
            }
        }

        return result;
    }

} // namespace ECS
} // namespace NovaEngine
