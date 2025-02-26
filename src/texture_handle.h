//
// Created by andre on 23/02/2025.
//
#pragma once

#include <vulkan/vulkan.h>

namespace veng {
struct TextureHandle {
    VkImage image = VK_NULL_HANDLE;
    VkImageView image_view = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};
}
