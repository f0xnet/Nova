#include "NovaEngine/Core/Logger.hpp"

#if NOVA_LOGGING_ENABLED

namespace NovaEngine {

    Logger& Logger::getInstance() {
        static Logger instance;
        return instance;
    }

    Logger::Logger()
        : m_minLevel(LogLevel::Trace), m_useColors(true) {
        // Par défaut, aucun fichier log. Seulement la console.
    }

    Logger::~Logger() {
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
    }

    void Logger::setLogFile(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
        m_logFile.open(filepath, std::ios::out | std::ios::app);
        if (!m_logFile) {
            std::cerr << "[Logger] Impossible d'ouvrir le fichier de log : " << filepath << std::endl;
        } else {
            std::cout << "[Logger] Fichier de log activé : " << filepath << std::endl;
        }
    }

    void Logger::setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_minLevel = level;
    }

    void Logger::enableAnsiColors(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_useColors = enable;
    }

    std::string Logger::getTimestamp() const {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto in_time_t = system_clock::to_time_t(now);
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::string Logger::levelToString(LogLevel level) const {
        switch (level) {
            case LogLevel::Trace:   return "TRACE";
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO ";
            case LogLevel::Warning: return "WARN ";
            case LogLevel::Error:   return "ERROR";
            case LogLevel::Fatal:   return "FATAL";
            default:                return "UNKNW";
        }
    }

    std::string Logger::levelToColor(LogLevel level) const {
        if (!m_useColors) return "";

        switch (level) {
            case LogLevel::Trace:   return "\033[37m";    // Gris
            case LogLevel::Debug:   return "\033[36m";    // Cyan
            case LogLevel::Info:    return "\033[32m";    // Vert
            case LogLevel::Warning: return "\033[33m";    // Jaune
            case LogLevel::Error:   return "\033[31m";    // Rouge
            case LogLevel::Fatal:   return "\033[41;37m"; // Fond rouge + texte blanc
            default:                return "\033[0m";
        }
    }

    std::string Logger::extractFileName(std::string_view path) const {
        // Utilisation de std::filesystem (C++17)
        std::filesystem::path p(path);
        return p.filename().string();
    }

    void Logger::log(LogLevel level, std::string_view module, std::string_view message) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (level < m_minLevel) return;

        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        std::string moduleStr = extractFileName(module);
        std::string color = levelToColor(level);
        const std::string reset = m_useColors ? "\033[0m" : "";

        // Format optimisé avec string_view
        std::ostringstream formatted;
        formatted << "[" << timestamp << "] "
                  << "[" << levelStr << "] "
                  << "[" << moduleStr << "] "
                  << message;

        std::string output = formatted.str();

        // Console avec couleurs
        if (m_useColors) {
            std::cout << color << output << reset << std::endl;
        } else {
            std::cout << output << std::endl;
        }

        // Fichier (sans couleurs)
        if (m_logFile.is_open()) {
            m_logFile << output << std::endl;
            m_logFile.flush(); // Force l'écriture immédiate
        }

        // Pour les erreurs fatales, flush stderr aussi
        if (level == LogLevel::Fatal) {
            std::cerr.flush();
            std::cout.flush();
        }
    }

} // namespace NovaEngine

#endif // NOVA_LOGGING_ENABLED