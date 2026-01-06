/**
 * Python bindings for GeantCAD C++ libraries using pybind11
 * Exposes core and generator functionality to Python for scripting and automation
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <memory>
#include <string>
#include <vector>

// Core includes
#include "../../core/include/SceneGraph.hh"
#include "../../core/include/VolumeNode.hh"
#include "../../core/include/Shape.hh"
#include "../../core/include/Material.hh"
#include "../../core/include/Transform.hh"
#include "../../core/include/Serialization.hh"
#include "../../core/include/PhysicsConfig.hh"
#include "../../core/include/OutputConfig.hh"
#include "../../core/include/ParticleGunConfig.hh"

// Generator includes
#include "../../generator/include/GDMLExporter.hh"
#include "../../generator/include/Geant4ProjectGenerator.hh"

namespace py = pybind11;
using namespace geantcad;

// Helper to convert QVector3D to/from Python tuple
struct Vector3D {
    float x, y, z;
    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(float x, float y, float z) : x(x), y(y), z(z) {}
};

PYBIND11_MODULE(geantcad_python, m) {
    m.doc() = "GeantCAD Python bindings for Geant4 project generation";
    
    // Vector3D helper
    py::class_<Vector3D>(m, "Vector3D")
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def_readwrite("x", &Vector3D::x)
        .def_readwrite("y", &Vector3D::y)
        .def_readwrite("z", &Vector3D::z);
    
    // Shape types enum
    py::enum_<ShapeType>(m, "ShapeType")
        .value("Box", ShapeType::Box)
        .value("Tube", ShapeType::Tube)
        .value("Sphere", ShapeType::Sphere)
        .value("Cone", ShapeType::Cone)
        .value("Trd", ShapeType::Trd)
        .value("Polycone", ShapeType::Polycone)
        .value("Polyhedra", ShapeType::Polyhedra);
    
    // Shape parameter structs
    py::class_<BoxParams>(m, "BoxParams")
        .def(py::init<>())
        .def_readwrite("x", &BoxParams::x)
        .def_readwrite("y", &BoxParams::y)
        .def_readwrite("z", &BoxParams::z);
    
    py::class_<TubeParams>(m, "TubeParams")
        .def(py::init<>())
        .def_readwrite("rmin", &TubeParams::rmin)
        .def_readwrite("rmax", &TubeParams::rmax)
        .def_readwrite("dz", &TubeParams::dz)
        .def_readwrite("sphi", &TubeParams::sphi)
        .def_readwrite("dphi", &TubeParams::dphi);
    
    py::class_<SphereParams>(m, "SphereParams")
        .def(py::init<>())
        .def_readwrite("rmin", &SphereParams::rmin)
        .def_readwrite("rmax", &SphereParams::rmax)
        .def_readwrite("sphi", &SphereParams::sphi)
        .def_readwrite("dphi", &SphereParams::dphi)
        .def_readwrite("stheta", &SphereParams::stheta)
        .def_readwrite("dtheta", &SphereParams::dtheta);
    
    py::class_<ConeParams>(m, "ConeParams")
        .def(py::init<>())
        .def_readwrite("rmin1", &ConeParams::rmin1)
        .def_readwrite("rmax1", &ConeParams::rmax1)
        .def_readwrite("rmin2", &ConeParams::rmin2)
        .def_readwrite("rmax2", &ConeParams::rmax2)
        .def_readwrite("dz", &ConeParams::dz)
        .def_readwrite("sphi", &ConeParams::sphi)
        .def_readwrite("dphi", &ConeParams::dphi);
    
    py::class_<TrdParams>(m, "TrdParams")
        .def(py::init<>())
        .def_readwrite("dx1", &TrdParams::dx1)
        .def_readwrite("dx2", &TrdParams::dx2)
        .def_readwrite("dy1", &TrdParams::dy1)
        .def_readwrite("dy2", &TrdParams::dy2)
        .def_readwrite("dz", &TrdParams::dz);
    
    // Shape class
    py::class_<Shape, std::shared_ptr<Shape>>(m, "Shape")
        .def("getType", &Shape::getType)
        .def("getName", &Shape::getName)
        .def("setName", &Shape::setName)
        .def("getParamsAsBox", [](Shape& s) -> BoxParams* {
            return s.getParamsAs<BoxParams>();
        }, py::return_value_policy::reference_internal)
        .def("getParamsAsTube", [](Shape& s) -> TubeParams* {
            return s.getParamsAs<TubeParams>();
        }, py::return_value_policy::reference_internal)
        .def("getParamsAsSphere", [](Shape& s) -> SphereParams* {
            return s.getParamsAs<SphereParams>();
        }, py::return_value_policy::reference_internal)
        .def("getParamsAsCone", [](Shape& s) -> ConeParams* {
            return s.getParamsAs<ConeParams>();
        }, py::return_value_policy::reference_internal)
        .def("getParamsAsTrd", [](Shape& s) -> TrdParams* {
            return s.getParamsAs<TrdParams>();
        }, py::return_value_policy::reference_internal);
    
    // Shape factory functions - return raw pointers for easier Python usage
    m.def("makeBox", [](double x, double y, double z) {
        return makeBox(x, y, z).release();
    }, py::return_value_policy::take_ownership, "Create a Box shape");
    m.def("makeTube", [](double rmin, double rmax, double dz, double sphi = 0.0, double dphi = 360.0) {
        return makeTube(rmin, rmax, dz, sphi, dphi).release();
    }, py::arg("rmin"), py::arg("rmax"), py::arg("dz"), py::arg("sphi") = 0.0, py::arg("dphi") = 360.0,
       py::return_value_policy::take_ownership, "Create a Tube shape");
    m.def("makeSphere", [](double rmin, double rmax, double sphi = 0.0, double dphi = 360.0, double stheta = 0.0, double dtheta = 180.0) {
        return makeSphere(rmin, rmax, sphi, dphi, stheta, dtheta).release();
    }, py::arg("rmin"), py::arg("rmax"), py::arg("sphi") = 0.0, py::arg("dphi") = 360.0,
       py::arg("stheta") = 0.0, py::arg("dtheta") = 180.0,
       py::return_value_policy::take_ownership, "Create a Sphere shape");
    m.def("makeCone", [](double rmin1, double rmax1, double rmin2, double rmax2, double dz, double sphi = 0.0, double dphi = 360.0) {
        return makeCone(rmin1, rmax1, rmin2, rmax2, dz, sphi, dphi).release();
    }, py::arg("rmin1"), py::arg("rmax1"), py::arg("rmin2"), py::arg("rmax2"), py::arg("dz"),
       py::arg("sphi") = 0.0, py::arg("dphi") = 360.0,
       py::return_value_policy::take_ownership, "Create a Cone shape");
    m.def("makeTrd", [](double dx1, double dx2, double dy1, double dy2, double dz) {
        return makeTrd(dx1, dx2, dy1, dy2, dz).release();
    }, py::return_value_policy::take_ownership, "Create a Trd shape");
    
    // Material class
    py::class_<Material, std::shared_ptr<Material>>(m, "Material")
        .def(py::init<const std::string&, const std::string&>())
        .def("getName", &Material::getName)
        .def("getNistName", &Material::getNistName)
        .def("getDensity", &Material::getDensity)
        .def("setDensity", &Material::setDensity)
        .def("getAtomicNumber", &Material::getAtomicNumber)
        .def("setAtomicNumber", &Material::setAtomicNumber)
        .def("getAtomicMass", &Material::getAtomicMass)
        .def("setAtomicMass", &Material::setAtomicMass)
        .def_static("makeNist", &Material::makeNist, "Create NIST material")
        .def_static("makeAir", &Material::makeAir, "Create Air material")
        .def_static("makeVacuum", &Material::makeVacuum, "Create Vacuum material")
        .def_static("makeWater", &Material::makeWater, "Create Water material")
        .def_static("makeLead", &Material::makeLead, "Create Lead material")
        .def_static("makeSilicon", &Material::makeSilicon, "Create Silicon material");
    
    // SensitiveDetectorConfig
    py::class_<SensitiveDetectorConfig>(m, "SensitiveDetectorConfig")
        .def(py::init<>())
        .def_readwrite("enabled", &SensitiveDetectorConfig::enabled)
        .def_readwrite("type", &SensitiveDetectorConfig::type)
        .def_readwrite("collectionName", &SensitiveDetectorConfig::collectionName)
        .def_readwrite("copyNumber", &SensitiveDetectorConfig::copyNumber);
    
    // OpticalSurfaceConfig
    py::class_<OpticalSurfaceConfig>(m, "OpticalSurfaceConfig")
        .def(py::init<>())
        .def_readwrite("enabled", &OpticalSurfaceConfig::enabled)
        .def_readwrite("model", &OpticalSurfaceConfig::model)
        .def_readwrite("finish", &OpticalSurfaceConfig::finish)
        .def_readwrite("reflectivity", &OpticalSurfaceConfig::reflectivity)
        .def_readwrite("sigmaAlpha", &OpticalSurfaceConfig::sigmaAlpha)
        .def_readwrite("preset", &OpticalSurfaceConfig::preset);
    
    // Transform wrapper (simplified - using tuples for vectors)
    py::class_<Transform>(m, "Transform")
        .def(py::init<>())
        .def("setTranslation", [](Transform& t, float x, float y, float z) {
            t.setTranslation(QVector3D(x, y, z));
        })
        .def("getTranslation", [](Transform& t) {
            auto v = t.getTranslation();
            return Vector3D(v.x(), v.y(), v.z());
        })
        .def("setRotationEuler", &Transform::setRotationEuler)
        .def("setScale", [](Transform& t, float x, float y, float z) {
            t.setScale(QVector3D(x, y, z));
        })
        .def("getScale", [](Transform& t) {
            auto v = t.getScale();
            return Vector3D(v.x(), v.y(), v.z());
        })
        .def_static("identity", &Transform::identity);
    
    // VolumeNode class
    py::class_<VolumeNode>(m, "VolumeNode")
        .def(py::init<const std::string&>())
        .def("getName", &VolumeNode::getName)
        .def("setName", &VolumeNode::setName)
        .def("getId", &VolumeNode::getId)
        .def("getParent", &VolumeNode::getParent, py::return_value_policy::reference_internal)
        .def("getChildren", &VolumeNode::getChildren, py::return_value_policy::reference_internal)
        .def("setParent", &VolumeNode::setParent)
        .def("addChild", &VolumeNode::addChild)
        .def("removeChild", &VolumeNode::removeChild)
        .def("getShape", (Shape*(VolumeNode::*)())&VolumeNode::getShape, py::return_value_policy::reference_internal)
        .def("setShape", [](VolumeNode& v, Shape* shape) {
            // Create unique_ptr from raw pointer (takes ownership)
            if (shape) {
                v.setShape(std::unique_ptr<Shape>(shape));
            }
        }, py::arg("shape"), py::keep_alive<0, 2>()) // Keep shape alive until VolumeNode is destroyed
        .def("getTransform", (Transform&(VolumeNode::*)())&VolumeNode::getTransform, py::return_value_policy::reference_internal)
        .def("getMaterial", &VolumeNode::getMaterial)
        .def("setMaterial", &VolumeNode::setMaterial)
        .def("getSDConfig", (SensitiveDetectorConfig&(VolumeNode::*)())&VolumeNode::getSDConfig, py::return_value_policy::reference_internal)
        .def("getOpticalConfig", (OpticalSurfaceConfig&(VolumeNode::*)())&VolumeNode::getOpticalConfig, py::return_value_policy::reference_internal);
    
    // SceneGraph class
    py::class_<SceneGraph>(m, "SceneGraph")
        .def(py::init<>())
        .def("getRoot", (VolumeNode*(SceneGraph::*)())&SceneGraph::getRoot, py::return_value_policy::reference_internal)
        .def("createVolume", &SceneGraph::createVolume, py::return_value_policy::reference_internal)
        .def("removeVolume", &SceneGraph::removeVolume)
        .def("findVolumeById", &SceneGraph::findVolumeById, py::return_value_policy::reference_internal)
        .def("findVolumeByName", &SceneGraph::findVolumeByName, py::return_value_policy::reference_internal)
        .def("getSelected", &SceneGraph::getSelected, py::return_value_policy::reference_internal)
        .def("setSelected", &SceneGraph::setSelected)
        .def("clearSelection", &SceneGraph::clearSelection)
        .def("getPhysicsConfig", (PhysicsConfig&(SceneGraph::*)())&SceneGraph::getPhysicsConfig, py::return_value_policy::reference_internal)
        .def("getOutputConfig", (OutputConfig&(SceneGraph::*)())&SceneGraph::getOutputConfig, py::return_value_policy::reference_internal)
        .def("getParticleGunConfig", (ParticleGunConfig&(SceneGraph::*)())&SceneGraph::getParticleGunConfig, py::return_value_policy::reference_internal);
    
    // GDMLExporter class
    py::class_<GDMLExporter>(m, "GDMLExporter")
        .def(py::init<>())
        .def("exportToFile", &GDMLExporter::exportToFile, "Export SceneGraph to GDML file");
    
    // Geant4ProjectGenerator class
    py::class_<Geant4ProjectGenerator>(m, "Geant4ProjectGenerator")
        .def(py::init<>())
        .def("setTemplateDir", &Geant4ProjectGenerator::setTemplateDir)
        .def("generateProject", &Geant4ProjectGenerator::generateProject, "Generate Geant4 project");
    
    // Serialization functions
    m.def("saveSceneToFile", &saveSceneToFile, "Save SceneGraph to JSON file");
    m.def("loadSceneFromFile", &loadSceneFromFile, "Load SceneGraph from JSON file");
}

