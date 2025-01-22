// AnimationManager.hpp
#ifndef ANIMATION_MANAGER_HPP
#define ANIMATION_MANAGER_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include <stack>
#include "Animation.hpp"

class AnimationManager {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Animation>>> entityAnimations;
    std::stack<std::shared_ptr<Animation>> animationStack;
    std::shared_ptr<Animation> currentAnimation;

public:
    bool addEntityAnimationsFromJson(const std::string& entityId, const std::string& jsonFilePath);
    void pushAnimation(const std::string& entityId, const std::string& animationName);
    void popAnimation();
    void setAnimationPosition(const sf::Vector2f& position);
    const sf::Vector2f& getAnimationPosition() const;
    void update(float deltaTime);
    const sf::Texture& getCurrentFrame() const;
    bool isCurrentAnimationFinished() const;
};

//
#endif // ANIMATION_MANAGER_HPP