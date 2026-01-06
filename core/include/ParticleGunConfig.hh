#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace geantcad {

/**
 * Configuration for Geant4 ParticleGun (primary particle source)
 */
struct ParticleGunConfig {
    // Particle type
    std::string particleType = "gamma";  // gamma, e-, e+, proton, neutron, etc.
    
    // Energy configuration
    enum class EnergyMode {
        Mono,      // Single energy
        Uniform,   // Uniform distribution [Emin, Emax]
        Gaussian   // Gaussian distribution (mean, sigma)
    };
    EnergyMode energyMode = EnergyMode::Mono;
    double energy = 1.0;  // MeV (for Mono)
    double energyMin = 0.5;  // MeV (for Uniform/Gaussian)
    double energyMax = 2.0;  // MeV (for Uniform)
    double energyMean = 1.0;  // MeV (for Gaussian)
    double energySigma = 0.1;  // MeV (for Gaussian)
    
    // Position configuration
    enum class PositionMode {
        Point,     // Single point
        Volume,    // Random in volume
        Surface    // Random on surface
    };
    PositionMode positionMode = PositionMode::Point;
    double positionX = 0.0;  // mm
    double positionY = 0.0;  // mm
    double positionZ = 0.0;  // mm
    double positionRadius = 10.0;  // mm (for Volume/Surface)
    std::string positionVolume = "";  // Volume name (for Volume/Surface)
    
    // Direction configuration
    enum class DirectionMode {
        Isotropic,  // Random isotropic
        Fixed,      // Fixed direction
        Cone        // Cone distribution
    };
    DirectionMode directionMode = DirectionMode::Isotropic;
    double directionX = 0.0;  // Unit vector
    double directionY = 0.0;
    double directionZ = 1.0;
    double coneAngle = 30.0;  // degrees (for Cone)
    
    // Number of particles per event
    int numberOfParticles = 1;
    
    // Serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    
    // Generate Geant4 macro commands
    std::string generateMacroCommands() const;
    
    // Helper: get energy mode name
    std::string getEnergyModeName() const;
    std::string getPositionModeName() const;
    std::string getDirectionModeName() const;
};

} // namespace geantcad

