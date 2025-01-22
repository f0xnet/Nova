//general include

//class LevelManager. This class is responsible for managing the levels of the game. It contains the level data and the logic to load and unload levels and process the level data and
// it's parralax effect. It also contains the logic to render the level and the entities in the level.

LevelManager::LevelManager() {
    //Initalize the level manager nor the level data
    //Load all assets needed for all levels and using an level asset manager (with a json file)
    if (!this->levelAssetManager || !this->levelAssetManager->Init()) {
        std::cerr << "Failed to load level assets" << std::endl;
    }
}

bool LevelManager::levelAssetManagerInit() {
    //Initialize the levels asset manager
    //Load all assets needed for all levels and using an level asset manager (with a json file)
    return true;
}

bool LevelManager::loadLevel(const json& levelData) {
    //Create a new level with the given level data
    //add it into the level class vector (from Level class in Level.hpp)
    //return true if the level is created successfully
    return true;
}

bool LevelManager::unloadLevel(const std::string& levelID) {
    //Unload the level with the given level ID
    //remove it from the level class vector (from Level class in Level.hpp)
    //return true if the level is unloaded successfully
    return true;
}

bool LevelManager::processLevelData(const json& levelData) {
    //Process the level data to create the level
    //This includes creating the level, loading the level assets, setting the level properties, etc.
    //return true if the level data is processed successfully

    //Set an array of asset's ID to stipulate all ID's assets needed for the level for better optimization
    return true;
}

bool LevelManager::renderLevel() {
    //Render the level and the entities in the level by using the array of asset's ID used in the current level
    //This includes rendering the level background, the level entities, the level effects, etc.
    //return true if the level is rendered successfully
    return true;
}

/* Squelette code LevelAssetLanager

Every assets are pre-loaded in the game to avoid loading time during the game. The LevelAssetManager class is responsible for loading all the assets needed for the levels of the game.
It contains the logic to load and unload assets and store them in memory for quick access. Each assets has it's texture surface, sound, music, etc. The assets are loaded from a json file
that contains the path to the assets.

The ID from the json data received from loadLevel is used to load the assets from the json file. The assets are stored in a map with the ID as the key and the asset as the value.
No new assets; juste moving/hidding/showing assets depends on the level.

*/