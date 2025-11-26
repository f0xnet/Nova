#include "include/EditorApplication.hpp"
#include <NovaEngine/Core/Logger.hpp>
#include <memory>

int main(int argc, char* argv[]) {
    // Configurer le logger
    NovaEngine::Logger::getInstance().setLogLevel(NovaEngine::LogLevel::Debug);
    NovaEngine::Logger::getInstance().enableAnsiColors(true);

    LOG_INFO("===========================================");
    LOG_INFO("    NovaEngine Level Editor v1.0");
    LOG_INFO("===========================================");
    LOG_INFO("");

    try {
        // Créer et lancer l'éditeur
        auto editor = std::make_unique<NovaEditor::EditorApplication>();

        // Lancer la boucle principale (gère init, loop, et shutdown)
        int exitCode = editor->run();

        if (exitCode != 0) {
            LOG_ERROR("Editor exited with error code: {}", exitCode);
            return exitCode;
        }

        LOG_INFO("Editor closed successfully");

    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error: {}", e.what());
        return 1;
    }

    return 0;
}
