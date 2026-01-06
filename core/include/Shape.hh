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
    Polyhedra
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

// Variant per i parametri shape
using ShapeParams = std::variant<BoxParams, TubeParams, SphereParams, ConeParams, TrdParams>;

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

} // namespace geantcad

