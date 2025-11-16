#pragma once

#include "../Backend/Core/BackendTypes.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace NovaEngine {

using ComponentTypeID = std::string;

/**
 * @brief Base class for all ECS components
 *
 * Components are pure data containers attached to entities.
 * They should not contain logic, only data.
 */
class Component {
public:
    virtual ~Component() = default;

    /**
     * @brief Get the unique type identifier for this component
     * @return The component type ID (class name)
     */
    virtual ComponentTypeID getTypeID() const = 0;

    /**
     * @brief Serialize the component to JSON
     * @param json The JSON object to write to
     */
    virtual void serialize(nlohmann::json& json) const = 0;

    /**
     * @brief Deserialize the component from JSON
     * @param json The JSON object to read from
     */
    virtual void deserialize(const nlohmann::json& json) = 0;
};

/**
 * @brief Helper macro to implement getTypeID() for components
 */
#define COMPONENT_TYPE_ID(TypeName) \
    ComponentTypeID getTypeID() const override { return #TypeName; }

} // namespace NovaEngine
