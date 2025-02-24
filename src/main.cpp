#include <precomp.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glfw_aux/glfw_initialization.h>
#include <glfw_aux/glfw_window.h>
#include "graphics.h"
#include "glm/gtc/matrix_transform.hpp"

std::int32_t main(std::int32_t argc, gsl::zstring *argv) {
    veng::GLFWInitialization _glfw;

    veng::GLFW_Window window{"Vulkan Engine", {800, 600}};
    if (!window.TryMoveToMonitor(0)) {
        std::cerr << "Failed to move monitor" << std::endl;
    }

    veng::Graphics graphics{gsl::make_not_null(&window)};

    std::array vertices = {
        veng::Vertex{glm::vec3{0.0f, -0.5f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f}},
        veng::Vertex{glm::vec3{0.5f, 0.5f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}},
        veng::Vertex{glm::vec3{-0.5f, 0.5f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}},
    };

    const veng::BufferHandle buffer = graphics.CreateVertexBuffer(vertices);

    std::array<std::uint32_t, 3> indices = {
        0, 1, 2
    };

    veng::BufferHandle index_buffer = graphics.CreateIndexBuffer(indices);

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    graphics.SetViewProjection(view, proj);

    while (!window.ShouldClose()) {
        glfwPollEvents();
        if (graphics.BeginFrame()) {
            graphics.RenderIndexedBuffer(buffer, index_buffer, indices.size());
            graphics.EndFrame();
        }
    }

    graphics.DestroyBuffer(buffer);
    graphics.DestroyBuffer(index_buffer);

    return EXIT_SUCCESS;
}
