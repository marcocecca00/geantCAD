#pragma once

#include "../../core/include/SceneGraph.hh"
#include "TemplateEngine.hh"
#include <string>
#include <map>

namespace geantcad {

class Geant4ProjectGenerator {
public:
    Geant4ProjectGenerator();
    ~Geant4ProjectGenerator();
    
    bool generateProject(SceneGraph* sceneGraph, const std::string& outputDir);
    
    // Set template directory (default: templates/geant4_project relative to source)
    void setTemplateDir(const std::string& dir) { templateDir_ = dir; }
    
private:
    std::string templateDir_;
    TemplateEngine templateEngine_;
    
    // Helper methods
    std::string readTemplateFile(const std::string& templatePath);
    bool writeGeneratedFile(const std::string& filePath, const std::string& content, bool preserveRegions = false);
    std::string readExistingFile(const std::string& filePath);
    bool createDirectoryStructure(const std::string& outputDir);
    std::string generatePhysicsConstructors(); // Default physics for now
    std::map<std::string, std::string> prepareTemplateVariables(const std::string& projectName);
};

} // namespace geantcad

