#include "PhysicsConfig.hh"

namespace geantcad {

PhysicsConfig::PhysicsConfig() {
}

PhysicsConfig::~PhysicsConfig() {
}

nlohmann::json PhysicsConfig::toJson() const {
    nlohmann::json j;
    j["em_enabled"] = emEnabled;
    j["decay_enabled"] = decayEnabled;
    j["optical_enabled"] = opticalEnabled;
    j["hadronic_enabled"] = hadronicEnabled;
    j["standard_list"] = standardList;
    return j;
}

void PhysicsConfig::fromJson(const nlohmann::json& j) {
    if (j.contains("em_enabled")) {
        emEnabled = j["em_enabled"];
    }
    if (j.contains("decay_enabled")) {
        decayEnabled = j["decay_enabled"];
    }
    if (j.contains("optical_enabled")) {
        opticalEnabled = j["optical_enabled"];
    }
    if (j.contains("hadronic_enabled")) {
        hadronicEnabled = j["hadronic_enabled"];
    }
    if (j.contains("standard_list")) {
        standardList = j["standard_list"];
    }
}

std::string PhysicsConfig::generatePhysicsCode() const {
    std::ostringstream oss;
    
    if (emEnabled) {
        oss << "    RegisterPhysics(new G4EmStandardPhysics());\n";
    }
    
    if (decayEnabled) {
        oss << "    RegisterPhysics(new G4DecayPhysics());\n";
    }
    
    if (opticalEnabled) {
        oss << "    RegisterPhysics(new G4OpticalPhysics());\n";
    }
    
    if (hadronicEnabled) {
        oss << "    RegisterPhysics(new G4HadronElasticPhysics());\n";
        
        // Standard list selection
        if (standardList == "FTFP_BERT") {
            oss << "    RegisterPhysics(new G4HadronPhysicsFTFP_BERT());\n";
        } else if (standardList == "QGSP_BERT") {
            oss << "    RegisterPhysics(new G4HadronPhysicsQGSP_BERT());\n";
        } else if (standardList == "FTFP_BERT_HP") {
            oss << "    RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());\n";
        } else {
            // Default to FTFP_BERT
            oss << "    RegisterPhysics(new G4HadronPhysicsFTFP_BERT());\n";
        }
        
        oss << "    RegisterPhysics(new G4IonPhysics());\n";
        oss << "    RegisterPhysics(new G4StoppingPhysics());\n";
    }
    
    return oss.str();
}

} // namespace geantcad

