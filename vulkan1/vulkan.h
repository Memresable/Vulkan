#ifndef VULKAN_H
#define VULKAN_H

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <optional>

#include "C:\VulkanSDK\1.3.224.1\Include\vulkan\vulkan.h"
#include <GLFW/glfw3.h>

#include "validation.h"

class VulkanEngine : public Validation
{
public:
// Main
    VulkanEngine();
    ~VulkanEngine();
    void run();

// Structs
    struct QueueFamilies
    {
        std::optional<uint32_t> ms_graphicsFamily;
        bool hasValue() { return ms_graphicsFamily.has_value(); }
    };

private:
// Functions
    void initWindow();
    std::vector<const char*> getRequiredExtensions();
    void createInstance();
    QueueFamilies findQueueFamilies(const VkPhysicalDevice& device);
    bool isDeviceSuitable(const VkPhysicalDevice& device);
    void pickPhysicalDevice();
    void mainLoop();

private:
// Objects
    VkDebugUtilsMessengerEXT m_debugMessenger;
    std::vector<const char*> m_validationLayers{"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
    bool m_isDEBUG = false;
#else
    bool m_isDEBUG = true;
#endif
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE}; // if m_instance destroyed, m_physicalDevice destroyed
    VkInstance m_instance;
    GLFWwindow* m_window;
    const uint32_t m_SCR_WIDTH = 800;
    const uint32_t m_SCR_HEIGHT = 600;
};

#endif