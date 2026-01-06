#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <memory>

namespace geantcad {

/**
 * Material rappresenta un materiale Geant4.
 * MVP: mapping a NIST materials + materiali custom semplici.
 */
class Material {
public:
    Material(const std::string& name, const std::string& nistName = "");
    
    const std::string& getName() const { return name_; }
    const std::string& getNistName() const { return nistName_; }
    
    // Custom material properties (se non NIST)
    double getDensity() const { return density_; } // g/cm³
    void setDensity(double d) { density_ = d; }
    
    int getAtomicNumber() const { return atomicNumber_; }
    void setAtomicNumber(int z) { atomicNumber_ = z; }
    
    double getAtomicMass() const { return atomicMass_; } // g/mol
    void setAtomicMass(double m) { atomicMass_ = m; }
    
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
    static std::shared_ptr<Material> makeAir();
    static std::shared_ptr<Material> makeVacuum();
    static std::shared_ptr<Material> makeWater();
    static std::shared_ptr<Material> makeLead();
    static std::shared_ptr<Material> makeSilicon();

private:
    std::string name_;
    std::string nistName_; // NIST material name (es. "G4_AIR", "G4_Si")
    double density_ = 0.0; // g/cm³ (solo per custom)
    int atomicNumber_ = 0;
    double atomicMass_ = 0.0; // g/mol
    
    Visual visual_;
};

} // namespace geantcad

