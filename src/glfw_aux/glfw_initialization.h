#pragma once
namespace veng {
    struct GLFWInitialization {
    public:
        GLFWInitialization();

        ~GLFWInitialization();

        GLFWInitialization(const GLFWInitialization &) = delete;

        GLFWInitialization &operator=(const GLFWInitialization &) = delete;
    };
} // namespace veng
