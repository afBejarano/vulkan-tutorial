#version 450
#include "common.glsl"

layout (location = 0) in vec3 input_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 input_uv;

layout (location = 0) out vec2 vertex_uv;
layout (location = 1) out vec3 oNormal;

layout (push_constant) uniform Model {
    mat4 transformation;
} model;

void main() {
    gl_Position = camera.proj * camera.view * model.transformation * vec4(input_position, 1.0);
    vertex_uv = input_uv;
    oNormal = normal;
}