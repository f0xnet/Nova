#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "Scene.hpp"

using json = nlohmann::json;

class SceneManager {
public:
    void load_from_file(const std::string& filename);
    void print_scene_details() const;
    std::string select_dialogue(const std::string& scene_id, int love_value) const;

private:
    std::vector<Scene> scenes;
};

#endif // SCENE_MANAGER_HPP
