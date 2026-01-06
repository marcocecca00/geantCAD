#include "PhysicsConfig.hh"

namespace geantcad {

PhysicsConfig::PhysicsConfig() {
}

PhysicsConfig::~PhysicsConfig() {
}

std::string PhysicsConfig::emOptionToString(EMOption opt) {
    switch (opt) {
        case EMOption::Standard: return "Standard";
        case EMOption::Option1: return "Option1";
        case EMOption::Option2: return "Option2";
        case EMOption::Option3: return "Option3";
        case EMOption::Option4: return "Option4";
        case EMOption::Penelope: return "Penelope";
        case EMOption::Livermore: return "Livermore";
        default: return "Standard";
    }
}

PhysicsConfig::EMOption PhysicsConfig::stringToEMOption(const std::string& str) {
    if (str == "Option1") return EMOption::Option1;
    if (str == "Option2") return EMOption::Option2;
    if (str == "Option3") return EMOption::Option3;
    if (str == "Option4") return EMOption::Option4;
    if (str == "Penelope") return EMOption::Penelope;
    if (str == "Livermore") return EMOption::Livermore;
    return EMOption::Standard;
}

std::string PhysicsConfig::hadronicModelToString(HadronicModel model) {
    switch (model) {
        case HadronicModel::FTFP_BERT: return "FTFP_BERT";
        case HadronicModel::QGSP_BERT: return "QGSP_BERT";
        case HadronicModel::QGSP_BIC: return "QGSP_BIC";
        case HadronicModel::FTFP_INCLXX: return "FTFP_INCLXX";
        default: return "FTFP_BERT";
    }
}

PhysicsConfig::HadronicModel PhysicsConfig::stringToHadronicModel(const std::string& str) {
    if (str == "QGSP_BERT") return HadronicModel::QGSP_BERT;
    if (str == "QGSP_BIC") return HadronicModel::QGSP_BIC;
    if (str == "FTFP_INCLXX") return HadronicModel::FTFP_INCLXX;
    return HadronicModel::FTFP_BERT;
}

nlohmann::json PhysicsConfig::toJson() const {
    nlohmann::json j;
    j["em_enabled"] = emEnabled;
    j["decay_enabled"] = decayEnabled;
    j["optical_enabled"] = opticalEnabled;
    j["hadronic_enabled"] = hadronicEnabled;
    j["standard_list"] = standardList;
    j["em_option"] = emOptionToString(emOption);
    j["hadronic_model"] = hadronicModelToString(hadronicModel);
    j["ion_physics_enabled"] = ionPhysicsEnabled;
    j["radioactive_decay_enabled"] = radioactiveDecayEnabled;
    j["step_limiter_enabled"] = stepLimiterEnabled;
    j["gamma_cut"] = gammaCut;
    j["electron_cut"] = electronCut;
    j["positron_cut"] = positronCut;
    j["proton_cut"] = protonCut;
    return j;
}

void PhysicsConfig::fromJson(const nlohmann::json& j) {
    if (j.contains("em_enabled")) emEnabled = j["em_enabled"];
    if (j.contains("decay_enabled")) decayEnabled = j["decay_enabled"];
    if (j.contains("optical_enabled")) opticalEnabled = j["optical_enabled"];
    if (j.contains("hadronic_enabled")) hadronicEnabled = j["hadronic_enabled"];
    if (j.contains("standard_list")) standardList = j["standard_list"];
    if (j.contains("em_option")) emOption = stringToEMOption(j["em_option"]);
    if (j.contains("hadronic_model")) hadronicModel = stringToHadronicModel(j["hadronic_model"]);
    if (j.contains("ion_physics_enabled")) ionPhysicsEnabled = j["ion_physics_enabled"];
    if (j.contains("radioactive_decay_enabled")) radioactiveDecayEnabled = j["radioactive_decay_enabled"];
    if (j.contains("step_limiter_enabled")) stepLimiterEnabled = j["step_limiter_enabled"];
    if (j.contains("gamma_cut")) gammaCut = j["gamma_cut"];
    if (j.contains("electron_cut")) electronCut = j["electron_cut"];
    if (j.contains("positron_cut")) positronCut = j["positron_cut"];
    if (j.contains("proton_cut")) protonCut = j["proton_cut"];
}

std::string PhysicsConfig::generatePhysicsCode() const {
    std::ostringstream oss;
    
    // Electromagnetic physics
    if (emEnabled) {
        switch (emOption) {
            case EMOption::Standard:
        oss << "    RegisterPhysics(new G4EmStandardPhysics());\n";
                break;
            case EMOption::Option1:
                oss << "    RegisterPhysics(new G4EmStandardPhysics_option1());\n";
                break;
            case EMOption::Option2:
                oss << "    RegisterPhysics(new G4EmStandardPhysics_option2());\n";
                break;
            case EMOption::Option3:
                oss << "    RegisterPhysics(new G4EmStandardPhysics_option3());\n";
                break;
            case EMOption::Option4:
                oss << "    RegisterPhysics(new G4EmStandardPhysics_option4());\n";
                break;
            case EMOption::Penelope:
                oss << "    RegisterPhysics(new G4EmPenelopePhysics());\n";
                break;
            case EMOption::Livermore:
                oss << "    RegisterPhysics(new G4EmLivermorePhysics());\n";
                break;
        }
    }
    
    // Decay physics
    if (decayEnabled) {
        oss << "    RegisterPhysics(new G4DecayPhysics());\n";
    }
    
    // Radioactive decay
    if (radioactiveDecayEnabled) {
        oss << "    RegisterPhysics(new G4RadioactiveDecayPhysics());\n";
    }
    
    // Optical physics
    if (opticalEnabled) {
        oss << "    RegisterPhysics(new G4OpticalPhysics());\n";
    }
    
    // Hadronic physics
    if (hadronicEnabled) {
        oss << "    RegisterPhysics(new G4HadronElasticPhysics());\n";
        
        switch (hadronicModel) {
            case HadronicModel::FTFP_BERT:
            oss << "    RegisterPhysics(new G4HadronPhysicsFTFP_BERT());\n";
                break;
            case HadronicModel::QGSP_BERT:
            oss << "    RegisterPhysics(new G4HadronPhysicsQGSP_BERT());\n";
                break;
            case HadronicModel::QGSP_BIC:
                oss << "    RegisterPhysics(new G4HadronPhysicsQGSP_BIC());\n";
                break;
            case HadronicModel::FTFP_INCLXX:
                oss << "    RegisterPhysics(new G4HadronPhysicsFTFP_INCLXX());\n";
                break;
        }
        
        oss << "    RegisterPhysics(new G4StoppingPhysics());\n";
    }
    
    // Ion physics
    if (ionPhysicsEnabled) {
        oss << "    RegisterPhysics(new G4IonPhysics());\n";
    }
    
    // Step limiter
    if (stepLimiterEnabled) {
        oss << "    RegisterPhysics(new G4StepLimiterPhysics());\n";
    }
    
    // Production cuts
    oss << "\n    // Set production cuts\n";
    oss << "    G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(100*eV, 1*GeV);\n";
    oss << "    G4Region* region = G4RegionStore::GetInstance()->GetRegion(\"DefaultRegionForTheWorld\");\n";
    oss << "    G4ProductionCuts* cuts = new G4ProductionCuts();\n";
    oss << "    cuts->SetProductionCut(" << gammaCut << "*mm, \"gamma\");\n";
    oss << "    cuts->SetProductionCut(" << electronCut << "*mm, \"e-\");\n";
    oss << "    cuts->SetProductionCut(" << positronCut << "*mm, \"e+\");\n";
    oss << "    cuts->SetProductionCut(" << protonCut << "*mm, \"proton\");\n";
    oss << "    region->SetProductionCuts(cuts);\n";
    
    return oss.str();
}

} // namespace geantcad

