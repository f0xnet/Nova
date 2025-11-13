#pragma once
#include "../Core/BackendTypes.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

namespace NovaEngine { 
namespace SFMLConv {
    inline sf::Color toSFML(const Color& c) { 
        return sf::Color(c.r, c.g, c.b, c.a); 
    }
    inline Color fromSFML(const sf::Color& c) { 
        return Color(c.r, c.g, c.b, c.a); 
    }
    
    inline sf::Vector2f toSFML(const Vec2f& v) { 
        return sf::Vector2f(v.x, v.y); 
    }
    inline Vec2f fromSFML(const sf::Vector2f& v) { 
        return Vec2f(v.x, v.y); 
    }
    
    inline sf::Vector2i toSFML(const Vec2i& v) { 
        return sf::Vector2i(v.x, v.y); 
    }
    inline Vec2i fromSFML(const sf::Vector2i& v) { 
        return Vec2i(v.x, v.y); 
    }
    
    inline sf::Vector2u toSFML(const Vec2u& v) { 
        return sf::Vector2u(v.x, v.y); 
    }
    inline Vec2u fromSFML(const sf::Vector2u& v) { 
        return Vec2u(v.x, v.y); 
    }
    
    inline sf::FloatRect toSFML(const Rect& r) { 
        return sf::FloatRect(r.left, r.top, r.width, r.height); 
    }
    inline Rect fromSFML(const sf::FloatRect& r) { 
        return Rect(r.left, r.top, r.width, r.height); 
    }
    
    inline sf::IntRect toSFML(const IntRect& r) { 
        return sf::IntRect(r.left, r.top, r.width, r.height); 
    }
    inline IntRect fromSFML(const sf::IntRect& r) { 
        return IntRect(r.left, r.top, r.width, r.height); 
    }
    
    inline sf::BlendMode toSFML(BlendMode m) {
        switch(m) {
            case BlendMode::Alpha: return sf::BlendAlpha;
            case BlendMode::Add: return sf::BlendAdd;
            case BlendMode::Multiply: return sf::BlendMultiply;
            case BlendMode::None: return sf::BlendNone;
            default: return sf::BlendAlpha;
        }
    }
    
    inline u32 toSFMLStyle(TextStyle s) {
        u32 style = 0;
        if(hasFlag(s, TextStyle::Bold)) style |= sf::Text::Bold;
        if(hasFlag(s, TextStyle::Italic)) style |= sf::Text::Italic;
        if(hasFlag(s, TextStyle::Underlined)) style |= sf::Text::Underlined;
        if(hasFlag(s, TextStyle::StrikeThrough)) style |= sf::Text::StrikeThrough;
        return style == 0 ? sf::Text::Regular : style;
    }
    
    inline KeyCode fromSFMLKey(sf::Keyboard::Key k) {
        switch(k) {
            case sf::Keyboard::A: return KeyCode::A;
            case sf::Keyboard::B: return KeyCode::B;
            case sf::Keyboard::C: return KeyCode::C;
            case sf::Keyboard::D: return KeyCode::D;
            case sf::Keyboard::E: return KeyCode::E;
            case sf::Keyboard::F: return KeyCode::F;
            case sf::Keyboard::G: return KeyCode::G;
            case sf::Keyboard::H: return KeyCode::H;
            case sf::Keyboard::I: return KeyCode::I;
            case sf::Keyboard::J: return KeyCode::J;
            case sf::Keyboard::K: return KeyCode::K;
            case sf::Keyboard::L: return KeyCode::L;
            case sf::Keyboard::M: return KeyCode::M;
            case sf::Keyboard::N: return KeyCode::N;
            case sf::Keyboard::O: return KeyCode::O;
            case sf::Keyboard::P: return KeyCode::P;
            case sf::Keyboard::Q: return KeyCode::Q;
            case sf::Keyboard::R: return KeyCode::R;
            case sf::Keyboard::S: return KeyCode::S;
            case sf::Keyboard::T: return KeyCode::T;
            case sf::Keyboard::U: return KeyCode::U;
            case sf::Keyboard::V: return KeyCode::V;
            case sf::Keyboard::W: return KeyCode::W;
            case sf::Keyboard::X: return KeyCode::X;
            case sf::Keyboard::Y: return KeyCode::Y;
            case sf::Keyboard::Z: return KeyCode::Z;
            case sf::Keyboard::Num0: return KeyCode::Num0;
            case sf::Keyboard::Num1: return KeyCode::Num1;
            case sf::Keyboard::Num2: return KeyCode::Num2;
            case sf::Keyboard::Num3: return KeyCode::Num3;
            case sf::Keyboard::Num4: return KeyCode::Num4;
            case sf::Keyboard::Num5: return KeyCode::Num5;
            case sf::Keyboard::Num6: return KeyCode::Num6;
            case sf::Keyboard::Num7: return KeyCode::Num7;
            case sf::Keyboard::Num8: return KeyCode::Num8;
            case sf::Keyboard::Num9: return KeyCode::Num9;
            case sf::Keyboard::Escape: return KeyCode::Escape;
            case sf::Keyboard::LControl: return KeyCode::LControl;
            case sf::Keyboard::LShift: return KeyCode::LShift;
            case sf::Keyboard::LAlt: return KeyCode::LAlt;
            case sf::Keyboard::LSystem: return KeyCode::LSystem;
            case sf::Keyboard::RControl: return KeyCode::RControl;
            case sf::Keyboard::RShift: return KeyCode::RShift;
            case sf::Keyboard::RAlt: return KeyCode::RAlt;
            case sf::Keyboard::RSystem: return KeyCode::RSystem;
            case sf::Keyboard::Space: return KeyCode::Space;
            case sf::Keyboard::Enter: return KeyCode::Enter;
            case sf::Keyboard::Backspace: return KeyCode::Backspace;
            case sf::Keyboard::Tab: return KeyCode::Tab;
            case sf::Keyboard::Left: return KeyCode::Left;
            case sf::Keyboard::Right: return KeyCode::Right;
            case sf::Keyboard::Up: return KeyCode::Up;
            case sf::Keyboard::Down: return KeyCode::Down;
            default: return KeyCode::Unknown;
        }
    }
    
    inline sf::Keyboard::Key toSFMLKey(KeyCode k) {
        switch(k) {
            case KeyCode::A: return sf::Keyboard::A;
            case KeyCode::B: return sf::Keyboard::B;
            case KeyCode::C: return sf::Keyboard::C;
            case KeyCode::D: return sf::Keyboard::D;
            case KeyCode::E: return sf::Keyboard::E;
            case KeyCode::F: return sf::Keyboard::F;
            case KeyCode::G: return sf::Keyboard::G;
            case KeyCode::H: return sf::Keyboard::H;
            case KeyCode::I: return sf::Keyboard::I;
            case KeyCode::J: return sf::Keyboard::J;
            case KeyCode::K: return sf::Keyboard::K;
            case KeyCode::L: return sf::Keyboard::L;
            case KeyCode::M: return sf::Keyboard::M;
            case KeyCode::N: return sf::Keyboard::N;
            case KeyCode::O: return sf::Keyboard::O;
            case KeyCode::P: return sf::Keyboard::P;
            case KeyCode::Q: return sf::Keyboard::Q;
            case KeyCode::R: return sf::Keyboard::R;
            case KeyCode::S: return sf::Keyboard::S;
            case KeyCode::T: return sf::Keyboard::T;
            case KeyCode::U: return sf::Keyboard::U;
            case KeyCode::V: return sf::Keyboard::V;
            case KeyCode::W: return sf::Keyboard::W;
            case KeyCode::X: return sf::Keyboard::X;
            case KeyCode::Y: return sf::Keyboard::Y;
            case KeyCode::Z: return sf::Keyboard::Z;
            case KeyCode::Num0: return sf::Keyboard::Num0;
            case KeyCode::Num1: return sf::Keyboard::Num1;
            case KeyCode::Num2: return sf::Keyboard::Num2;
            case KeyCode::Num3: return sf::Keyboard::Num3;
            case KeyCode::Num4: return sf::Keyboard::Num4;
            case KeyCode::Num5: return sf::Keyboard::Num5;
            case KeyCode::Num6: return sf::Keyboard::Num6;
            case KeyCode::Num7: return sf::Keyboard::Num7;
            case KeyCode::Num8: return sf::Keyboard::Num8;
            case KeyCode::Num9: return sf::Keyboard::Num9;
            case KeyCode::Escape: return sf::Keyboard::Escape;
            case KeyCode::LControl: return sf::Keyboard::LControl;
            case KeyCode::LShift: return sf::Keyboard::LShift;
            case KeyCode::LAlt: return sf::Keyboard::LAlt;
            case KeyCode::LSystem: return sf::Keyboard::LSystem;
            case KeyCode::RControl: return sf::Keyboard::RControl;
            case KeyCode::RShift: return sf::Keyboard::RShift;
            case KeyCode::RAlt: return sf::Keyboard::RAlt;
            case KeyCode::RSystem: return sf::Keyboard::RSystem;
            case KeyCode::Space: return sf::Keyboard::Space;
            case KeyCode::Enter: return sf::Keyboard::Enter;
            case KeyCode::Backspace: return sf::Keyboard::Backspace;
            case KeyCode::Tab: return sf::Keyboard::Tab;
            case KeyCode::Left: return sf::Keyboard::Left;
            case KeyCode::Right: return sf::Keyboard::Right;
            case KeyCode::Up: return sf::Keyboard::Up;
            case KeyCode::Down: return sf::Keyboard::Down;
            default: return sf::Keyboard::Unknown;
        }
    }
    
    inline MouseButton fromSFMLMouse(sf::Mouse::Button b) {
        switch(b) {
            case sf::Mouse::Left: return MouseButton::Left;
            case sf::Mouse::Right: return MouseButton::Right;
            case sf::Mouse::Middle: return MouseButton::Middle;
            default: return MouseButton::Left;
        }
    }
    
    inline sf::Mouse::Button toSFMLMouse(MouseButton b) {
        switch(b) {
            case MouseButton::Left: return sf::Mouse::Left;
            case MouseButton::Right: return sf::Mouse::Right;
            case MouseButton::Middle: return sf::Mouse::Middle;
            default: return sf::Mouse::Left;
        }
    }
    
    inline SoundStatus fromSFMLStatus(sf::Sound::Status s) {
        switch(s) {
            case sf::Sound::Stopped: return SoundStatus::Stopped;
            case sf::Sound::Paused: return SoundStatus::Paused;
            case sf::Sound::Playing: return SoundStatus::Playing;
            default: return SoundStatus::Stopped;
        }
    }
    
    inline SoundStatus fromSFMLMusicStatus(sf::Music::Status s) {
        switch(s) {
            case sf::Music::Stopped: return SoundStatus::Stopped;
            case sf::Music::Paused: return SoundStatus::Paused;
            case sf::Music::Playing: return SoundStatus::Playing;
            default: return SoundStatus::Stopped;
        }
    }
}
}
