#pragma once

#include "NovaEngine/Core/Types.hpp"
#include "Entity.hpp"
#include <memory>
#include <typeindex>

namespace NovaEngine {
namespace ECS {

    /**
     * @brief Classe de base pour tous les composants
     *
     * Les composants sont des données pures attachées aux entités.
     * Ils ne doivent contenir que des données, pas de logique.
     */
    class Component {
    public:
        virtual ~Component() = default;

        /**
         * @brief Retourne le type du composant
         */
        virtual std::type_index getType() const = 0;

        /**
         * @brief Appelé quand le composant est initialisé
         */
        virtual void onInit() {}

        /**
         * @brief Appelé quand le composant est activé
         */
        virtual void onEnable() {}

        /**
         * @brief Appelé quand le composant est désactivé
         */
        virtual void onDisable() {}

        /**
         * @brief Appelé quand le composant est détruit
         */
        virtual void onDestroy() {}

        /**
         * @brief Active ou désactive le composant
         */
        void setEnabled(bool enabled);

        /**
         * @brief Vérifie si le composant est activé
         */
        bool isEnabled() const;

        /**
         * @brief Retourne l'entité propriétaire du composant
         */
        Entity getOwner() const;

    protected:
        Component() : m_enabled(true), m_owner() {}

    private:
        bool m_enabled;
        Entity m_owner;

        friend class World;
    };

    /**
     * @brief Template de base pour créer des composants typés
     */
    template<typename T>
    class ComponentBase : public Component {
    public:
        std::type_index getType() const override {
            return std::type_index(typeid(T));
        }
    };

    /**
     * @brief Conteneur pour stocker les composants de manière polymorphique
     */
    using ComponentPtr = Unique<Component>;

} // namespace ECS
} // namespace NovaEngine
