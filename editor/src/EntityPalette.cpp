#include "../include/EntityPalette.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <algorithm>

namespace NovaEditor {

EntityPalette::EntityPalette() {
    LOG_INFO("EntityPalette initialized");
}

void EntityPalette::loadFromDefinitionManager(const NovaEngine::DefinitionManager& defManager) {
    LOG_INFO("Loading entity palette from definitions...");

    loadSprites(defManager);
    loadLights(defManager);
    loadNPCs(defManager);
    loadActivators(defManager);
    loadAudio(defManager);

    LOG_INFO("Entity palette loaded: {} categories", m_itemsByCategory.size());
}

const std::vector<PaletteItem>& EntityPalette::getItemsByCategory(EntityCategory category) const {
    static std::vector<PaletteItem> empty;

    auto it = m_itemsByCategory.find(category);
    if (it != m_itemsByCategory.end()) {
        return it->second;
    }

    return empty;
}

const PaletteItem* EntityPalette::getItem(const std::string& id) const {
    auto it = m_itemsById.find(id);
    if (it != m_itemsById.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<EntityCategory> EntityPalette::getAllCategories() const {
    std::vector<EntityCategory> categories;
    categories.reserve(m_itemsByCategory.size());

    for (const auto& pair : m_itemsByCategory) {
        categories.push_back(pair.first);
    }

    return categories;
}

std::string EntityPalette::getCategoryName(EntityCategory category) const {
    switch (category) {
        case EntityCategory::Sprites: return "Sprites";
        case EntityCategory::Lights: return "Lights";
        case EntityCategory::NPCs: return "NPCs";
        case EntityCategory::Activators: return "Activators";
        case EntityCategory::Audio: return "Audio";
        case EntityCategory::Decorations: return "Decorations";
        case EntityCategory::Waypoints: return "Waypoints";
        default: return "Unknown";
    }
}

std::vector<PaletteItem> EntityPalette::search(const std::string& query) const {
    std::vector<PaletteItem> results;

    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& pair : m_itemsById) {
        std::string lowerName = pair.second.displayName;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        if (lowerName.find(lowerQuery) != std::string::npos) {
            results.push_back(pair.second);
        }
    }

    return results;
}

void EntityPalette::addToFavorites(const std::string& id) {
    const PaletteItem* item = getItem(id);
    if (item) {
        auto it = std::find_if(m_favorites.begin(), m_favorites.end(),
            [&id](const PaletteItem& fav) { return fav.id == id; });

        if (it == m_favorites.end()) {
            m_favorites.push_back(*item);
            LOG_DEBUG("Added '{}' to favorites", id);
        }
    }
}

void EntityPalette::removeFromFavorites(const std::string& id) {
    auto it = std::find_if(m_favorites.begin(), m_favorites.end(),
        [&id](const PaletteItem& fav) { return fav.id == id; });

    if (it != m_favorites.end()) {
        m_favorites.erase(it);
        LOG_DEBUG("Removed '{}' from favorites", id);
    }
}

const std::vector<PaletteItem>& EntityPalette::getFavorites() const {
    return m_favorites;
}

void EntityPalette::loadSprites(const NovaEngine::DefinitionManager& defManager) {
    // Note: DefinitionManager devrait exposer une méthode pour itérer toutes les définitions
    // Pour l'instant, on suppose que les IDs courants sont connus ou on les charge depuis JSON

    // Exemple d'items (à remplacer par itération réelle des définitions)
    addItem(PaletteItem("torch", "Torch", EntityCategory::Sprites, "sprite"));
    addItem(PaletteItem("player", "Player", EntityCategory::Sprites, "sprite"));

    LOG_DEBUG("Loaded sprites into palette");
}

void EntityPalette::loadLights(const NovaEngine::DefinitionManager& defManager) {
    addItem(PaletteItem("torch_light", "Torch Light", EntityCategory::Lights, "light"));
    addItem(PaletteItem("sunlight", "Sunlight", EntityCategory::Lights, "light"));

    LOG_DEBUG("Loaded lights into palette");
}

void EntityPalette::loadNPCs(const NovaEngine::DefinitionManager& defManager) {
    addItem(PaletteItem("merchant", "Merchant", EntityCategory::NPCs, "npc"));
    addItem(PaletteItem("guard", "Guard", EntityCategory::NPCs, "npc"));

    LOG_DEBUG("Loaded NPCs into palette");
}

void EntityPalette::loadActivators(const NovaEngine::DefinitionManager& defManager) {
    addItem(PaletteItem("door_trigger", "Door Trigger", EntityCategory::Activators, "activator"));
    addItem(PaletteItem("pressure_plate", "Pressure Plate", EntityCategory::Activators, "activator"));

    LOG_DEBUG("Loaded activators into palette");
}

void EntityPalette::loadAudio(const NovaEngine::DefinitionManager& defManager) {
    addItem(PaletteItem("ambient_music", "Ambient Music", EntityCategory::Audio, "audio"));
    addItem(PaletteItem("fireplace_sound", "Fireplace Sound", EntityCategory::Audio, "audio"));

    LOG_DEBUG("Loaded audio into palette");
}

void EntityPalette::addItem(const PaletteItem& item) {
    m_itemsByCategory[item.category].push_back(item);
    m_itemsById[item.id] = item;
}

} // namespace NovaEditor
