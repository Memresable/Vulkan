#ifndef _VULKAN_COMPATIBILITY_H
#define _VULKAN_COMPATIBILITY_H

#include "../utilities/general_utilities.h"
#include "../utilities/types.h"
#include "../utilities/unique_array.h"

#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"
#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan_win32.h"

void
VK_createSurface(VkInstance f_instance, VkSurfaceKHR* f_surface, WindowInfo* f_windowInfo)
{
    VkWin32SurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = *f_windowInfo->handle;
    createInfo.hinstance = GetModuleHandle(NULL);
    
    MEMRE_ASSERT(vkCreateWin32SurfaceKHR(f_instance, &createInfo, NULL, f_surface) != VK_SUCCESS,
                 "Failed to create window surface\n");
}

typedef struct
{
    union
    {
        struct
        {
            uint32_t* graphicsFamily;
            uint32_t* presentFamily;
        };
        struct
        {
            uint32_t* array[2];
        };
    };
} QueueFamilyIndices;

QueueFamilyIndices
VK_findQueueFamilies(VkPhysicalDevice f_device, VkSurfaceKHR f_surface)
{
    QueueFamilyIndices result = {0};
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(f_device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilies = ALLOCATE_MEMORY(VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(f_device, &queueFamilyCount, &queueFamilies[0]);
    
    for(uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            result.graphicsFamily = ALLOCATE_MEMORY(uint32_t, 1);
            *result.graphicsFamily = i;
            
            VkBool32 presentSupport = MEMRE_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(f_device, i, f_surface, &presentSupport);
            if(presentSupport)
            {
                result.presentFamily = ALLOCATE_MEMORY(uint32_t, 1);
                *result.presentFamily = i;
            }
        }
    }
    return(result);
}

typedef struct
{
    VkSurfaceCapabilitiesKHR capabilities;
    
    VkSurfaceFormatKHR* formats;
    uint32_t formatsArraySize;
    
    VkPresentModeKHR* presentModes;
    uint32_t presentModesArraySize;
} SwapChainSupportDetails;

SwapChainSupportDetails
VK_querySwapchainSupport(VkPhysicalDevice f_device, VkSurfaceKHR f_surface)
{
    SwapChainSupportDetails result = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(f_device, f_surface, &result.capabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(f_device, f_surface, &formatCount, NULL);
    result.formatsArraySize = formatCount;
    if(formatCount != 0)
    {
        result.formats = ALLOCATE_MEMORY(VkSurfaceFormatKHR, formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(f_device, f_surface, &formatCount, &result.formats[0]);
    }
    
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(f_device, f_surface, &presentModeCount, NULL);
    result.presentModesArraySize = presentModeCount;
    if(presentModeCount != 0)
    {
        result.presentModes = ALLOCATE_MEMORY(VkPresentModeKHR, presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(f_device, f_surface, &presentModeCount, &result.presentModes[0]);
    }
    
    return(result);
}

typedef struct
{
    union
    {
        struct
        {
            string_t extSwapchain;
        };
        struct
        {
            string_t array[1];
        };
    };
} VulkanSwapchain;

int
VK_checkDeviceExtensionSupport(VkPhysicalDevice f_device, VulkanSwapchain* f_swapchainExtensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(f_device, NULL, &extensionCount, NULL);
    VkExtensionProperties* availableExtensions = ALLOCATE_MEMORY(VkExtensionProperties, extensionCount);
    vkEnumerateDeviceExtensionProperties(f_device, NULL, &extensionCount, &availableExtensions[0]);
    
    for(uint32_t i = 0;i < ARRAY_SIZE(f_swapchainExtensions->array); i++)
    {
        for(uint32_t j = 0; j < extensionCount; j++)
        {
            if(compareTwoStrings(f_swapchainExtensions->array[i], availableExtensions[j].extensionName))
            {
                return(MEMRE_TRUE);
            }
        }
    }
    return(MEMRE_FALSE);
}

int
VK_isDeviceSuitable(VkPhysicalDevice f_device, VkSurfaceKHR f_surface, VulkanSwapchain* f_swapchainExtensions)
{
    /*
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(f_device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(f_device, &deviceFeatures);
*/
    
    QueueFamilyIndices indices = VK_findQueueFamilies(f_device, f_surface);
    
    int extensionsSupported = VK_checkDeviceExtensionSupport(f_device, f_swapchainExtensions);
    
    int swapChainAdequte = MEMRE_FALSE;
    SwapChainSupportDetails swapChainSupport = VK_querySwapchainSupport(f_device, f_surface);
    swapChainAdequte = (swapChainSupport.formatsArraySize != 0) && (swapChainSupport.presentModesArraySize != 0);
    
    return((indices.graphicsFamily != NULL) &&
           (extensionsSupported) &&
           (swapChainAdequte));
}

void
VK_pickPhysicalDevice(VkInstance f_instance, VkSurfaceKHR f_surface,
                      VulkanSwapchain* f_swapchainExtensions,
                      VkPhysicalDevice* f_physicalDevice)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(f_instance, &deviceCount, NULL);
    MEMRE_ASSERT(!deviceCount, "Failed to find vulkan compatible GPUs\n");
    VkPhysicalDevice* devices = ALLOCATE_MEMORY(VkPhysicalDevice, deviceCount);
    vkEnumeratePhysicalDevices(f_instance, &deviceCount, &devices[0]);
    
    for(uint32_t i = 0; i < deviceCount; i++)
    {
        if(VK_isDeviceSuitable(devices[i], f_surface, f_swapchainExtensions))
        {
            *f_physicalDevice = devices[i];
            break;
        }
    }
    MEMRE_ASSERT(f_physicalDevice == VK_NULL_HANDLE, "Failed to find a suitable GPU\n");
}

void
VK_createLogicalDevice(VkDevice* f_device, VkPhysicalDevice f_physicalDevice, VkSurfaceKHR f_surface,
                       VkQueue* f_graphicsQueue, VkQueue* f_presentQueue,
                       VulkanSwapchain* f_swapchainExtensions,
                       string_t* f_validationExtensionsArray, uint32_t f_validationExtensionsSize)
{
    QueueFamilyIndices indices = VK_findQueueFamilies(f_physicalDevice, f_surface);
    
    uint32_t* copiedIndicesArray = ALLOCATE_MEMORY(uint32_t, ARRAY_SIZE(indices.array));
    for(uint32_t i = 0; i < ARRAY_SIZE(indices.array); i++)
    {
        copiedIndicesArray[i] = *indices.array[i];
    }
    
    UniqueIntegerArray uniqueIndices = createUniqueIntegerArray(copiedIndicesArray, ARRAY_SIZE(indices.array));
    
    VkDeviceQueueCreateInfo* queueCreateInfos = ALLOCATE_MEMORY(VkDeviceQueueCreateInfo, ARRAY_SIZE(indices.array));
    
    float32_t queuePriority = 1.f;
    for(uint32_t i = 0; i < uniqueIndices.size; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {0};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = (uint32_t)uniqueIndices.array[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }
    
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    vkGetPhysicalDeviceFeatures(f_physicalDevice, &deviceFeatures);
    
    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32_t)uniqueIndices.size;
    createInfo.pQueueCreateInfos = &queueCreateInfos[0];
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = (uint32_t)ARRAY_SIZE(f_swapchainExtensions->array);
    createInfo.ppEnabledExtensionNames = &f_swapchainExtensions->array[0];
    
    // TODO: check if the GPU i'm currently using requires this
    if(globalEnableValidationLayers)
    {
        createInfo.enabledLayerCount = f_validationExtensionsSize;
        createInfo.ppEnabledLayerNames = &f_validationExtensionsArray[0];
    }
    
    MEMRE_ASSERT(vkCreateDevice(f_physicalDevice, &createInfo, NULL, f_device) != VK_SUCCESS,
                 "Failed to create logical device\n");
    
    vkGetDeviceQueue(*f_device, *indices.graphicsFamily, 0, f_graphicsQueue);
    vkGetDeviceQueue(*f_device, *indices.presentFamily, 0, f_presentQueue);
}

#endif