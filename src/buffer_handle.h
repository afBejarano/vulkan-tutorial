//
// Created by andre on 23/02/2025.
//
#pragma once

#include <vulkan/vulkan.h>

namespace veng {

struct BufferHandle {
    VkBuffer buffer;
    VkDeviceMemory memory;
};

}
