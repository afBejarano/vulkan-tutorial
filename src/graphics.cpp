//
// Created by andre on 27/01/2025.
//

#include "graphics.h"
#include <iostream>
#include <precomp.h>
#include <cstring>
#include <set>
#include <spdlog/spdlog.h>
#include "object.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.cpp"
#include "uniform_transformations.h"
#include "utilities.h"
#include "vertex.h"
#include "spdlog/fmt/bundled/chrono.h"

#pragma region VK_FUNCITON_EXT_IMPL

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT"));

    if (function != nullptr)
        return function(instance, pCreateInfo, pAllocator, pDebugMessenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                           VkDebugUtilsMessengerEXT pDebugMessenger,
                                                           const VkAllocationCallbacks *pAllocator) {
    auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (function != nullptr)
        return function(instance, pDebugMessenger, pAllocator);
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
    return streq(layer_name, properties.layerName);
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

    VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "FirstEngine", VK_MAKE_VERSION(0, 0, 1), "VulkanEngine",
        VK_MAKE_VERSION(0, 0, 1), VK_API_VERSION_1_2
    };

    VkInstanceCreateInfo create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &app_info, 0, nullptr,
        static_cast<std::uint32_t>(required_extensions.size()), required_extensions.data()
    };

    const VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = GetCreateDebugMessengerInfo();

    if (validation_) {
        create_info.pNext = &messenger_create_info;
        create_info.enabledLayerCount = validationLayers.size();
        create_info.ppEnabledLayerNames = validationLayers.data();
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
    return streq(extension_name, extension.extensionName);
}

bool IsExtensionSupported(gsl::span<VkExtensionProperties> extensions, gsl::czstring extension_name) {
    return std::ranges::any_of(extensions, std::bind_front(&ExtensionMatchesName, extension_name));
}

bool Graphics::AreAllExtensionsSupported(const gsl::span<gsl::czstring> &extensions) {
    return std::ranges::all_of(extensions, std::bind_front(IsExtensionSupported, GetSupportedInstanceExtensions()));
}
#pragma endregion

#pragma region DEVICES_AND_QUEUES

Graphics::QueueFamilyIndices Graphics::FindQueueFamilies(VkPhysicalDevice device) const {
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

Graphics::SwapChainSupportDetails Graphics::FindSwapChainSupport(VkPhysicalDevice device) const {
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

std::vector<VkExtensionProperties> Graphics::GetDeviceAvailableExtensions(VkPhysicalDevice device) {
    std::uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

    return extensions;
}

bool IsDeviceExtensionWithinList(const std::vector<VkExtensionProperties> &extensions,
                                 gsl::czstring extension_name) {
    return std::ranges::any_of(extensions, [extension_name](const VkExtensionProperties &property) {
        return streq(extension_name, property.extensionName);
    });
}

bool Graphics::AreAllDeviceExtensionsSupported(VkPhysicalDevice device) {
    std::vector<VkExtensionProperties> extensions = GetDeviceAvailableExtensions(device);
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

    std::set unique_queue_families = {
        indices.graphics_family.value(), indices.present_family.value()
    };

    std::float_t queue_priorities = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

    for (std::uint32_t queue_family: unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, queue_family, 1, &queue_priorities
        };
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures required_features = {};
    required_features.depthBounds = true;
    required_features.depthClamp = true;

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

VkExtent2D Graphics::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
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

std::uint32_t Graphics::ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities) {
    std::uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }
    return image_count;
}


void Graphics::CreateSwapChain() {
    SwapChainSupportDetails properties = FindSwapChainSupport(physical_device_);

    surface_format_ = ChooseSwapchainSurfaceFormat(properties.formats);
    present_mode_ = ChooseSwapchainPresentMode(properties.present_modes);
    extent_ = ChooseSwapchainExtent(properties.capabilities);

    std::uint32_t image_count = ChooseImageCount(properties.capabilities);

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface_;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format_.format;
    create_info.imageColorSpace = surface_format_.colorSpace;
    create_info.imageExtent = extent_;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.presentMode = present_mode_;
    create_info.preTransform = properties.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    QueueFamilyIndices indices = FindQueueFamilies(physical_device_);

    std::array queue_family_indices = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };

    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = queue_family_indices.size();
        create_info.pQueueFamilyIndices = queue_family_indices.data();
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &swap_chain_) != VK_SUCCESS) {
        spdlog::error("Failed to create swap chain!");
        std::exit(EXIT_FAILURE);
    }

    std::uint32_t actual_image_count = 0;
    vkGetSwapchainImagesKHR(device_, swap_chain_, &actual_image_count, nullptr);

    swap_chain_images_.resize(actual_image_count);
    vkGetSwapchainImagesKHR(device_, swap_chain_, &actual_image_count, swap_chain_images_.data());
}

VkImageView Graphics::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) const {
    VkImageViewCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = aspect_flags;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkImageView image_view;

    if (vkCreateImageView(device_, &create_info, nullptr, &image_view) != VK_SUCCESS) {
        spdlog::error("Failed to create image view!");
        std::exit(EXIT_FAILURE);
    }

    return image_view;
}

void Graphics::CreateImageViews() {
    swap_chain_image_views_.resize(swap_chain_images_.size());

    auto iter = swap_chain_image_views_.begin();
    for (VkImage image: swap_chain_images_) {
        *iter = CreateImageView(image, surface_format_.format, VK_IMAGE_ASPECT_COLOR_BIT);
        std::advance(iter, 1);
    }
}

#pragma endregion

#pragma region GRAPHICS_PIPELINE

VkShaderModule Graphics::CreateShaderModule(gsl::span<std::uint8_t> buffer) const {
    if (buffer.empty()) {
        return VK_NULL_HANDLE;
    }

    VkShaderModule shader_module;

    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = buffer.size();
    create_info.pCode = reinterpret_cast<std::uint32_t *>(buffer.data());

    if (vkCreateShaderModule(device_, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return shader_module;
}

void Graphics::CreateGraphicsPipeline() {
    std::vector<uint8_t> basic_vertex_data = ReadFile("./basic.vert.spv");
    VkShaderModule vertex_shader_module = CreateShaderModule(basic_vertex_data);
    gsl::final_action _destroy_vertex_shader([this, vertex_shader_module]() {
        vkDestroyShaderModule(device_, vertex_shader_module, nullptr);
    });

    std::vector<uint8_t> basic_fragment_data = ReadFile("./basic.frag.spv");
    VkShaderModule fragment_shader_module = CreateShaderModule(basic_fragment_data);
    gsl::final_action _destroy_fragment_shader([this, fragment_shader_module]() {
        vkDestroyShaderModule(device_, fragment_shader_module, nullptr);
    });

    if (vertex_shader_module == VK_NULL_HANDLE || fragment_shader_module == VK_NULL_HANDLE) {
        spdlog::error("Failed to create shader modules!");
        exit(EXIT_FAILURE);
    }

    VkPipelineShaderStageCreateInfo vertex_create_info = {};
    vertex_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_create_info.module = vertex_shader_module;
    vertex_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_create_info = {};
    fragment_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_create_info.module = fragment_shader_module;
    fragment_create_info.pName = "main";

    std::array stage_infos = {vertex_create_info, fragment_create_info};

    std::array dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
    dynamic_state_create_info.pDynamicStates = dynamic_states.data();

    VkViewport viewport = GetViewport();

    VkRect2D scissor = GetScissor();

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    auto vertex_binding_description = oVertex::GetBindingDescription();
    auto vertex_attribute_descriptions = oVertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
    rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_create_info.depthClampEnable = VK_FALSE;
    rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_create_info.lineWidth = 1.0f;
    rasterization_create_info.cullMode = VK_CULL_MODE_NONE;
    rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_create_info.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.sampleShadingEnable = VK_FALSE;
    multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil_create_info = {};
    depthStencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil_create_info.depthTestEnable = VK_TRUE;
    depthStencil_create_info.depthWriteEnable = VK_TRUE;
    depthStencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil_create_info.depthBoundsTestEnable = VK_FALSE;
    depthStencil_create_info.minDepthBounds = 0.0f;
    depthStencil_create_info.maxDepthBounds = 1.0f;
    depthStencil_create_info.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    color_blend_attachment_state.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
    color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_create_info.logicOpEnable = VK_FALSE;
    color_blend_create_info.attachmentCount = 1;
    color_blend_create_info.pAttachments = &color_blend_attachment_state;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPushConstantRange model_matrix_range = {};
    model_matrix_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    model_matrix_range.offset = 0;
    model_matrix_range.size = sizeof(glm::mat4);

    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &model_matrix_range;

    std::array descriptor_set_layouts = {
        uniform_set_layout_,
        uniform_bp_set_layout_,
        texture_set_layout_,
    };

    pipeline_layout_create_info.setLayoutCount = descriptor_set_layouts.size();
    pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();

    if (vkCreatePipelineLayout(device_, &pipeline_layout_create_info, nullptr, &pipeline_layout_) != VK_SUCCESS) {
        spdlog::error("failed to create pipeline layout!");
        exit(EXIT_FAILURE);
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = stage_infos.size();
    pipeline_create_info.pStages = stage_infos.data();
    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_create_info;
    pipeline_create_info.pMultisampleState = &multisample_create_info;
    pipeline_create_info.pDepthStencilState = &depthStencil_create_info;
    pipeline_create_info.pColorBlendState = &color_blend_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    pipeline_create_info.layout = pipeline_layout_;
    pipeline_create_info.renderPass = render_pass_;
    pipeline_create_info.subpass = 0;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphics_pipeline_) !=
        VK_SUCCESS) {
        spdlog::error("failed to create graphics pipeline!");
        exit(EXIT_FAILURE);
    }
}

VkViewport Graphics::GetViewport() const {
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<std::float_t>(extent_.width);
    viewport.height = static_cast<float>(extent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    return viewport;
}

VkRect2D Graphics::GetScissor() const {
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extent_;

    return scissor;
}

void Graphics::CreateRenderPass() {
    VkAttachmentDescription color_attachments_description = {};
    color_attachments_description.format = surface_format_.format;
    color_attachments_description.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachments_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachments_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachments_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachments_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachments_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachments_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_reference = {};
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment_description = {};
    depth_attachment_description.format = VK_FORMAT_D32_SFLOAT;
    depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_reference = {};
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription main_subpass = {};
    main_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    main_subpass.colorAttachmentCount = 1;
    main_subpass.pColorAttachments = &color_attachment_reference;
    main_subpass.pDepthStencilAttachment = &depth_attachment_reference;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array attachments = {color_attachments_description, depth_attachment_description};

    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = attachments.size();
    render_pass_create_info.pAttachments = attachments.data();
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &main_subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &dependency;

    if (vkCreateRenderPass(device_, &render_pass_create_info, nullptr, &render_pass_) != VK_SUCCESS) {
        spdlog::error("failed to create render pass!");
        exit(EXIT_FAILURE);
    }
}

#pragma endregion

#pragma region DRAWING

void Graphics::CreateFramebuffers() {
    swap_chain_framebuffers_.resize(swap_chain_image_views_.size());
    for (uint32_t i = 0; i < swap_chain_image_views_.size(); i++) {
        std::array attachments = {swap_chain_image_views_[i], depth_texture_.image_view};

        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = render_pass_;
        framebuffer_create_info.attachmentCount = attachments.size();
        framebuffer_create_info.pAttachments = attachments.data();
        framebuffer_create_info.width = extent_.width;
        framebuffer_create_info.height = extent_.height;
        framebuffer_create_info.layers = 1;

        if (vkCreateFramebuffer(device_, &framebuffer_create_info, nullptr, &swap_chain_framebuffers_[i]) !=
            VK_SUCCESS) {
            spdlog::error("failed to create framebuffer!");
            exit(EXIT_FAILURE);
        }
    }
}

void Graphics::CreateCommandPool() {
    QueueFamilyIndices queue_families = FindQueueFamilies(physical_device_);

    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = queue_families.graphics_family.value();

    if (vkCreateCommandPool(device_, &command_pool_create_info, nullptr, &command_pool_) != VK_SUCCESS) {
        spdlog::error("failed to create command pool!");
        exit(EXIT_FAILURE);
    };
}

void Graphics::CreateCommandBuffer() {
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool_;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    for (Frame &buffered_frame: buffered_frames_) {
        if (vkAllocateCommandBuffers(device_, &command_buffer_allocate_info, &buffered_frame.command_buffer) !=
            VK_SUCCESS) {
            spdlog::error("failed to allocate command buffers!");
            exit(EXIT_FAILURE);
        }
    }
}

void Graphics::BeginCommands() const {
    vkResetCommandBuffer(buffered_frames_[current_frame_].command_buffer, 0);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(buffered_frames_[current_frame_].command_buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin command buffer");
    }

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass_;
    render_pass_info.framebuffer = swap_chain_framebuffers_[current_image_index_];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = extent_;

    std::array<VkClearValue, 2> clear_value = {};
    clear_value[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_value[1].depthStencil = {1.0f, 0};
    render_pass_info.clearValueCount = clear_value.size();
    render_pass_info.pClearValues = clear_value.data();

    vkCmdBeginRenderPass(buffered_frames_[current_frame_].command_buffer, &render_pass_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphics_pipeline_);
    const VkViewport viewport = GetViewport();
    const VkRect2D scissor = GetScissor();

    vkCmdSetViewport(buffered_frames_[current_frame_].command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(buffered_frames_[current_frame_].command_buffer, 0, 1, &scissor);
}

void Graphics::EndCommands() const {
    vkCmdEndRenderPass(buffered_frames_[current_frame_].command_buffer);
    if (vkEndCommandBuffer(buffered_frames_[current_frame_].command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Graphics::CreateSignals() {
    for (Frame &buffered_frame: buffered_frames_) {
        VkSemaphoreCreateInfo semaphore_create_info = {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device_, &semaphore_create_info, nullptr,
                              &buffered_frame.image_available_semaphore) != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }

        if (vkCreateSemaphore(device_, &semaphore_create_info, nullptr,
                              &buffered_frame.render_finished_semaphore) != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(device_, &fence_create_info, nullptr, &buffered_frame.still_rendering_fence) !=
            VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }
}

bool Graphics::BeginFrame() {
    vkWaitForFences(device_, 1, &buffered_frames_[current_frame_].still_rendering_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device_, 1, &buffered_frames_[current_frame_].still_rendering_fence);

    VkResult result = vkAcquireNextImageKHR(device_, swap_chain_, UINT64_MAX,
                                            buffered_frames_[current_frame_].image_available_semaphore,
                                            VK_NULL_HANDLE,
                                            &current_image_index_);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    BeginCommands();
    SetModelMatrix(glm::mat4(1.0f));

    return true;
}

void Graphics::EndFrame() {
    EndCommands();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &buffered_frames_[current_frame_].image_available_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage_flags;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &buffered_frames_[current_frame_].command_buffer;

    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &buffered_frames_[current_frame_].render_finished_semaphore;

    if (vkQueueSubmit(graphics_queue_, 1, &submit_info, buffered_frames_[current_frame_].still_rendering_fence) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to submit framebuffer command buffer submission");
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &buffered_frames_[current_frame_].render_finished_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swap_chain_;
    present_info.pImageIndices = &current_image_index_;

    VkResult result = vkQueuePresentKHR(present_queue_, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present!");
    }

    current_frame_ = (current_frame_++) % MAX_BUFFERED_FRAMES;
}

void Graphics::RecreateSwapchain() {
    glm::ivec2 window_size = window_->GetWindowSize();
    while (window_size.x == 0 || window_size.y == 0) {
        window_size = window_->GetFrameBufferSize();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device_);
    CleanupSwapchain();

    CreateSwapChain();
    CreateImageViews();
    CreateFramebuffers();
}

void Graphics::CleanupSwapchain() {
    if (device_ == VK_NULL_HANDLE)
        return;

    for (const auto framebuffer: swap_chain_framebuffers_)
        vkDestroyFramebuffer(device_, framebuffer, nullptr);

    for (auto image_view: swap_chain_image_views_)
        vkDestroyImageView(device_, image_view, nullptr);

    if (swap_chain_ != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
}


#pragma endregion

#pragma region BUFFERS

std::uint32_t Graphics::FindMemoryType(const std::uint32_t memory_type_bits,
                                       const VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties);
    const gsl::span<VkMemoryType> memory_types(memory_properties.memoryTypes, memory_properties.memoryTypeCount);

    for (uint32_t i = 0; i < memory_types.size(); i++)
        if (memory_type_bits & (1 << i) && memory_types[i].propertyFlags & properties)
            return i;

    throw std::runtime_error("failed to find suitable memory type!");
}

BufferHandle Graphics::CreateBuffer(const VkDeviceSize size, VkBufferUsageFlags usage,
                                    VkMemoryPropertyFlags properties) const {
    BufferHandle buffer = {};

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (VkResult res = vkCreateBuffer(device_, &buffer_info, VK_NULL_HANDLE, &buffer.buffer); res != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex buffer!" + std::to_string(res));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device_, buffer.buffer, &memory_requirements);

    const std::uint32_t memory_type_index = FindMemoryType(memory_requirements.memoryTypeBits, properties);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_type_index;

    if (vkAllocateMemory(device_, &memory_allocate_info, VK_NULL_HANDLE, &buffer.memory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!");

    vkBindBufferMemory(device_, buffer.buffer, buffer.memory, 0);

    return buffer;
}

BufferHandle Graphics::CreateoVertexBuffer(std::vector<oVertex> vertices) const {
    const VkDeviceSize size = vertices.size() * sizeof(oVertex);
    BufferHandle staging_handle = CreateBuffer(
        size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );

    void *mapped_data;
    vkMapMemory(device_, staging_handle.memory, 0, size, 0, &mapped_data);
    std::memcpy(mapped_data, vertices.data(), size);
    vkUnmapMemory(device_, staging_handle.memory);

    BufferHandle gpu_handle = CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

    VkBufferCopy copy_info = {0, 0, size};
    vkCmdCopyBuffer(transient_commands, staging_handle.buffer, gpu_handle.buffer, 1, &copy_info);

    EndTransientCommandBuffer(transient_commands);

    DestroyBuffer(staging_handle);

    return gpu_handle;
}

BufferHandle Graphics::CreateVertexBuffer(const gsl::span<Vertex> vertices) const {
    const VkDeviceSize size = vertices.size() * sizeof(Vertex);
    BufferHandle staging_handle = CreateBuffer(
        size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );

    void *mapped_data;
    vkMapMemory(device_, staging_handle.memory, 0, size, 0, &mapped_data);
    std::memcpy(mapped_data, vertices.data(), size);
    vkUnmapMemory(device_, staging_handle.memory);

    BufferHandle gpu_handle = CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

    VkBufferCopy copy_info = {0, 0, size};
    vkCmdCopyBuffer(transient_commands, staging_handle.buffer, gpu_handle.buffer, 1, &copy_info);

    EndTransientCommandBuffer(transient_commands);

    DestroyBuffer(staging_handle);

    return gpu_handle;
}

BufferHandle Graphics::CreateIndexBuffer(gsl::span<std::uint32_t> indices) const {
    const VkDeviceSize size = indices.size() * sizeof(std::uint32_t);
    BufferHandle staging_handle = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *mapped_data;
    vkMapMemory(device_, staging_handle.memory, 0, size, 0, &mapped_data);
    std::memcpy(mapped_data, indices.data(), size);
    vkUnmapMemory(device_, staging_handle.memory);

    BufferHandle gpu_handle = CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

    VkBufferCopy copy_info = {0, 0, size};
    vkCmdCopyBuffer(transient_commands, staging_handle.buffer, gpu_handle.buffer, 1, &copy_info);

    EndTransientCommandBuffer(transient_commands);

    DestroyBuffer(staging_handle);

    return gpu_handle;
}

BufferHandle Graphics::CreateIndexBuffer(const std::vector<std::uint32_t> &indices) const {
    const VkDeviceSize size = indices.size() * sizeof(std::uint32_t);
    BufferHandle staging_handle = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *mapped_data;
    vkMapMemory(device_, staging_handle.memory, 0, size, 0, &mapped_data);
    std::memcpy(mapped_data, indices.data(), size);
    vkUnmapMemory(device_, staging_handle.memory);

    BufferHandle gpu_handle = CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

    VkBufferCopy copy_info = {0, 0, size};
    vkCmdCopyBuffer(transient_commands, staging_handle.buffer, gpu_handle.buffer, 1, &copy_info);

    EndTransientCommandBuffer(transient_commands);

    DestroyBuffer(staging_handle);

    return gpu_handle;
}

void Graphics::DestroyBuffer(const BufferHandle handle) const {
    vkDeviceWaitIdle(device_);
    vkDestroyBuffer(device_, handle.buffer, VK_NULL_HANDLE);
    vkFreeMemory(device_, handle.memory, VK_NULL_HANDLE);
}

void Graphics::RenderBuffer(const BufferHandle buffer_handle, const std::uint32_t vertex_count) const {
    VkDeviceSize offset = 0;
    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout_, 0, 1, &buffered_frames_[current_frame_].uniform_set,
                            0, VK_NULL_HANDLE);
    vkCmdBindVertexBuffers(buffered_frames_[current_frame_].command_buffer, 0, 1, &buffer_handle.buffer, &offset);
    vkCmdDraw(buffered_frames_[current_frame_].command_buffer, vertex_count, 1, 0, 0);
    SetModelMatrix(glm::mat4(1.0f));
}

void Graphics::RenderIndexedBuffer(BufferHandle vertex_buffer, BufferHandle index_buffer,
                                   std::uint32_t index_count, std::int32_t index_offset) const {
    VkDeviceSize offset = 0;
    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout_, 0, 2, std::array{
                                buffered_frames_[current_frame_].uniform_set, buffered_frames_[current_frame_].bp_set
                            }.data(),
                            0, VK_NULL_HANDLE);
    vkCmdBindVertexBuffers(buffered_frames_[current_frame_].command_buffer, 0, 1, &vertex_buffer.buffer, &offset);
    vkCmdBindIndexBuffer(buffered_frames_[current_frame_].command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(buffered_frames_[current_frame_].command_buffer, index_count, 1, 0, index_offset, 0);
    SetModelMatrix(glm::mat4(1.0f));
}

void Graphics::RenderModel(BufferHandle vertex_buffer, BufferHandle index_buffer, object object,
                           const std::vector<TextureHandle> &textures, std::vector<Material_UBO> material_ubos,
                           const glm::mat4 &modelMatrix) const {
    int offset = 0;
    VkDeviceSize dOffset = 0;
    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout_, 0, 2, std::array{
                                buffered_frames_[current_frame_].uniform_set, buffered_frames_[current_frame_].bp_set
                            }.data(),
                            0, VK_NULL_HANDLE);
    vkCmdBindVertexBuffers(buffered_frames_[current_frame_].command_buffer, 0, 1, &vertex_buffer.buffer, &dOffset);
    vkCmdBindIndexBuffer(buffered_frames_[current_frame_].command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    SetModelMatrix(modelMatrix);
    for (const auto &[indices, materialId]: object.getMeshes()) {
        SetTexture(textures[materialId]);
        SetUbo(material_ubos[materialId]);
        vkCmdDrawIndexed(buffered_frames_[current_frame_].command_buffer, indices.size(), 1, offset, 0, 0);
        offset += static_cast<int>(indices.size());
    }
}

void Graphics::SetModelMatrix(const glm::mat4 &model) const {
    vkCmdPushConstants(buffered_frames_[current_frame_].command_buffer, pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(model), &model);
}

void Graphics::SetViewProjection(const glm::mat4 &view, const glm::mat4 &proj) const {
    UniformTransformations uniforms{view, proj};
    memcpy(buffered_frames_[current_frame_].uniform_buffer_location, &uniforms, sizeof(UniformTransformations));
}

void Graphics::SetUbo(Material_UBO &material_ubos) const {
    memcpy(buffered_frames_[current_frame_].bp_buffer_location, &material_ubos, sizeof(Material_UBO));
}

VkCommandBuffer Graphics::BeginTransientCommandBuffer() const {
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = command_pool_;
    allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;

    vkAllocateCommandBuffers(device_, &allocate_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void Graphics::EndTransientCommandBuffer(VkCommandBuffer command_buffer) const {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue_);
    vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer);
}

void Graphics::CreateUniformBuffers() {
    for (Frame &frame: buffered_frames_) {
        VkDeviceSize buffer_size = sizeof(UniformTransformations);
        frame.uniform_buffer_handle = CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkMapMemory(device_, frame.uniform_buffer_handle.memory, 0, buffer_size, 0, &frame.uniform_buffer_location);

        VkDeviceSize bp_size = sizeof(Material_UBO);
        frame.bp_buffer_handle = CreateBuffer(bp_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkMapMemory(device_, frame.bp_buffer_handle.memory, 0, bp_size, 0, &frame.bp_buffer_location);
    }
}

void Graphics::CreateDescriptorSetLayouts() {
    VkDescriptorSetLayoutBinding uniform_layout_binding = {};
    uniform_layout_binding.binding = 0;
    uniform_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_layout_binding.descriptorCount = 1;
    uniform_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

    VkDescriptorSetLayoutBinding uniform_bp_layout_binding = {};
    uniform_bp_layout_binding.binding = 0;
    uniform_bp_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_bp_layout_binding.descriptorCount = 1;
    uniform_bp_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo uniform_layout_info = {};
    uniform_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uniform_layout_info.bindingCount = 1;
    uniform_layout_info.pBindings = &uniform_layout_binding;

    VkDescriptorSetLayoutCreateInfo bp_uniform_layout_info = {};
    bp_uniform_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    bp_uniform_layout_info.bindingCount = 1;
    bp_uniform_layout_info.pBindings = &uniform_bp_layout_binding;

    if (vkCreateDescriptorSetLayout(device_, &uniform_layout_info, nullptr, &uniform_set_layout_) !=
        VK_SUCCESS) {
        spdlog::error("Failed to create descriptor set layout!");
        std::exit(EXIT_FAILURE);
    }

    if (vkCreateDescriptorSetLayout(device_, &bp_uniform_layout_info, nullptr, &uniform_bp_set_layout_) !=
        VK_SUCCESS) {
        spdlog::error("Failed to create descriptor set layout!");
        std::exit(EXIT_FAILURE);
    }

    VkDescriptorSetLayoutBinding texture_layout_binding = {};
    texture_layout_binding.binding = 0;
    texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_layout_binding.descriptorCount = 1;
    texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo texture_layout_info = {};
    texture_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    texture_layout_info.bindingCount = 1;
    texture_layout_info.pBindings = &texture_layout_binding;

    if (vkCreateDescriptorSetLayout(device_, &texture_layout_info, nullptr, &texture_set_layout_) !=
        VK_SUCCESS) {
        spdlog::error("Failed to create descriptor set layout!");
        std::exit(EXIT_FAILURE);
    }
}

void Graphics::CreateDescriptorPools() {
    VkDescriptorPoolSize uniform_pool_size = {};
    uniform_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_pool_size.descriptorCount = MAX_BUFFERED_FRAMES * 2;

    VkDescriptorPoolCreateInfo uniform_pool_info = {};
    uniform_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    uniform_pool_info.poolSizeCount = 1;
    uniform_pool_info.pPoolSizes = &uniform_pool_size;
    uniform_pool_info.maxSets = MAX_BUFFERED_FRAMES * 2;

    if (vkCreateDescriptorPool(device_, &uniform_pool_info, nullptr, &uniform_pool_) != VK_SUCCESS) {
        spdlog::error("Failed to create descriptor pool!");
        std::exit(EXIT_FAILURE);
    }

    VkPhysicalDeviceProperties device_properties = {};
    vkGetPhysicalDeviceProperties(physical_device_, &device_properties);

    VkDescriptorPoolSize texture_pool_size = {};
    texture_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_pool_size.descriptorCount = device_properties.limits.maxSamplerAllocationCount;

    VkDescriptorPoolCreateInfo texture_pool_info = {};
    texture_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    texture_pool_info.poolSizeCount = 1;
    texture_pool_info.pPoolSizes = &texture_pool_size;
    texture_pool_info.maxSets = device_properties.limits.maxSamplerAllocationCount;
    texture_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(device_, &texture_pool_info, nullptr, &texture_pool_) != VK_SUCCESS) {
        spdlog::error("Failed to create descriptor pool!");
        std::exit(EXIT_FAILURE);
    }
}

void Graphics::CreateDescriptorSets() {
    for (Frame &frame: buffered_frames_) {
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = uniform_pool_;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        descriptor_set_allocate_info.pSetLayouts = &uniform_set_layout_;

        VkDescriptorSetAllocateInfo bp_descriptor_set_allocate_info = {};
        bp_descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        bp_descriptor_set_allocate_info.descriptorPool = uniform_pool_;
        bp_descriptor_set_allocate_info.descriptorSetCount = 1;
        bp_descriptor_set_allocate_info.pSetLayouts = &uniform_bp_set_layout_;

        VkResult res = vkAllocateDescriptorSets(device_, &descriptor_set_allocate_info, &frame.uniform_set);

        if (res != VK_SUCCESS) {
            spdlog::error("Failed to allocate descriptor sets!");
            std::exit(EXIT_FAILURE);
        }

        if (vkAllocateDescriptorSets(device_, &bp_descriptor_set_allocate_info, &frame.bp_set) != VK_SUCCESS) {
            spdlog::error("Failed to allocate descriptor sets!");
            std::exit(EXIT_FAILURE);
        }

        VkDescriptorBufferInfo descriptor_buffer_info = {};
        descriptor_buffer_info.buffer = frame.uniform_buffer_handle.buffer;
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = sizeof(UniformTransformations);

        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = frame.uniform_set;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &descriptor_buffer_info;

        vkUpdateDescriptorSets(device_, 1, &descriptor_write, 0, nullptr);

        VkDescriptorBufferInfo bp_descriptor_buffer_info = {};
        bp_descriptor_buffer_info.buffer = frame.bp_buffer_handle.buffer;
        bp_descriptor_buffer_info.offset = 0;
        bp_descriptor_buffer_info.range = sizeof(Material_UBO);

        VkWriteDescriptorSet bp_descriptor_write = {};
        bp_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bp_descriptor_write.dstSet = frame.bp_set;
        bp_descriptor_write.dstBinding = 0;
        bp_descriptor_write.dstArrayElement = 0;
        bp_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bp_descriptor_write.descriptorCount = 1;
        bp_descriptor_write.pBufferInfo = &bp_descriptor_buffer_info;

        vkUpdateDescriptorSets(device_, 1, &bp_descriptor_write, 0, nullptr);
    }
}

#pragma endregion

#pragma region TEXTURE

void Graphics::CreateTextureSampler() {
    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable = VK_FALSE;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = 0.0f;
    sampler_create_info.maxAnisotropy = 1.0f;

    if (vkCreateSampler(device_, &sampler_create_info, nullptr, &texture_sampler_) != VK_SUCCESS) {
        spdlog::error("failed to create texture sampler!");
        std::exit(EXIT_FAILURE);
    }
}

void Graphics::CreateDepthResources() {
    VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
    depth_texture_ = CreateImage({extent_.width, extent_.height}, depth_format,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    depth_texture_.image_view = CreateImageView(depth_texture_.image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

TextureHandle Graphics::CreateImage(const glm::ivec2 extent, VkFormat image_format, VkBufferUsageFlags usage,
                                    VkMemoryPropertyFlags properties) const {
    TextureHandle handle = {};

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.usage = usage;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = extent.x;
    image_create_info.extent.height = extent.y;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = image_format;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.flags = 0;

    if (vkCreateImage(device_, &image_create_info, VK_NULL_HANDLE, &handle.image) != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(device_, handle.image, &memory_requirements);

    const std::uint32_t memory_type_index = FindMemoryType(memory_requirements.memoryTypeBits, properties);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_type_index;

    if (vkAllocateMemory(device_, &memory_allocate_info, VK_NULL_HANDLE, &handle.memory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");

    vkBindImageMemory(device_, handle.image, handle.memory, 0);

    return handle;
}

TextureHandle Graphics::CreateTexture(gsl::czstring path) const {
    glm::ivec2 image_extents;
    std::int32_t channels;
    std::vector<std::uint8_t> data = ReadFile(path);

    stbi_uc *pixel_data = stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &image_extents.x,
                                                &image_extents.y, &channels,
                                                STBI_rgb_alpha);

    VkDeviceSize size = image_extents.x * image_extents.y * 4;
    BufferHandle staging_buffer = CreateBuffer(size,
                                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *data_location;
    vkMapMemory(device_, staging_buffer.memory, 0, size, 0, &data_location);
    memcpy(data_location, pixel_data, size);
    vkUnmapMemory(device_, staging_buffer.memory);

    stbi_image_free(pixel_data);

    TextureHandle texture_handle = CreateImage(image_extents,
                                               VK_FORMAT_R8G8B8A8_SRGB,
                                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    TransitionImageLayout(texture_handle.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(staging_buffer.buffer, texture_handle.image, image_extents);
    TransitionImageLayout(texture_handle.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    texture_handle.image_view = CreateImageView(texture_handle.image, VK_FORMAT_R8G8B8A8_SRGB,
                                                VK_IMAGE_ASPECT_COLOR_BIT);

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.descriptorPool = texture_pool_;
    descriptor_set_allocate_info.descriptorSetCount = 1;
    descriptor_set_allocate_info.pSetLayouts = &texture_set_layout_;

    if (vkAllocateDescriptorSets(device_, &descriptor_set_allocate_info, &texture_handle.descriptor_set) !=
        VK_SUCCESS) {
        spdlog::error("Failed to allocate descriptor sets!");
        std::exit(EXIT_FAILURE);
    }

    VkDescriptorImageInfo descriptor_image_info = {};
    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor_image_info.imageView = texture_handle.image_view;
    descriptor_image_info.sampler = texture_sampler_;

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = texture_handle.descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &descriptor_image_info;

    vkUpdateDescriptorSets(device_, 1, &descriptor_write, 0, nullptr);

    DestroyBuffer(staging_buffer);

    return texture_handle;
}

void Graphics::TransitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) const {
    VkCommandBuffer local_command_buffer = BeginTransientCommandBuffer();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags src_stage_flags = 0;
    VkPipelineStageFlags dst_stage_flags = 0;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout ==
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage_flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout ==
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCmdPipelineBarrier(local_command_buffer, src_stage_flags, dst_stage_flags, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);

    EndTransientCommandBuffer(local_command_buffer);
}

void Graphics::CopyBufferToImage(VkBuffer buffer, VkImage image, glm::ivec2 size) const {
    VkCommandBuffer local_command_buffer = BeginTransientCommandBuffer();

    VkBufferImageCopy copy_region = {};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageOffset = {0, 0, 0};
    copy_region.imageExtent = {static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y), 1};

    vkCmdCopyBufferToImage(local_command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    EndTransientCommandBuffer(local_command_buffer);
}

void Graphics::DestroyTexture(const TextureHandle &handle) const {
    vkDeviceWaitIdle(device_);
    vkFreeDescriptorSets(device_, texture_pool_, 1, &handle.descriptor_set);
    vkDestroyImageView(device_, handle.image_view, nullptr);
    vkDestroyImage(device_, handle.image, nullptr);
    vkFreeMemory(device_, handle.memory, nullptr);
}

void Graphics::SetTexture(const TextureHandle &handle) const {
    vkCmdBindDescriptorSets(buffered_frames_[current_frame_].command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout_, 2, 1,
                            &handle.descriptor_set, 0, VK_NULL_HANDLE);
}

#pragma endregion

#pragma region CLASS

Graphics::Graphics(const gsl::not_null<GLFW_Window *> window): window_(window) {
#if !defined(NDEBUG)
    validation_ = true;
#endif
    InitializeVulkan();
}

Graphics::~Graphics() {
    if (device_ != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device_);

        CleanupSwapchain();

        DestroyTexture(depth_texture_);

        if (texture_sampler_ != VK_NULL_HANDLE)
            vkDestroySampler(device_, texture_sampler_, nullptr);

        if (texture_pool_ != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(device_, texture_pool_, VK_NULL_HANDLE);

        if (texture_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(device_, texture_set_layout_, VK_NULL_HANDLE);

        if (uniform_pool_ != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(device_, uniform_pool_, VK_NULL_HANDLE);

        for (Frame &buffered_frame: buffered_frames_) {
            DestroyBuffer(buffered_frame.uniform_buffer_handle);
            DestroyBuffer(buffered_frame.bp_buffer_handle);

            if (buffered_frame.image_available_semaphore != VK_NULL_HANDLE)
                vkDestroySemaphore(device_, buffered_frame.image_available_semaphore, VK_NULL_HANDLE);

            if (buffered_frame.render_finished_semaphore != VK_NULL_HANDLE)
                vkDestroySemaphore(device_, buffered_frame.render_finished_semaphore, VK_NULL_HANDLE);

            if (buffered_frame.still_rendering_fence != VK_NULL_HANDLE)
                vkDestroyFence(device_, buffered_frame.still_rendering_fence, VK_NULL_HANDLE);
        }

        if (uniform_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(device_, uniform_set_layout_, VK_NULL_HANDLE);

        if (uniform_bp_set_layout_ != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(device_, uniform_bp_set_layout_, VK_NULL_HANDLE);

        if (command_pool_ != VK_NULL_HANDLE)
            vkDestroyCommandPool(device_, command_pool_, VK_NULL_HANDLE);

        if (graphics_pipeline_ != VK_NULL_HANDLE)
            vkDestroyPipeline(device_, graphics_pipeline_, VK_NULL_HANDLE);

        if (pipeline_layout_ != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(device_, pipeline_layout_, VK_NULL_HANDLE);

        if (render_pass_ != VK_NULL_HANDLE)
            vkDestroyRenderPass(device_, render_pass_, VK_NULL_HANDLE);

        vkDestroyDevice(device_, VK_NULL_HANDLE);
    }

    if (instance_ != VK_NULL_HANDLE) {
        if (surface_ != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(instance_, surface_, VK_NULL_HANDLE);

        if (debug_messenger_ != VK_NULL_HANDLE)
            vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, VK_NULL_HANDLE);

        vkDestroyInstance(instance_, VK_NULL_HANDLE);
    }
}

void Graphics::InitializeVulkan() {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDeviceAndQueues();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayouts();
    CreateGraphicsPipeline();
    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSignals();
    CreateUniformBuffers();
    CreateDescriptorPools();
    CreateDescriptorSets();
    CreateTextureSampler();
    TransitionImageLayout(depth_texture_.image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

#pragma endregion
} // veng
