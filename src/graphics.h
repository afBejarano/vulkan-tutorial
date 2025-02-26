//
// Created by andre on 27/01/2025.
//
#pragma once
#include <optional>
#include <vulkan/vulkan.h>

#include <glfw_aux/glfw_window.h>
#include <gsl/algorithm>

#include "buffer_handle.h"
#include "vertex.h"
#include "texture_handle.h"

namespace veng {
class Graphics final {
public:
    explicit Graphics(gsl::not_null<GLFW_Window *> window);

    ~Graphics();

    bool BeginFrame();
    void SetModelMatrix(glm::mat4 model);
    void SetViewProjection(glm::mat4 view, glm::mat4 proj);
    void SetTexture(TextureHandle handle);
    void RenderBuffer(BufferHandle buffer_handle, std::uint32_t vertex_count);
    void RenderIndexedBuffer(BufferHandle vertex_buffer, BufferHandle index_buffer, std::uint32_t index_count);
    void EndFrame();

    BufferHandle CreateVertexBuffer(gsl::span<Vertex> vertices);
    BufferHandle CreateIndexBuffer(gsl::span<std::uint32_t> indices);
    void DestroyBuffer(BufferHandle handle);
    TextureHandle CreateTexture(gsl::czstring path);
    void DestroyTexture(TextureHandle handle);

private:
    struct QueueFamilyIndices {
        std::optional<std::uint32_t> graphics_family = std::nullopt;
        std::optional<std::uint32_t> present_family = std::nullopt;

        bool IsValid() const {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};

        bool IsValid() const {
            return !formats.empty() && !present_modes.empty();
        }
    };


    void InitializeVulkan();

    // Initialization
    void CreateInstance();
    void SetupDebugMessenger();
    void PickPhysicalDevice();
    void CreateLogicalDeviceAndQueues();
    void CreateSurface();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void CreateSignals();
    void CreateDescriptorSetLayouts();
    void CreateDescriptorPools();
    void CreateDescriptorSets();

    void RecreateSwapchain();
    void CleanupSwapchain();
    void CreateTextureSampler();

    // Rendering

    void BeginCommands();
    void EndCommands();

    std::vector<gsl::czstring> GetRequiredInstanceExtensions() const;
    static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
    static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
    static bool AreAllExtensionsSupported(const gsl::span<gsl::czstring> &extensions);

    static std::vector<VkLayerProperties> GetSupportedValidationLayers();
    static bool AreAllLayersSupported(const gsl::span<gsl::czstring> &extensions);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails FindSwapChainSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    std::vector<VkPhysicalDevice> GetPhysicalDevices() const;
    std::vector<VkExtensionProperties> GetDeviceAvaliableExtensions(VkPhysicalDevice device);
    bool AreAllDeviceExtensionsSupported(VkPhysicalDevice device);

    VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(gsl::span<VkSurfaceFormatKHR> formats);
    VkPresentModeKHR ChooseSwapchainPresentMode(gsl::span<VkPresentModeKHR> present_modes);
    VkExtent2D ChooseSwapchainExtent(VkSurfaceCapabilitiesKHR capabilities);
    std::uint32_t ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities);

    VkShaderModule CreateShaderModule(gsl::span<std::uint8_t> buffer) const;

    std::uint32_t FindMemoryType(std::uint32_t memory_type_bits, VkMemoryPropertyFlags properties);

    BufferHandle CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    VkCommandBuffer BeginTransientCommandBuffer();
    void EndTransientCommandBuffer(VkCommandBuffer command_buffer);
    void CreateUniformBuffers();

    TextureHandle CreateImage(glm::ivec2 extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    VkViewport GetViewport() const;
    VkRect2D GetScissor() const;

    std::array<gsl::czstring, 1> required_device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_;

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surface_format_;
    VkPresentModeKHR present_mode_;
    VkExtent2D extent_;

    std::vector<VkImage> swap_chain_images_;
    std::vector<VkImageView> swap_chain_image_views_;
    std::vector<VkFramebuffer> swap_chain_framebuffers_;

    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;

    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;

    VkSemaphore image_available_semaphore_ = VK_NULL_HANDLE;
    VkSemaphore render_finished_semaphore_ = VK_NULL_HANDLE;
    VkFence still_rendering_fence_ = VK_NULL_HANDLE;

    std::uint32_t current_image_index_ = 0;

    VkDescriptorSetLayout uniform_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool uniform_pool_ = VK_NULL_HANDLE;
    VkDescriptorSet uniform_set_ = VK_NULL_HANDLE;
    BufferHandle uniform_buffer_handle_;
    void *uniform_buffer_location_;

    VkDescriptorSetLayout texture_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool texture_pool_ = VK_NULL_HANDLE;
    VkSampler texture_sampler_ = VK_NULL_HANDLE;

    gsl::not_null<GLFW_Window *> window_;
    bool validation_ = false;
};
} // veng
