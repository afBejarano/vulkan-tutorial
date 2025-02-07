//
// Created by andre on 27/01/2025.
//

#include "graphics.h"

#include <iostream>
#include <precomp.h>
#include <cstring>

namespace veng {
    static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *user_data) {
        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            std::cout << "! ERROR: [InstanceCreation] (Error Code: " << pCallbackData->messageIdNumber << " ) " <<
                    pCallbackData->pMessage << std::endl;
        } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cout << "! WARNING: [InstanceCreation] (Warning Code: " << pCallbackData->messageIdNumber << " ) " <<
                    pCallbackData->pMessage << std::endl;
        } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            std::cout << "INFO: [InstanceCreation] (Info Code: " << pCallbackData->messageIdNumber << " ) " <<
                    pCallbackData->pMessage << std::endl;
        } else {
            std::cout << "VERBOSE: [InstanceCreation] (Warning Code: " << pCallbackData->messageIdNumber << " ) " <<
                    pCallbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }

    static VkDebugUtilsMessengerCreateInfoEXT GetCreateDebugMessengerInfo() {
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
        messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        messengerCreateInfo.pfnUserCallback = ValidationCallback;
        messengerCreateInfo.pUserData = nullptr;

        return messengerCreateInfo;
    }

    Graphics::Graphics(const gsl::not_null<GLFW_Window *> window): window_(window) {
#if !defined(NDEBUG)
        validation_ = true;
#endif
        InitializeVulkan();
    }

    Graphics::~Graphics() {
        if (instance_ != nullptr) {
            vkDestroyInstance(instance_, nullptr);
        }
    }

    void Graphics::InitializeVulkan() {
        CreateInstance();
    }

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

        VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = GetCreateDebugMessengerInfo();

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

    std::vector<gsl::czstring> Graphics::GetRequiredInstanceExtensions() {
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
} // veng
