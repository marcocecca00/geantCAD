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
    if (nistName == "G4_AIR") {
        mat->visual_ = {0.9f, 0.9f, 0.9f, 0.3f, false};
    } else if (nistName == "G4_WATER") {
        mat->visual_ = {0.2f, 0.4f, 0.8f, 0.5f, false};
    } else if (nistName == "G4_Pb") {
        mat->visual_ = {0.3f, 0.3f, 0.3f, 1.0f, false};
    } else if (nistName == "G4_Si") {
        mat->visual_ = {0.7f, 0.7f, 0.8f, 1.0f, false};
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

} // namespace geantcad

