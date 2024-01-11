# Set the toolchain file before calling project()
set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

cmake_minimum_required(VERSION 3.10)
project(YourProjectName)

# Set the C++ standard and compilation flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3") # -Wall

# Find required packages
find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(.)
include_directories(${CMAKE_BINARY_DIR}/../vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include)
include_directories(${CMAKE_BINARY_DIR}/../vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include/glm)
include_directories(libQ/include)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})

add_library(libQ OBJECT
    libQ/src/shader.cpp
    libQ/src/camera.cpp
    libQ/src/xml.cpp
)

add_library(glscene OBJECT
    scene/scene.cpp
    scene/collada.cpp
    origin/origin.cpp
    renderable/renderable.cpp
    model/model.cpp
    model/cube.cpp
    model/window.cpp
    model/blank.cpp
)

# Add executables and link libraries
add_executable(split split.cpp ./glad/glad.c)
target_link_libraries(split
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
    imgui::imgui
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    ${OpenCV_LIBS}
    glm::glm
    libQ
    glscene
)