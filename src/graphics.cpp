//
// Created by andre on 27/01/2025.
//

#include "graphics.h"

#include <iostream>
#include <precomp.h>
#include <cstring>
#include <set>
#include <spdlog/spdlog.h>

#pragma region VK_FUNCITON_EXT_IMPL

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT"));

    if (function != nullptr) {
        return function(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                           VkDebugUtilsMessengerEXT pDebugMessenger,
                                                           const VkAllocationCallbacks *pAllocator) {
    auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (function != nullptr) {
        return function(instance, pDebugMessenger, pAllocator);
    }
}

#pragma endregion

namespace veng {
#pragma region VALIDATION_LAYERS
    static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                             void *user_data) {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            spdlog::error("Vulkan Validation: {}", pCallbackData->pMessage);
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            spdlog::warn("Vulkan Validation: {}", pCallbackData->pMessage);
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            spdlog::info("Vulkan Validation: {}", pCallbackData->pMessage);
        } else {
            spdlog::debug("Vulkan Validation: {}", pCallbackData->pMessage);
        }
        return VK_FALSE;
    }

    static VkDebugUtilsMessengerCreateInfoEXT GetCreateDebugMessengerInfo() {
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
        messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.pNext = nullptr;

        messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

        messengerCreateInfo.pfnUserCallback = ValidationCallback;
        messengerCreateInfo.pUserData = nullptr;

        return messengerCreateInfo;
    }

    void Graphics::SetupDebugMessenger() {
        if (!validation_) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT info = GetCreateDebugMessengerInfo();
        if (vkCreateDebugUtilsMessengerEXT(instance_, &info, nullptr, &debug_messenger_) != VK_SUCCESS) {
            spdlog::error("Failed to create debug messenger");
            return;
        };
    }

    std::vector<VkLayerProperties> Graphics::GetSupportedValidationLayers() {
        std::uint32_t validation_layers_count = 0;
        vkEnumerateInstanceLayerProperties(&validation_layers_count, nullptr);

        if (validation_layers_count == 0) { return {}; }

        std::vector<VkLayerProperties> layers(validation_layers_count);
        vkEnumerateInstanceLayerProperties(&validation_layers_count, layers.data());
        return layers;
    }

    bool LayerMatchesName(const gsl::czstring layer_name, const VkLayerProperties &properties) {
        return strcmp(properties.layerName, layer_name) == 0;
    }

    bool IsLayerSupported(gsl::span<VkLayerProperties> layers, gsl::czstring layer_name) {
        return std::ranges::any_of(layers, std::bind_front(&LayerMatchesName, layer_name));
    }

    bool Graphics::AreAllLayersSupported(const gsl::span<gsl::czstring> &extensions) {
        return std::ranges::all_of(extensions, std::bind_front(IsLayerSupported, GetSupportedValidationLayers()));
    }

#pragma endregion

#pragma region INSTANCE_AND_EXTENSIONS

    void Graphics::CreateInstance() {
        std::array<gsl::czstring, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        if (!AreAllLayersSupported(validationLayers)) {
            validation_ = false;
        }

        std::vector<gsl::czstring> required_extensions = GetRequiredInstanceExtensions();

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr;
        app_info.pApplicationName = "Udemy Course";
        app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        app_info.pEngineName = "Vulkan Engine";
        app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        app_info.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = required_extensions.size();
        create_info.ppEnabledExtensionNames = required_extensions.data();

        const VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = GetCreateDebugMessengerInfo();

        if (validation_) {
            create_info.pNext = &messenger_create_info;
            create_info.enabledLayerCount = validationLayers.size();
            create_info.ppEnabledLayerNames = validationLayers.data();
        } else {
            create_info.enabledLayerCount = 0;
            create_info.ppEnabledLayerNames = nullptr;
        }

        if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
            std::cout << "Failed to create instance" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    gsl::span<gsl::czstring> Graphics::GetSuggestedInstanceExtensions() {
        std::uint32_t extension_count = 0;
        gsl::czstring *extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
        return {extension_names, extension_count};
    }

    std::vector<gsl::czstring> Graphics::GetRequiredInstanceExtensions() const {
        gsl::span<gsl::czstring> suggested_extensions = GetSuggestedInstanceExtensions();
        std::vector<gsl::czstring> required_extensions(suggested_extensions.size());
        std::ranges::copy(suggested_extensions, required_extensions.begin());

        if (validation_) {
            required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        if (!AreAllExtensionsSupported(required_extensions)) {
            std::cout << "No supported extensions found" << std::endl;
            exit(EXIT_FAILURE);
        }

        return required_extensions;
    }

    std::vector<VkExtensionProperties> Graphics::GetSupportedInstanceExtensions() {
        std::uint32_t extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

        if (extension_count == 0) { return {}; }

        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
        return extensions;
    }

    bool ExtensionMatchesName(const gsl::czstring extension_name, const VkExtensionProperties &extension) {
        return strcmp(extension.extensionName, extension_name) == 0;
    }

    bool IsExtensionSupported(gsl::span<VkExtensionProperties> extensions, gsl::czstring extension_name) {
        return std::ranges::any_of(extensions, std::bind_front(&ExtensionMatchesName, extension_name));
    }

    bool Graphics::AreAllExtensionsSupported(const gsl::span<gsl::czstring> &extensions) {
        return std::ranges::all_of(extensions, std::bind_front(IsExtensionSupported, GetSupportedInstanceExtensions()));
    }
#pragma endregion

#pragma region DEVICES_AND_QUEUES

    Graphics::QueueFamilyIndices Graphics::FindQueueFamilies(VkPhysicalDevice device) {
        std::uint32_t graphics_families = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &graphics_families, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(graphics_families);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &graphics_families, queue_families.data());

        auto graphics_family_it = std::ranges::find_if(queue_families, [](const VkQueueFamilyProperties &props) {
            return props.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
        });

        QueueFamilyIndices indices;
        indices.graphics_family = graphics_family_it - queue_families.begin();

        for (std::uint32_t i = 0; i < queue_families.size(); ++i) {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present_support);
            if (present_support) {
                indices.present_family = i;
                break;
            }
        }

        return indices;
    }

    Graphics::SwapChainSupportDetails Graphics::FindSwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

        std::uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, nullptr);
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, details.formats.data());

        std::uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, nullptr);
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, details.present_modes.data());

        return details;
    }

    std::vector<VkExtensionProperties> Graphics::GetDeviceAvaliableExtensions(VkPhysicalDevice device) {
        std::uint32_t extension_count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

        return extensions;
    }

    bool IsDeviceExtensionWithinList(const std::vector<VkExtensionProperties> &extensions,
                                     gsl::czstring extension_name) {
        return std::ranges::any_of(extensions, [extension_name](const VkExtensionProperties &property) {
            return strcmp(extension_name, property.extensionName) == 0;
        });
    }

    bool Graphics::AreAllDeviceExtensionsSupported(VkPhysicalDevice device) {
        std::vector<VkExtensionProperties> extensions = GetDeviceAvaliableExtensions(device);
        return std::ranges::all_of(required_device_extensions_,
                                   std::bind_front(IsDeviceExtensionWithinList, extensions));
    }

    bool Graphics::IsDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = FindQueueFamilies(device);

        return indices.IsValid() && AreAllDeviceExtensionsSupported(device) && FindSwapChainSupport(device).IsValid();
    }

    void Graphics::PickPhysicalDevice() {
        auto devices = GetPhysicalDevices();

        std::erase_if(devices, std::not_fn(std::bind_front(&Graphics::IsDeviceSuitable, this)));
        if (devices.empty()) {
            spdlog::error("Failed to find a suitable GPU!");
            std::exit(EXIT_FAILURE);
        }

        physical_device_ = devices[0];
    }

    std::vector<VkPhysicalDevice> Graphics::GetPhysicalDevices() const {
        std::uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

        if (device_count == 0) {
            return {};
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

        return devices;
    }

    void Graphics::CreateLogicalDeviceAndQueues() {
        QueueFamilyIndices indices = Graphics::FindQueueFamilies(physical_device_);

        if (!indices.IsValid()) {
            std::exit(EXIT_FAILURE);
        }

        std::set<std::uint32_t> unique_queue_families = {
            indices.graphics_family.value(), indices.present_family.value()
        };

        std::float_t queue_priorities = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        for (std::uint32_t queue_family: unique_queue_families) {
            VkDeviceQueueCreateInfo queue_create_info = {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priorities;
            queue_create_info.pNext = nullptr;
            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures required_features = {};

        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = queue_create_infos.size();
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.pEnabledFeatures = &required_features;
        create_info.enabledExtensionCount = required_device_extensions_.size();
        create_info.ppEnabledExtensionNames = required_device_extensions_.data();
        create_info.enabledLayerCount = 0;

        if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS) {
            spdlog::error("Failed to create logical device!");
            std::exit(EXIT_FAILURE);
        }

        vkGetDeviceQueue(device_, indices.graphics_family.value(), 0, &graphics_queue_);
        vkGetDeviceQueue(device_, indices.present_family.value(), 0, &present_queue_);
    }

#pragma endregion

#pragma region PRESENTATION

    void Graphics::CreateSurface() {
        if (glfwCreateWindowSurface(instance_, window_->GetHandle(), nullptr, &surface_) != VK_SUCCESS) {
            spdlog::error("Failed to create window surface!");
            std::exit(EXIT_FAILURE);
        };
    }

    bool IsRgbaTypeFormat(const VkSurfaceFormatKHR &format) {
        return format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB;
    }

    bool IsSrgbColorSpace(const VkSurfaceFormatKHR &format) {
        return format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    bool IsCorrectFormat(const VkSurfaceFormatKHR &format) {
        return IsRgbaTypeFormat(format) && IsSrgbColorSpace(format);
    }

    VkSurfaceFormatKHR Graphics::ChooseSwapchainSurfaceFormat(gsl::span<VkSurfaceFormatKHR> formats) {
        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            return {VK_FORMAT_R8G8B8A8_SNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        if (auto it = std::ranges::find_if(formats, IsCorrectFormat); it != formats.end()) {
            return *it;
        }
        return formats[0];
    }

    bool IsMailboxPresent(const VkPresentModeKHR &present) {
        return present == VK_PRESENT_MODE_MAILBOX_KHR;
    }

    VkPresentModeKHR Graphics::ChooseSwapchainPresentMode(gsl::span<VkPresentModeKHR> present_modes) {
        if (std::ranges::any_of(present_modes, IsMailboxPresent)) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Graphics::ChooseSwapchainExtent(VkSurfaceCapabilitiesKHR capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        glm::ivec2 size = window_->GetFrameBufferSize();
        VkExtent2D actual_extend = {
            static_cast<std::uint32_t>(size.x),
            static_cast<std::uint32_t>(size.y)
        };

        actual_extend.width = std::clamp(actual_extend.width, capabilities.minImageExtent.width,
                                         capabilities.maxImageExtent.width);
        actual_extend.height = std::clamp(actual_extend.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);
        return actual_extend;
    }

    void Graphics::CreateSwapChain() {
        SwapChainSupportDetails properties = FindSwapChainSupport(physical_device_);

        VkSurfaceFormatKHR surface_format = ChooseSwapchainSurfaceFormat(properties.formats);
        VkPresentModeKHR present_mode = ChooseSwapchainPresentMode(properties.present_modes);
        VkExtent2D extent = ChooseSwapchainExtent(properties.capabilities);
    }


#pragma endregion

    Graphics::Graphics(const gsl::not_null<GLFW_Window *> window): window_(window) {
#if !defined(NDEBUG)
        validation_ = true;
#endif
        InitializeVulkan();
    }

    Graphics::~Graphics() {
        if (device_ != nullptr) {
            vkDestroyDevice(device_, nullptr);
        }

        if (instance_ != nullptr) {
            if (surface_ != nullptr) {
                vkDestroySurfaceKHR(instance_, surface_, nullptr);
            }

            if (debug_messenger_ != nullptr) {
                vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
            }
            vkDestroyInstance(instance_, nullptr);
        }
    }

    void Graphics::InitializeVulkan() {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDeviceAndQueues();
        CreateSwapChain();
    }
} // veng
