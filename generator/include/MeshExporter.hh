#pragma once

#include "../../core/include/SceneGraph.hh"
#include <string>

namespace geantcad {

/**
 * MeshExporter exports the scene graph to mesh formats (STL, OBJ)
 * Uses VTK for mesh generation and export
 */
class MeshExporter {
public:
    enum class Format {
        STL,
        OBJ
    };
    
    MeshExporter();
    ~MeshExporter();
    
    /**
     * Export scene to a mesh file
     * @param sceneGraph The scene graph to export
     * @param filePath Output file path
     * @param format Export format (STL or OBJ)
     * @return true on success, false on failure
     */
    bool exportToFile(SceneGraph* sceneGraph, const std::string& filePath, Format format);
    
    /**
     * Export scene to STL format
     */
    bool exportToSTL(SceneGraph* sceneGraph, const std::string& filePath);
    
    /**
     * Export scene to OBJ format (includes material references)
     */
    bool exportToOBJ(SceneGraph* sceneGraph, const std::string& filePath);
    
    /**
     * Get the last error message
     */
    const std::string& getLastError() const { return lastError_; }
    
private:
    std::string lastError_;
};

} // namespace geantcad

