#pragma once

#include "NovaEngine/Core/Types.hpp"
#include "Entity.hpp"
#include <vector>
#include <typeindex>
#include <set>

namespace NovaEngine {
namespace ECS {

    class World;

    /**
     * @brief Classe de base pour tous les systèmes
     *
     * Les systèmes contiennent la logique qui opère sur les composants.
     * Ils sont mis à jour par le World à chaque frame.
     */
    class System {
    public:
        virtual ~System() = default;

        /**
         * @brief Appelé quand le système est initialisé
         * @param world Référence au world
         */
        virtual void onInit(World* world) {
            m_world = world;
        }

        /**
         * @brief Appelé au début de chaque frame
         * @param deltaTime Temps écoulé depuis la dernière frame
         */
        virtual void onUpdate(f32 deltaTime) {}

        /**
         * @brief Appelé pour le rendu
         */
        virtual void onRender() {}

        /**
         * @brief Appelé à la fin de chaque frame
         */
        virtual void onLateUpdate(f32 deltaTime) {}

        /**
         * @brief Appelé quand le système est détruit
         */
        virtual void onDestroy() {}

        /**
         * @brief Appelé quand une entité est ajoutée au système
         */
        virtual void onEntityAdded(Entity entity) {}

        /**
         * @brief Appelé quand une entité est retirée du système
         */
        virtual void onEntityRemoved(Entity entity) {}

        /**
         * @brief Active ou désactive le système
         */
        void setEnabled(bool enabled);

        /**
         * @brief Vérifie si le système est activé
         */
        bool isEnabled() const;

        /**
         * @brief Définit la priorité du système (ordre d'exécution)
         * Plus la priorité est basse, plus le système sera exécuté tôt
         */
        void setPriority(i32 priority);

        /**
         * @brief Retourne la priorité du système
         */
        i32 getPriority() const;

        /**
         * @brief Retourne les types de composants requis par ce système
         */
        const std::set<std::type_index>& getRequiredComponents() const;

    protected:
        System() : m_enabled(true), m_priority(0), m_world(nullptr) {}

        /**
         * @brief Ajoute un type de composant requis
         */
        template<typename T>
        void requireComponent() {
            m_requiredComponents.insert(std::type_index(typeid(T)));
        }

        /**
         * @brief Récupère toutes les entités qui correspondent aux composants requis
         */
        std::vector<Entity> getEntities() const;

        /**
         * @brief Accès au World
         */
        World* getWorld() const { return m_world; }

    private:
        bool m_enabled;
        i32 m_priority;
        World* m_world;
        std::set<std::type_index> m_requiredComponents;

        friend class World;
    };

    /**
     * @brief Pointeur unique vers un système
     */
    using SystemPtr = Unique<System>;

} // namespace ECS
} // namespace NovaEngine
