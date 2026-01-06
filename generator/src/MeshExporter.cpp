#include "MeshExporter.hh"
#include "../../core/include/Shape.hh"
#include "../../core/include/VolumeNode.hh"
#include "../../core/include/Transform.hh"

#ifndef GEANTCAD_NO_VTK
#include <vtkSmartPointer.h>
#include <vtkAppendPolyData.h>
#include <vtkSTLWriter.h>
#include <vtkOBJWriter.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

// VTK sources for shapes
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkSphereSource.h>
#include <vtkConeSource.h>
#endif

#include <fstream>
#include <sstream>

namespace geantcad {

MeshExporter::MeshExporter() {
}

MeshExporter::~MeshExporter() {
}

#ifndef GEANTCAD_NO_VTK
namespace {
    // Helper to create VTK source from Shape
    vtkSmartPointer<vtkPolyData> createPolyDataFromShape(const Shape* shape) {
        if (!shape) return nullptr;
        
        vtkSmartPointer<vtkPolyData> polyData;
        
        switch (shape->getType()) {
            case ShapeType::Box: {
                auto* params = shape->getParamsAs<BoxParams>();
                if (!params) return nullptr;
                
                auto source = vtkSmartPointer<vtkCubeSource>::New();
                source->SetXLength(params->x * 2.0);
                source->SetYLength(params->y * 2.0);
                source->SetZLength(params->z * 2.0);
                source->Update();
                polyData = source->GetOutput();
                break;
            }
            case ShapeType::Tube: {
                auto* params = shape->getParamsAs<TubeParams>();
                if (!params) return nullptr;
                
                // VTK cylinder is oriented along Y, we want Z
                auto source = vtkSmartPointer<vtkCylinderSource>::New();
                source->SetRadius(params->rmax);
                source->SetHeight(params->dz * 2.0);
                source->SetResolution(36);
                source->Update();
                
                // Rotate to align with Z axis
                auto transform = vtkSmartPointer<vtkTransform>::New();
                transform->RotateX(90);
                
                auto filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
                filter->SetInputData(source->GetOutput());
                filter->SetTransform(transform);
                filter->Update();
                polyData = filter->GetOutput();
                break;
            }
            case ShapeType::Sphere: {
                auto* params = shape->getParamsAs<SphereParams>();
                if (!params) return nullptr;
                
                auto source = vtkSmartPointer<vtkSphereSource>::New();
                source->SetRadius(params->rmax);
                source->SetPhiResolution(32);
                source->SetThetaResolution(32);
                source->Update();
                polyData = source->GetOutput();
                break;
            }
            case ShapeType::Cone: {
                auto* params = shape->getParamsAs<ConeParams>();
                if (!params) return nullptr;
                
                auto source = vtkSmartPointer<vtkConeSource>::New();
                source->SetRadius(params->rmax1);
                source->SetHeight(params->dz * 2.0);
                source->SetResolution(36);
                source->Update();
                polyData = source->GetOutput();
                break;
            }
            case ShapeType::Trd: {
                // Trapezoid - approximate with cube for now (would need custom mesh)
                auto* params = shape->getParamsAs<TrdParams>();
                if (!params) return nullptr;
                
                auto source = vtkSmartPointer<vtkCubeSource>::New();
                source->SetXLength((params->dx1 + params->dx2));
                source->SetYLength((params->dy1 + params->dy2));
                source->SetZLength(params->dz * 2.0);
                source->Update();
                polyData = source->GetOutput();
                break;
            }
            default:
                return nullptr;
        }
        
        return polyData;
    }
    
    // Transform polydata with the node's world transform
    vtkSmartPointer<vtkPolyData> transformPolyData(vtkPolyData* input, const Transform& transform) {
        if (!input) return nullptr;
        
        auto matrix = transform.getMatrix();
        
        auto vtkMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        const float* data = matrix.constData();
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                vtkMatrix->SetElement(i, j, data[i * 4 + j]);
            }
        }
        
        auto vtkXform = vtkSmartPointer<vtkTransform>::New();
        vtkXform->SetMatrix(vtkMatrix);
        
        auto filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        filter->SetInputData(input);
        filter->SetTransform(vtkXform);
        filter->Update();
        
        return filter->GetOutput();
    }
}
#endif

bool MeshExporter::exportToFile(SceneGraph* sceneGraph, const std::string& filePath, Format format) {
    switch (format) {
        case Format::STL:
            return exportToSTL(sceneGraph, filePath);
        case Format::OBJ:
            return exportToOBJ(sceneGraph, filePath);
        default:
            lastError_ = "Unknown export format";
            return false;
    }
}

bool MeshExporter::exportToSTL(SceneGraph* sceneGraph, const std::string& filePath) {
#ifdef GEANTCAD_NO_VTK
    lastError_ = "VTK support required for STL export";
    return false;
#else
    if (!sceneGraph) {
        lastError_ = "No scene graph provided";
        return false;
    }
    
    try {
        auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
        bool hasData = false;
        
        // Traverse scene graph and combine all meshes
        sceneGraph->traverse([&](VolumeNode* node) {
            if (!node || !node->getShape()) return;
            if (node->getName() == "World") return;
            if (!node->isVisible()) return;
            
            auto polyData = createPolyDataFromShape(node->getShape());
            if (!polyData) return;
            
            // Apply world transform
            auto transformed = transformPolyData(polyData, node->getWorldTransform());
            if (transformed) {
                appendFilter->AddInputData(transformed);
                hasData = true;
            }
        });
        
        if (!hasData) {
            lastError_ = "No exportable geometry found";
            return false;
        }
        
        appendFilter->Update();
        
        auto writer = vtkSmartPointer<vtkSTLWriter>::New();
        writer->SetFileName(filePath.c_str());
        writer->SetInputData(appendFilter->GetOutput());
        writer->Write();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Export failed: ") + e.what();
        return false;
    }
#endif
}

bool MeshExporter::exportToOBJ(SceneGraph* sceneGraph, const std::string& filePath) {
#ifdef GEANTCAD_NO_VTK
    lastError_ = "VTK support required for OBJ export";
    return false;
#else
    if (!sceneGraph) {
        lastError_ = "No scene graph provided";
        return false;
    }
    
    try {
        auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
        bool hasData = false;
        
        // Traverse scene graph and combine all meshes
        sceneGraph->traverse([&](VolumeNode* node) {
            if (!node || !node->getShape()) return;
            if (node->getName() == "World") return;
            if (!node->isVisible()) return;
            
            auto polyData = createPolyDataFromShape(node->getShape());
            if (!polyData) return;
            
            // Apply world transform
            auto transformed = transformPolyData(polyData, node->getWorldTransform());
            if (transformed) {
                appendFilter->AddInputData(transformed);
                hasData = true;
            }
        });
        
        if (!hasData) {
            lastError_ = "No exportable geometry found";
            return false;
        }
        
        appendFilter->Update();
        
        auto writer = vtkSmartPointer<vtkOBJWriter>::New();
        writer->SetFileName(filePath.c_str());
        writer->SetInputData(appendFilter->GetOutput());
        writer->Write();
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Export failed: ") + e.what();
        return false;
    }
#endif
}

} // namespace geantcad

