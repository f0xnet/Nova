// DatabaseManager.cpp
#include "headers/DatabaseManager.hpp"

#include <iostream>

DatabaseManager::DatabaseManager() {
    this->loggerManager = std::make_unique<LoggerManager>();
    this->accountDB = std::make_unique<AccountDBClass>();
}

bool DatabaseManager::init() {
    if(this->accountDB->init("database/users.db") == false) {
        return false;
    }
    return true;
}

DatabaseManager::~DatabaseManager() {
}
