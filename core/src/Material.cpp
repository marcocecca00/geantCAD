#include "Material.hh"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace geantcad {

// ============= Element Implementation =============

nlohmann::json Element::toJson() const {
    return {
        {"symbol", symbol},
        {"name", name},
        {"atomicNumber", atomicNumber},
        {"atomicMass", atomicMass}
    };
}

Element Element::fromJson(const nlohmann::json& j) {
    Element e;
    e.symbol = j.value("symbol", "");
    e.name = j.value("name", "");
    e.atomicNumber = j.value("atomicNumber", 0);
    e.atomicMass = j.value("atomicMass", 0.0);
    return e;
}

// ============= MaterialComponent Implementation =============

nlohmann::json MaterialComponent::toJson() const {
    nlohmann::json j;
    j["type"] = (type == Type::Element) ? "element" : "material";
    
    if (type == Type::Element) {
        j["element"] = element.toJson();
    } else if (material) {
        j["materialName"] = material->getName();
    }
    
    if (nAtoms > 0) {
        j["nAtoms"] = nAtoms;
    } else {
        j["fraction"] = fraction;
    }
    
    return j;
}

MaterialComponent MaterialComponent::fromJson(const nlohmann::json& j) {
    MaterialComponent comp;
    
    std::string typeStr = j.value("type", "element");
    comp.type = (typeStr == "element") ? Type::Element : Type::Material;
    
    if (comp.type == Type::Element && j.contains("element")) {
        comp.element = Element::fromJson(j["element"]);
    }
    // Note: material references need to be resolved separately
    
    comp.nAtoms = j.value("nAtoms", 0);
    comp.fraction = j.value("fraction", 0.0);
    
    return comp;
}

// ============= Material Implementation =============

Material::Material(const std::string& name, const std::string& nistName)
    : name_(name)
    , nistName_(nistName)
{
    if (!nistName.empty()) {
        type_ = Type::NIST;
    }
}

nlohmann::json Material::toJson() const {
    nlohmann::json j;
    j["name"] = name_;
    j["nistName"] = nistName_;
    j["density"] = density_;
    
    // Material type
    switch (type_) {
        case Type::NIST: j["materialType"] = "nist"; break;
        case Type::SingleElement: j["materialType"] = "singleElement"; break;
        case Type::Compound: j["materialType"] = "compound"; break;
        case Type::Mixture: j["materialType"] = "mixture"; break;
    }
    
    // For single element
    if (type_ == Type::SingleElement) {
        j["atomicNumber"] = atomicNumber_;
        j["atomicMass"] = atomicMass_;
    }
    
    // For compounds/mixtures
    if (!components_.empty()) {
        nlohmann::json comps = nlohmann::json::array();
        for (const auto& comp : components_) {
            comps.push_back(comp.toJson());
        }
        j["components"] = comps;
    }
    
    // State properties
    switch (state_) {
        case State::Solid: j["state"] = "solid"; break;
        case State::Liquid: j["state"] = "liquid"; break;
        case State::Gas: j["state"] = "gas"; break;
    }
    j["temperature"] = temperature_;
    j["pressure"] = pressure_;
    
    // Visual properties
    j["visual"] = {
        {"r", visual_.r}, {"g", visual_.g}, {"b", visual_.b}, {"a", visual_.a},
        {"wireframe", visual_.wireframe}
    };
    
    return j;
}

std::shared_ptr<Material> Material::fromJson(const nlohmann::json& j) {
    auto mat = std::make_shared<Material>(j["name"], j.value("nistName", ""));
    mat->density_ = j.value("density", 0.0);
    
    // Material type
    std::string typeStr = j.value("materialType", "nist");
    if (typeStr == "nist") mat->type_ = Type::NIST;
    else if (typeStr == "singleElement") mat->type_ = Type::SingleElement;
    else if (typeStr == "compound") mat->type_ = Type::Compound;
    else if (typeStr == "mixture") mat->type_ = Type::Mixture;
    
    // For single element
    mat->atomicNumber_ = j.value("atomicNumber", 0);
    mat->atomicMass_ = j.value("atomicMass", 0.0);
    
    // For compounds/mixtures
    if (j.contains("components")) {
        for (const auto& compJson : j["components"]) {
            mat->components_.push_back(MaterialComponent::fromJson(compJson));
        }
    }
    
    // State properties
    std::string stateStr = j.value("state", "solid");
    if (stateStr == "solid") mat->state_ = State::Solid;
    else if (stateStr == "liquid") mat->state_ = State::Liquid;
    else if (stateStr == "gas") mat->state_ = State::Gas;
    
    mat->temperature_ = j.value("temperature", 293.15);
    mat->pressure_ = j.value("pressure", 1.0);
    
    // Visual properties
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

// Factory methods for custom materials

std::shared_ptr<Material> Material::makeCompoundByMass(
    const std::string& name, 
    double density, 
    const std::vector<std::pair<Element, double>>& elementFractions) {
    
    auto mat = std::make_shared<Material>(name, "");
    mat->type_ = Type::Compound;
    mat->density_ = density;
    
    for (const auto& [elem, fraction] : elementFractions) {
        MaterialComponent comp;
        comp.type = MaterialComponent::Type::Element;
        comp.element = elem;
        comp.fraction = fraction;
        comp.nAtoms = 0;
        mat->components_.push_back(comp);
    }
    
    return mat;
}

std::shared_ptr<Material> Material::makeCompoundByAtoms(
    const std::string& name, 
    double density, 
    const std::vector<std::pair<Element, int>>& elementAtoms) {
    
    auto mat = std::make_shared<Material>(name, "");
    mat->type_ = Type::Compound;
    mat->density_ = density;
    
    for (const auto& [elem, nAtoms] : elementAtoms) {
        MaterialComponent comp;
        comp.type = MaterialComponent::Type::Element;
        comp.element = elem;
        comp.nAtoms = nAtoms;
        comp.fraction = 0.0;
        mat->components_.push_back(comp);
    }
    
    return mat;
}

std::shared_ptr<Material> Material::makeFromElement(
    const std::string& name,
    double density,
    const Element& element) {
    
    auto mat = std::make_shared<Material>(name, "");
    mat->type_ = Type::SingleElement;
    mat->density_ = density;
    mat->atomicNumber_ = element.atomicNumber;
    mat->atomicMass_ = element.atomicMass;
    
    return mat;
}

std::shared_ptr<Material> Material::makeGas(
    const std::string& name,
    double density,
    const std::vector<std::pair<Element, double>>& elementFractions,
    double temperature,
    double pressure) {
    
    auto mat = makeCompoundByMass(name, density, elementFractions);
    mat->state_ = State::Gas;
    mat->temperature_ = temperature;
    mat->pressure_ = pressure;
    
    return mat;
}

std::string Material::toGeant4Code() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    
    std::string varName = name_;
    // Replace spaces and invalid chars with underscores
    for (char& c : varName) {
        if (!isalnum(c)) c = '_';
    }
    
    switch (type_) {
        case Type::NIST:
            ss << "G4Material* " << varName << " = nist->FindOrBuildMaterial(\"" 
               << nistName_ << "\");";
            break;
            
        case Type::SingleElement:
            ss << "G4Material* " << varName << " = new G4Material(\"" << name_ << "\", "
               << atomicNumber_ << ", " << atomicMass_ << "*g/mole, "
               << density_ << "*g/cm3);";
            break;
            
        case Type::Compound: {
            // First define elements if needed
            for (size_t i = 0; i < components_.size(); ++i) {
                const auto& comp = components_[i];
                if (comp.type == MaterialComponent::Type::Element) {
                    ss << "G4Element* el" << comp.element.symbol << " = new G4Element(\""
                       << comp.element.name << "\", \"" << comp.element.symbol << "\", "
                       << comp.element.atomicNumber << ", "
                       << comp.element.atomicMass << "*g/mole);\n";
                }
            }
            
            // Now define the material
            ss << "G4Material* " << varName << " = new G4Material(\"" << name_ << "\", "
               << density_ << "*g/cm3, " << components_.size() << ");\n";
            
            // Add components
            for (const auto& comp : components_) {
                if (comp.type == MaterialComponent::Type::Element) {
                    if (comp.nAtoms > 0) {
                        ss << varName << "->AddElement(el" << comp.element.symbol 
                           << ", " << comp.nAtoms << ");\n";
                    } else {
                        ss << varName << "->AddElement(el" << comp.element.symbol 
                           << ", " << comp.fraction << ");\n";
                    }
                }
            }
            break;
        }
            
        case Type::Mixture:
            // Similar to Compound but for materials
            ss << "// Mixture material - components are other materials\n";
            ss << "G4Material* " << varName << " = new G4Material(\"" << name_ << "\", "
               << density_ << "*g/cm3, " << components_.size() << ");\n";
            for (const auto& comp : components_) {
                if (comp.material) {
                    ss << varName << "->AddMaterial(" << comp.material->getName() 
                       << ", " << comp.fraction << ");\n";
                }
            }
            break;
    }
    
    return ss.str();
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

