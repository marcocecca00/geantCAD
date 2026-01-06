cmake_minimum_required(VERSION 3.20)
project(GeantCAD VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(GEANTCAD_USE_ROOT "Enable ROOT support" OFF)
option(GEANTCAD_BUILD_EXAMPLES "Build example projects" ON)
option(GEANTCAD_BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

# Find dependencies
# Qt5 is required (for VTK compatibility)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL Gui)
message(STATUS "Qt5 found: ${Qt5_VERSION}")

find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    # Try to find it as a header-only library
    find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp)
    if(NLOHMANN_JSON_INCLUDE_DIR)
        message(STATUS "Found nlohmann/json at: ${NLOHMANN_JSON_INCLUDE_DIR}")
        add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
        set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${NLOHMANN_JSON_INCLUDE_DIR}"
        )
    else()
        message(FATAL_ERROR "nlohmann/json not found. Install via: sudo apt install nlohmann-json3-dev")
    endif()
endif()

if(GEANTCAD_USE_ROOT)
    find_package(ROOT QUIET)
    if(ROOT_FOUND)
        message(STATUS "ROOT found: ${ROOT_VERSION}")
        add_definitions(-DGEANTCAD_USE_ROOT)
    else()
        message(WARNING "ROOT requested but not found. Disabling ROOT support.")
        set(GEANTCAD_USE_ROOT OFF)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/app/include
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/include
)

# Core library (no GUI)
add_library(geantcad_core
    core/src/VolumeNode.cpp
    core/src/SceneGraph.cpp
    core/src/Transform.cpp
    core/src/Shape.cpp
    core/src/Material.cpp
    core/src/Serialization.cpp
    core/src/CommandStack.cpp
    core/src/Command.cpp
    core/src/PhysicsConfig.cpp
    core/src/OutputConfig.cpp
    core/src/ParticleGunConfig.cpp
)

target_link_libraries(geantcad_core
    Qt5::Core
    Qt5::Gui
    nlohmann_json::nlohmann_json
)

# Generator library
add_library(geantcad_generator
    generator/src/GDMLExporter.cpp
    generator/src/Geant4ProjectGenerator.cpp
    generator/src/TemplateEngine.cpp
)

target_link_libraries(geantcad_generator
    geantcad_core
)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Main application
add_executable(geantcad
    app/src/main.cpp
    app/src/MainWindow.cpp
    app/src/Viewport3D.cpp
    app/src/Outliner.cpp
    app/src/Inspector.cpp
    app/src/Toolbar.cpp
    app/src/PhysicsPanel.cpp
    app/src/OutputPanel.cpp
    app/src/ParticleGunPanel.cpp
    app/src/BuildRunDialog.cpp
    app/src/CollapsibleGroupBox.cpp
    app/include/Toolbar.hh
    app/include/MainWindow.hh
    app/include/Viewport3D.hh
    app/include/Outliner.hh
    app/include/Inspector.hh
    app/include/PhysicsPanel.hh
    app/include/OutputPanel.hh
    app/include/ParticleGunPanel.hh
    app/include/BuildRunDialog.hh
    app/include/CollapsibleGroupBox.hh
    app/resources/resources.qrc
)

target_link_libraries(geantcad
    geantcad_core
    geantcad_generator
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
)

# VTK - try to find it with Qt support
find_package(VTK QUIET COMPONENTS 
    CommonCore
    CommonDataModel
    FiltersSources
    InteractionStyle
    RenderingCore
    RenderingOpenGL2
    RenderingAnnotation
    GUISupportQt
)

if(VTK_FOUND)
    if(TARGET VTK::GUISupportQt)
        message(STATUS "VTK found: ${VTK_VERSION} (with Qt GUI support)")
        
        # Link VTK libraries
        target_link_libraries(geantcad 
            ${VTK_LIBRARIES}
            VTK::GUISupportQt
        )
        target_include_directories(geantcad PRIVATE ${VTK_INCLUDE_DIRS})
        
        # VTK is enabled
        message(STATUS "VTK enabled - viewport will be available")
    else()
        message(WARNING "VTK found but GUISupportQt not available. Viewport will be disabled.")
        add_definitions(-DGEANTCAD_NO_VTK)
    endif()
else()
    message(WARNING "VTK not found - viewport will show placeholder. Install VTK with Qt support.")
    add_definitions(-DGEANTCAD_NO_VTK)
endif()

# Qt5::Gui is needed
target_link_libraries(geantcad Qt5::Gui)

# Python bindings (optional)
if(GEANTCAD_BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)
    if(NOT pybind11_FOUND)
        # Try to find Python
        find_package(Python COMPONENTS Interpreter Development QUIET)
        if(Python_FOUND)
            # Use FetchContent to get pybind11
            include(FetchContent)
            FetchContent_Declare(
                pybind11
                GIT_REPOSITORY https://github.com/pybind/pybind11.git
                GIT_TAG v2.11.1
            )
            FetchContent_MakeAvailable(pybind11)
            message(STATUS "Using pybind11 from FetchContent")
        else()
            message(WARNING "Python not found. Python bindings disabled.")
            set(GEANTCAD_BUILD_PYTHON_BINDINGS OFF)
        endif()
    else()
        message(STATUS "pybind11 found: ${pybind11_VERSION}")
    endif()
    
    if(pybind11_FOUND OR TARGET pybind11::module)
        # Find Python
        find_package(Python COMPONENTS Interpreter Development REQUIRED)
        
        # Python module
        pybind11_add_module(geantcad_python
            generator/bindings/pybind11_wrapper.cpp
        )
        
        target_link_libraries(geantcad_python PRIVATE
            geantcad_core
            geantcad_generator
            pybind11::module
        )
        
        # Set output name (remove .so extension on some systems)
        set_target_properties(geantcad_python PROPERTIES
            OUTPUT_NAME "geantcad_python"
            PREFIX ""
        )
        
        message(STATUS "Python bindings enabled - module: geantcad_python")
    endif()
endif()

# Install
install(TARGETS geantcad DESTINATION bin)
install(DIRECTORY templates/ DESTINATION share/geantcad/templates)

