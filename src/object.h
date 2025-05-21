#pragma once
#include <filesystem>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace veng {
struct Mesh {
    std::vector<int> indices;
    int materialId;
};

struct Material_UBO {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

struct material {
    Material_UBO bp_material_ubo_;
    std::string diffuse_texName;
};

struct oVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription() {
        return VkVertexInputBindingDescription{0, sizeof(oVertex), VK_VERTEX_INPUT_RATE_VERTEX};
    }

    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
        constexpr VkVertexInputAttributeDescription position_attribute_description = {
            0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(oVertex, position)
        };

        constexpr VkVertexInputAttributeDescription normal_attribute_description = {
            1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(oVertex, normal)
        };

        constexpr VkVertexInputAttributeDescription color_attribute_description = {
            2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(oVertex, texCoord)
        };

        return {position_attribute_description, normal_attribute_description, color_attribute_description};
    }

    bool operator==(const oVertex &other) const {
        return position == other.position && normal == other.normal && texCoord == other.texCoord;
    }
};

struct VertexHash {
    size_t operator()(const oVertex &vertex) const {
        const size_t h1 = std::hash<float>()(vertex.position[0]);
        const size_t h2 = std::hash<float>()(vertex.position[1]);
        const size_t h3 = std::hash<float>()(vertex.position[2]);
        const size_t h4 = std::hash<float>()(vertex.normal[0]);
        const size_t h5 = std::hash<float>()(vertex.normal[1]);
        const size_t h6 = std::hash<float>()(vertex.normal[2]);
        const size_t h7 = std::hash<float>()(vertex.texCoord[0]);
        const size_t h8 = std::hash<float>()(vertex.texCoord[1]);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5) ^ (h7 << 6) ^ (h8 << 7);
    }
};

class object {
public:
    object(const char *obj, const char *basedir) : obj_(obj), basedir_(basedir) {
        loadObj();
    };

    std::vector<oVertex> getOVertices() {
        return vertices_;
    };

    std::vector<Mesh> getMeshes() {
        return meshes_;
    }

    std::vector<Material_UBO> getMaterialUBOs() {
        std::vector<Material_UBO> materialUBOs;
        for (auto material: materials_)
            materialUBOs.push_back(material.bp_material_ubo_);
        return materialUBOs;
    }

    std::vector<std::string> getTextures() {
        std::vector<std::string> textures;
        for (auto material: materials_) {
            textures.push_back(material.diffuse_texName);
        }
        return textures;
    }

    std::vector<std::uint32_t> getIndices() {
        std::vector<std::uint32_t> indices;
        for (const auto &mesh: meshes_)
            for (auto vertex: mesh.indices)
                indices.push_back(vertex);
        return indices;
    }

private:
    const char *obj_;
    const char *basedir_;
    std::vector<oVertex> vertices_;
    std::vector<material> materials_;
    std::vector<Mesh> meshes_;
    void loadObj();
};
}
