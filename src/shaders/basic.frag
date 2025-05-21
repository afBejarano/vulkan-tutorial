#version 450
#include "common.glsl"

layout (location = 0) in vec2 vertex_uv;
layout (location = 1) in vec3 normal;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D texture_sampler;
layout(set = 1, binding = 0) uniform UniformBufferObject {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} ubo;

void main() {
    vec3 ambient = ubo.ambient;
    vec3 diffuse = texture(texture_sampler, vertex_uv).xyz;
    vec3 specular = ubo.specular;
    out_color = vec4(diffuse, 1.0);
}