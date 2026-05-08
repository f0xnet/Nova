#include "headers/LoggerManager.hpp"

LoggerManager::LoggerManager() : keepRunning(true) {
    saveThread = std::thread(&LoggerManager::saveLogsToFile, this);
}

LoggerManager::~LoggerManager() {
    keepRunning = false;
    cv.notify_one();
    if (saveThread.joinable()) {
        saveThread.join();
    }
}

void LoggerManager::log(const std::string& type, const std::string& module, const std::string& message) {
    std::lock_guard<std::mutex> guard(logMutex);
    std::string color = (type == "ERROR" ? RED : (type == "WARNING" ? YELLOW : GREEN));
    // Ajout de la balise du module dans le message de log
    std::string logMessage = color + "[" + type + "] " + "[" + module + "] " + RESET + message;
    std::cout << logMessage << std::endl;
    logStack.push_back(logMessage);
}

void LoggerManager::error(const std::string& module, const std::string& message) {
    log("ERROR", module, message);
}

void LoggerManager::warning(const std::string& module, const std::string& message) {
    log("WARNING", module, message);
}

void LoggerManager::info(const std::string& module, const std::string& message) {
    log("INFO", module, message);
}

void LoggerManager::saveLogsToFile() {
    std::unique_lock<std::mutex> lk(cv_m);
    while (keepRunning) {
        if (cv.wait_for(lk, std::chrono::minutes(5), [this] { return !keepRunning; })) {
            // Réveillé parce que keepRunning est devenu false
            break;
        }

        // Sauvegarde des logs ici (identique à ce qui était déjà présent)
        std::lock_guard<std::mutex> guard(logMutex);
        std::ofstream logFile("server.log", std::ios::app);
        if (logFile.is_open()) {
            for (const auto& log : logStack) {
                std::string cleanLog = removeAnsiColors(log);
                logFile << cleanLog << std::endl;
            }
            logStack.clear();
        }
        logFile.close();
    }
}