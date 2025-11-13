// Animation.cpp
#include "headers/Animation.hpp"
#include <iostream>
#include <fstream>

Animation::Animation(const std::string& entityId)
    : entityId(entityId), frameIndex(0), duration(0.1f), elapsedTime(0.0f), position(0.0f, 0.0f) {}

bool Animation::loadFromJson(const nlohmann::json& animationGroupData) {
    if (!animationGroupData.contains("frames") || !animationGroupData["frames"].is_array()) {
        std::cerr << "Invalid animation data: missing frames" << std::endl;
        return false;
    }

    for (const auto& framePath : animationGroupData["frames"]) {
        sf::Texture texture;
        if (!texture.loadFromFile(framePath)) {
            std::cerr << "Failed to load frame: " << framePath << std::endl;
            return false;
        }
        frames.push_back(texture);
    }

    if (animationGroupData.contains("duration")) {
        duration = animationGroupData["duration"].get<float>();
    }

    return true;
}

void Animation::update(float deltaTime) {
    elapsedTime += deltaTime;
    if (elapsedTime >= duration) {
        frameIndex = (frameIndex + 1) % frames.size();
        elapsedTime = 0.0f;
    }
}

void Animation::setPosition(const sf::Vector2f& pos) {
    position = pos;
}

const sf::Vector2f& Animation::getPosition() const {
    return position;
}

void Animation::reset() {
    frameIndex = 0;
    elapsedTime = 0.0f;
}

const sf::Texture& Animation::getCurrentFrame() const {
    return frames[frameIndex];
}

bool Animation::isFinished() const {
    return frameIndex == static_cast<int>(frames.size()) - 1 && elapsedTime >= duration;
}

const std::string& Animation::getEntityId() const {
    return entityId;
}
//