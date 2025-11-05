# CMake entry point
cmake_minimum_required(VERSION 3.5...3.28)

include(CMakePrintHelpers)
project(VC_2)

cmake_print_variables(CMAKE_PREFIX_PATH)
cmake_print_variables(CMAKE_SOURCE_DIR)

# --- Dependencies ---
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(
    ${GLM_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    "external"
    ${GLFW_INCLUDE_DIRS}
    .
)

# Use experimental glm features
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

set(ALL_LIBS
    ${OPENGL_LIBRARY}
    glfw
    ${OpenCV_LIBS}
)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)

# --------------------------------------------------------------------------
# Assignment 2 - OpenCV camera feed on textured quad
# --------------------------------------------------------------------------
add_executable(VC_2_app
    common/Shader.cpp
    common/Shader.hpp
    common/ColorShader.cpp
    common/ColorShader.hpp
    common/Camera.cpp
    common/Camera.hpp
    common/Scene.cpp
    common/Scene.hpp
    common/Object.cpp
    common/Object.hpp
    common/Triangle.cpp
    common/Triangle.hpp
    common/Texture.cpp
    common/Texture.hpp
    common/TextureShader.cpp
    common/TextureShader.hpp
    common/Quad.cpp
    common/Quad.hpp
    common/Filters.cpp
    common/Filters.hpp
    common/PixelationShader.cpp
    common/PixelationShader.hpp
    src/webcamQuad.cpp
)
target_link_libraries(VC_2_app
    ${ALL_LIBS}
)

# --------------------------------------------------------------------------
# Automatically copy shaders from src/ to the executable folder
# --------------------------------------------------------------------------
file(GLOB SHADERS "src/*.vert" "src/*.frag")
foreach(SHADER ${SHADERS})
    add_custom_command(TARGET VC_2_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SHADER}
        $<TARGET_FILE_DIR:VC_2_app>
    )
endforeach()

# --------------------------------------------------------------------------
# Source grouping for IDE organization
# --------------------------------------------------------------------------
SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*")
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$")