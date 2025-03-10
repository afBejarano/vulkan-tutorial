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
struct Frame {
    VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
    VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;
    VkFence still_rendering_fence = VK_NULL_HANDLE;

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;

    VkDescriptorSet uniform_set = VK_NULL_HANDLE;
    BufferHandle uniform_buffer_handle;
    void *uniform_buffer_location;
};

class Graphics final {
public:
    explicit Graphics(gsl::not_null<GLFW_Window *> window);

    ~Graphics();

    bool BeginFrame();
    void SetModelMatrix(const glm::mat4 &model) const;
    void SetViewProjection(const glm::mat4 &view, const glm::mat4 &proj) const;
    void SetTexture(const TextureHandle &handle) const;
    void RenderBuffer(BufferHandle buffer_handle, std::uint32_t vertex_count) const;
    void RenderIndexedBuffer(BufferHandle vertex_buffer, BufferHandle index_buffer, std::uint32_t index_count) const;
    void EndFrame();

    [[nodiscard]] BufferHandle CreateVertexBuffer(gsl::span<Vertex> vertices) const;
    [[nodiscard]] BufferHandle CreateIndexBuffer(gsl::span<std::uint32_t> indices) const;
    void DestroyBuffer(BufferHandle handle) const;
    TextureHandle CreateTexture(gsl::czstring path) const;
    void DestroyTexture(const TextureHandle &handle) const;

private:
    struct QueueFamilyIndices {
        std::optional<std::uint32_t> graphics_family = std::nullopt;
        std::optional<std::uint32_t> present_family = std::nullopt;

        [[nodiscard]] bool IsValid() const {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};

        [[nodiscard]] bool IsValid() const {
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
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) const;
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
    void CreateDepthResources();

    // Rendering

    void BeginCommands() const;
    void EndCommands() const;

    [[nodiscard]] std::vector<gsl::czstring> GetRequiredInstanceExtensions() const;
    static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
    static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
    static bool AreAllExtensionsSupported(const gsl::span<gsl::czstring> &extensions);

    static std::vector<VkLayerProperties> GetSupportedValidationLayers();
    static bool AreAllLayersSupported(const gsl::span<gsl::czstring> &extensions);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
    SwapChainSupportDetails FindSwapChainSupport(VkPhysicalDevice device) const;
    bool IsDeviceSuitable(VkPhysicalDevice device);
    [[nodiscard]] std::vector<VkPhysicalDevice> GetPhysicalDevices() const;
    static std::vector<VkExtensionProperties> GetDeviceAvailableExtensions(VkPhysicalDevice device);
    bool AreAllDeviceExtensionsSupported(VkPhysicalDevice device);

    static VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(gsl::span<VkSurfaceFormatKHR> formats);
    static VkPresentModeKHR ChooseSwapchainPresentMode(gsl::span<VkPresentModeKHR> present_modes);
    [[nodiscard]] VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
    static std::uint32_t ChooseImageCount(const VkSurfaceCapabilitiesKHR &capabilities);

    [[nodiscard]] VkShaderModule CreateShaderModule(gsl::span<std::uint8_t> buffer) const;

    [[nodiscard]] std::uint32_t FindMemoryType(std::uint32_t memory_type_bits, VkMemoryPropertyFlags properties) const;

    [[nodiscard]] BufferHandle CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
    [[nodiscard]] VkCommandBuffer BeginTransientCommandBuffer() const;
    void EndTransientCommandBuffer(VkCommandBuffer command_buffer) const;
    void CreateUniformBuffers();

    [[nodiscard]] TextureHandle CreateImage(glm::ivec2 extent, VkFormat image_format, VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags properties) const;
    void TransitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) const;
    void CopyBufferToImage(VkBuffer buffer, VkImage image, glm::ivec2 size) const;

    [[nodiscard]] VkViewport GetViewport() const;
    [[nodiscard]] VkRect2D GetScissor() const;

    std::array<gsl::czstring, 1> required_device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_{};

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surface_format_{};
    VkPresentModeKHR present_mode_{};
    VkExtent2D extent_{};

    std::vector<VkImage> swap_chain_images_;
    std::vector<VkImageView> swap_chain_image_views_;
    std::vector<VkFramebuffer> swap_chain_framebuffers_;

    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkPipeline graphics_pipeline_ = VK_NULL_HANDLE;

    VkCommandPool command_pool_ = VK_NULL_HANDLE;

    std::uint32_t current_image_index_ = 0;

    VkDescriptorSetLayout uniform_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool uniform_pool_ = VK_NULL_HANDLE;

    VkDescriptorSetLayout texture_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool texture_pool_ = VK_NULL_HANDLE;
    VkSampler texture_sampler_ = VK_NULL_HANDLE;
    TextureHandle depth_texture_;

    std::array<Frame, MAX_BUFFERED_FRAMES> buffered_frames_;
    std::int32_t current_frame_ = 0;

    gsl::not_null<GLFW_Window *> window_;
    bool validation_ = false;
};
} // veng
