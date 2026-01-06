#include "Material.hh"

namespace geantcad {

Material::Material(const std::string& name, const std::string& nistName)
    : name_(name)
    , nistName_(nistName)
{
}

nlohmann::json Material::toJson() const {
    nlohmann::json j;
    j["name"] = name_;
    j["nistName"] = nistName_;
    
    if (!nistName_.empty()) {
        // NIST material
        j["type"] = "nist";
    } else {
        // Custom material
        j["type"] = "custom";
        j["density"] = density_;
        j["atomicNumber"] = atomicNumber_;
        j["atomicMass"] = atomicMass_;
    }
    
    j["visual"] = {
        {"r", visual_.r}, {"g", visual_.g}, {"b", visual_.b}, {"a", visual_.a},
        {"wireframe", visual_.wireframe}
    };
    
    return j;
}

std::shared_ptr<Material> Material::fromJson(const nlohmann::json& j) {
    auto mat = std::make_shared<Material>(j["name"], j.value("nistName", ""));
    
    if (j.value("type", "nist") == "custom") {
        mat->density_ = j.value("density", 0.0);
        mat->atomicNumber_ = j.value("atomicNumber", 0);
        mat->atomicMass_ = j.value("atomicMass", 0.0);
    }
    
    if (j.contains("visual")) {
        auto v = j["visual"];
        mat->visual_.r = v.value("r", 0.8f);
        mat->visual_.g = v.value("g", 0.8f);
        mat->visual_.b = v.value("b", 0.8f);
        mat->visual_.a = v.value("a", 1.0f);
        mat->visual_.wireframe = v.value("wireframe", false);
    }
    
    return mat;
}

std::shared_ptr<Material> Material::makeNist(const std::string& nistName) {
    auto mat = std::make_shared<Material>(nistName, nistName);
    // Set default visual based on material
    // Common materials with appropriate colors
    if (nistName == "G4_AIR" || nistName == "G4_Galactic") {
        mat->visual_ = {0.9f, 0.9f, 0.9f, 0.3f, false};
    } else if (nistName == "G4_WATER") {
        mat->visual_ = {0.2f, 0.4f, 0.8f, 0.5f, false};
    } else if (nistName == "G4_Pb" || nistName == "G4_LEAD") {
        mat->visual_ = {0.3f, 0.3f, 0.3f, 1.0f, false};
    } else if (nistName == "G4_Si" || nistName == "G4_SILICON_DIOXIDE") {
        mat->visual_ = {0.7f, 0.7f, 0.8f, 1.0f, false};
    } else if (nistName == "G4_Al" || nistName == "G4_ALUMINUM") {
        mat->visual_ = {0.8f, 0.8f, 0.85f, 1.0f, false};
    } else if (nistName == "G4_Fe" || nistName == "G4_IRON") {
        mat->visual_ = {0.5f, 0.5f, 0.5f, 1.0f, false};
    } else if (nistName == "G4_Cu" || nistName == "G4_COPPER") {
        mat->visual_ = {0.8f, 0.5f, 0.2f, 1.0f, false};
    } else if (nistName == "G4_Ti" || nistName == "G4_TITANIUM") {
        mat->visual_ = {0.7f, 0.7f, 0.7f, 1.0f, false};
    } else if (nistName == "G4_STAINLESS-STEEL") {
        mat->visual_ = {0.6f, 0.6f, 0.65f, 1.0f, false};
    } else if (nistName == "G4_BRASS") {
        mat->visual_ = {0.8f, 0.7f, 0.4f, 1.0f, false};
    } else if (nistName == "G4_BRONZE") {
        mat->visual_ = {0.7f, 0.5f, 0.3f, 1.0f, false};
    } else if (nistName == "G4_GLASS_PLATE" || nistName == "G4_Pyrex_Glass") {
        mat->visual_ = {0.9f, 0.95f, 1.0f, 0.6f, false};
    } else if (nistName == "G4_POLYSTYRENE" || nistName == "G4_POLYETHYLENE") {
        mat->visual_ = {0.95f, 0.95f, 0.95f, 0.8f, false};
    } else if (nistName == "G4_PLEXIGLASS") {
        mat->visual_ = {0.9f, 0.9f, 1.0f, 0.7f, false};
    } else if (nistName == "G4_CARBON_DIOXIDE") {
        mat->visual_ = {0.85f, 0.85f, 0.85f, 0.2f, false};
    } else if (nistName == "G4_Ar" || nistName == "G4_ARGON") {
        mat->visual_ = {0.9f, 0.9f, 0.95f, 0.1f, false};
    } else if (nistName == "G4_He" || nistName == "G4_HELIUM") {
        mat->visual_ = {0.95f, 0.95f, 1.0f, 0.1f, false};
    } else if (nistName == "G4_N" || nistName == "G4_NITROGEN") {
        mat->visual_ = {0.9f, 0.9f, 0.9f, 0.1f, false};
    } else if (nistName == "G4_O" || nistName == "G4_OXYGEN") {
        mat->visual_ = {0.9f, 0.9f, 0.95f, 0.1f, false};
    } else if (nistName == "G4_Na" || nistName == "G4_SODIUM") {
        mat->visual_ = {0.9f, 0.9f, 0.7f, 1.0f, false};
    } else if (nistName == "G4_I" || nistName == "G4_IODINE") {
        mat->visual_ = {0.7f, 0.5f, 0.9f, 1.0f, false};
    } else if (nistName == "G4_CsI" || nistName == "G4_CESIUM_IODIDE") {
        mat->visual_ = {0.9f, 0.9f, 0.7f, 1.0f, false};
    } else if (nistName == "G4_NaI" || nistName == "G4_SODIUM_IODIDE") {
        mat->visual_ = {0.9f, 0.95f, 0.8f, 1.0f, false};
    } else if (nistName == "G4_BGO" || nistName == "G4_BARIUM_FLUORIDE") {
        mat->visual_ = {0.8f, 0.9f, 0.9f, 1.0f, false};
    } else if (nistName == "G4_LYSO" || nistName == "G4_LUTETIUM_OXYORTHOSILICATE") {
        mat->visual_ = {0.7f, 0.8f, 0.9f, 1.0f, false};
    } else {
        // Default: light gray for unknown materials
        mat->visual_ = {0.8f, 0.8f, 0.8f, 1.0f, false};
    }
    return mat;
}

std::shared_ptr<Material> Material::makeAir() {
    return makeNist("G4_AIR");
}

std::shared_ptr<Material> Material::makeVacuum() {
    return makeNist("G4_Galactic");
}

std::shared_ptr<Material> Material::makeWater() {
    return makeNist("G4_WATER");
}

std::shared_ptr<Material> Material::makeLead() {
    return makeNist("G4_Pb");
}

std::shared_ptr<Material> Material::makeSilicon() {
    return makeNist("G4_Si");
}

std::shared_ptr<Material> Material::makeAluminum() {
    return makeNist("G4_Al");
}

std::shared_ptr<Material> Material::makeIron() {
    return makeNist("G4_Fe");
}

std::shared_ptr<Material> Material::makeCopper() {
    return makeNist("G4_Cu");
}

std::shared_ptr<Material> Material::makeTitanium() {
    return makeNist("G4_Ti");
}

std::shared_ptr<Material> Material::makeStainlessSteel() {
    return makeNist("G4_STAINLESS-STEEL");
}

std::shared_ptr<Material> Material::makeBrass() {
    return makeNist("G4_BRASS");
}

std::shared_ptr<Material> Material::makeBronze() {
    return makeNist("G4_BRONZE");
}

std::shared_ptr<Material> Material::makeGlass() {
    return makeNist("G4_GLASS_PLATE");
}

std::shared_ptr<Material> Material::makePolystyrene() {
    return makeNist("G4_POLYSTYRENE");
}

std::shared_ptr<Material> Material::makePolyethylene() {
    return makeNist("G4_POLYETHYLENE");
}

std::shared_ptr<Material> Material::makePlexiglass() {
    return makeNist("G4_PLEXIGLASS");
}

std::shared_ptr<Material> Material::makeCarbonDioxide() {
    return makeNist("G4_CARBON_DIOXIDE");
}

std::shared_ptr<Material> Material::makeArgon() {
    return makeNist("G4_Ar");
}

std::shared_ptr<Material> Material::makeHelium() {
    return makeNist("G4_He");
}

std::shared_ptr<Material> Material::makeNitrogen() {
    return makeNist("G4_N");
}

std::shared_ptr<Material> Material::makeOxygen() {
    return makeNist("G4_O");
}

std::shared_ptr<Material> Material::makeSodium() {
    return makeNist("G4_Na");
}

std::shared_ptr<Material> Material::makeIodine() {
    return makeNist("G4_I");
}

std::shared_ptr<Material> Material::makeCesiumIodide() {
    return makeNist("G4_CESIUM_IODIDE");
}

std::shared_ptr<Material> Material::makeSodiumIodide() {
    return makeNist("G4_SODIUM_IODIDE");
}

std::shared_ptr<Material> Material::makeBGO() {
    return makeNist("G4_BARIUM_FLUORIDE");
}

std::shared_ptr<Material> Material::makeLYSO() {
    return makeNist("G4_LUTETIUM_OXYORTHOSILICATE");
}

} // namespace geantcad

