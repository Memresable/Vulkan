#ifndef VALIDATION_H
#define VALIDATION_H

#include "C:\VulkanSDK\1.3.224.1\Include\vulkan\vulkan.h"
#include "utility.h"

class Validation
{
public:
    bool checkValidationSupport(const std::vector<const char*>& validationLayers);

    void setupDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger, bool isDEBUG);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
    (
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    VkResult CreateDebugUtilsMessengerEXT
    (
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    );

    void DestroyDebugUtilsMessengerEXT
    (
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
    );

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
};
#endif