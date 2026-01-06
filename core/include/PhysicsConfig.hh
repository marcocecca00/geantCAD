#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace geantcad {

/**
 * Physics configuration for Geant4 physics list
 */
class PhysicsConfig {
public:
    PhysicsConfig();
    ~PhysicsConfig();
    
    // Physics toggles
    bool emEnabled = true;        // Electromagnetic physics (default ON)
    bool decayEnabled = false;    // Decay physics
    bool opticalEnabled = false;  // Optical physics (Cerenkov + Scintillation)
    bool hadronicEnabled = true;  // Hadronic physics (default ON)
    
    // Standard physics list
    std::string standardList = "FTFP_BERT"; // FTFP_BERT, QGSP_BERT, etc.
    
    // Electromagnetic physics options
    enum class EMOption {
        Standard,   // G4EmStandardPhysics
        Option1,    // G4EmStandardPhysics_option1 (high-energy)
        Option2,    // G4EmStandardPhysics_option2 (low-energy)
        Option3,    // G4EmStandardPhysics_option3 (WVI)
        Option4,    // G4EmStandardPhysics_option4 (best precision)
        Penelope,   // G4EmPenelopePhysics
        Livermore   // G4EmLivermorePhysics
    };
    EMOption emOption = EMOption::Standard;
    
    // Hadronic physics options  
    enum class HadronicModel {
        FTFP_BERT,  // Fritiof + Bertini cascade (default)
        QGSP_BERT,  // Quark-Gluon String + Bertini
        QGSP_BIC,   // QGS + Binary cascade
        FTFP_INCLXX // Fritiof + INCL++ cascade (for low-energy)
    };
    HadronicModel hadronicModel = HadronicModel::FTFP_BERT;
    
    // Additional physics options
    bool ionPhysicsEnabled = false;      // Ion physics
    bool radioactiveDecayEnabled = false; // Radioactive decay
    bool stepLimiterEnabled = false;     // Step limiter for sensitive regions
    
    // Production cuts (mm)
    double gammaCut = 0.1;   // Production cut for gamma
    double electronCut = 0.1; // Production cut for e-
    double positronCut = 0.1; // Production cut for e+
    double protonCut = 0.1;   // Production cut for protons
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    
    // Generate physics constructor code for Geant4
    std::string generatePhysicsCode() const;
    
    // Helper methods
    static std::string emOptionToString(EMOption opt);
    static EMOption stringToEMOption(const std::string& str);
    static std::string hadronicModelToString(HadronicModel model);
    static HadronicModel stringToHadronicModel(const std::string& str);
};

} // namespace geantcad

