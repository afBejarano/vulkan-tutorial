#include <precomp.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glfw_aux/glfw_initialization.h>
#include <glfw_aux/glfw_window.h>
#include "graphics.h"
#include "object.h"
#include "glm/gtc/matrix_transform.hpp"

std::int32_t main(std::int32_t argc, gsl::zstring *argv) {
    veng::GLFWInitialization _glfw;

    veng::GLFW_Window window{"Vulkan Engine", {800, 600}, false};
    if (!window.TryMoveToMonitor(0)) {
        std::cerr << "Failed to move monitor" << std::endl;
    }

    veng::Graphics graphics{gsl::make_not_null(&window)};

    glm::vec2 size = window.GetWindowSize();

    glm::mat4 rotation = rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.1f, 0.1f, 0.1f));
    glm::mat4 view = translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), size.x / size.y, 0.1f, 100.0f);

    veng::object object("./assets/Spider/spider.obj", "./assets/Spider/");

    veng::BufferHandle buffer = graphics.CreateoVertexBuffer(object.getOVertices());
    veng::BufferHandle index_buffer = graphics.CreateIndexBuffer(object.getIndices());
    graphics.SetViewProjection(view, proj);

    std::vector<veng::TextureHandle> texture_handles;

    for (const auto &texture: object.getTextures())
        texture_handles.push_back(graphics.CreateTexture(texture.c_str()));

    while (!window.ShouldClose()) {
        glfwPollEvents();
        if (graphics.BeginFrame()) {
            graphics.RenderModel(buffer, index_buffer, object, texture_handles, object.getMaterialUBOs(), rotation);
            graphics.EndFrame();
        }
    }

    for (const auto &texture: texture_handles)
        graphics.DestroyTexture(texture);

    graphics.DestroyBuffer(buffer);
    graphics.DestroyBuffer(index_buffer);

    return EXIT_SUCCESS;
}
