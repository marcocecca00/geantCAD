#include "Shape.hh"
#include <stdexcept>

namespace geantcad {

Shape::Shape(ShapeType type, const std::string& name, ShapeParams params)
    : type_(type)
    , name_(name)
    , params_(params)
{
}

nlohmann::json Shape::toJson() const {
    nlohmann::json j;
    j["type"] = static_cast<int>(type_);
    j["name"] = name_;
    
    // Serialize params based on type
    switch (type_) {
        case ShapeType::Box: {
            if (auto* p = std::get_if<BoxParams>(&params_)) {
                j["params"] = {{"x", p->x}, {"y", p->y}, {"z", p->z}};
            }
            break;
        }
        case ShapeType::Tube: {
            if (auto* p = std::get_if<TubeParams>(&params_)) {
                j["params"] = {
                    {"rmin", p->rmin}, {"rmax", p->rmax}, {"dz", p->dz},
                    {"sphi", p->sphi}, {"dphi", p->dphi}
                };
            }
            break;
        }
        case ShapeType::Sphere: {
            if (auto* p = std::get_if<SphereParams>(&params_)) {
                j["params"] = {
                    {"rmin", p->rmin}, {"rmax", p->rmax},
                    {"sphi", p->sphi}, {"dphi", p->dphi},
                    {"stheta", p->stheta}, {"dtheta", p->dtheta}
                };
            }
            break;
        }
        case ShapeType::Cone: {
            if (auto* p = std::get_if<ConeParams>(&params_)) {
                j["params"] = {
                    {"rmin1", p->rmin1}, {"rmax1", p->rmax1},
                    {"rmin2", p->rmin2}, {"rmax2", p->rmax2},
                    {"dz", p->dz}, {"sphi", p->sphi}, {"dphi", p->dphi}
                };
            }
            break;
        }
        case ShapeType::Trd: {
            if (auto* p = std::get_if<TrdParams>(&params_)) {
                j["params"] = {
                    {"dx1", p->dx1}, {"dx2", p->dx2},
                    {"dy1", p->dy1}, {"dy2", p->dy2}, {"dz", p->dz}
                };
            }
            break;
        }
        case ShapeType::Polycone: {
            if (auto* p = std::get_if<PolyconeParams>(&params_)) {
                j["params"] = {
                    {"sphi", p->sphi}, {"dphi", p->dphi},
                    {"zPlanes", p->zPlanes},
                    {"rmin", p->rmin},
                    {"rmax", p->rmax}
                };
            }
            break;
        }
        case ShapeType::Polyhedra: {
            if (auto* p = std::get_if<PolyhedraParams>(&params_)) {
                j["params"] = {
                    {"numSides", p->numSides},
                    {"sphi", p->sphi}, {"dphi", p->dphi},
                    {"zPlanes", p->zPlanes},
                    {"rmin", p->rmin},
                    {"rmax", p->rmax}
                };
            }
            break;
        }
        case ShapeType::BooleanSolid: {
            if (auto* p = std::get_if<BooleanParams>(&params_)) {
                j["params"] = {
                    {"operation", static_cast<int>(p->operation)},
                    {"solidA_name", p->solidA_name},
                    {"solidB_name", p->solidB_name},
                    {"relPosX", p->relPosX},
                    {"relPosY", p->relPosY},
                    {"relPosZ", p->relPosZ},
                    {"relRotX", p->relRotX},
                    {"relRotY", p->relRotY},
                    {"relRotZ", p->relRotZ}
                };
            }
            break;
        }
        default:
            break;
    }
    
    return j;
}

std::unique_ptr<Shape> Shape::fromJson(const nlohmann::json& j) {
    if (!j.contains("type") || !j.contains("name")) {
        throw std::runtime_error("Invalid shape JSON: missing type or name");
    }
    
    ShapeType type = static_cast<ShapeType>(j["type"]);
    std::string name = j["name"];
    
    switch (type) {
        case ShapeType::Box: {
            auto p = j["params"];
            return makeBox(p["x"], p["y"], p["z"]);
        }
        case ShapeType::Tube: {
            auto p = j["params"];
            return makeTube(p["rmin"], p["rmax"], p["dz"], 
                          p.value("sphi", 0.0), p.value("dphi", 360.0));
        }
        case ShapeType::Sphere: {
            auto p = j["params"];
            return makeSphere(p["rmin"], p["rmax"],
                            p.value("sphi", 0.0), p.value("dphi", 360.0),
                            p.value("stheta", 0.0), p.value("dtheta", 180.0));
        }
        case ShapeType::Cone: {
            auto p = j["params"];
            return makeCone(p["rmin1"], p["rmax1"], p["rmin2"], p["rmax2"], p["dz"],
                          p.value("sphi", 0.0), p.value("dphi", 360.0));
        }
        case ShapeType::Trd: {
            auto p = j["params"];
            return makeTrd(p["dx1"], p["dx2"], p["dy1"], p["dy2"], p["dz"]);
        }
        case ShapeType::Polycone: {
            auto p = j["params"];
            return makePolycone(
                p.value("sphi", 0.0),
                p.value("dphi", 360.0),
                p["zPlanes"].get<std::vector<double>>(),
                p["rmin"].get<std::vector<double>>(),
                p["rmax"].get<std::vector<double>>()
            );
        }
        case ShapeType::Polyhedra: {
            auto p = j["params"];
            return makePolyhedra(
                p.value("numSides", 6),
                p.value("sphi", 0.0),
                p.value("dphi", 360.0),
                p["zPlanes"].get<std::vector<double>>(),
                p["rmin"].get<std::vector<double>>(),
                p["rmax"].get<std::vector<double>>()
            );
        }
        case ShapeType::BooleanSolid: {
            auto p = j["params"];
            return makeBooleanSolid(
                static_cast<BooleanOperation>(p.value("operation", 0)),
                p["solidA_name"],
                p["solidB_name"],
                p.value("relPosX", 0.0),
                p.value("relPosY", 0.0),
                p.value("relPosZ", 0.0),
                p.value("relRotX", 0.0),
                p.value("relRotY", 0.0),
                p.value("relRotZ", 0.0)
            );
        }
        default:
            throw std::runtime_error("Unsupported shape type in JSON");
    }
}

// Factory functions
std::unique_ptr<Shape> makeBox(double x, double y, double z) {
    BoxParams params;
    params.x = x;
    params.y = y;
    params.z = z;
    return std::unique_ptr<Shape>(new Shape(ShapeType::Box, "Box", params));
}

std::unique_ptr<Shape> makeTube(double rmin, double rmax, double dz, double sphi, double dphi) {
    TubeParams params;
    params.rmin = rmin;
    params.rmax = rmax;
    params.dz = dz;
    params.sphi = sphi;
    params.dphi = dphi;
    return std::unique_ptr<Shape>(new Shape(ShapeType::Tube, "Tube", params));
}

std::unique_ptr<Shape> makeSphere(double rmin, double rmax, double sphi, double dphi, double stheta, double dtheta) {
    SphereParams params;
    params.rmin = rmin;
    params.rmax = rmax;
    params.sphi = sphi;
    params.dphi = dphi;
    params.stheta = stheta;
    params.dtheta = dtheta;
    return std::unique_ptr<Shape>(new Shape(ShapeType::Sphere, "Sphere", params));
}

std::unique_ptr<Shape> makeCone(double rmin1, double rmax1, double rmin2, double rmax2, double dz, double sphi, double dphi) {
    ConeParams params;
    params.rmin1 = rmin1;
    params.rmax1 = rmax1;
    params.rmin2 = rmin2;
    params.rmax2 = rmax2;
    params.dz = dz;
    params.sphi = sphi;
    params.dphi = dphi;
    return std::unique_ptr<Shape>(new Shape(ShapeType::Cone, "Cone", params));
}

std::unique_ptr<Shape> makeTrd(double dx1, double dx2, double dy1, double dy2, double dz) {
    TrdParams params;
    params.dx1 = dx1;
    params.dx2 = dx2;
    params.dy1 = dy1;
    params.dy2 = dy2;
    params.dz = dz;
    return std::unique_ptr<Shape>(new Shape(ShapeType::Trd, "Trd", params));
}

std::unique_ptr<Shape> makePolycone(double sphi, double dphi, const std::vector<double>& zPlanes, const std::vector<double>& rmin, const std::vector<double>& rmax) {
    if (zPlanes.size() != rmin.size() || zPlanes.size() != rmax.size()) {
        throw std::runtime_error("Polycone: zPlanes, rmin, and rmax must have the same size");
    }
    if (zPlanes.size() < 2) {
        throw std::runtime_error("Polycone: at least 2 z planes required");
    }
    
    PolyconeParams params;
    params.sphi = sphi;
    params.dphi = dphi;
    params.zPlanes = zPlanes;
    params.rmin = rmin;
    params.rmax = rmax;
    return std::unique_ptr<Shape>(new Shape(ShapeType::Polycone, "Polycone", params));
}

std::unique_ptr<Shape> makePolyhedra(int numSides, double sphi, double dphi, const std::vector<double>& zPlanes, const std::vector<double>& rmin, const std::vector<double>& rmax) {
    if (numSides < 3) {
        throw std::runtime_error("Polyhedra: numSides must be at least 3");
    }
    if (zPlanes.size() != rmin.size() || zPlanes.size() != rmax.size()) {
        throw std::runtime_error("Polyhedra: zPlanes, rmin, and rmax must have the same size");
    }
    if (zPlanes.size() < 2) {
        throw std::runtime_error("Polyhedra: at least 2 z planes required");
    }
    
    PolyhedraParams params;
    params.numSides = numSides;
    params.sphi = sphi;
    params.dphi = dphi;
    params.zPlanes = zPlanes;
    params.rmin = rmin;
    params.rmax = rmax;
    return std::unique_ptr<Shape>(new Shape(ShapeType::Polyhedra, "Polyhedra", params));
}

std::unique_ptr<Shape> makeBooleanSolid(
    BooleanOperation operation,
    const std::string& solidA_name,
    const std::string& solidB_name,
    double relPosX, double relPosY, double relPosZ,
    double relRotX, double relRotY, double relRotZ)
{
    BooleanParams params;
    params.operation = operation;
    params.solidA_name = solidA_name;
    params.solidB_name = solidB_name;
    params.relPosX = relPosX;
    params.relPosY = relPosY;
    params.relPosZ = relPosZ;
    params.relRotX = relRotX;
    params.relRotY = relRotY;
    params.relRotZ = relRotZ;
    
    std::string name = booleanOperationToString(operation);
    name += "_" + solidA_name + "_" + solidB_name;
    
    return std::unique_ptr<Shape>(new Shape(ShapeType::BooleanSolid, name, params));
}

} // namespace geantcad

