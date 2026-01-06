#include "Geant4ProjectGenerator.hh"
#include "GDMLExporter.hh"
#include "../../core/include/PhysicsConfig.hh"
#include "../../core/include/ParticleGunConfig.hh"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <set>

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

std::string Geant4ProjectGenerator::generateSensitiveDetectorSetup(SceneGraph* sceneGraph) {
    if (!sceneGraph) return "";
    
    std::ostringstream oss;
    oss << "    // Auto-generated sensitive detector setup\n";
    oss << "    G4SDManager* sdManager = G4SDManager::GetSDMpointer();\n\n";
    
    // Collect all volumes with SD enabled
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> sdByType;
    
    sceneGraph->traverseConst([&](const VolumeNode* node) {
        if (node && node->getSDConfig().enabled) {
            const auto& sdConfig = node->getSDConfig();
            std::string type = sdConfig.type;
            std::string volumeName = node->getName();
            std::string collectionName = sdConfig.collectionName.empty() 
                ? volumeName + "HitsCollection" 
                : sdConfig.collectionName;
            
            sdByType[type].push_back({volumeName, collectionName});
        }
    });
    
    // Generate SD instances and registration
    for (const auto& typePair : sdByType) {
        const std::string& type = typePair.first;
        const auto& volumes = typePair.second;
        
        if (type == "calorimeter") {
            for (const auto& volPair : volumes) {
                const std::string& volName = volPair.first;
                const std::string& collName = volPair.second;
                
                oss << "    // Calorimeter SD for " << volName << "\n";
                oss << "    CalorimeterSD* " << volName << "_SD = new CalorimeterSD(\"" 
                    << volName << "_SD\", \"" << collName << "\");\n";
                oss << "    sdManager->AddNewDetector(" << volName << "_SD);\n";
                oss << "    G4LogicalVolume* " << volName << "_LV = G4LogicalVolumeStore::GetInstance()->GetVolume(\"" 
                    << volName << "\", false);\n";
                oss << "    if (" << volName << "_LV) {\n";
                oss << "        " << volName << "_LV->SetSensitiveDetector(" << volName << "_SD);\n";
                oss << "    }\n\n";
            }
        } else if (type == "tracker") {
            for (const auto& volPair : volumes) {
                const std::string& volName = volPair.first;
                const std::string& collName = volPair.second;
                
                oss << "    // Tracker SD for " << volName << "\n";
                oss << "    TrackerSD* " << volName << "_SD = new TrackerSD(\"" 
                    << volName << "_SD\", \"" << collName << "\");\n";
                oss << "    sdManager->AddNewDetector(" << volName << "_SD);\n";
                oss << "    G4LogicalVolume* " << volName << "_LV = G4LogicalVolumeStore::GetInstance()->GetVolume(\"" 
                    << volName << "\", false);\n";
                oss << "    if (" << volName << "_LV) {\n";
                oss << "        " << volName << "_LV->SetSensitiveDetector(" << volName << "_SD);\n";
                oss << "    }\n\n";
            }
        } else if (type == "optical") {
            for (const auto& volPair : volumes) {
                const std::string& volName = volPair.first;
                const std::string& collName = volPair.second;
                
                oss << "    // Optical SD for " << volName << "\n";
                oss << "    OpticalSD* " << volName << "_SD = new OpticalSD(\"" 
                    << volName << "_SD\", \"" << collName << "\");\n";
                oss << "    sdManager->AddNewDetector(" << volName << "_SD);\n";
                oss << "    G4LogicalVolume* " << volName << "_LV = G4LogicalVolumeStore::GetInstance()->GetVolume(\"" 
                    << volName << "\", false);\n";
                oss << "    if (" << volName << "_LV) {\n";
                oss << "        " << volName << "_LV->SetSensitiveDetector(" << volName << "_SD);\n";
                oss << "    }\n\n";
            }
        }
    }
    
    return oss.str();
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
    
    // Generate sensitive detector setup code
    vars["sensitive_detector_setup"] = generateSensitiveDetectorSetup(sceneGraph);
    
    // Generate output configuration code
    vars["output_config"] = sceneGraph->getOutputConfig().generateOutputCode();
    vars["event_action_output"] = sceneGraph->getOutputConfig().generateEventActionCode();
    vars["run_action_output"] = sceneGraph->getOutputConfig().generateRunActionCode();
    
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
    
    // Generate Sensitive Detector files if needed
    std::set<std::string> sdTypes;
    sceneGraph->traverseConst([&](const VolumeNode* node) {
        if (node && node->getSDConfig().enabled) {
            sdTypes.insert(node->getSDConfig().type);
        }
    });
    
    // Generate SD files based on types used
    if (sdTypes.find("calorimeter") != sdTypes.end()) {
        std::vector<std::pair<std::string, std::string>> files = {
            {"CalorimeterHit.cc", "CalorimeterHit.cc.template"},
            {"CalorimeterHit.hh", "CalorimeterHit.hh.template"},
            {"CalorimeterSD.cc", "CalorimeterSD.cc.template"},
            {"CalorimeterSD.hh", "CalorimeterSD.hh.template"}
        };
        for (const auto& pair : files) {
            std::string templatePath = templateBase + "/" + pair.second;
            std::string templateContent = readTemplateFile(templatePath);
            if (!templateContent.empty()) {
                std::string rendered = templateEngine_.render(templateContent, vars);
                std::string filePath = outputDir + "/src/" + pair.first;
                if (pair.first.find(".hh") != std::string::npos) {
                    filePath = outputDir + "/include/" + pair.first;
                }
                writeGeneratedFile(filePath, rendered, false);
            }
        }
    }
    
    if (sdTypes.find("tracker") != sdTypes.end()) {
        std::vector<std::pair<std::string, std::string>> files = {
            {"TrackerHit.cc", "TrackerHit.cc.template"},
            {"TrackerHit.hh", "TrackerHit.hh.template"},
            {"TrackerSD.cc", "TrackerSD.cc.template"},
            {"TrackerSD.hh", "TrackerSD.hh.template"}
        };
        for (const auto& pair : files) {
            std::string templatePath = templateBase + "/" + pair.second;
            std::string templateContent = readTemplateFile(templatePath);
            if (!templateContent.empty()) {
                std::string rendered = templateEngine_.render(templateContent, vars);
                std::string filePath = outputDir + "/src/" + pair.first;
                if (pair.first.find(".hh") != std::string::npos) {
                    filePath = outputDir + "/include/" + pair.first;
                }
                writeGeneratedFile(filePath, rendered, false);
            }
        }
    }
    
    if (sdTypes.find("optical") != sdTypes.end()) {
        std::vector<std::pair<std::string, std::string>> files = {
            {"OpticalHit.cc", "OpticalHit.cc.template"},
            {"OpticalHit.hh", "OpticalHit.hh.template"},
            {"OpticalSD.cc", "OpticalSD.cc.template"},
            {"OpticalSD.hh", "OpticalSD.hh.template"}
        };
        for (const auto& pair : files) {
            std::string templatePath = templateBase + "/" + pair.second;
            std::string templateContent = readTemplateFile(templatePath);
            if (!templateContent.empty()) {
                std::string rendered = templateEngine_.render(templateContent, vars);
                std::string filePath = outputDir + "/src/" + pair.first;
                if (pair.first.find(".hh") != std::string::npos) {
                    filePath = outputDir + "/include/" + pair.first;
                }
                writeGeneratedFile(filePath, rendered, false);
            }
        }
    }
    
    // Generate PrimaryGeneratorAction files (always generated)
    {
        std::vector<std::pair<std::string, std::string>> pgaFiles = {
            {"PrimaryGeneratorAction.cc", "PrimaryGeneratorAction.cc.template"},
            {"PrimaryGeneratorAction.hh", "PrimaryGeneratorAction.hh.template"}
        };
        for (const auto& pair : pgaFiles) {
            std::string templatePath = templateBase + "/" + pair.second;
            std::string templateContent = readTemplateFile(templatePath);
            if (!templateContent.empty()) {
                // Generate configuration code for PrimaryGeneratorAction
                std::ostringstream pgaConfig;
                const auto& pgConfig = sceneGraph->getParticleGunConfig();
                
                pgaConfig << "    // Configure PrimaryGeneratorAction\n";
                pgaConfig << "    PrimaryGeneratorAction* pga = dynamic_cast<PrimaryGeneratorAction*>(GetPrimaryGenerator());\n";
                pgaConfig << "    if (pga) {\n";
                pgaConfig << "        pga->SetParticleType(\"" << pgConfig.particleType << "\");\n";
                pgaConfig << "        pga->SetEnergyMode(" << static_cast<int>(pgConfig.energyMode) << ");\n";
                if (pgConfig.energyMode == ParticleGunConfig::EnergyMode::Mono) {
                    pgaConfig << "        pga->SetEnergy(" << pgConfig.energy << "*MeV);\n";
                } else if (pgConfig.energyMode == ParticleGunConfig::EnergyMode::Uniform) {
                    pgaConfig << "        pga->SetEnergyRange(" << pgConfig.energyMin << "*MeV, " << pgConfig.energyMax << "*MeV);\n";
                } else if (pgConfig.energyMode == ParticleGunConfig::EnergyMode::Gaussian) {
                    pgaConfig << "        pga->SetEnergyGaussian(" << pgConfig.energyMean << "*MeV, " << pgConfig.energySigma << "*MeV);\n";
                }
                pgaConfig << "        pga->SetPositionMode(" << static_cast<int>(pgConfig.positionMode) << ");\n";
                pgaConfig << "        pga->SetPosition(" << pgConfig.positionX << "*mm, " << pgConfig.positionY << "*mm, " << pgConfig.positionZ << "*mm);\n";
                if (!pgConfig.positionVolume.empty()) {
                    pgaConfig << "        pga->SetPositionVolume(\"" << pgConfig.positionVolume << "\");\n";
                }
                pgaConfig << "        pga->SetPositionRadius(" << pgConfig.positionRadius << "*mm);\n";
                pgaConfig << "        pga->SetDirectionMode(" << static_cast<int>(pgConfig.directionMode) << ");\n";
                pgaConfig << "        pga->SetDirection(" << pgConfig.directionX << ", " << pgConfig.directionY << ", " << pgConfig.directionZ << ");\n";
                pgaConfig << "        pga->SetConeAngle(" << pgConfig.coneAngle << "*degree);\n";
                pgaConfig << "        pga->SetNumberOfParticles(" << pgConfig.numberOfParticles << ");\n";
                pgaConfig << "    }\n";
                
                vars["primary_generator_config"] = pgaConfig.str();
                
                std::string rendered = templateEngine_.render(templateContent, vars);
                std::string filePath = outputDir + "/src/" + pair.first;
                if (pair.first.find(".hh") != std::string::npos) {
                    filePath = outputDir + "/include/" + pair.first;
                }
                std::string existingContent = readExistingFile(filePath);
                if (!existingContent.empty()) {
                    rendered = templateEngine_.renderWithPreservation(rendered, vars, existingContent);
                }
                writeGeneratedFile(filePath, rendered, false);
            }
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

