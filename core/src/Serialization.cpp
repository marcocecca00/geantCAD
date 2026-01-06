#include "Serialization.hh"
#include "SceneGraph.hh"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

namespace geantcad {

namespace fs = std::filesystem;

// Current format version
constexpr int CURRENT_FORMAT_VERSION = 1;

bool saveSceneToFile(SceneGraph* sceneGraph, const std::string& filePath) {
    if (!sceneGraph) return false;
    
    try {
        // Determine if filePath is a directory or file
        fs::path path(filePath);
        
        // If extension is .geantcad, treat as directory name
        fs::path projectDir;
        if (path.extension() == ".geantcad") {
            projectDir = path;
        } else if (path.extension() == ".json") {
            // Legacy: single JSON file
            nlohmann::json j = sceneGraph->toJson();
            std::ofstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "Failed to open file for writing: " << filePath << std::endl;
                return false;
            }
            file << j.dump(2);
            return true;
        } else {
            // Default: treat as directory name
            projectDir = path;
        }
        
        // Create project directory
        if (!fs::exists(projectDir)) {
            fs::create_directories(projectDir);
        }
        
        // Save version.json
        nlohmann::json versionJson;
        versionJson["version"] = CURRENT_FORMAT_VERSION;
        versionJson["format"] = "geantcad";
        {
            std::ofstream file(projectDir / "version.json");
            if (!file.is_open()) {
                std::cerr << "Failed to create version.json" << std::endl;
                return false;
            }
            file << versionJson.dump(2);
        }
        
        // Save scene.json (geometry and hierarchy)
        nlohmann::json sceneJson = sceneGraph->toJson();
        {
            std::ofstream file(projectDir / "scene.json");
            if (!file.is_open()) {
                std::cerr << "Failed to create scene.json" << std::endl;
                return false;
            }
            file << sceneJson.dump(2);
        }
        
        // Save physics.json
        nlohmann::json physicsJson = sceneGraph->getPhysicsConfig().toJson();
        {
            std::ofstream file(projectDir / "physics.json");
            if (!file.is_open()) {
                std::cerr << "Failed to create physics.json" << std::endl;
                return false;
            }
            file << physicsJson.dump(2);
        }
        
        // Save output.json
        nlohmann::json outputJson = sceneGraph->getOutputConfig().toJson();
        {
            std::ofstream file(projectDir / "output.json");
            if (!file.is_open()) {
                std::cerr << "Failed to create output.json" << std::endl;
                return false;
            }
            file << outputJson.dump(2);
        }
        
        // Save particleGun.json
        nlohmann::json particleGunJson = sceneGraph->getParticleGunConfig().toJson();
        {
            std::ofstream file(projectDir / "particleGun.json");
            if (!file.is_open()) {
                std::cerr << "Failed to create particleGun.json" << std::endl;
                return false;
            }
            file << particleGunJson.dump(2);
        }
        
        // Extract and save custom materials to materials.json
        nlohmann::json materialsJson = nlohmann::json::array();
        sceneGraph->traverseConst([&](const VolumeNode* node) {
            if (node && node->getMaterial()) {
                auto mat = node->getMaterial();
                // Only save custom materials (non-NIST)
                if (mat->getNistName().empty()) {
                    materialsJson.push_back(mat->toJson());
                }
            }
        });
        {
            std::ofstream file(projectDir / "materials.json");
            if (!file.is_open()) {
                std::cerr << "Failed to create materials.json" << std::endl;
                return false;
            }
            file << materialsJson.dump(2);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving scene: " << e.what() << std::endl;
        return false;
    }
}

bool loadSceneFromFile(SceneGraph* sceneGraph, const std::string& filePath) {
    if (!sceneGraph) return false;
    
    try {
        fs::path path(filePath);
        
        // Check if it's a directory (project format) or single JSON file (legacy)
        if (fs::is_directory(path)) {
            // Project directory format
            fs::path projectDir = path;
            
            // Check version.json
            fs::path versionFile = projectDir / "version.json";
            if (fs::exists(versionFile)) {
                std::ifstream file(versionFile);
                nlohmann::json versionJson;
                file >> versionJson;
                int version = versionJson.value("version", 0);
                if (version > CURRENT_FORMAT_VERSION) {
                    std::cerr << "Warning: File format version " << version 
                              << " is newer than supported version " << CURRENT_FORMAT_VERSION << std::endl;
                }
            }
            
            // Load scene.json
            fs::path sceneFile = projectDir / "scene.json";
            if (!fs::exists(sceneFile)) {
                std::cerr << "scene.json not found in project directory" << std::endl;
                return false;
            }
            {
                std::ifstream file(sceneFile);
                nlohmann::json sceneJson;
                file >> sceneJson;
                sceneGraph->fromJson(sceneJson);
            }
            
            // Load physics.json if exists
            fs::path physicsFile = projectDir / "physics.json";
            if (fs::exists(physicsFile)) {
                std::ifstream file(physicsFile);
                nlohmann::json physicsJson;
                file >> physicsJson;
                sceneGraph->getPhysicsConfig().fromJson(physicsJson);
            }
            
            // Load output.json if exists
            fs::path outputFile = projectDir / "output.json";
            if (fs::exists(outputFile)) {
                std::ifstream file(outputFile);
                nlohmann::json outputJson;
                file >> outputJson;
                sceneGraph->getOutputConfig().fromJson(outputJson);
            }
            
            // Load particleGun.json if exists
            fs::path particleGunFile = projectDir / "particleGun.json";
            if (fs::exists(particleGunFile)) {
                std::ifstream file(particleGunFile);
                nlohmann::json particleGunJson;
                file >> particleGunJson;
                sceneGraph->getParticleGunConfig().fromJson(particleGunJson);
            }
            
            // Load materials.json if exists (custom materials)
            fs::path materialsFile = projectDir / "materials.json";
            if (fs::exists(materialsFile)) {
                std::ifstream file(materialsFile);
                nlohmann::json materialsJson;
                file >> materialsJson;
                // Note: Custom materials would need to be registered with SceneGraph
                // For now, they're loaded but not automatically assigned
                // This could be extended to store material references by name/ID
            }
            
            return true;
        } else {
            // Legacy: single JSON file
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
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading scene: " << e.what() << std::endl;
        return false;
    }
}

} // namespace geantcad
