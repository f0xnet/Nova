// DatabaseManager.hpp
#ifndef DATABASEMANAGER_HPP
#define DATABASEMANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <memory>
#include "../../system/headers/LoggerManager.hpp"
#include "AccountDBClass.hpp"

class DatabaseManager {
public:
    std::unique_ptr<AccountDBClass> accountDB;

    DatabaseManager();
    ~DatabaseManager();

    bool init();

private:
    std::string databasePath;

    std::unique_ptr<LoggerManager> loggerManager;
};

#endif // DATABASEMANAGER_HPP
