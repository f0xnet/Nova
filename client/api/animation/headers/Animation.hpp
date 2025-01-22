// Animation.hpp
#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>

class Animation {
private:
    std::string entityId;
    std::vector<sf::Texture> frames;
    int frameIndex;
    float duration;
    float elapsedTime;
    sf::Vector2f position;

public:
    Animation(const std::string& entityId);
    bool loadFromJson(const nlohmann::json& animationGroupData);
    void update(float deltaTime);
    void setPosition(const sf::Vector2f& pos);
    const sf::Vector2f& getPosition() const;
    void reset();
    const sf::Texture& getCurrentFrame() const;
    bool isFinished() const;
    const std::string& getEntityId() const;
};

//
#endif // ANIMATION_HPP
