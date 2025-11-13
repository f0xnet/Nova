#include "NovaEngine/Game.hpp"
#include "NovaEngine/Core/Logger.hpp"

int main() {
    NovaEngine::Logger::getInstance().setLogLevel(NovaEngine::LogLevel::Trace);
   
    LOG_INFO("=== NovaEngine Game Starting ===");
    
    Game game;
    int exitCode = game.run();
    
    LOG_INFO("=== Game terminated with code {} ===", exitCode);
    return exitCode;
}