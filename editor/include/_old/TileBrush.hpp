#pragma once

#include <NovaEngine/Core/Types.hpp>
#include <string>
#include <vector>

namespace NovaEditor {

class TileBrush {
public:
    TileBrush();
    
    void setBrushSize(NovaEngine::i32 size) { m_brushSize = size; }
    NovaEngine::i32 getBrushSize() const { return m_brushSize; }
    
    void setTileType(const std::string& type) { m_tileType = type; }
    const std::string& getTileType() const { return m_tileType; }
    
    void setRandomRotation(bool enabled) { m_randomRotation = enabled; }
    void setRandomFlip(bool enabled) { m_randomFlip = enabled; }
    
    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }
    
private:
    NovaEngine::i32 m_brushSize;
    std::string m_tileType;
    bool m_randomRotation;
    bool m_randomFlip;
    bool m_active;
};

} // namespace NovaEditor
