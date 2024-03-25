# Set the toolchain file before calling project()

list(APPEND VCPKG_PACKAGES glm glad sdl2 opencv "imgui[core,opengl3-binding,sdl2-binding,freetype]" lua)
include(libQ/vcpkg_install/vcpkg_install.cmake)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

cmake_minimum_required(VERSION 3.10)
project(split)

# Set the C++ standard and compilation flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3") # -Wall

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# Find required packages
find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)

include_directories(.)
include_directories(${CMAKE_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include)
include_directories(${CMAKE_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include/glm)
include_directories(libQ/include)
include_directories(glad/)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${LUA_INCLUDE_DIR})

add_subdirectory(libQ/)
add_library(glscene OBJECT
    scene/scene.cpp
    scene/collada.cpp
    scene/loader.cpp
    origin/origin.cpp
    renderable/renderable.cpp
    renderable/gldata.cpp
    model/model.cpp
    model/cube.cpp
    model/window.cpp
)

add_executable(cvdemo cvdemo.cpp)
target_link_libraries(cvdemo
  ${OpenCV_LIBS}
)

add_executable(sdlgl sdlgl.cpp ./glad/glad.c)
target_link_libraries(sdlgl
  ${OPENGL_LIBRARIES}
  ${CMAKE_DL_LIBS}
  glm::glm
  $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
  $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
  Q
)

add_executable(sdlglimgui sdlglimgui.cpp ./glad/glad.c)
target_link_libraries(sdlglimgui
  ${OPENGL_LIBRARIES}
  ${CMAKE_DL_LIBS}
  glm::glm
  $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
  $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
  imgui::imgui
  Q
)

add_executable(sdlglimguitextured sdlglimguitextured.cpp ./glad/glad.c origin/origin.cpp)
target_link_libraries(sdlglimguitextured
  ${OPENGL_LIBRARIES}
  ${CMAKE_DL_LIBS}
  glm::glm
  $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
  $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
  imgui::imgui
  Q
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
    Q
    glscene
    ${LUA_LIBRARIES}
)
