//
// Created by andre on 27/01/2025.
//
#pragma once
#include <optional>
#include <vulkan/vulkan.h>

#include <glfw_aux/glfw_window.h>

namespace veng {
    class Graphics final {
    public:
        explicit Graphics(gsl::not_null<GLFW_Window *> window);

        ~Graphics();

    private:
        struct QueueFamilyIndices {
            std::optional<std::uint32_t> graphics_family = std::nullopt;
            std::optional<std::uint32_t> present_family = std::nullopt;

            bool IsValid() const {
                return graphics_family.has_value() /* && present_family.has_value()*/;
            }
        };

        void InitializeVulkan();

        void CreateInstance();

        void SetupDebugMessenger();

        void PickPhysicalDevice();

        void CreateLogicalDeviceAndQueues();

        void CreateSurface();

        static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();

        std::vector<gsl::czstring> GetRequiredInstanceExtensions() const;

        static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();

        static bool AreAllExtensionsSupported(const gsl::span<gsl::czstring> &extensions);

        static std::vector<VkLayerProperties> GetSupportedValidationLayers();

        static bool AreAllLayersSupported(const gsl::span<gsl::czstring> &extensions);

        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

        bool IsDeviceSuitable(VkPhysicalDevice device);

        std::vector<VkPhysicalDevice> GetPhysicalDevices() const;

        VkInstance instance_ = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_messenger_;

        VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
        VkDevice device_ = VK_NULL_HANDLE;
        VkQueue graphics_queue_ = VK_NULL_HANDLE;

        VkSurfaceKHR surface_ = VK_NULL_HANDLE;
        gsl::not_null<GLFW_Window *> window_;
        bool validation_ = false;
    };
} // veng
