//
// Created by andre on 23/02/2025.
//
#pragma once

#include <array>
#include <vulkan/vulkan.h>

#include "glm/vec3.hpp"

namespace veng {
struct Vertex {
    Vertex(glm::vec3 _position, glm::vec2 _uv) : position(_position), uv(_uv) {};
    Vertex() : position({0.0, 0.0, 0.0}), uv({0.0, 0.0}) {};

    glm::vec3 position;
    glm::vec2 uv;

    static VkVertexInputBindingDescription GetBindingDescription() {
        return VkVertexInputBindingDescription{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
        constexpr VkVertexInputAttributeDescription position_attribute_description = {
            0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)
        };

        constexpr VkVertexInputAttributeDescription color_attribute_description = {
            1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)
        };

        return {position_attribute_description, color_attribute_description};
    }
};
}
