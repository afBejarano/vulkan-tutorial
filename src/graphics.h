//
// Created by andre on 27/01/2025.
//
#pragma once
#include <vulkan/vulkan.h>

#include "glfw_aux/glfw_window.h"

namespace veng {
    class Graphics final {
    public:
        explicit Graphics(gsl::not_null<GLFW_Window *> window);

        ~Graphics();

    private:
        void InitializeVulkan();

        void CreateInstance();

        gsl::span<gsl::czstring> CreateInstanceExtensions();

        VkInstance instance_;
        gsl::not_null<GLFW_Window *> window_;
    };
} // veng
