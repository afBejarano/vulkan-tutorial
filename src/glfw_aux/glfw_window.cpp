#include "glfw_window.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <precomp.h>

#include "glfw_monitor.h"

namespace veng {
    GLFW_Window::GLFW_Window(const gsl::czstring &title, const glm::ivec2 size) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ = glfwCreateWindow(size.x, size.y, title, nullptr, nullptr);
        if (window_ == nullptr) {
            std::cout << "Failed to create GLFW window" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    GLFW_Window::~GLFW_Window() {
        glfwDestroyWindow(window_);
    }

    glm::ivec2 GLFW_Window::GetWindowSize() const {
        glm::ivec2 size;
        glfwGetWindowSize(window_, &size.x, &size.y);
        return size;
    }

    glm::ivec2 GLFW_Window::GetFrameBufferSize() const {
        glm::ivec2 size;
        glfwGetFramebufferSize(window_, &size.x, &size.y);
        return size;
    }

    bool GLFW_Window::ShouldClose() const {
        return glfwWindowShouldClose(window_);
    }

    GLFWwindow *GLFW_Window::GetHandle() const {
        return window_;
    }

    bool GLFW_Window::TryMoveToMonitor(const std::uint16_t monitor) const {
        if (const gsl::span<GLFWmonitor *> monitors = GetMonitors(); monitor < monitors.size()) {
            MoveWindowToMonitor(window_, monitors[monitor]);
            return true;
        }

        return false;
    }
}
