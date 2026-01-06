#include "GDMLExporter.hh"
#include "../../core/include/SceneGraph.hh"
#include "../../core/include/Shape.hh"
#include "../../core/include/Material.hh"
#include "../../core/include/Transform.hh"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace geantcad {

GDMLExporter::GDMLExporter() {
}

GDMLExporter::~GDMLExporter() {
}

namespace {
    // Helper to generate unique GDML names (sanitize)
    std::string sanitizeName(const std::string& name) {
        std::string result = name;
        // Replace invalid chars with underscore
        for (char& c : result) {
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
                  (c >= '0' && c <= '9') || c == '_')) {
                c = '_';
            }
        }
        // Must start with letter or underscore
        if (!result.empty() && (result[0] >= '0' && result[0] <= '9')) {
            result = "_" + result;
        }
        return result;
    }
    
    // Convert mm to cm (GDML uses cm)
    double mmToCm(double mm) {
        return mm / 10.0;
    }
    
    // Format double for XML
    std::string formatDouble(double value, int precision = 6) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        std::string str = oss.str();
        // Remove trailing zeros
        str.erase(str.find_last_not_of('0') + 1, std::string::npos);
        str.erase(str.find_last_not_of('.') + 1, std::string::npos);
        return str;
    }
    
    // Write transform to GDML (position only for MVP)
    void writeTransform(std::ostream& os, const Transform& transform, const std::string& name) {
        auto pos = transform.getTranslation();
        std::string posName = sanitizeName(name) + "_pos";
        os << "  <position name=\"" << posName << "\" unit=\"cm\" "
           << "x=\"" << formatDouble(mmToCm(pos.x())) << "\" "
           << "y=\"" << formatDouble(mmToCm(pos.y())) << "\" "
           << "z=\"" << formatDouble(mmToCm(pos.z())) << "\"/>\n";
        // TODO: Add rotation if needed
    }
    
    // Write shape to GDML
    void writeShape(std::ostream& os, const Shape* shape, const std::string& name) {
        if (!shape) return;
        
        std::string shapeName = sanitizeName(name) + "_shape";
        
        switch (shape->getType()) {
            case ShapeType::Box: {
                if (auto* params = shape->getParamsAs<BoxParams>()) {
                    os << "    <box name=\"" << shapeName << "\" "
                       << "x=\"" << formatDouble(mmToCm(params->x * 2.0)) << "\" "
                       << "y=\"" << formatDouble(mmToCm(params->y * 2.0)) << "\" "
                       << "z=\"" << formatDouble(mmToCm(params->z * 2.0)) << "\" "
                       << "lunit=\"cm\"/>\n";
                }
                break;
            }
            case ShapeType::Tube: {
                if (auto* params = shape->getParamsAs<TubeParams>()) {
                    os << "    <tube name=\"" << shapeName << "\" "
                       << "rmin=\"" << formatDouble(mmToCm(params->rmin)) << "\" "
                       << "rmax=\"" << formatDouble(mmToCm(params->rmax)) << "\" "
                       << "z=\"" << formatDouble(mmToCm(params->dz * 2.0)) << "\" "
                       << "startphi=\"" << formatDouble(params->sphi) << "\" "
                       << "deltaphi=\"" << formatDouble(params->dphi) << "\" "
                       << "aunit=\"deg\" lunit=\"cm\"/>\n";
                }
                break;
            }
            case ShapeType::Sphere: {
                if (auto* params = shape->getParamsAs<SphereParams>()) {
                    os << "    <sphere name=\"" << shapeName << "\" "
                       << "rmin=\"" << formatDouble(mmToCm(params->rmin)) << "\" "
                       << "rmax=\"" << formatDouble(mmToCm(params->rmax)) << "\" "
                       << "startphi=\"" << formatDouble(params->sphi) << "\" "
                       << "deltaphi=\"" << formatDouble(params->dphi) << "\" "
                       << "starttheta=\"" << formatDouble(params->stheta) << "\" "
                       << "deltatheta=\"" << formatDouble(params->dtheta) << "\" "
                       << "aunit=\"deg\" lunit=\"cm\"/>\n";
                }
                break;
            }
            case ShapeType::Cone: {
                if (auto* params = shape->getParamsAs<ConeParams>()) {
                    os << "    <cone name=\"" << shapeName << "\" "
                       << "rmin1=\"" << formatDouble(mmToCm(params->rmin1)) << "\" "
                       << "rmax1=\"" << formatDouble(mmToCm(params->rmax1)) << "\" "
                       << "rmin2=\"" << formatDouble(mmToCm(params->rmin2)) << "\" "
                       << "rmax2=\"" << formatDouble(mmToCm(params->rmax2)) << "\" "
                       << "z=\"" << formatDouble(mmToCm(params->dz * 2.0)) << "\" "
                       << "startphi=\"" << formatDouble(params->sphi) << "\" "
                       << "deltaphi=\"" << formatDouble(params->dphi) << "\" "
                       << "aunit=\"deg\" lunit=\"cm\"/>\n";
                }
                break;
            }
            case ShapeType::Trd: {
                if (auto* params = shape->getParamsAs<TrdParams>()) {
                    os << "    <trd name=\"" << shapeName << "\" "
                       << "x1=\"" << formatDouble(mmToCm(params->dx1 * 2.0)) << "\" "
                       << "x2=\"" << formatDouble(mmToCm(params->dx2 * 2.0)) << "\" "
                       << "y1=\"" << formatDouble(mmToCm(params->dy1 * 2.0)) << "\" "
                       << "y2=\"" << formatDouble(mmToCm(params->dy2 * 2.0)) << "\" "
                       << "z=\"" << formatDouble(mmToCm(params->dz * 2.0)) << "\" "
                       << "lunit=\"cm\"/>\n";
                }
                break;
            }
            case ShapeType::Polycone: {
                if (auto* params = shape->getParamsAs<PolyconeParams>()) {
                    os << "    <polycone name=\"" << shapeName << "\" "
                       << "startphi=\"" << formatDouble(params->sphi) << "\" "
                       << "deltaphi=\"" << formatDouble(params->dphi) << "\" "
                       << "aunit=\"deg\" lunit=\"cm\">\n";
                    
                    // Write zplanes
                    os << "      <zplane z=\"" << formatDouble(mmToCm(params->zPlanes[0])) << "\" "
                       << "rmin=\"" << formatDouble(mmToCm(params->rmin[0])) << "\" "
                       << "rmax=\"" << formatDouble(mmToCm(params->rmax[0])) << "\"/>\n";
                    
                    for (size_t i = 1; i < params->zPlanes.size(); ++i) {
                        os << "      <zplane z=\"" << formatDouble(mmToCm(params->zPlanes[i])) << "\" "
                           << "rmin=\"" << formatDouble(mmToCm(params->rmin[i])) << "\" "
                           << "rmax=\"" << formatDouble(mmToCm(params->rmax[i])) << "\"/>\n";
                    }
                    
                    os << "    </polycone>\n";
                }
                break;
            }
            case ShapeType::Polyhedra: {
                if (auto* params = shape->getParamsAs<PolyhedraParams>()) {
                    os << "    <polyhedra name=\"" << shapeName << "\" "
                       << "numsides=\"" << params->numSides << "\" "
                       << "startphi=\"" << formatDouble(params->sphi) << "\" "
                       << "deltaphi=\"" << formatDouble(params->dphi) << "\" "
                       << "aunit=\"deg\" lunit=\"cm\">\n";
                    
                    // Write zplanes
                    os << "      <zplane z=\"" << formatDouble(mmToCm(params->zPlanes[0])) << "\" "
                       << "rmin=\"" << formatDouble(mmToCm(params->rmin[0])) << "\" "
                       << "rmax=\"" << formatDouble(mmToCm(params->rmax[0])) << "\"/>\n";
                    
                    for (size_t i = 1; i < params->zPlanes.size(); ++i) {
                        os << "      <zplane z=\"" << formatDouble(mmToCm(params->zPlanes[i])) << "\" "
                           << "rmin=\"" << formatDouble(mmToCm(params->rmin[i])) << "\" "
                           << "rmax=\"" << formatDouble(mmToCm(params->rmax[i])) << "\"/>\n";
                    }
                    
                    os << "    </polyhedra>\n";
                }
                break;
            }
            default:
                break;
        }
    }
    
    // Write material reference
    std::string getMaterialRef(const Material* material) {
        if (!material) return "G4_AIR";
        
        std::string nistName = material->getNistName();
        if (!nistName.empty()) {
            return nistName;
        }
        
        // Default to AIR if no NIST name
        return "G4_AIR";
    }
    
    // Write optical surface definition
    void writeOpticalSurface(std::ostream& os, const OpticalSurfaceConfig& config, const std::string& name) {
        std::string surfName = sanitizeName(name) + "_optical_surface";
        
        os << "  <opticalsurface name=\"" << surfName << "\" ";
        os << "model=\"" << config.model << "\" ";
        os << "finish=\"" << config.finish << "\" ";
        os << "type=\"dielectric_metal\" ";
        os << "value=\"" << formatDouble(config.reflectivity) << "\"";
        if (config.sigmaAlpha > 0.0) {
            os << " sigmaalpha=\"" << formatDouble(config.sigmaAlpha) << "\"";
        }
        os << "/>\n";
    }
    
    // Recursive volume export
    void exportVolume(std::ostream& os, VolumeNode* node, int indent = 1) {
        if (!node) return;
        
        std::string indentStr(indent * 2, ' ');
        std::string volName = sanitizeName(node->getName());
        
        // Write shape
        if (node->getShape()) {
            writeShape(os, node->getShape(), volName);
        }
        
        // Write volume
        os << indentStr << "<volume name=\"" << volName << "\">\n";
        
        // Material reference
        std::string matRef = getMaterialRef(node->getMaterial().get());
        os << indentStr << "  <materialref ref=\"" << matRef << "\"/>\n";
        
        // Solid reference
        std::string shapeName = sanitizeName(node->getName()) + "_shape";
        os << indentStr << "  <solidref ref=\"" << shapeName << "\"/>\n";
        
        // Write children (physvols)
        for (auto* child : node->getChildren()) {
            std::string childName = sanitizeName(child->getName());
            std::string posName = childName + "_pos";
            
            // Write position
            writeTransform(os, child->getTransform(), childName);
            
            os << indentStr << "  <physvol>\n";
            os << indentStr << "    <volumeref ref=\"" << childName << "\"/>\n";
            os << indentStr << "    <positionref ref=\"" << posName << "_pos\"/>\n";
            os << indentStr << "  </physvol>\n";
        }
        
        os << indentStr << "</volume>\n";
        
        // Recursively export children
        for (auto* child : node->getChildren()) {
            exportVolume(os, child, indent + 1);
        }
    }
    
    // Export optical surfaces (called after volumes)
    void exportOpticalSurfaces(std::ostream& os, VolumeNode* node) {
        if (!node) return;
        
        const auto& opticalConfig = node->getOpticalConfig();
        if (opticalConfig.enabled) {
            std::string volName = sanitizeName(node->getName());
            std::string surfName = volName + "_optical_surface";
            
            // Write optical surface definition (in <define> section)
            writeOpticalSurface(os, opticalConfig, volName);
            
            // Write skin surface (in <setup> section, will be handled separately)
            // For now, we'll add it to a list to write later
        }
        
        // Recursively process children
        for (auto* child : node->getChildren()) {
            exportOpticalSurfaces(os, child);
        }
    }
}

bool GDMLExporter::exportToFile(SceneGraph* sceneGraph, const std::string& filePath) {
    if (!sceneGraph) return false;
    
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        // Write GDML header
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<gdml xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
             << "xsi:noNamespaceSchemaLocation=\"http://service-spi.web.cern.ch/service-spi/app/releases/GDML/schema/gdml.xsd\">\n";
        
        file << "<define>\n";
        file << "  <position name=\"world_pos\" unit=\"cm\" x=\"0\" y=\"0\" z=\"0\"/>\n";
        
        // Export optical surface definitions
        VolumeNode* root = sceneGraph->getRoot();
        if (root) {
            std::function<void(VolumeNode*)> writeOpticalSurfaces = [&](VolumeNode* node) {
                const auto& opticalConfig = node->getOpticalConfig();
                if (opticalConfig.enabled) {
                    writeOpticalSurface(file, opticalConfig, sanitizeName(node->getName()));
                }
                for (auto* child : node->getChildren()) {
                    writeOpticalSurfaces(child);
                }
            };
            writeOpticalSurfaces(root);
        }
        
        file << "<solids>\n";
        // Shapes will be written by exportVolume
        file << "</solids>\n";
        
        file << "<structure>\n";
        
        // Export volumes (starting from root's children, not root itself)
        // Reuse root variable from above
        root = sceneGraph->getRoot();
        if (root) {
            // Write shapes first
            file << "<solids>\n";
            std::function<void(VolumeNode*)> writeShapes = [&](VolumeNode* node) {
                if (node->getShape()) {
                    writeShape(file, node->getShape(), sanitizeName(node->getName()));
                }
                for (auto* child : node->getChildren()) {
                    writeShapes(child);
                }
            };
            writeShapes(root);
            file << "</solids>\n";
            
            // Write positions
            file << "<define>\n";
            std::function<void(VolumeNode*)> writePositions = [&](VolumeNode* node) {
                for (auto* child : node->getChildren()) {
                    writeTransform(file, child->getTransform(), sanitizeName(child->getName()));
                    writePositions(child);
                }
            };
            writePositions(root);
            file << "</define>\n";
            
            // Write volumes
            for (auto* child : root->getChildren()) {
                exportVolume(file, child, 1);
            }
            
            // World volume
            file << "  <volume name=\"world\">\n";
            file << "    <materialref ref=\"G4_Galactic\"/>\n";
            file << "    <solidref ref=\"world_shape\"/>\n";
            for (auto* child : root->getChildren()) {
                std::string childName = sanitizeName(child->getName());
                file << "    <physvol>\n";
                file << "      <volumeref ref=\"" << childName << "\"/>\n";
                file << "      <positionref ref=\"" << childName << "_pos\"/>\n";
                file << "    </physvol>\n";
            }
            file << "  </volume>\n";
        }
        
        file << "</structure>\n";
        
        // Write setup section with skin surfaces
        file << "<setup name=\"Default\" version=\"1.0\">\n";
        file << "  <world ref=\"world\"/>\n";
        
        // Export skin surfaces for volumes with optical surfaces
        if (root) {
            std::function<void(VolumeNode*)> writeSkinSurfaces = [&](VolumeNode* node) {
                const auto& opticalConfig = node->getOpticalConfig();
                if (opticalConfig.enabled) {
                    std::string volName = sanitizeName(node->getName());
                    std::string surfName = volName + "_optical_surface";
                    file << "  <skinsurface name=\"" << volName << "_skin\" surface=\"" 
                         << surfName << "\">\n";
                    file << "    <volumeref ref=\"" << volName << "\"/>\n";
                    file << "  </skinsurface>\n";
                }
                for (auto* child : node->getChildren()) {
                    writeSkinSurfaces(child);
                }
            };
            writeSkinSurfaces(root);
        }
        
        file << "</setup>\n";
        file << "</gdml>\n";
        
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace geantcad
