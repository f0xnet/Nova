// DatabaseManager.cpp
#include "headers/DatabaseManager.hpp"

#include <iostream>

DatabaseManager::DatabaseManager() : user_db(nullptr) {
    this->loggerManager = std::make_unique<LoggerManager>();
}

bool DatabaseManager::init() {
    this->openConnection(&this->user_db, "database/accounts.sqlite");
    this->addUser("f0x", "H3lls1ng88", "heaea@gmail.com");
    return true;
}

DatabaseManager::~DatabaseManager() {
    closeConnection();
}

bool DatabaseManager::openConnection(sqlite3** db, const std::string& databasePath) {
    if (sqlite3_open(databasePath.c_str(), db) != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(*db) << std::endl;
        return false;
    }
    loggerManager->warning("Database", "Database loaded with success !");
    return true;
}

bool DatabaseManager::addUser(const std::string& username, const std::string& password, const std::string& email) {
    // Premièrement, vérifier si l'utilisateur ou l'email existe déjà
    std::string checkQuery = "SELECT 1 FROM Users WHERE username = ? OR email = ?;";
    sqlite3_stmt* checkStmt = nullptr;

    if (sqlite3_prepare_v2(this->user_db, checkQuery.c_str(), -1, &checkStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erreur de préparation de la vérification: " << sqlite3_errmsg(this->user_db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(checkStmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(checkStmt, 2, email.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
        std::cerr << "Erreur: le nom d'utilisateur ou l'email existe déjà." << std::endl;
        sqlite3_finalize(checkStmt);
        return false;
    }
    sqlite3_finalize(checkStmt);

    // Insérer le nouvel utilisateur
    std::string insertQuery = "INSERT INTO Users (username, password, email) VALUES (?, ?, ?);";
    sqlite3_stmt* insertStmt = nullptr;

    if (sqlite3_prepare_v2(this->user_db, insertQuery.c_str(), -1, &insertStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Erreur de préparation de l'insertion: " << sqlite3_errmsg(this->user_db) << std::endl;
        return false;
    }

    // Ici, le mot de passe devrait être hashé avec une fonction de hachage sécurisée
    std::string hashedPassword = password; // Placeholder pour le hachage

    sqlite3_bind_text(insertStmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStmt, 3, email.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(insertStmt) != SQLITE_DONE) {
        std::cerr << "Échec de l'insertion de l'utilisateur: " << sqlite3_errmsg(this->user_db) << std::endl;
        sqlite3_finalize(insertStmt);
        return false;
    }

    std::cout << "Utilisateur ajouté avec succès." << std::endl;
    sqlite3_finalize(insertStmt);
    return true;
}

void DatabaseManager::closeConnection() {
    if (user_db) {
        sqlite3_close(user_db);
        user_db = nullptr; 
    }
}
