#include "SceneGraph.hh"
#include <fstream>
#include <iostream>

namespace geantcad {

// Serialization utilities - implemented using SceneGraph's toJson/fromJson
// File I/O helpers

bool saveSceneToFile(SceneGraph* sceneGraph, const std::string& filePath) {
    if (!sceneGraph) return false;
    
    try {
        nlohmann::json j = sceneGraph->toJson();
        
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            return false;
        }
        
        file << j.dump(2); // Pretty print with 2-space indent
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving scene: " << e.what() << std::endl;
        return false;
    }
}

bool loadSceneFromFile(SceneGraph* sceneGraph, const std::string& filePath) {
    if (!sceneGraph) return false;
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filePath << std::endl;
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        file.close();
        
        sceneGraph->fromJson(j);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading scene: " << e.what() << std::endl;
        return false;
    }
}

} // namespace geantcad
