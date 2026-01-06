#include "Geant4ProjectGenerator.hh"
#include "GDMLExporter.hh"
#include "../../core/include/PhysicsConfig.hh"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <iomanip>

namespace geantcad {

namespace fs = std::filesystem;

Geant4ProjectGenerator::Geant4ProjectGenerator() 
    : templateDir_("templates/geant4_project")
{
}

Geant4ProjectGenerator::~Geant4ProjectGenerator() {
}

std::string Geant4ProjectGenerator::readTemplateFile(const std::string& templatePath) {
    std::ifstream file(templatePath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Geant4ProjectGenerator::readExistingFile(const std::string& filePath) {
    if (!fs::exists(filePath)) {
        return "";
    }
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool Geant4ProjectGenerator::writeGeneratedFile(
    const std::string& filePath,
    const std::string& content,
    bool preserveRegions)
{
    std::string finalContent = content;
    
    if (preserveRegions) {
        std::string existingContent = readExistingFile(filePath);
        if (!existingContent.empty()) {
            finalContent = templateEngine_.renderWithPreservation(content, {}, existingContent);
        }
    }
    
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << finalContent;
    file.close();
    return true;
}

bool Geant4ProjectGenerator::createDirectoryStructure(const std::string& outputDir) {
    try {
        fs::create_directories(outputDir);
        fs::create_directories(outputDir + "/src");
        fs::create_directories(outputDir + "/include");
        fs::create_directories(outputDir + "/macros");
        return true;
    } catch (...) {
        return false;
    }
}

std::string Geant4ProjectGenerator::generatePhysicsConstructors() {
    // Use PhysicsConfig from SceneGraph if available
    // For now, use default (will be updated when PhysicsConfig is passed)
    PhysicsConfig defaultConfig;
    return defaultConfig.generatePhysicsCode();
}

std::map<std::string, std::string> Geant4ProjectGenerator::prepareTemplateVariables(const std::string& projectName) {
    std::map<std::string, std::string> vars;
    vars["project_name"] = projectName;
    vars["physics_constructors"] = generatePhysicsConstructors();
    
    // Generation date
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream dateStream;
    dateStream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    vars["generation_date"] = dateStream.str();
    
    return vars;
}

bool Geant4ProjectGenerator::generateProject(SceneGraph* sceneGraph, const std::string& outputDir) {
    if (!sceneGraph) {
        return false;
    }
    
    // Create directory structure
    if (!createDirectoryStructure(outputDir)) {
        return false;
    }
    
    // Extract project name from output directory
    std::string projectName = fs::path(outputDir).filename().string();
    if (projectName.empty()) {
        projectName = "geant4_project";
    }
    
    // Prepare template variables
    auto vars = prepareTemplateVariables(projectName);
    
    // Use physics config from scene graph
    vars["physics_constructors"] = sceneGraph->getPhysicsConfig().generatePhysicsCode();
    
    // Use particle gun config from scene graph
    vars["particle_gun_commands"] = sceneGraph->getParticleGunConfig().generateMacroCommands();
    
    // Find template directory (try relative to current working directory first)
    std::string templateBase = templateDir_;
    if (!fs::exists(templateBase)) {
        // Try relative to source directory (common in build scenarios)
        templateBase = "../" + templateDir_;
        if (!fs::exists(templateBase)) {
            templateBase = "../../" + templateDir_;
        }
    }
    
    // Generate CMakeLists.txt
    {
        std::string templatePath = templateBase + "/CMakeLists.txt.template";
        std::string templateContent = readTemplateFile(templatePath);
        if (templateContent.empty()) {
            return false;
        }
        std::string rendered = templateEngine_.render(templateContent, vars);
        if (!writeGeneratedFile(outputDir + "/CMakeLists.txt", rendered, true)) {
            return false;
        }
    }
    
    // Generate source files
    std::vector<std::pair<std::string, std::string>> sourceFiles = {
        {"main.cc", "main.cc.template"},
        {"DetectorConstruction.cc", "DetectorConstruction.cc.template"},
        {"PhysicsList.cc", "PhysicsList.cc.template"},
        {"ActionInitialization.cc", "ActionInitialization.cc.template"},
        {"RunAction.cc", "RunAction.cc.template"},
        {"EventAction.cc", "EventAction.cc.template"},
        {"SteppingAction.cc", "SteppingAction.cc.template"}
    };
    
    for (const auto& pair : sourceFiles) {
        std::string templatePath = templateBase + "/" + pair.second;
        std::string templateContent = readTemplateFile(templatePath);
        if (templateContent.empty()) {
            continue; // Skip if template not found
        }
        std::string rendered = templateEngine_.render(templateContent, vars);
        std::string existingContent = readExistingFile(outputDir + "/src/" + pair.first);
        if (!existingContent.empty()) {
            rendered = templateEngine_.renderWithPreservation(rendered, vars, existingContent);
        }
        if (!writeGeneratedFile(outputDir + "/src/" + pair.first, rendered, false)) {
            return false;
        }
    }
    
    // Generate header files
    std::vector<std::pair<std::string, std::string>> headerFiles = {
        {"DetectorConstruction.hh", "DetectorConstruction.hh.template"},
        {"PhysicsList.hh", "PhysicsList.hh.template"},
        {"ActionInitialization.hh", "ActionInitialization.hh.template"},
        {"RunAction.hh", "RunAction.hh.template"},
        {"EventAction.hh", "EventAction.hh.template"},
        {"SteppingAction.hh", "SteppingAction.hh.template"}
    };
    
    for (const auto& pair : headerFiles) {
        std::string templatePath = templateBase + "/" + pair.second;
        std::string templateContent = readTemplateFile(templatePath);
        if (templateContent.empty()) {
            continue;
        }
        std::string rendered = templateEngine_.render(templateContent, vars);
        std::string existingContent = readExistingFile(outputDir + "/include/" + pair.first);
        if (!existingContent.empty()) {
            rendered = templateEngine_.renderWithPreservation(rendered, vars, existingContent);
        }
        if (!writeGeneratedFile(outputDir + "/include/" + pair.first, rendered, false)) {
            return false;
        }
    }
    
    // Generate macro files
    std::vector<std::pair<std::string, std::string>> macroFiles = {
        {"vis.mac", "vis.mac.template"},
        {"run.mac", "run.mac.template"}
    };
    
    for (const auto& pair : macroFiles) {
        std::string templatePath = templateBase + "/" + pair.second;
        std::string templateContent = readTemplateFile(templatePath);
        if (templateContent.empty()) {
            continue;
        }
        std::string rendered = templateEngine_.render(templateContent, vars);
        std::string existingContent = readExistingFile(outputDir + "/macros/" + pair.first);
        if (!existingContent.empty()) {
            rendered = templateEngine_.renderWithPreservation(rendered, vars, existingContent);
        }
        if (!writeGeneratedFile(outputDir + "/macros/" + pair.first, rendered, false)) {
            return false;
        }
    }
    
    // Generate README
    {
        std::string templatePath = templateBase + "/README.md.template";
        std::string templateContent = readTemplateFile(templatePath);
        if (!templateContent.empty()) {
            std::string rendered = templateEngine_.render(templateContent, vars);
            writeGeneratedFile(outputDir + "/README.md", rendered, false);
        }
    }
    
    // Export GDML
    GDMLExporter exporter;
    std::string gdmlPath = outputDir + "/scene.gdml";
    if (!exporter.exportToFile(sceneGraph, gdmlPath)) {
        return false;
    }
    
    return true;
}

} // namespace geantcad

