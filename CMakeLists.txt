cmake_minimum_required(VERSION 3.20)
project(GeantCAD VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(GEANTCAD_USE_ROOT "Enable ROOT support" OFF)
option(GEANTCAD_BUILD_EXAMPLES "Build example projects" ON)
option(GEANTCAD_BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(GEANTCAD_PREFER_QT6 "Prefer Qt6 over Qt5" ON)

# ===== Qt Detection =====
# Try to find Qt6 first if preferred, fall back to Qt5
set(QT_FOUND FALSE)

if(GEANTCAD_PREFER_QT6)
    find_package(Qt6 QUIET COMPONENTS Core Widgets OpenGL Gui OpenGLWidgets)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
        set(QT_FOUND TRUE)
        message(STATUS "Qt6 found: ${Qt6_VERSION}")
        message(STATUS "Using Qt6 for modern UI features")
        
        # Qt6 requires OpenGLWidgets component
        set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::Gui Qt6::OpenGLWidgets)
        set(QT_CORE_LIBS Qt6::Core Qt6::Gui)
        
        # Qt6 compatibility macros
        add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
    endif()
endif()

if(NOT QT_FOUND)
    find_package(Qt5 QUIET COMPONENTS Core Widgets OpenGL Gui)
    if(Qt5_FOUND)
        set(QT_VERSION_MAJOR 5)
        set(QT_FOUND TRUE)
        message(STATUS "Qt5 found: ${Qt5_VERSION}")
        message(STATUS "Note: For Qt6 support, set GEANTCAD_PREFER_QT6=ON and ensure Qt6 is installed")
        
        set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::OpenGL Qt5::Gui)
        set(QT_CORE_LIBS Qt5::Core Qt5::Gui)
    endif()
endif()

if(NOT QT_FOUND)
    message(FATAL_ERROR "Neither Qt6 nor Qt5 found. Please install Qt development packages.")
endif()

# ===== JSON Library =====
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

# ===== ROOT (Optional) =====
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

# ===== Include directories =====
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/app/include
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/include
)

# ===== Core library (no GUI) =====
add_library(geantcad_core
    core/src/VolumeNode.cpp
    core/src/SceneGraph.cpp
    core/src/Transform.cpp
    core/src/Shape.cpp
    core/src/Material.cpp
    core/src/NistMaterialDatabase.cpp
    core/src/Serialization.cpp
    core/src/CommandStack.cpp
    core/src/Command.cpp
    core/src/PhysicsConfig.cpp
    core/src/OutputConfig.cpp
    core/src/ParticleGunConfig.cpp
)

target_link_libraries(geantcad_core
    ${QT_CORE_LIBS}
    nlohmann_json::nlohmann_json
)

# ===== Generator library =====
add_library(geantcad_generator
    generator/src/GDMLExporter.cpp
    generator/src/Geant4ProjectGenerator.cpp
    generator/src/TemplateEngine.cpp
    generator/src/MeshExporter.cpp
)

target_link_libraries(geantcad_generator
    geantcad_core
)

# Link VTK to generator if available (for mesh export)
if(VTK_FOUND)
    target_include_directories(geantcad_generator PRIVATE ${VTK_INCLUDE_DIRS})
    target_link_libraries(geantcad_generator ${VTK_LIBRARIES})
else()
    target_compile_definitions(geantcad_generator PRIVATE GEANTCAD_NO_VTK)
endif()

# ===== Enable Qt MOC =====
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ===== Main application =====
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
    app/src/PropertiesPanel.cpp
    app/src/SimulationConfigPanel.cpp
    app/src/CameraControlWidget.cpp
    app/src/ProjectManagerPanel.cpp
    app/src/RightPanelContainer.cpp
    # New UI components
    app/src/ViewCube.cpp
    app/src/ClippingPlaneWidget.cpp
    app/src/MeasurementTool.cpp
    app/src/HistoryPanel.cpp
    app/src/ThemeManager.cpp
    app/src/PreferencesDialog.cpp
    app/src/ShortcutsDialog.cpp
    # Headers (for MOC)
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
    app/include/PropertiesPanel.hh
    app/include/SimulationConfigPanel.hh
    app/include/CameraControlWidget.hh
    app/include/ProjectManagerPanel.hh
    app/include/RightPanelContainer.hh
    # New headers
    app/include/ViewCube.hh
    app/include/ClippingPlaneWidget.hh
    app/include/MeasurementTool.hh
    app/include/HistoryPanel.hh
    app/include/ThemeManager.hh
    app/include/PreferencesDialog.hh
    app/include/ShortcutsDialog.hh
    # Resources
    app/resources/resources.qrc
)

target_link_libraries(geantcad
    geantcad_core
    geantcad_generator
    ${QT_LIBS}
)

# ===== VTK Integration =====
# Try to find VTK with Qt support
find_package(VTK QUIET COMPONENTS 
    CommonCore
    CommonDataModel
    FiltersSources
    InteractionStyle
    InteractionWidgets
    RenderingCore
    RenderingOpenGL2
    RenderingAnnotation
    GUISupportQt
)

if(VTK_FOUND)
    if(TARGET VTK::GUISupportQt)
        message(STATUS "VTK found: ${VTK_VERSION} (with Qt${QT_VERSION_MAJOR} GUI support)")
        
        # Check VTK Qt version compatibility
        get_target_property(VTK_QT_VERSION VTK::GUISupportQt INTERFACE_LINK_LIBRARIES)
        string(FIND "${VTK_QT_VERSION}" "Qt${QT_VERSION_MAJOR}" VTK_QT_MATCH)
        
        if(VTK_QT_MATCH EQUAL -1)
            message(WARNING "VTK may be compiled with a different Qt version. "
                           "If you experience issues, recompile VTK with -DVTK_QT_VERSION=${QT_VERSION_MAJOR}")
        endif()
        
        # Link VTK libraries
        target_link_libraries(geantcad 
            ${VTK_LIBRARIES}
            VTK::GUISupportQt
            VTK::InteractionWidgets
        )
        target_include_directories(geantcad PRIVATE ${VTK_INCLUDE_DIRS})
        
        # Apply VTK module autoinit
        vtk_module_autoinit(
            TARGETS geantcad
            MODULES ${VTK_LIBRARIES}
        )
        
        message(STATUS "VTK enabled - full 3D viewport available")
    else()
        message(WARNING "VTK found but GUISupportQt not available. Viewport will be disabled.")
        add_definitions(-DGEANTCAD_NO_VTK)
    endif()
else()
    message(WARNING "VTK not found - viewport will show placeholder. Install VTK with Qt support.")
    add_definitions(-DGEANTCAD_NO_VTK)
endif()

# ===== Python bindings (optional) =====
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
        
        # Set output name
        set_target_properties(geantcad_python PROPERTIES
            OUTPUT_NAME "geantcad_python"
            PREFIX ""
        )
        
        message(STATUS "Python bindings enabled - module: geantcad_python")
    endif()
endif()

# ===== Install =====
install(TARGETS geantcad DESTINATION bin)
install(DIRECTORY templates/ DESTINATION share/geantcad/templates)

# ===== Summary =====
message(STATUS "")
message(STATUS "GeantCAD Build Configuration:")
message(STATUS "  Version:      ${PROJECT_VERSION}")
message(STATUS "  Qt Version:   ${QT_VERSION_MAJOR}")
message(STATUS "  VTK Support:  ${VTK_FOUND}")
message(STATUS "  ROOT Support: ${GEANTCAD_USE_ROOT}")
message(STATUS "  Python:       ${GEANTCAD_BUILD_PYTHON_BINDINGS}")
message(STATUS "")
