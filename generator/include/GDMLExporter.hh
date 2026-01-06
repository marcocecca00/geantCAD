#pragma once

#include "../../core/include/SceneGraph.hh"
#include <string>

namespace geantcad {

class GDMLExporter {
public:
    GDMLExporter();
    ~GDMLExporter();
    
    bool exportToFile(SceneGraph* sceneGraph, const std::string& filePath);
    
private:
    // Implementation
};

} // namespace geantcad

