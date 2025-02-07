#include <precomp.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glfw_aux/glfw_initialization.h>
#include <glfw_aux/glfw_window.h>
#include "graphics.h"

std::int32_t main(std::int32_t argc, gsl::zstring *argv) {
    veng::GLFWInitialization _glfw;

    veng::GLFW_Window window{"Vulkan Engine", {800, 600}};
    if (!window.TryMoveToMonitor(0)) {
        std::cerr << "Failed to move monitor" << std::endl;
    }

    veng::Graphics graphics{gsl::make_not_null(&window)};

    while (!window.ShouldClose()) {
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
