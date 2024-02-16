#ifndef ACCOUNTDBCLASS_HPP
#define ACCOUNTDBCLASS_HPP

#include <sqlite3.h>
#include <string>
#include <memory>
#include "../../system/headers/LoggerManager.hpp"

class AccountDBClass {
public:
    AccountDBClass();
    ~AccountDBClass();

    bool addNewUser(const std::string& username, const std::string& password, const std::string& email, const std::string& lastip);
    int checkUserExistence(const std::string& username, const std::string& email);
    int checkUserCredentials(const std::string& username, const std::string& password);

    bool init(const std::string& databasePath);

private:
    std::string databasePath;
    std::unique_ptr<LoggerManager> loggerManager;
    sqlite3* db;

    bool openConnection(const std::string& databasePath);
};

#endif