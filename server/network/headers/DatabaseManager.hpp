// DatabaseManager.hpp
#ifndef DATABASEMANAGER_HPP
#define DATABASEMANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <memory>
#include "../../system/headers/LoggerManager.hpp"

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool init();
    bool openConnection(sqlite3** db, const std::string& databasePath);
    void closeConnection();
    bool addUser(const std::string& username, const std::string& password, const std::string& email);

private:
    std::string databasePath;

    sqlite3* user_db;
    sqlite3* item_db;
    sqlite3* player_db;
    sqlite3* npc_db;
    sqlite3* quest_db;
    sqlite3* guild_db;
    sqlite3* chat_db;
    sqlite3* log_db;

    std::unique_ptr<LoggerManager> loggerManager;
};

#endif // DATABASEMANAGER_HPP
