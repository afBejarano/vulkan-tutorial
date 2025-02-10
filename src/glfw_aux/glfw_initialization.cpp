#include "glfw_initialization.h"
#include <iostream>
#include <ostream>
#include <precomp.h>

#include <GLFW/glfw3.h>
#include "spdlog/spdlog.h"

namespace veng {
    void glfw_error_callback(std::int32_t error, gsl::czstring description) {
        spdlog::error("Glfw Validation: {}", description);
    }

    GLFWInitialization::GLFWInitialization() {
        glfwSetErrorCallback(glfw_error_callback);

        if (glfwInit() != GLFW_TRUE) {
            std::cout << "Failed to initialize GLFW" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    GLFWInitialization::~GLFWInitialization() {
        glfwTerminate();
    }
}
