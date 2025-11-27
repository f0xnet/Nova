#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <NovaEngine/ECS/DefinitionManager.hpp>
#include <string>
#include <vector>
#include <unordered_map>

namespace NovaEditor {

/**
 * @brief Catégorie d'entités dans la palette
 */
enum class EntityCategory {
    Sprites,
    Lights,
    NPCs,
    Activators,
    Audio,
    Decorations,
    Waypoints
};

/**
 * @brief Item dans la palette d'entités
 */
struct PaletteItem {
    std::string id;                  // ID de la définition
    std::string displayName;         // Nom à afficher
    std::string description;         // Description
    EntityCategory category;         // Catégorie
    std::string iconPath;            // Chemin vers icône (optionnel)
    std::string entityType;          // Type d'entité ("sprite", "light", etc.)

    PaletteItem() = default;
    PaletteItem(const std::string& id, const std::string& name, EntityCategory cat, const std::string& type)
        : id(id), displayName(name), category(cat), entityType(type) {}
};

/**
 * @brief Palette d'entités disponibles pour placement
 *
 * Charge toutes les définitions et les organise par catégorie
 * pour faciliter la sélection dans l'éditeur.
 */
class EntityPalette {
public:
    EntityPalette();
    ~EntityPalette() = default;

    // Chargement
    void loadFromDefinitionManager(const NovaEngine::DefinitionManager& defManager);

    // Recherche
    const std::vector<PaletteItem>& getItemsByCategory(EntityCategory category) const;
    const PaletteItem* getItem(const std::string& id) const;

    // Toutes catégories
    std::vector<EntityCategory> getAllCategories() const;
    std::string getCategoryName(EntityCategory category) const;

    // Recherche textuelle
    std::vector<PaletteItem> search(const std::string& query) const;

    // Favoris (pour futur)
    void addToFavorites(const std::string& id);
    void removeFromFavorites(const std::string& id);
    const std::vector<PaletteItem>& getFavorites() const;

private:
    void loadSprites(const NovaEngine::DefinitionManager& defManager);
    void loadLights(const NovaEngine::DefinitionManager& defManager);
    void loadNPCs(const NovaEngine::DefinitionManager& defManager);
    void loadActivators(const NovaEngine::DefinitionManager& defManager);
    void loadAudio(const NovaEngine::DefinitionManager& defManager);

    void addItem(const PaletteItem& item);

private:
    // Items organisés par catégorie
    std::unordered_map<EntityCategory, std::vector<PaletteItem>> m_itemsByCategory;

    // Map ID → Item pour recherche rapide
    std::unordered_map<std::string, PaletteItem> m_itemsById;

    // Favoris
    std::vector<PaletteItem> m_favorites;
};

} // namespace NovaEditor
