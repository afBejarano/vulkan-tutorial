#pragma once
#include "GLFW/glfw3.h"

struct GLFWwindow;

namespace veng {
    class GLFW_Window {
    public:
        GLFW_Window(const gsl::czstring &title, glm::ivec2 size);

        ~GLFW_Window();

        glm::ivec2 GetWindowSize() const;

        bool ShouldClose() const;

        GLFWwindow *GetHandle() const;

        bool TryMoveToMonitor(std::uint16_t monitor) const;

    private:
        GLFWwindow *window_;
    };
}
