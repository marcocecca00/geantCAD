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
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    
    // Generate physics constructor code for Geant4
    std::string generatePhysicsCode() const;
};

} // namespace geantcad

