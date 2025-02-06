#include "glfw_initialization.h"
#include <iostream>
#include <ostream>
#include <precomp.h>

#include "GLFW/glfw3.h"

namespace veng {
    GLFWInitialization::GLFWInitialization() {
        if (glfwInit() != GLFW_TRUE) {
            std::cout << "Failed to initialize GLFW" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    GLFWInitialization::~GLFWInitialization() {
        glfwTerminate();
    }
}
