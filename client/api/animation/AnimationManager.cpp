// AnimationManager.cpp
#include "headers/AnimationManager.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

bool AnimationManager::addEntityAnimationsFromJson(const std::string& entityId, const std::string& jsonFilePath) {
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonFilePath << std::endl;
        return false;
    }

    nlohmann::json animationData;
    file >> animationData;

    for (auto& [animationName, animData] : animationData.items()) {
        std::shared_ptr<Animation> animation = std::make_shared<Animation>(entityId);
        if (!animation->loadFromJson(animData)) {
            std::cerr << "Failed to load animation: " << animationName << " for entity: " << entityId << std::endl;
            return false;
        }
        entityAnimations[entityId][animationName] = animation;
    }

    return true;
}

void AnimationManager::pushAnimation(const std::string& entityId, const std::string& animationName) {
    auto entityIt = entityAnimations.find(entityId);
    if (entityIt != entityAnimations.end()) {
        auto animIt = entityIt->second.find(animationName);
        if (animIt != entityIt->second.end()) {
            animationStack.push(animIt->second);
            currentAnimation = animIt->second;
            currentAnimation->reset();
        } else {
            std::cerr << "Animation not found: " << animationName << " for entity: " << entityId << std::endl;
        }
    } else {
        std::cerr << "Entity not found: " << entityId << std::endl;
    }
}

void AnimationManager::popAnimation() {
    if (!animationStack.empty()) {
        animationStack.pop();
        currentAnimation = animationStack.empty() ? nullptr : animationStack.top();
    }
}

void AnimationManager::setAnimationPosition(const sf::Vector2f& position) {
    if (currentAnimation) {
        currentAnimation->setPosition(position);
    }
}

const sf::Vector2f& AnimationManager::getAnimationPosition() const {
    static sf::Vector2f defaultPos(0.0f, 0.0f);
    return currentAnimation ? currentAnimation->getPosition() : defaultPos;
}

void AnimationManager::update(float deltaTime) {
    if (currentAnimation) {
        currentAnimation->update(deltaTime);
        if (currentAnimation->isFinished() && !animationStack.empty()) {
            popAnimation();
        }
    }
}

const sf::Texture& AnimationManager::getCurrentFrame() const {
    static sf::Texture defaultTexture;
    return currentAnimation ? currentAnimation->getCurrentFrame() : defaultTexture;
}

bool AnimationManager::isCurrentAnimationFinished() const {
    return currentAnimation ? currentAnimation->isFinished() : true;
}