#pragma once

#include "SceneGraph.hh"
#include <string>

namespace geantcad {

/**
 * Save scene graph to JSON file
 */
bool saveSceneToFile(SceneGraph* sceneGraph, const std::string& filePath);

/**
 * Load scene graph from JSON file
 */
bool loadSceneFromFile(SceneGraph* sceneGraph, const std::string& filePath);

} // namespace geantcad

