#pragma once

#include <string>
#include <string_view>
#include <mutex>
#include <fstream>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstring>

// Contrôle global de l'activation du logger
#define NOVA_LOGGING_ENABLED true

#if NOVA_LOGGING_ENABLED

namespace NovaEngine {

    /**
     * @brief Niveau de log pour le système de journalisation.
     */
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    /**
     * @brief Classe Logger thread-safe en singleton, pour log console + fichier.
     */
    class Logger {
    public:
        /**
         * @brief Retourne l'instance unique du logger.
         */
        static Logger& getInstance();

        /**
         * @brief Configure la sortie vers un fichier log.
         * @param filepath Chemin du fichier de log.
         */
        void setLogFile(const std::string& filepath);

        /**
         * @brief Définit le niveau minimal de log à afficher.
         */
        void setLogLevel(LogLevel level);

        /**
         * @brief Active/désactive les couleurs ANSI pour la console.
         */
        void enableAnsiColors(bool enable);

        /**
         * @brief Journalise un message simple.
         */
        void log(LogLevel level, std::string_view module, std::string_view message);

        /**
         * @brief Journalise un message formaté avec arguments variés (C++17)
         */
        template<typename... Args>
        void logf(LogLevel level, std::string_view module, std::string_view format, Args&&... args) {
            if constexpr (sizeof...(args) == 0) {
                log(level, module, format);
            } else {
                std::string formatted = formatString(format, std::forward<Args>(args)...);
                log(level, module, formatted);
            }
        }

    private:
        Logger();
        ~Logger();

        // Formatage simple style {} avec fold expressions C++17
        template<typename T>
        std::string toString(T&& value) {
            if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
                return value;
            } else if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
                return std::string(value);
            } else if constexpr (std::is_same_v<std::decay_t<T>, char*>) {
                return std::string(value);
            } else if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
                return value ? "true" : "false";
            } else {
                std::ostringstream oss;
                oss << value;
                return oss.str();
            }
        }

        template<typename... Args>
        std::string formatString(std::string_view format, Args&&... args) {
            std::string result{format};
            std::string values[] = {toString(std::forward<Args>(args))...};
            
            size_t i = 0;
            size_t pos = 0;
            while ((pos = result.find("{}", pos)) != std::string::npos && i < sizeof...(args)) {
                result.replace(pos, 2, values[i]);
                pos += values[i].length();
                ++i;
            }
            
            return result;
        }

        std::string getTimestamp() const;
        std::string levelToString(LogLevel level) const;
        std::string levelToColor(LogLevel level) const;
        std::string extractFileName(std::string_view path) const;

        std::mutex m_mutex;
        std::ofstream m_logFile;
        LogLevel m_minLevel;
        bool m_useColors;
    };

} // namespace NovaEngine

// Helper pour extraire le nom du fichier depuis __FILE__
#define NOVA_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : \
                      (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))

// CORRECTION: Utiliser :: global pour éviter les problèmes de namespace
#define LOG_TRACE(format, ...) ::NovaEngine::Logger::getInstance().logf(::NovaEngine::LogLevel::Trace, NOVA_FILENAME, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) ::NovaEngine::Logger::getInstance().logf(::NovaEngine::LogLevel::Debug, NOVA_FILENAME, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  ::NovaEngine::Logger::getInstance().logf(::NovaEngine::LogLevel::Info,  NOVA_FILENAME, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  ::NovaEngine::Logger::getInstance().logf(::NovaEngine::LogLevel::Warning, NOVA_FILENAME, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) ::NovaEngine::Logger::getInstance().logf(::NovaEngine::LogLevel::Error, NOVA_FILENAME, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) ::NovaEngine::Logger::getInstance().logf(::NovaEngine::LogLevel::Fatal, NOVA_FILENAME, format, ##__VA_ARGS__)

#else

// Si NOVA_LOGGING_ENABLED est false : macros no-op
#define LOG_TRACE(format, ...) ((void)0)
#define LOG_DEBUG(format, ...) ((void)0)
#define LOG_INFO(format, ...)  ((void)0)
#define LOG_WARN(format, ...)  ((void)0)
#define LOG_ERROR(format, ...) ((void)0)
#define LOG_FATAL(format, ...) ((void)0)

#endif // NOVA_LOGGING_ENABLED