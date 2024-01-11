# Set the toolchain file before calling project()
set(CMAKE_TOOLCHAIN_FILE "/home/iquezada/Code/cpp/demogl/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

cmake_minimum_required(VERSION 3.10)
project(YourProjectName)

# Set the C++ standard and compilation flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

# Platform-specific settings
if(WIN32) # Set Windows specific flags
elseif(APPLE) # Set macOS specific flags
    link_libraries("-framework CoreAudio -framework GameController -framework IOKit -framework AudioToolbox -framework CoreHaptics -framework CoreVideo -framework ForceFeedback -framework Metal -framework Foundation -framework Cocoa -framework CoreFoundation -framework OpenGL -liconv -framework CoreGraphics -framework Carbon")
elseif(UNIX AND NOT APPLE) # Set Linux specific flags
    link_libraries("-lGL")
endif()

# Find required packages
find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

# Include directories
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
add_executable(split split.cpp)
target_link_libraries(split
    PRIVATE
    imgui::imgui
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    ${OpenCV_LIBS}
    glm::glm
    libQ
    glscene
)