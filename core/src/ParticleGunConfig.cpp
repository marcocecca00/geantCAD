#include "ParticleGunConfig.hh"
#include <sstream>
#include <iomanip>

namespace geantcad {

nlohmann::json ParticleGunConfig::toJson() const {
    nlohmann::json j;
    j["particleType"] = particleType;
    j["energyMode"] = static_cast<int>(energyMode);
    j["energy"] = energy;
    j["energyMin"] = energyMin;
    j["energyMax"] = energyMax;
    j["energyMean"] = energyMean;
    j["energySigma"] = energySigma;
    j["positionMode"] = static_cast<int>(positionMode);
    j["positionX"] = positionX;
    j["positionY"] = positionY;
    j["positionZ"] = positionZ;
    j["positionRadius"] = positionRadius;
    j["positionVolume"] = positionVolume;
    j["directionMode"] = static_cast<int>(directionMode);
    j["directionX"] = directionX;
    j["directionY"] = directionY;
    j["directionZ"] = directionZ;
    j["coneAngle"] = coneAngle;
    j["numberOfParticles"] = numberOfParticles;
    return j;
}

void ParticleGunConfig::fromJson(const nlohmann::json& j) {
    if (j.contains("particleType")) particleType = j["particleType"];
    if (j.contains("energyMode")) energyMode = static_cast<EnergyMode>(j["energyMode"]);
    if (j.contains("energy")) energy = j["energy"];
    if (j.contains("energyMin")) energyMin = j["energyMin"];
    if (j.contains("energyMax")) energyMax = j["energyMax"];
    if (j.contains("energyMean")) energyMean = j["energyMean"];
    if (j.contains("energySigma")) energySigma = j["energySigma"];
    if (j.contains("positionMode")) positionMode = static_cast<PositionMode>(j["positionMode"]);
    if (j.contains("positionX")) positionX = j["positionX"];
    if (j.contains("positionY")) positionY = j["positionY"];
    if (j.contains("positionZ")) positionZ = j["positionZ"];
    if (j.contains("positionRadius")) positionRadius = j["positionRadius"];
    if (j.contains("positionVolume")) positionVolume = j["positionVolume"];
    if (j.contains("directionMode")) directionMode = static_cast<DirectionMode>(j["directionMode"]);
    if (j.contains("directionX")) directionX = j["directionX"];
    if (j.contains("directionY")) directionY = j["directionY"];
    if (j.contains("directionZ")) directionZ = j["directionZ"];
    if (j.contains("coneAngle")) coneAngle = j["coneAngle"];
    if (j.contains("numberOfParticles")) numberOfParticles = j["numberOfParticles"];
}

std::string ParticleGunConfig::getEnergyModeName() const {
    switch (energyMode) {
        case EnergyMode::Mono: return "Mono";
        case EnergyMode::Uniform: return "Uniform";
        case EnergyMode::Gaussian: return "Gaussian";
        default: return "Mono";
    }
}

std::string ParticleGunConfig::getPositionModeName() const {
    switch (positionMode) {
        case PositionMode::Point: return "Point";
        case PositionMode::Volume: return "Volume";
        case PositionMode::Surface: return "Surface";
        default: return "Point";
    }
}

std::string ParticleGunConfig::getDirectionModeName() const {
    switch (directionMode) {
        case DirectionMode::Isotropic: return "Isotropic";
        case DirectionMode::Fixed: return "Fixed";
        case DirectionMode::Cone: return "Cone";
        default: return "Isotropic";
    }
}

std::string ParticleGunConfig::generateMacroCommands() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    
    // Set particle type
    oss << "/gun/particle " << particleType << "\n";
    
    // Set number of particles
    oss << "/gun/number " << numberOfParticles << "\n";
    
    // Energy configuration
    switch (energyMode) {
        case EnergyMode::Mono:
            oss << "/gun/energy " << energy << " MeV\n";
            break;
        case EnergyMode::Uniform:
            oss << "/gun/energy " << energyMin << " MeV\n";
            oss << "/gun/energy " << energyMax << " MeV\n";
            // Note: Geant4 doesn't have built-in uniform distribution in macro,
            // this would need custom PrimaryGeneratorAction
            break;
        case EnergyMode::Gaussian:
            oss << "/gun/energy " << energyMean << " MeV\n";
            // Note: Gaussian would need custom PrimaryGeneratorAction
            break;
    }
    
    // Position configuration
    switch (positionMode) {
        case PositionMode::Point:
            oss << "/gun/position " << positionX << " " << positionY << " " << positionZ << " mm\n";
            break;
        case PositionMode::Volume:
            if (!positionVolume.empty()) {
                oss << "# Position: random in volume '" << positionVolume << "'\n";
                oss << "# Note: Requires custom PrimaryGeneratorAction\n";
            } else {
                oss << "/gun/position " << positionX << " " << positionY << " " << positionZ << " mm\n";
            }
            break;
        case PositionMode::Surface:
            if (!positionVolume.empty()) {
                oss << "# Position: random on surface of volume '" << positionVolume << "'\n";
                oss << "# Note: Requires custom PrimaryGeneratorAction\n";
            } else {
                oss << "/gun/position " << positionX << " " << positionY << " " << positionZ << " mm\n";
            }
            break;
    }
    
    // Direction configuration
    switch (directionMode) {
        case DirectionMode::Isotropic:
            oss << "/gun/direction 0 0 1\n";  // Will be randomized by Geant4
            // Note: Isotropic requires /gun/direction randomization or custom action
            break;
        case DirectionMode::Fixed:
            {
                // Normalize direction vector
                double norm = std::sqrt(directionX*directionX + directionY*directionY + directionZ*directionZ);
                if (norm > 0.0) {
                    oss << "/gun/direction " << (directionX/norm) << " " 
                        << (directionY/norm) << " " << (directionZ/norm) << "\n";
                } else {
                    oss << "/gun/direction 0 0 1\n";
                }
            }
            break;
        case DirectionMode::Cone:
            oss << "/gun/direction " << directionX << " " << directionY << " " << directionZ << "\n";
            oss << "# Cone angle: " << coneAngle << " degrees\n";
            oss << "# Note: Cone distribution requires custom PrimaryGeneratorAction\n";
            break;
    }
    
    return oss.str();
}

} // namespace geantcad

