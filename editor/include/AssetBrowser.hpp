#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <NovaEngine/Backend/Core/BackendTypes.hpp>
#include <string>
#include <vector>
#include <map>

namespace NovaEditor {

/**
 * @brief Type d'asset
 */
enum class AssetType {
    Texture,
    Animation,
    Audio,
    Scene,
    Definition,
    Shader,
    Font,
    Unknown
};

/**
 * @brief Info sur un asset
 */
struct AssetInfo {
    std::string name;
    std::string path;
    std::string relativePath;
    AssetType type;
    NovaEngine::u64 size;  // Bytes
    std::string extension;

    // Preview (pour textures)
    NovaEngine::TextureHandle thumbnail;
    bool hasThumbnail;
};

/**
 * @brief Navigateur d'assets avec thumbnails et recherche
 */
class AssetBrowser {
public:
    AssetBrowser();
    ~AssetBrowser() = default;

    // Initialisation
    void initialize(const std::string& rootPath);
    void refresh();

    // Navigation
    void setCurrentDirectory(const std::string& path);
    const std::string& getCurrentDirectory() const { return m_currentDirectory; }
    void navigateUp();
    void navigateToRoot();

    // Récupération d'assets
    const std::vector<AssetInfo>& getCurrentAssets() const { return m_currentAssets; }
    const std::vector<std::string>& getSubdirectories() const { return m_subdirectories; }

    // Recherche
    void setSearchQuery(const std::string& query);
    const std::string& getSearchQuery() const { return m_searchQuery; }
    std::vector<AssetInfo> searchAssets(const std::string& query, AssetType type = AssetType::Unknown);

    // Filtre par type
    void setTypeFilter(AssetType type);
    AssetType getTypeFilter() const { return m_typeFilter; }

    // Favoris
    void addFavorite(const std::string& path);
    void removeFavorite(const std::string& path);
    bool isFavorite(const std::string& path) const;
    const std::vector<std::string>& getFavorites() const { return m_favorites; }

    // Utilitaires
    static AssetType getAssetTypeFromExtension(const std::string& extension);
    static const char* getAssetTypeName(AssetType type);

private:
    void scanDirectory(const std::string& path);
    void loadThumbnail(AssetInfo& asset);
    bool matchesSearchQuery(const AssetInfo& asset) const;
    bool matchesTypeFilter(const AssetInfo& asset) const;

private:
    std::string m_rootPath;
    std::string m_currentDirectory;
    std::vector<AssetInfo> m_currentAssets;
    std::vector<AssetInfo> m_allAssets;  // Cache de tous les assets
    std::vector<std::string> m_subdirectories;

    std::string m_searchQuery;
    AssetType m_typeFilter;

    std::vector<std::string> m_favorites;
};

} // namespace NovaEditor
