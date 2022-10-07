#ifndef VULKAN_H
#define VULKAN_H

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <set>

#define NOMINMAX
#include <limits>

#include <algorithm>
#include <fstream>

// TODO: Get glfw be able to identify Vulkan through: #define GLFW_INCLUDE_VULKAN
#include "C:\VulkanSDK\1.3.224.1\Include\vulkan\vulkan.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "validation.h"
#include "utility.h"

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
        std::optional<uint32_t> ms_presentFamily;

        bool isComplete()
        {
            return
                ms_graphicsFamily.has_value() &&
                ms_presentFamily.has_value();
        }
	};

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR ms_capabilities;
        VkExtent2D ms_extent;
        std::vector<VkSurfaceFormatKHR> ms_formats;
        std::vector<VkPresentModeKHR> ms_presentModes;
    };

private:
// Functions
    void initWindow();
    std::vector<const char*> getRequiredExtensions();
    SwapChainSupportDetails getSwapchainDetails
    (
        const VkSurfaceCapabilitiesKHR& capabilities,
        const std::vector<VkSurfaceFormatKHR>& availableFormats,
        const std::vector<VkPresentModeKHR>& availablePresents
    );
    SwapChainSupportDetails querySwapchainSupport(const VkPhysicalDevice& device);
    void createInstance();
    QueueFamilies findQueueFamilies(const VkPhysicalDevice& device);
    bool checkExtensionSupport(const VkPhysicalDevice& device);
    bool isDeviceSuitable(const VkPhysicalDevice& device);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
	void createRenderPass();
	static std::vector<char> loadSPRV(const std::string& fileName);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createGraphicsPipeline();
    void mainLoop();

private:
// Objects
    VkDebugUtilsMessengerEXT m_debugMessenger;
    const std::vector<const char*> m_deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    const std::vector<const char*> m_validationLayers{ "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
    bool m_isDEBUG = false;
#else
    bool m_isDEBUG = true;
#endif
	VkPipeline m_graphicsPipeline;
	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;

    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkImage> m_swapchainImages;
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_swapchainExtent;
    VkSwapchainKHR m_swapchain;

    VkQueue m_presentQueue;
    VkQueue m_graphicsQueue;
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface;

    VkInstance m_instance;
    GLFWwindow* m_window;
    const uint32_t m_SCR_WIDTH = 800;
    const uint32_t m_SCR_HEIGHT = 600;
};

#endif