//
// Created by andre on 27/01/2025.
//

#include "graphics.h"

#include <iostream>
#include <precomp.h>

namespace veng {
    Graphics::Graphics(const gsl::not_null<GLFW_Window *> window): window_(window) {
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
        gsl::span<gsl::czstring> suggested_extensions = CreateInstanceExtensions();

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
        create_info.enabledExtensionCount = suggested_extensions.size();
        create_info.ppEnabledExtensionNames = suggested_extensions.data();
        create_info.enabledLayerCount = 0;
        create_info.ppEnabledLayerNames = nullptr;

        if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
            std::cout << "Failed to create instance" << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    gsl::span<gsl::czstring> Graphics::CreateInstanceExtensions() {
        std::uint32_t extension_count = 0;
        gsl::czstring *extension_names = glfwGetRequiredInstanceExtensions(&extension_count);
        return {extension_names, extension_count};
    }
} // veng
