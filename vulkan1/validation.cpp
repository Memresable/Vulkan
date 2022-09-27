#include <iostream>
#include <vector>

#include "validation.h"

bool Validation::checkValidationSupport(const std::vector<const char*>& validationLayers)
{
    uint32_t layerCount{};
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &layerCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> availableLayers(layerCount);
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &layerCount, availableLayers.data());

    for (const auto& validLayer : validationLayers)
    {
        bool isFound = false;
        for (const auto& layer : availableLayers)
        {
            if (strcmp(validLayer, layer.extensionName))
            {
                isFound = true;
                break;
            }
        }

        return isFound;
    }

    return false;
}

// For more ways to setup this:
// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap50.html#VK_EXT_debug_utils
void Validation::setupDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger, bool isDEBUG)
{
    if (!isDEBUG) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    Validation::populateDebugMessengerCreateInfo(createInfo);

    if (Validation::CreateDebugUtilsMessengerEXT(instance, &createInfo, VK_NULL_HANDLE, &debugMessenger) != VK_SUCCESS)
        throw std::exception("Failed to setup debug messenger");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Validation::debugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        std::cerr <<
        "Validation layer(ID: " << pCallbackData->messageIdNumber << "): " << pCallbackData->pMessage
        << std::endl;

    return VK_FALSE;
}

VkResult Validation::CreateDebugUtilsMessengerEXT
(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr
    (
        instance,
        "vkCreateDebugUtilsMessengerEXT"
    );
    if (func != VK_NULL_HANDLE)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Validation::DestroyDebugUtilsMessengerEXT
(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != VK_NULL_HANDLE) func(instance, debugMessenger, pAllocator);
}

void Validation::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}