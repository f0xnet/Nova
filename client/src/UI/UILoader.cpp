#include "NovaEngine/UI/UILoader.hpp"
#include "NovaEngine/Core/Logger.hpp"
#include "NovaEngine/Core/ConfigManager.hpp"
#include "NovaEngine/UI/Components/Button.hpp"
#include "NovaEngine/UI/Components/Panel.hpp"
#include "NovaEngine/UI/Components/Text.hpp"
#include "NovaEngine/UI/Components/Image.hpp"
#include "NovaEngine/UI/Components/TextInput.hpp"
#include "NovaEngine/Backend/BackendManager.hpp"

#include <fstream>
#include <sstream>

namespace NovaEngine {

UILoader::UILoader() {
    LOG_DEBUG("UILoader created (extended for legacy JSON format)");
}

UILoader::~UILoader() {
    LOG_DEBUG("UILoader destroyed");
}

bool UILoader::loadFromFile(const std::string& path, UIManager& uiManager) {
    LOG_INFO("Loading UI layout from file: {}", path);

    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open UI file: {}", path);
        return false;
    }

    nlohmann::json jsonData;
    try {
        file >> jsonData;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse UI JSON: {}", e.what());
        return false;
    }

    return loadFromData(jsonData, uiManager);
}

bool UILoader::loadUI(const ID& uiID, UIManager& uiManager) {
    std::string jsonFile = "data/ui/json/" + uiID + ".json";
    LOG_INFO("Loading UI by UIID: {} from {}", uiID, jsonFile);
    return loadFromFile(jsonFile, uiManager);
}

bool UILoader::loadFromData(const nlohmann::json& jsonData, UIManager& uiManager) {
    UILayoutData layoutData;
    from_json(jsonData, layoutData);

    LOG_INFO("Loading UI layout: {} (UIID: {}, Layers: {})", layoutData.name, layoutData.uiID, layoutData.layers);

    if (!layoutData.background.empty()) {
        loadTextureResource(layoutData.background, "bg_" + layoutData.uiID);
    }

    if (jsonData.contains("buttons")) {
        parseButtons(jsonData["buttons"], uiManager, layoutData);
    }
    if (jsonData.contains("images")) {
        parseImages(jsonData["images"], uiManager, layoutData);
    }
    if (jsonData.contains("text")) {
        parseTexts(jsonData["text"], uiManager, layoutData);
    }
    if (jsonData.contains("userInput")) {
        parseInputs(jsonData["userInput"], uiManager, layoutData);
    }

    LOG_INFO("UI layout '{}' loaded successfully", layoutData.name);
    return true;
}

void UILoader::parseButtons(const nlohmann::json& buttonsJson, UIManager& uiManager, const UILayoutData& layoutData) {
    for (size_t i = 0; i < buttonsJson.size(); ++i) {
        const auto& buttonData = buttonsJson[i];
        
        auto button = std::make_shared<Button>();
        
        ID buttonID = buttonData.value("ID", generateComponentID("btn", layoutData.uiID, static_cast<i32>(i)));
        button->setID(buttonID);
        button->setUIID(layoutData.uiID);
        button->setGroupID(buttonData.value("groupID", "main"));
        button->setLayer(buttonData.value("layer", 0));
        button->setActive(buttonData.value("isActive", true));
        
        Vec2f rescaledPos = applyRescale(
            static_cast<f32>(buttonData.value("x", 0)),
            static_cast<f32>(buttonData.value("y", 0))
        );
        Vec2f rescaledSize = applyRescaleSize(
            static_cast<f32>(buttonData.value("width", 100)),
            static_cast<f32>(buttonData.value("height", 30))
        );
        
        button->setPosition(rescaledPos);
        button->setSize(rescaledSize);
        
        button->setHaveText(buttonData.value("haveText", true));
        
        if (buttonData.contains("text")) {
            button->setText(buttonData["text"].get<std::string>());
        }
        
        if (buttonData.contains("font")) {
            std::string fontPath = buttonData["font"].get<std::string>();
            FontHandle fontHandle = FONTS().loadFont(fontPath);
            if (fontHandle != INVALID_HANDLE) {
                button->setFont(fontHandle);
            }
        }
        
        if (buttonData.contains("fontSize")) {
            u32 rescaledFontSize = applyRescaleFontSize(buttonData.value("fontSize", 16));
            button->setFontSize(rescaledFontSize);
        }
        
        if (buttonData.contains("color")) {
            std::string colorStr = buttonData["color"].get<std::string>();
            button->setTextColor(parseColor(colorStr));
        }
        
        button->setAction(buttonData.value("action", ""));
        button->setValue(buttonData.value("value", ""));
        
        std::string path = buttonData.value("path", "");
        std::string pathHover = buttonData.value("path_hover", "");
        std::string pathPressed = buttonData.value("path_pressed", "");

        if (!path.empty()) {
            TextureHandle normalHandle = RESOURCES().loadTexture(path);
            TextureHandle hoverHandle = INVALID_HANDLE;
            TextureHandle pressedHandle = INVALID_HANDLE;

            if (!pathHover.empty()) {
                hoverHandle = RESOURCES().loadTexture(pathHover);
            }

            if (!pathPressed.empty()) {
                pressedHandle = RESOURCES().loadTexture(pathPressed);
            }

            button->setTextures(normalHandle, hoverHandle, pressedHandle);
            LOG_DEBUG("Button '{}' textures loaded (normal={}, hover={}, pressed={})", 
                     buttonID, normalHandle, hoverHandle, pressedHandle);
        }

        button->setOnClickWithAction([&uiManager](const std::string& action, const std::string& value, const ID& id) {
            uiManager.handleAction(action, value, id);
        });

        uiManager.addComponent(button);
        LOG_DEBUG("Button '{}' added (action: '{}', group: '{}')", buttonID, button->getAction(), button->getGroupID());
    }
}

void UILoader::parseImages(const nlohmann::json& imagesJson, UIManager& uiManager, const UILayoutData& layoutData) {
    for (size_t i = 0; i < imagesJson.size(); ++i) {
        const auto& imageData = imagesJson[i];
        
        auto image = std::make_shared<Image>();
        
        ID imageID = imageData.value("ID", generateComponentID("img", layoutData.uiID, static_cast<i32>(i)));
        image->setID(imageID);
        image->setUIID(layoutData.uiID);
        image->setGroupID(imageData.value("groupID", "main"));
        image->setLayer(imageData.value("layer", 0));
        
        Vec2f rescaledPos = applyRescale(
            static_cast<f32>(imageData.value("x", 0)),
            static_cast<f32>(imageData.value("y", 0))
        );
        Vec2f rescaledSize = applyRescaleSize(
            static_cast<f32>(imageData.value("width", 100)),
            static_cast<f32>(imageData.value("height", 100))
        );
        
        image->setPosition(rescaledPos);
        image->setSize(rescaledSize);
        
        std::string imagePath = imageData.value("image", "");
        if (!imagePath.empty()) {
            LOG_ERROR("🖼️ Image '{}': loading texture from '{}'", imageID, imagePath);
            TextureHandle texHandle = RESOURCES().loadTexture(imagePath);
            LOG_ERROR("   Result: handle={} (0=INVALID)", texHandle);
            
            if (texHandle != INVALID_HANDLE) {
                image->setTexture(texHandle);
                LOG_ERROR("   ✅ Texture SET for '{}'", imageID);
            } else {
                LOG_ERROR("   ❌ Failed to load texture for '{}'", imageID);
            }
        }
        
        uiManager.addComponent(image);
        LOG_DEBUG("Image '{}' added", imageID);
    }
}

void UILoader::parseTexts(const nlohmann::json& textsJson, UIManager& uiManager, const UILayoutData& layoutData) {
    for (size_t i = 0; i < textsJson.size(); ++i) {
        const auto& textData = textsJson[i];
        
        auto text = std::make_shared<Text>();
        
        ID textID = textData.value("ID", generateComponentID("txt", layoutData.uiID, static_cast<i32>(i)));
        text->setID(textID);
        text->setUIID(layoutData.uiID);
        text->setGroupID(textData.value("groupID", "main"));
        text->setLayer(textData.value("layer", 0));
        text->setActive(textData.value("isActive", true));
        
        Vec2f rescaledPos = applyRescale(
            static_cast<f32>(textData.value("x", 0)),
            static_cast<f32>(textData.value("y", 0))
        );
        
        text->setPosition(rescaledPos);
        
        std::string content = textData.value("content", textData.value("text", ""));
        text->setString(content);
        
        std::string fontPath = textData.value("font", "");
        if (!fontPath.empty()) {
            FontHandle fontHandle = FONTS().loadFont(fontPath);
            if (fontHandle != INVALID_HANDLE) {
                text->setFont(fontHandle);
            }
        }
        
        u32 rescaledFontSize = applyRescaleFontSize(textData.value("fontSize", 20));
        text->setCharacterSize(rescaledFontSize);
        
        std::string colorStr = textData.value("color", "255,255,255");
        text->setTextColor(parseColor(colorStr));
        
        uiManager.addComponent(text);
        LOG_DEBUG("Text '{}' added: '{}'", textID, content);
    }
}

void UILoader::parseInputs(const nlohmann::json& inputsJson, UIManager& uiManager, const UILayoutData& layoutData) {
    for (size_t i = 0; i < inputsJson.size(); ++i) {
        const auto& inputData = inputsJson[i];
        
        auto textInput = std::make_shared<TextInput>();
        
        ID inputID = inputData.value("ID", generateComponentID("input", layoutData.uiID, static_cast<i32>(i)));
        textInput->setID(inputID);
        textInput->setUIID(layoutData.uiID);
        textInput->setGroupID(inputData.value("groupID", "main"));
        textInput->setLayer(inputData.value("layer", 0));
        
        Vec2f rescaledPos = applyRescale(
            static_cast<f32>(inputData.value("x", 0)),
            static_cast<f32>(inputData.value("y", 0))
        );
        Vec2f rescaledSize = applyRescaleSize(
            static_cast<f32>(inputData.value("width", 200)),
            static_cast<f32>(inputData.value("height", 30))
        );
        
        textInput->setPosition(rescaledPos);
        textInput->setSize(rescaledSize);
        
        uiManager.addComponent(textInput);
        LOG_DEBUG("TextInput '{}' added", inputID);
    }
}

Color UILoader::parseColor(const std::string& colorStr) const {
    std::stringstream ss(colorStr);
    std::string token;
    u8 r = 255, g = 255, b = 255;
    
    if (std::getline(ss, token, ',')) r = static_cast<u8>(std::stoi(token));
    if (std::getline(ss, token, ',')) g = static_cast<u8>(std::stoi(token));
    if (std::getline(ss, token, ',')) b = static_cast<u8>(std::stoi(token));
    
    return Color(r, g, b, 255);
}

ID UILoader::generateComponentID(const ID& baseID, const ID& uiID, i32 index) const {
    return baseID + "_" + uiID + "_" + std::to_string(index);
}

// Dans UILoader.cpp, remplacer les 3 fonctions par :

Vec2f UILoader::applyRescale(f32 x, f32 y) const {
    const auto& displayConfig = ConfigManager::getInstance().getDisplayConfig();
    const Vec2u nativeResolution(displayConfig.nativeWidth, displayConfig.nativeHeight);
    
    u32 currentWidth = WINDOW().getWidth();
    u32 currentHeight = WINDOW().getHeight();
    
    // Calculer la position en pourcentage du design 4K
    f32 percentX = x / static_cast<f32>(nativeResolution.x);
    f32 percentY = y / static_cast<f32>(nativeResolution.y);
    
    // Appliquer ce pourcentage à la résolution actuelle
    return Vec2f(percentX * currentWidth, percentY * currentHeight);
}

Vec2f UILoader::applyRescaleSize(f32 width, f32 height) const {
    const auto& displayConfig = ConfigManager::getInstance().getDisplayConfig();
    const Vec2u nativeResolution(displayConfig.nativeWidth, displayConfig.nativeHeight);
    
    u32 currentWidth = WINDOW().getWidth();
    u32 currentHeight = WINDOW().getHeight();
    
    f32 scaleX = static_cast<f32>(currentWidth) / static_cast<f32>(nativeResolution.x);
    f32 scaleY = static_cast<f32>(currentHeight) / static_cast<f32>(nativeResolution.y);
    
    return Vec2f(width * scaleX, height * scaleY);
}

u32 UILoader::applyRescaleFontSize(u32 fontSize) const {
    const auto& displayConfig = ConfigManager::getInstance().getDisplayConfig();
    const Vec2u nativeResolution(displayConfig.nativeWidth, displayConfig.nativeHeight);
    
    u32 currentHeight = WINDOW().getHeight();
    
    f32 scaleY = static_cast<f32>(currentHeight) / static_cast<f32>(nativeResolution.y);
    
    return static_cast<u32>(fontSize * scaleY);
}

bool UILoader::loadTextureResource(const std::string& path, const ID& resourceID) {
    if (path.empty()) return false;
    TextureHandle handle = RESOURCES().loadTexture(path);
    return handle != INVALID_HANDLE;
}

bool UILoader::loadFontResource(const std::string& path, const ID& resourceID) {
    if (path.empty()) return false;
    FontHandle handle = FONTS().loadFont(path);
    return handle != INVALID_HANDLE;
}

void from_json(const nlohmann::json& j, UILayoutData& data) {
    data.name = j.value("name", "");
    data.uiID = j.value("UIID", "");
    data.language = j.value("language", "en");
    data.description = j.value("description", "");
    data.background = j.value("background", "");
    data.layers = j.value("layers", 1);
}

}