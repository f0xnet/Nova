#pragma once
#include <cstdint>
#include <string>
#include <functional>

namespace NovaEngine {
    using i8 = int8_t; using i16 = int16_t; using i32 = int32_t; using i64 = int64_t;
    using u8 = uint8_t; using u16 = uint16_t; using u32 = uint32_t; using u64 = uint64_t;
    using f32 = float; using f64 = double;
    using String = std::string;
    
    using TextureHandle = u64; using FontHandle = u64; using SoundHandle = u64;
    using MusicHandle = u64; using ShaderHandle = u64;
    constexpr u64 INVALID_HANDLE = 0;
    
    enum class BackendType { SFML, SDL, Custom };
    
    struct Vec2f { 
        f32 x, y; 
        Vec2f() : x(0), y(0) {} 
        Vec2f(f32 x, f32 y) : x(x), y(y) {} 
        Vec2f operator+(const Vec2f& o) const { return Vec2f(x + o.x, y + o.y); }
        Vec2f operator-(const Vec2f& o) const { return Vec2f(x - o.x, y - o.y); }
        Vec2f operator*(f32 s) const { return Vec2f(x * s, y * s); }
        Vec2f operator/(f32 s) const { return Vec2f(x / s, y / s); }
        Vec2f& operator+=(const Vec2f& o) { x += o.x; y += o.y; return *this; }
        Vec2f& operator-=(const Vec2f& o) { x -= o.x; y -= o.y; return *this; }
        Vec2f& operator*=(f32 s) { x *= s; y *= s; return *this; }
        Vec2f& operator/=(f32 s) { x /= s; y /= s; return *this; }
    };
    
    struct Vec2i { 
        i32 x, y; 
        Vec2i() : x(0), y(0) {} 
        Vec2i(i32 x, i32 y) : x(x), y(y) {}
        Vec2i operator+(const Vec2i& o) const { return Vec2i(x + o.x, y + o.y); }
        Vec2i operator-(const Vec2i& o) const { return Vec2i(x - o.x, y - o.y); }
    };
    
    struct Vec2u { 
        u32 x, y; 
        Vec2u() : x(0), y(0) {} 
        Vec2u(u32 x, u32 y) : x(x), y(y) {} 
    };
    
    struct Rect { 
        f32 left, top, width, height; 
        Rect() : left(0), top(0), width(0), height(0) {}
        Rect(f32 l, f32 t, f32 w, f32 h) : left(l), top(t), width(w), height(h) {}
        bool contains(f32 x, f32 y) const { 
            return x >= left && x <= left + width && y >= top && y <= top + height; 
        }
        bool contains(const Vec2f& p) const { return contains(p.x, p.y); }
    };
    
    struct IntRect { 
        i32 left, top, width, height;
        IntRect() : left(0), top(0), width(0), height(0) {}
        IntRect(i32 l, i32 t, i32 w, i32 h) : left(l), top(t), width(w), height(h) {}
    };
    
    struct Color { 
        u8 r, g, b, a;
        Color() : r(0), g(0), b(0), a(255) {}
        Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}
        static const Color Black, White, Red, Green, Blue, Yellow, Transparent;
        bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
        bool operator!=(const Color& o) const { return !(*this == o); }
    };
    
    enum class KeyCode { 
        Unknown = -1, 
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
        Escape, LControl, LShift, LAlt, LSystem, RControl, RShift, RAlt, RSystem,
        Space, Enter, Backspace, Tab, Left, Right, Up, Down 
    };
    
    enum class MouseButton { Left, Right, Middle };
    
    enum class InputEventType { 
        Closed, Resized, 
        KeyPressed, KeyReleased, 
        MouseButtonPressed, MouseButtonReleased, MouseMoved, 
        TextEntered 
    };
    
    struct InputEvent {
        InputEventType type;
        union {
            struct { u32 width, height; } size;
            struct { u32 unicode; } text;
            struct { KeyCode code; bool alt, control, shift, system; } key;
            struct { MouseButton button; i32 x, y; } mouseButton;
            struct { i32 x, y; } mouseMove;
        };
        InputEvent() : type(InputEventType::Closed) { size.width = 0; size.height = 0; }
    };
    
    enum class BlendMode { Alpha, Add, Multiply, None };
    
    enum class TextStyle { 
        Regular = 0, 
        Bold = 1 << 0, 
        Italic = 1 << 1, 
        Underlined = 1 << 2,
        StrikeThrough = 1 << 3
    };
    
    inline TextStyle operator|(TextStyle a, TextStyle b) {
        return static_cast<TextStyle>(static_cast<int>(a) | static_cast<int>(b));
    }
    inline TextStyle operator&(TextStyle a, TextStyle b) {
        return static_cast<TextStyle>(static_cast<int>(a) & static_cast<int>(b));
    }
    inline bool hasFlag(TextStyle style, TextStyle flag) {
        return (static_cast<int>(style) & static_cast<int>(flag)) != 0;
    }
    
    struct Transform { 
        Vec2f position; 
        f32 rotation; 
        Vec2f scale; 
        Vec2f origin;
        Transform() : position(0,0), rotation(0), scale(1,1), origin(0,0) {}
    };
    
    struct SpriteData { 
        TextureHandle texture; 
        Vec2f position; 
        Vec2f size; 
        f32 rotation;
        Vec2f scale; 
        Vec2f origin; 
        IntRect textureRect; 
        Color color; 
        BlendMode blendMode;
        SpriteData() : texture(INVALID_HANDLE), position(0,0), size(0,0), rotation(0),
            scale(1,1), origin(0,0), textureRect(0,0,0,0), color(Color::White), blendMode(BlendMode::Alpha) {}
    };
    
    struct RectData { 
        Vec2f position; 
        Vec2f size; 
        Color fillColor; 
        Color outlineColor; 
        f32 outlineThickness;
        f32 rotation;
        Vec2f origin;
        RectData() : position(0,0), size(0,0), fillColor(Color::White), 
            outlineColor(Color::Transparent), outlineThickness(0), rotation(0), origin(0,0) {}
    };
    
    struct TextData { 
        String text; 
        FontHandle font; 
        u32 characterSize; 
        Color fillColor; 
        Color outlineColor;
        f32 outlineThickness; 
        TextStyle style; 
        Vec2f position; 
        f32 rotation; 
        Vec2f scale; 
        Vec2f origin; 
        BlendMode blendMode;
        TextData() : text(""), font(INVALID_HANDLE), characterSize(30), fillColor(Color::White),
            outlineColor(Color::Transparent), outlineThickness(0), style(TextStyle::Regular),
            position(0,0), rotation(0), scale(1,1), origin(0,0), blendMode(BlendMode::Alpha) {}
    };
    
    struct TextMetrics { 
        f32 width, height, baseline;
        TextMetrics() : width(0), height(0), baseline(0) {}
    };
    
    struct ViewportData { 
        Rect viewport; 
        Vec2f center; 
        Vec2f size; 
        f32 rotation;
        ViewportData() : viewport(0,0,1,1), center(0,0), size(800,600), rotation(0) {}
    };
    
    enum class SoundStatus { Stopped, Paused, Playing };
}
