#version 450
#include "common.glsl"

layout (location = 0) in vec3 input_position;
layout (location = 1) in vec3 input_color;

layout (location = 0) out vec4 color;

layout (push_constant) uniform Model {
    mat4 transformation;
} model;

void main() {
    gl_Position = camera.proj * camera.view * model.transformation * vec4(input_position, 1.0);
    color = vec4(input_color, 1.0);
}