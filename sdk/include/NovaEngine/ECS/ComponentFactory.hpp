#pragma once
#include "Component.hpp"
#include <functional>
#include <unordered_map>
#include <memory>

namespace NovaEngine {

/**
 * @brief Factory permettant de créer des composants par leur TypeID (string).
 *
 * Utilisé principalement par le système de scripting et le chargement JSON
 * pour instancier des composants sans connaître leur type C++ à la compilation.
 *
 * Usage :
 *   ComponentFactory::get().registerComponent(
 *       MyComponent::staticTypeID(),
 *       []() { return std::make_unique<MyComponent>(); }
 *   );
 *   auto comp = ComponentFactory::get().create("MyComponent");
 */
class ComponentFactory {
public:
    using Creator = std::function<std::unique_ptr<Component>()>;

    static ComponentFactory& get() {
        static ComponentFactory instance;
        return instance;
    }

    ComponentFactory(const ComponentFactory&) = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;

    void registerComponent(const ComponentTypeID& typeID, Creator creator) {
        m_creators[typeID] = std::move(creator);
    }

    std::unique_ptr<Component> create(const ComponentTypeID& typeID) const {
        auto it = m_creators.find(typeID);
        if (it == m_creators.end()) return nullptr;
        return it->second();
    }

    bool isRegistered(const ComponentTypeID& typeID) const {
        return m_creators.find(typeID) != m_creators.end();
    }

    const std::unordered_map<ComponentTypeID, Creator>& registeredTypes() const {
        return m_creators;
    }

private:
    ComponentFactory() = default;
    std::unordered_map<ComponentTypeID, Creator> m_creators;
};

#define REGISTER_COMPONENT(Type) \
    NovaEngine::ComponentFactory::get().registerComponent( \
        Type::staticTypeID(), \
        []() -> std::unique_ptr<NovaEngine::Component> { return std::make_unique<Type>(); })

} // namespace NovaEngine
