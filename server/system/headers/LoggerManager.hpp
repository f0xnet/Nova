#ifndef LOGGERMANAGER_HPP
#define LOGGERMANAGER_HPP

#include <string>
#include <mutex>
#include <vector>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <regex>

class LoggerManager {
public:
    LoggerManager();
    ~LoggerManager();

    // Log avec type, module, et message
    void log(const std::string& type, const std::string& module, const std::string& message);

    // Méthodes spécialisées pour les différents niveaux de log
    void error(const std::string& module, const std::string& message);
    void warning(const std::string& module, const std::string& message);
    void info(const std::string& module, const std::string& message);

    // Méthode pour sauvegarder les logs dans un fichier
    void saveLogsToFile();

private:
    // Pile pour stocker les logs
    std::vector<std::string> logStack;
    // Mutex pour sécuriser l'accès à la pile de logs
    std::mutex logMutex;
    // Thread pour la sauvegarde périodique des logs
    std::thread saveThread;
    // Variable conditionnelle pour gérer l'attente
    std::condition_variable cv;
    // Mutex pour la variable conditionnelle
    std::mutex cv_m;
    // Contrôle de la boucle de sauvegarde
    bool keepRunning;

    // Séquences d'échappement ANSI pour les couleurs
    const std::string RED = "\033[31m";
    const std::string YELLOW = "\033[33m";
    const std::string GREEN = "\033[32m";
    const std::string RESET = "\033[0m";

    std::string removeAnsiColors(const std::string& text) {
        std::regex ansi_pattern("\033\\[[0-9;]*m");
        return std::regex_replace(text, ansi_pattern, "");
    }
};

#endif // LOGGERMANAGER_HPP
