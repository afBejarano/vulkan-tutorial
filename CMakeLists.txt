cmake_minimum_required(VERSION 3.20)
project(VulkanEngine)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

include(cmake/Shaders.cmake)
include(FetchContent)

include_directories(external)

FetchContent_Declare(
        glm
        GIT_REPOSITORY "https://github.com/g-truc/glm.git"
        GIT_TAG "1.0.1"
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        microsoft-gsl
        GIT_REPOSITORY "https://github.com/microsoft/GSL.git"
        GIT_TAG "v4.1.0"
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(microsoft-gsl)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        GIT_TAG "3.4"
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
        GIT_TAG "v1.15.1"
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(spdlog)

file(GLOB_RECURSE VulkanEngineSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(VulkanEngine src/main.cpp
        src/glfw_aux/glfw_initialization.h src/glfw_aux/glfw_initialization.cpp
        src/precomp.h
        src/glfw_aux/glfw_monitor.cpp src/glfw_aux/glfw_monitor.h
        src/glfw_aux/glfw_window.cpp src/glfw_aux/glfw_window.h
        src/graphics.cpp src/graphics.h
        src/utilities.cpp src/utilities.h
        src/vertex.h
        src/buffer_handle.h
        src/uniform_transformations.h
        src/texture_handle.h
        src/stb_image.h src/stb_image.cpp
        src/object.h src/object.cpp
)

target_link_libraries(VulkanEngine PRIVATE Vulkan::Vulkan)
target_link_libraries(VulkanEngine PRIVATE glm)
target_link_libraries(VulkanEngine PRIVATE glfw)
target_link_libraries(VulkanEngine PRIVATE Microsoft.GSL::GSL)
target_link_libraries(VulkanEngine PRIVATE spdlog)

target_compile_definitions(VulkanEngine PRIVATE TINYOBJLOADER_IMPLEMENTATION)

target_include_directories(VulkanEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_features(VulkanEngine PRIVATE cxx_std_20)

target_precompile_headers(VulkanEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/precomp.h")

file(GLOB_RECURSE ShaderSources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.geom"
)

add_shaders(VulkanEngineShaders ${ShaderSources})
add_dependencies(VulkanEngine VulkanEngineShaders)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
