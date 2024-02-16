#include "headers/AccountDBClass.hpp"

AccountDBClass::AccountDBClass() {
    this->loggerManager = std::make_unique<LoggerManager>();
}

bool AccountDBClass::init(const std::string& databasePath) {
    this->databasePath = databasePath;
    if (!openConnection(databasePath)) {
        return false;
    }
    return true;
}

bool AccountDBClass::openConnection(const std::string& databasePath) {
    if (sqlite3_open(databasePath.c_str(), &this->db) != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(this->db) << std::endl;
        return false;
    }
    loggerManager->warning("Database", "Database loaded with success !");
    return true;
}

int AccountDBClass::checkUserCredentials(const std::string& username, const std::string& password) {
    sqlite3_stmt* stmt = nullptr;
    std::string sqlCheck = "SELECT username, password FROM users WHERE username = ? AND password = ?";
    
    if (sqlite3_prepare_v2(this->db, sqlCheck.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* dbUsername = sqlite3_column_text(stmt, 0);
            const unsigned char* dbPassword = sqlite3_column_text(stmt, 1);

            if (username == std::string(reinterpret_cast<const char*>(dbUsername)) && password == std::string(reinterpret_cast<const char*>(dbPassword))) {
                sqlite3_finalize(stmt);
                return 1; // Credentials are valid
            }
        }
        sqlite3_finalize(stmt);
        return -1; // Credentials are invalid
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(this->db) << std::endl;
        sqlite3_finalize(stmt);
        return -1; // Treat SQL preparation error as invalid credentials to avoid false negatives
    }
}

int AccountDBClass::checkUserExistence(const std::string& username, const std::string& email) {
    sqlite3_stmt* stmt = nullptr;
    std::string sqlCheck = "SELECT username, email FROM users WHERE username = ? OR email = ?";
    
    if (sqlite3_prepare_v2(this->db, sqlCheck.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* dbUsername = sqlite3_column_text(stmt, 0);
            const unsigned char* dbEmail = sqlite3_column_text(stmt, 1);

            if (username == std::string(reinterpret_cast<const char*>(dbUsername))) {
                sqlite3_finalize(stmt);
                return -2; // Username exists
            }
            if (email == std::string(reinterpret_cast<const char*>(dbEmail))) {
                sqlite3_finalize(stmt);
                return -1; // Email exists
            }
        }
        sqlite3_finalize(stmt);
        return 0; // Neither username nor email exist
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(this->db) << std::endl;
        sqlite3_finalize(stmt);
        return 0; // Treat SQL preparation error as non-existence to avoid false negatives
    }
}

bool AccountDBClass::addNewUser(const std::string& username, const std::string& password, const std::string& email, const std::string& lastip) {
    sqlite3_stmt* stmt = nullptr;
    std::string sqlInsert = "INSERT INTO users (username, password, email, lastip) VALUES (?, ?, ?, ?)";
    
    if (sqlite3_prepare_v2(this->db, sqlInsert.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, lastip.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            this->loggerManager->log("Info", "AccountDBClass", "User added successfully.");
            return true;
        } else {
            std::cerr << "SQL error: " << sqlite3_errmsg(this->db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(this->db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
}

AccountDBClass::~AccountDBClass() {
}