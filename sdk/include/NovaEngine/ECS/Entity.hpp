#pragma once

#include "NovaEngine/Core/Types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace NovaEngine {
namespace ECS {

    class Component;
    class World;

    /**
     * @brief Identifiant unique d'une entité
     */
    using EntityID = u64;

    /**
     * @brief Version d'une entité pour détecter les entités invalides
     */
    using EntityVersion = u32;

    /**
     * @brief Handle d'entité qui combine l'ID et la version
     */
    struct EntityHandle {
        EntityID id;
        EntityVersion version;

        bool operator==(const EntityHandle& other) const {
            return id == other.id && version == other.version;
        }

        bool operator!=(const EntityHandle& other) const {
            return !(*this == other);
        }

        bool isValid() const {
            return id != 0;
        }
    };

    /**
     * @brief Classe représentant une entité dans le système ECS
     *
     * Une entité est un conteneur d'ID qui peut avoir des composants attachés.
     * Les entités sont créées et gérées par la classe World.
     */
    class Entity {
    public:
        /**
         * @brief Constructeur par défaut - crée une entité invalide
         */
        Entity();

        /**
         * @brief Constructeur avec ID et World
         */
        Entity(EntityHandle handle, World* world);

        /**
         * @brief Vérifie si l'entité est valide
         */
        bool isValid() const;

        /**
         * @brief Retourne l'ID de l'entité
         */
        EntityID getID() const;

        /**
         * @brief Retourne le handle complet de l'entité
         */
        EntityHandle getHandle() const;

        /**
         * @brief Retourne la version de l'entité
         */
        EntityVersion getVersion() const;

        /**
         * @brief Ajoute un composant à l'entité
         * @tparam T Type du composant
         * @tparam Args Arguments pour le constructeur du composant
         * @return Référence au composant créé
         */
        template<typename T, typename... Args>
        T& addComponent(Args&&... args);

        /**
         * @brief Retire un composant de l'entité
         * @tparam T Type du composant à retirer
         */
        template<typename T>
        void removeComponent();

        /**
         * @brief Vérifie si l'entité possède un composant
         * @tparam T Type du composant
         * @return true si le composant existe
         */
        template<typename T>
        bool hasComponent() const;

        /**
         * @brief Récupère un composant de l'entité
         * @tparam T Type du composant
         * @return Référence au composant
         */
        template<typename T>
        T& getComponent();

        /**
         * @brief Récupère un composant de l'entité (version const)
         * @tparam T Type du composant
         * @return Référence const au composant
         */
        template<typename T>
        const T& getComponent() const;

        /**
         * @brief Essaie de récupérer un composant
         * @tparam T Type du composant
         * @return Pointeur vers le composant ou nullptr
         */
        template<typename T>
        T* tryGetComponent();

        /**
         * @brief Essaie de récupérer un composant (version const)
         * @tparam T Type du composant
         * @return Pointeur const vers le composant ou nullptr
         */
        template<typename T>
        const T* tryGetComponent() const;

        /**
         * @brief Détruit l'entité
         */
        void destroy();

        /**
         * @brief Active ou désactive l'entité
         */
        void setActive(bool active);

        /**
         * @brief Vérifie si l'entité est active
         */
        bool isActive() const;

        /**
         * @brief Définit le tag de l'entité
         */
        void setTag(const String& tag);

        /**
         * @brief Récupère le tag de l'entité
         */
        const String& getTag() const;

        /**
         * @brief Opérateurs de comparaison
         */
        bool operator==(const Entity& other) const;
        bool operator!=(const Entity& other) const;

    private:
        EntityHandle m_handle;
        World* m_world;

        friend class World;
    };

} // namespace ECS
} // namespace NovaEngine

// Spécialisation du hash pour EntityHandle
namespace std {
    template<>
    struct hash<NovaEngine::ECS::EntityHandle> {
        size_t operator()(const NovaEngine::ECS::EntityHandle& handle) const {
            return hash<NovaEngine::u64>()(handle.id) ^ (hash<NovaEngine::u32>()(handle.version) << 1);
        }
    };
}
