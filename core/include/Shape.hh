#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <variant>

namespace geantcad {

// Shape types
enum class ShapeType {
    Box,
    Tube,
    Sphere,
    Cone,
    Trd,
    Polycone,
    Polyhedra,
    BooleanSolid  // For G4UnionSolid, G4SubtractionSolid, G4IntersectionSolid
};

// Boolean operation types (maps to G4 boolean solids)
enum class BooleanOperation {
    Union,        // G4UnionSolid
    Subtraction,  // G4SubtractionSolid  
    Intersection  // G4IntersectionSolid
};

/**
 * Parametri per Box
 */
struct BoxParams {
    double x = 10.0; // half-lengths in mm
    double y = 10.0;
    double z = 10.0;
};

/**
 * Parametri per Tube
 */
struct TubeParams {
    double rmin = 0.0; // inner radius in mm
    double rmax = 10.0; // outer radius in mm
    double dz = 10.0;   // half-height in mm
    double sphi = 0.0;  // start phi in degrees
    double dphi = 360.0; // delta phi in degrees
};

/**
 * Parametri per Sphere
 */
struct SphereParams {
    double rmin = 0.0;
    double rmax = 10.0;
    double sphi = 0.0;
    double dphi = 360.0;
    double stheta = 0.0;
    double dtheta = 180.0;
};

/**
 * Parametri per Cone
 */
struct ConeParams {
    double rmin1 = 0.0; // inner radius at -z
    double rmax1 = 5.0;  // outer radius at -z
    double rmin2 = 0.0; // inner radius at +z
    double rmax2 = 10.0; // outer radius at +z
    double dz = 10.0;    // half-height
    double sphi = 0.0;
    double dphi = 360.0;
};

/**
 * Parametri per Trd
 */
struct TrdParams {
    double dx1 = 10.0; // half-length x at -z
    double dx2 = 5.0;  // half-length x at +z
    double dy1 = 10.0; // half-length y at -z
    double dy2 = 5.0;  // half-length y at +z
    double dz = 10.0;  // half-height z
};

/**
 * Parametri per Polycone
 * Sezioni coniche multiple lungo l'asse Z
 */
struct PolyconeParams {
    double sphi = 0.0;  // start phi in degrees
    double dphi = 360.0; // delta phi in degrees
    std::vector<double> zPlanes;    // z positions (mm) - half-heights
    std::vector<double> rmin;      // inner radius at each z plane (mm)
    std::vector<double> rmax;      // outer radius at each z plane (mm)
};

/**
 * Parametri per Polyhedra
 * Come Polycone ma con facce poligonali invece di circolari
 */
struct PolyhedraParams {
    int numSides = 6;   // number of sides (e.g., 6 = hexagon)
    double sphi = 0.0;  // start phi in degrees
    double dphi = 360.0; // delta phi in degrees
    std::vector<double> zPlanes;    // z positions (mm) - half-heights
    std::vector<double> rmin;      // inner radius at each z plane (mm)
    std::vector<double> rmax;      // outer radius at each z plane (mm)
};

/**
 * Parametri per BooleanSolid
 * Rappresenta G4UnionSolid, G4SubtractionSolid, G4IntersectionSolid
 * Le due shape vengono combinate con l'operazione specificata
 */
struct BooleanParams {
    BooleanOperation operation = BooleanOperation::Union;
    
    // Operand shapes (stored as unique_ptr in the actual Shape objects)
    // Here we store references by name/ID for serialization
    std::string solidA_name;  // First operand (base solid)
    std::string solidB_name;  // Second operand (tool solid)
    
    // Relative transform of solidB with respect to solidA
    // Position (translation) in mm
    double relPosX = 0.0;
    double relPosY = 0.0;
    double relPosZ = 0.0;
    
    // Rotation (Euler angles) in degrees
    double relRotX = 0.0;
    double relRotY = 0.0;
    double relRotZ = 0.0;
};

// Variant per i parametri shape
using ShapeParams = std::variant<BoxParams, TubeParams, SphereParams, ConeParams, TrdParams, PolyconeParams, PolyhedraParams, BooleanParams>;

/**
 * Classe base astratta per le shape geometriche.
 * MVP: solo Box implementato, altri aggiunti progressivamente.
 */
class Shape {
public:
    virtual ~Shape() = default;

    ShapeType getType() const { return type_; }
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    // Accesso ai parametri (pattern visitor-like)
    template<typename T>
    T* getParamsAs() {
        return std::get_if<T>(&params_);
    }

    template<typename T>
    const T* getParamsAs() const {
        return std::get_if<T>(&params_);
    }

    ShapeParams& getParams() { return params_; }
    const ShapeParams& getParams() const { return params_; }

    // Serialization
    virtual nlohmann::json toJson() const;
    static std::unique_ptr<Shape> fromJson(const nlohmann::json& j);

    // Constructor (public, but prefer using factory functions)
    Shape(ShapeType type, const std::string& name, ShapeParams params);

private:
    ShapeType type_;
    std::string name_;
    ShapeParams params_;
};

// Factory functions
std::unique_ptr<Shape> makeBox(double x, double y, double z);
std::unique_ptr<Shape> makeTube(double rmin, double rmax, double dz, double sphi = 0.0, double dphi = 360.0);
std::unique_ptr<Shape> makeSphere(double rmin, double rmax, double sphi = 0.0, double dphi = 360.0, double stheta = 0.0, double dtheta = 180.0);
std::unique_ptr<Shape> makeCone(double rmin1, double rmax1, double rmin2, double rmax2, double dz, double sphi = 0.0, double dphi = 360.0);
std::unique_ptr<Shape> makeTrd(double dx1, double dx2, double dy1, double dy2, double dz);
std::unique_ptr<Shape> makePolycone(double sphi, double dphi, const std::vector<double>& zPlanes, const std::vector<double>& rmin, const std::vector<double>& rmax);
std::unique_ptr<Shape> makePolyhedra(int numSides, double sphi, double dphi, const std::vector<double>& zPlanes, const std::vector<double>& rmin, const std::vector<double>& rmax);

// Boolean solid factory
std::unique_ptr<Shape> makeBooleanSolid(
    BooleanOperation operation,
    const std::string& solidA_name,
    const std::string& solidB_name,
    double relPosX = 0.0, double relPosY = 0.0, double relPosZ = 0.0,
    double relRotX = 0.0, double relRotY = 0.0, double relRotZ = 0.0
);

// Helper to get operation name for Geant4 export
inline const char* booleanOperationToG4Class(BooleanOperation op) {
    switch (op) {
        case BooleanOperation::Union: return "G4UnionSolid";
        case BooleanOperation::Subtraction: return "G4SubtractionSolid";
        case BooleanOperation::Intersection: return "G4IntersectionSolid";
    }
    return "G4UnionSolid";
}

inline const char* booleanOperationToString(BooleanOperation op) {
    switch (op) {
        case BooleanOperation::Union: return "union";
        case BooleanOperation::Subtraction: return "subtraction";
        case BooleanOperation::Intersection: return "intersection";
    }
    return "union";
}

} // namespace geantcad

