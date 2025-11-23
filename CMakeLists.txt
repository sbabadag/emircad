cmake_minimum_required(VERSION 3.16)
project(TeklaLikeCAD VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt5 path
set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.12.12/5.12.12/msvc2017_64" CACHE PATH "Qt5 installation path")

# Find Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)

# Set up paths for 3rdparty dependencies
set(VTK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opencascade/3rdparty-vc14-64/vtk-9.4.1-x64/lib/cmake/vtk-9.4" CACHE PATH "VTK directory")

# Find VTK first (required by OpenCascade)
find_package(VTK QUIET)

# Find OpenCascade
# Adjust the path according to your OpenCascade installation
set(OpenCASCADE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opencascade/occt-vc14-64/cmake" CACHE PATH "Path to OpenCASCADE CMake directory")
find_package(OpenCASCADE REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCASCADE_INCLUDE_DIR}
)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/OCCTViewer.cpp
    src/GeometryBuilder.cpp
    src/CADCommand.cpp
    src/BeamCommand.cpp
    src/ColumnCommand.cpp
    src/SlabCommand.cpp
    src/CADController.cpp
    src/SteelProfile.cpp
    src/ProfileSelectionDialog.cpp
    src/TGraphicObject.cpp
    src/TObjectCollection.cpp
    src/TBeam.cpp
    src/TColumn.cpp
    src/TSlab.cpp
    src/PropertiesPanel.cpp
    src/WorkPlane.cpp
    src/WorkPlaneDialog.cpp
    src/SnapManager.cpp
    src/SnapToolbar.cpp
)

# Header files
set(HEADERS
    include/MainWindow.h
    include/OCCTViewer.h
    include/GeometryBuilder.h
    include/CADCommand.h
    include/BeamCommand.h
    include/ColumnCommand.h
    include/SlabCommand.h
    include/CADController.h
    include/SteelProfile.h
    include/ProfileSelectionDialog.h
    include/TGraphicObject.h
    include/TObjectCollection.h
    include/TBeam.h
    include/TColumn.h
    include/TSlab.h
    include/PropertiesPanel.h
    include/WorkPlane.h
    include/WorkPlaneDialog.h
    include/SnapManager.h
    include/SnapToolbar.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    ${OpenCASCADE_LIBRARIES}
)

# OpenCascade specific libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKGeomAlgo
    TKBRep
    TKTopAlgo
    TKPrim
    TKBO
    TKBool
    TKOffset
    TKService
    TKV3d
    TKOpenGl
    TKMesh
    TKHLR
    TKFillet
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Windows specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

