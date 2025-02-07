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

        static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
        std::vector<gsl::czstring> GetRequiredInstanceExtensions();
        static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
        static bool AreAllExtensionsSupported(const gsl::span<gsl::czstring> &extensions);

        static std::vector<VkLayerProperties> GetSupportedValidationLayers();
        static bool AreAllLayersSupported(const gsl::span<gsl::czstring> &extensions);

        VkInstance instance_{};
        gsl::not_null<GLFW_Window *> window_;
        bool validation_ = false;
    };
} // veng
