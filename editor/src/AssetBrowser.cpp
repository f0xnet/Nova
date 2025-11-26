#include "../include/AssetBrowser.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace NovaEditor {

AssetBrowser::AssetBrowser()
    : m_typeFilter(AssetType::Unknown)
{
}

void AssetBrowser::initialize(const std::string& rootPath) {
    m_rootPath = rootPath;
    m_currentDirectory = rootPath;

    LOG_INFO("Asset Browser initialized: {}", rootPath);
    refresh();
}

void AssetBrowser::refresh() {
    m_currentAssets.clear();
    m_subdirectories.clear();

    scanDirectory(m_currentDirectory);

    LOG_INFO("Asset Browser refreshed: {} assets, {} subdirectories",
        m_currentAssets.size(), m_subdirectories.size());
}

void AssetBrowser::setCurrentDirectory(const std::string& path) {
    if (!fs::exists(path) || !fs::is_directory(path)) {
        LOG_WARN("Directory does not exist: {}", path);
        return;
    }

    m_currentDirectory = path;
    refresh();
    LOG_INFO("Navigated to: {}", path);
}

void AssetBrowser::navigateUp() {
    fs::path current(m_currentDirectory);
    if (current.has_parent_path() && current.parent_path() != current) {
        setCurrentDirectory(current.parent_path().string());
    }
}

void AssetBrowser::navigateToRoot() {
    setCurrentDirectory(m_rootPath);
}

void AssetBrowser::setSearchQuery(const std::string& query) {
    m_searchQuery = query;
    refresh();
}

std::vector<AssetInfo> AssetBrowser::searchAssets(const std::string& query, AssetType type) {
    std::vector<AssetInfo> results;

    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& asset : m_allAssets) {
        std::string lowerName = asset.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        bool matchesQuery = lowerName.find(lowerQuery) != std::string::npos;
        bool matchesType = (type == AssetType::Unknown) || (asset.type == type);

        if (matchesQuery && matchesType) {
            results.push_back(asset);
        }
    }

    return results;
}

void AssetBrowser::setTypeFilter(AssetType type) {
    m_typeFilter = type;
    refresh();
}

void AssetBrowser::addFavorite(const std::string& path) {
    if (!isFavorite(path)) {
        m_favorites.push_back(path);
        LOG_INFO("Added to favorites: {}", path);
    }
}

void AssetBrowser::removeFavorite(const std::string& path) {
    auto it = std::find(m_favorites.begin(), m_favorites.end(), path);
    if (it != m_favorites.end()) {
        m_favorites.erase(it);
        LOG_INFO("Removed from favorites: {}", path);
    }
}

bool AssetBrowser::isFavorite(const std::string& path) const {
    return std::find(m_favorites.begin(), m_favorites.end(), path) != m_favorites.end();
}

AssetType AssetBrowser::getAssetTypeFromExtension(const std::string& extension) {
    if (extension == ".png" || extension == ".jpg" || extension == ".bmp") return AssetType::Texture;
    if (extension == ".wav" || extension == ".ogg" || extension == ".mp3") return AssetType::Audio;
    if (extension == ".json") return AssetType::Scene;  // ou Definition
    if (extension == ".frag" || extension == ".vert" || extension == ".glsl") return AssetType::Shader;
    if (extension == ".ttf" || extension == ".otf") return AssetType::Font;
    return AssetType::Unknown;
}

const char* AssetBrowser::getAssetTypeName(AssetType type) {
    switch (type) {
        case AssetType::Texture: return "Texture";
        case AssetType::Animation: return "Animation";
        case AssetType::Audio: return "Audio";
        case AssetType::Scene: return "Scene";
        case AssetType::Definition: return "Definition";
        case AssetType::Shader: return "Shader";
        case AssetType::Font: return "Font";
        default: return "Unknown";
    }
}

void AssetBrowser::scanDirectory(const std::string& path) {
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_directory()) {
                m_subdirectories.push_back(entry.path().filename().string());
            }
            else if (entry.is_regular_file()) {
                AssetInfo asset;
                asset.name = entry.path().filename().string();
                asset.path = entry.path().string();
                asset.relativePath = fs::relative(entry.path(), m_rootPath).string();
                asset.extension = entry.path().extension().string();
                asset.type = getAssetTypeFromExtension(asset.extension);
                asset.size = entry.file_size();
                asset.hasThumbnail = false;

                // Filtrer selon recherche et type
                if (matchesSearchQuery(asset) && matchesTypeFilter(asset)) {
                    // Charger thumbnail si texture
                    if (asset.type == AssetType::Texture) {
                        loadThumbnail(asset);
                    }

                    m_currentAssets.push_back(asset);
                }

                // Ajouter au cache global
                m_allAssets.push_back(asset);
            }
        }

        // Trier alphabétiquement
        std::sort(m_subdirectories.begin(), m_subdirectories.end());
        std::sort(m_currentAssets.begin(), m_currentAssets.end(),
            [](const AssetInfo& a, const AssetInfo& b) {
                return a.name < b.name;
            });

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to scan directory: {}", e.what());
    }
}

void AssetBrowser::loadThumbnail(AssetInfo& asset) {
    // TODO: Charger thumbnail via ResourceManager
    // asset.thumbnail = RESOURCES().loadTexture(asset.path);
    // asset.hasThumbnail = (asset.thumbnail != INVALID_HANDLE);
    asset.hasThumbnail = false;
}

bool AssetBrowser::matchesSearchQuery(const AssetInfo& asset) const {
    if (m_searchQuery.empty()) return true;

    std::string lowerName = asset.name;
    std::string lowerQuery = m_searchQuery;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    return lowerName.find(lowerQuery) != std::string::npos;
}

bool AssetBrowser::matchesTypeFilter(const AssetInfo& asset) const {
    if (m_typeFilter == AssetType::Unknown) return true;
    return asset.type == m_typeFilter;
}

} // namespace NovaEditor
