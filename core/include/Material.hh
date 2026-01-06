#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <memory>

namespace geantcad {

/**
 * Element rappresenta un elemento chimico per la composizione di materiali.
 */
struct Element {
    std::string symbol;      // Es: "H", "O", "C"
    std::string name;        // Es: "Hydrogen", "Oxygen", "Carbon"
    int atomicNumber = 0;    // Z
    double atomicMass = 0.0; // A (g/mol)
    
    Element() = default;
    Element(const std::string& sym, const std::string& n, int z, double a)
        : symbol(sym), name(n), atomicNumber(z), atomicMass(a) {}
    
    // Common elements factory
    static Element Hydrogen() { return {"H", "Hydrogen", 1, 1.008}; }
    static Element Carbon() { return {"C", "Carbon", 6, 12.011}; }
    static Element Nitrogen() { return {"N", "Nitrogen", 7, 14.007}; }
    static Element Oxygen() { return {"O", "Oxygen", 8, 15.999}; }
    static Element Aluminum() { return {"Al", "Aluminum", 13, 26.982}; }
    static Element Silicon() { return {"Si", "Silicon", 14, 28.086}; }
    static Element Iron() { return {"Fe", "Iron", 26, 55.845}; }
    static Element Copper() { return {"Cu", "Copper", 29, 63.546}; }
    static Element Lead() { return {"Pb", "Lead", 82, 207.2}; }
    static Element Sodium() { return {"Na", "Sodium", 11, 22.990}; }
    static Element Iodine() { return {"I", "Iodine", 53, 126.904}; }
    static Element Cesium() { return {"Cs", "Cesium", 55, 132.905}; }
    static Element Barium() { return {"Ba", "Barium", 56, 137.327}; }
    static Element Germanium() { return {"Ge", "Germanium", 32, 72.630}; }
    static Element Bismuth() { return {"Bi", "Bismuth", 83, 208.980}; }
    static Element Lutetium() { return {"Lu", "Lutetium", 71, 174.967}; }
    static Element Yttrium() { return {"Y", "Yttrium", 39, 88.906}; }
    
    nlohmann::json toJson() const;
    static Element fromJson(const nlohmann::json& j);
};

/**
 * Componente di un materiale composto.
 * Può essere un elemento o un altro materiale con una frazione.
 */
struct MaterialComponent {
    enum class Type { Element, Material };
    
    Type type = Type::Element;
    Element element;                         // Se type == Element
    std::shared_ptr<class Material> material; // Se type == Material
    
    // Frazione: può essere massa (0-1) o numero di atomi (intero)
    double fraction = 0.0;  // Frazione di massa (0.0 - 1.0)
    int nAtoms = 0;         // Numero di atomi (per composizione stoichiometrica)
    
    bool useMassFraction() const { return nAtoms == 0; }
    
    nlohmann::json toJson() const;
    static MaterialComponent fromJson(const nlohmann::json& j);
};

/**
 * Material rappresenta un materiale Geant4.
 * Supporta:
 * - Materiali NIST predefiniti
 * - Materiali da singolo elemento
 * - Materiali composti da elementi (per frazione di massa o atomi)
 * - Materiali composti da altri materiali (per frazione di massa)
 */
class Material {
public:
    enum class Type {
        NIST,           // Materiale NIST predefinito
        SingleElement,  // Singolo elemento (Z, A, densità)
        Compound,       // Composto da elementi
        Mixture         // Miscela di materiali
    };
    
    Material(const std::string& name, const std::string& nistName = "");
    
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    const std::string& getNistName() const { return nistName_; }
    
    Type getMaterialType() const { return type_; }
    void setMaterialType(Type t) { type_ = t; }
    
    // Custom material properties (per singolo elemento)
    double getDensity() const { return density_; } // g/cm³
    void setDensity(double d) { density_ = d; }
    
    int getAtomicNumber() const { return atomicNumber_; }
    void setAtomicNumber(int z) { atomicNumber_ = z; }
    
    double getAtomicMass() const { return atomicMass_; } // g/mol
    void setAtomicMass(double m) { atomicMass_ = m; }
    
    // Composition (per materiali composti)
    const std::vector<MaterialComponent>& getComponents() const { return components_; }
    void setComponents(const std::vector<MaterialComponent>& components) { components_ = components; }
    void addComponent(const MaterialComponent& comp) { components_.push_back(comp); }
    void clearComponents() { components_.clear(); }
    
    // State properties (for gases)
    enum class State { Solid, Liquid, Gas };
    State getState() const { return state_; }
    void setState(State s) { state_ = s; }
    
    double getTemperature() const { return temperature_; } // Kelvin
    void setTemperature(double t) { temperature_ = t; }
    
    double getPressure() const { return pressure_; } // atmosphere
    void setPressure(double p) { pressure_ = p; }
    
    // Visual properties (per viewport)
    struct Visual {
        float r = 0.8f;
        float g = 0.8f;
        float b = 0.8f;
        float a = 1.0f;
        bool wireframe = false;
    };
    
    const Visual& getVisual() const { return visual_; }
    Visual& getVisual() { return visual_; }
    
    // Serialization
    nlohmann::json toJson() const;
    static std::shared_ptr<Material> fromJson(const nlohmann::json& j);
    
    // Factory: common NIST materials
    static std::shared_ptr<Material> makeNist(const std::string& nistName);
    
    // Factory: custom compound materials
    // Create material from elements by mass fraction (fractions must sum to 1.0)
    static std::shared_ptr<Material> makeCompoundByMass(
        const std::string& name, 
        double density, 
        const std::vector<std::pair<Element, double>>& elementFractions);
    
    // Create material from elements by atom count (stoichiometric formula)
    static std::shared_ptr<Material> makeCompoundByAtoms(
        const std::string& name, 
        double density, 
        const std::vector<std::pair<Element, int>>& elementAtoms);
    
    // Create material from single element
    static std::shared_ptr<Material> makeFromElement(
        const std::string& name,
        double density,
        const Element& element);
    
    // Create gas material
    static std::shared_ptr<Material> makeGas(
        const std::string& name,
        double density,
        const std::vector<std::pair<Element, double>>& elementFractions,
        double temperature = 293.15,  // K (20°C default)
        double pressure = 1.0);       // atm
    
    // Common material factories
    static std::shared_ptr<Material> makeAir();
    static std::shared_ptr<Material> makeVacuum();
    static std::shared_ptr<Material> makeWater();
    static std::shared_ptr<Material> makeLead();
    static std::shared_ptr<Material> makeSilicon();
    static std::shared_ptr<Material> makeAluminum();
    static std::shared_ptr<Material> makeIron();
    static std::shared_ptr<Material> makeCopper();
    static std::shared_ptr<Material> makeTitanium();
    static std::shared_ptr<Material> makeStainlessSteel();
    static std::shared_ptr<Material> makeBrass();
    static std::shared_ptr<Material> makeBronze();
    static std::shared_ptr<Material> makeGlass();
    static std::shared_ptr<Material> makePolystyrene();
    static std::shared_ptr<Material> makePolyethylene();
    static std::shared_ptr<Material> makePlexiglass();
    static std::shared_ptr<Material> makeCarbonDioxide();
    static std::shared_ptr<Material> makeArgon();
    static std::shared_ptr<Material> makeHelium();
    static std::shared_ptr<Material> makeNitrogen();
    static std::shared_ptr<Material> makeOxygen();
    static std::shared_ptr<Material> makeSodium();
    static std::shared_ptr<Material> makeIodine();
    static std::shared_ptr<Material> makeCesiumIodide();
    static std::shared_ptr<Material> makeSodiumIodide();
    static std::shared_ptr<Material> makeBGO();
    static std::shared_ptr<Material> makeLYSO();
    
    // Get Geant4 C++ code for this material definition
    std::string toGeant4Code() const;

private:
    std::string name_;
    std::string nistName_;       // NIST material name (es. "G4_AIR", "G4_Si")
    Type type_ = Type::NIST;     // Material type
    double density_ = 0.0;       // g/cm³
    int atomicNumber_ = 0;       // Z (for single element)
    double atomicMass_ = 0.0;    // A (g/mol, for single element)
    
    // Composition (for compounds/mixtures)
    std::vector<MaterialComponent> components_;
    
    // State properties
    State state_ = State::Solid;
    double temperature_ = 293.15; // Kelvin (default 20°C)
    double pressure_ = 1.0;       // atmosphere
    
    Visual visual_;
};

} // namespace geantcad

