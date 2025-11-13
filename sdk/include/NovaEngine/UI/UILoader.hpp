#pragma once

#include "../Core/Types.hpp"
#include "../Backend/Core/BackendTypes.hpp"
#include "UIManager.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace NovaEngine {

struct UILayoutData {
    std::string name;
    ID uiID;
    std::string language;
    std::string description;
    std::string background;
    i32 layers;
    
    UILayoutData() : layers(1) {}
};

class UILoader {
public:
    UILoader();
    ~UILoader();

    bool loadFromFile(const std::string& path, UIManager& uiManager);
    bool loadUI(const ID& uiID, UIManager& uiManager);
    bool loadFromData(const nlohmann::json& jsonData, UIManager& uiManager);

private:
    void parseButtons(const nlohmann::json& buttonsJson, UIManager& uiManager, const UILayoutData& layoutData);
    void parseImages(const nlohmann::json& imagesJson, UIManager& uiManager, const UILayoutData& layoutData);
    void parseTexts(const nlohmann::json& textsJson, UIManager& uiManager, const UILayoutData& layoutData);
    void parseInputs(const nlohmann::json& inputsJson, UIManager& uiManager, const UILayoutData& layoutData);

    Color parseColor(const std::string& colorStr) const;
    ID generateComponentID(const ID& baseID, const ID& uiID, i32 index) const;
    Vec2f applyRescale(f32 x, f32 y) const;
    Vec2f applyRescaleSize(f32 width, f32 height) const;
    u32 applyRescaleFontSize(u32 fontSize) const;
    bool loadTextureResource(const std::string& path, const ID& resourceID);
    bool loadFontResource(const std::string& path, const ID& resourceID);
};

void from_json(const nlohmann::json& j, UILayoutData& data);

}
