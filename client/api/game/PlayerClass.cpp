// PLayerClass concept implementation
// Date: 2021-06-15
// Version: 1.0

#include "playerClass.h"

PlayerClass::PlayerClass() {
   //if the player is local, we initialize the local data
   this->initLocalData();
   //else we send a request to the server to get the player data
}

void PlayerClass::initLocalData() {
    //read saved data
}

void PlayerClass::initOnlineData() {
    //used by the server to update the player data
}

void PlayerClass::setPlayerName(std::string name) {
    this->playerName = name;
}

std::string PlayerClass::getPlayerName() {
    return this->playerName;
}

bool PlayerClass::update() {
    this->positionX = x;
    this->positionY = y;
    //animation update of the player character
    return true;
}

int PlayerClass::getPositionX() {
    return this->positionX;
}

int PlayerClass::getPositionY() {
    return this->positionY;
}

bool PlayerClass::setAnimation() {
    //set animation for player character ? send to the animation class the id of the animation and other needed value (loop, speed, etc.)
    return true;
}
//