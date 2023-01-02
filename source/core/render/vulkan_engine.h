#pragma once

// TODO: replace malloc with VirtualAlloc & remove any non-platform library
#include <stdlib.h>

#include "../utilities/general_utilities.h"
#include "../utilities/unique_array.h"

#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"
#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan_win32.h"

#define APPLICATION_NAME "MemreVK1"
#define ENGINE_NAME "MemrEngine"

const uint32_t globalWidth = 800;
const uint32_t globalHeight = 600;

const int globalEnableValidationLayers = MEMRE_TRUE;

typedef struct
{
    string_t* extensions;
    uint32_t size;
} VulkanExtensionsData;

typedef struct
{
    union
    {
        struct
        {
            string_t extSurface;
            string_t extWin32Surface;
        };
        struct
        {
            string_t array[2];
        };
    };
} VulkanSurface;

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

typedef struct
{
    union
    {
        struct
        {
            string_t extValidation;
        };
        struct
        {
            string_t array[1];
        };
    };
} VulkanValidationLayers;

typedef struct
{
	VkInstance instance;
    
    VulkanValidationLayers validationExtensions;
    VkDebugUtilsMessengerEXT debugMessenger;
    
    HWND* mainWindowHandle;
    VulkanSurface surfaceExtensions;
    VkSurfaceKHR surface;
    
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    
    VulkanSwapchain swapchainExtensions;
    VkSwapchainKHR swapchain;
    VkExtent2D swapchainExtent;
    VkFormat swapchainImageFormat;
    VkImage* swapchainImages;
    uint32_t swapchainImagesArraySize;
} VulkanEngine;

// TODO: make this better
VulkanExtensionsData
VK_getRequiredExtensions(VulkanEngine* f_engine)
{
    string_t* array;
    
    /* total size: 3 */
    uint32_t requiredExtensionsSize = 
        ARRAY_SIZE(f_engine->surfaceExtensions.array) /* (size: 2) | rank 1-2  */ +
        1 /* extDebugUtils (size: 1) | rank 3 */;
    
    array = (string_t*)malloc(sizeof(string_t) * requiredExtensionsSize);
    
    for(uint32_t i = 0; i < requiredExtensionsSize; i++)
    {
        if(i < 2)
        {
            array[i] = f_engine->surfaceExtensions.array[i];
        }
        if(i == 2)
        {
            array[i] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }
    }
    
    VulkanExtensionsData result = {array, requiredExtensionsSize};
    return(result);
}

int
VK_validationSupport(VulkanEngine* f_engine)
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, &availableLayers[0]);
    
    for(uint32_t i = 0;i < ARRAY_SIZE(f_engine->validationExtensions.array); i++)
    {
        for(uint32_t j = 0; j < layerCount; j++)
        {
            if(compareTwoStrings(f_engine->validationExtensions.array[i], availableLayers[j].layerName))
            {
                return(MEMRE_TRUE);
            }
        }
    }
    return(MEMRE_FALSE);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
VK_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                 void* pUserData)
{
    char buffer[8192];
    sprintf(buffer, "[Validation layer]: %s\n", pCallbackData->pMessage);
    OutputDebugStringA(buffer);
    return(VK_FALSE);
}

VkResult
VK_createDebugUtilsMessengerEXT(VkInstance instance,
                                const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator,
                                VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT function =
    (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(function != NULL)
    {
        return(function(instance, pCreateInfo, pAllocator, pDebugMessenger));
    }
    return(VK_ERROR_EXTENSION_NOT_PRESENT);
}

void
VK_destroyDebugUtilsMessengerEXT(VulkanEngine* f_engine,
                                 const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT function =
    (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(f_engine->instance, "vkDestroyDebugUtilsMessengerEXT");
    if(function != NULL)
    {
        function(f_engine->instance, f_engine->debugMessenger, pAllocator);
    }
}

void
VK_populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo)
{
    *createInfo = (VkDebugUtilsMessengerCreateInfoEXT){0};
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = VK_debugCallback;
}

void
VK_setupDebugMessenger(VulkanEngine* f_engine)
{
    if(!globalEnableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    VK_populateDebugMessengerCreateInfo(&createInfo);
    
    MEMRE_ASSERT(VK_createDebugUtilsMessengerEXT(f_engine->instance, &createInfo, NULL, 
                                                 &f_engine->debugMessenger) != VK_SUCCESS,
                 "Failed to setup a debug messenger\n");
}

void
VK_createInstance(VulkanEngine* f_engine)
{
    MEMRE_ASSERT(!VK_validationSupport(f_engine) && globalEnableValidationLayers,
                 "Validation Layers Requested, but not available\n");
    
	VkApplicationInfo appInfo = {0};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = APPLICATION_NAME;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;
    
	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
    
    VulkanExtensionsData requiredExtensions = VK_getRequiredExtensions(f_engine);
	createInfo.enabledExtensionCount = requiredExtensions.size;
	createInfo.ppEnabledExtensionNames = requiredExtensions.extensions;
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
    if(globalEnableValidationLayers)
    {
        createInfo.enabledLayerCount = (uint32_t)ARRAY_SIZE(f_engine->validationExtensions.array);
        createInfo.ppEnabledLayerNames = f_engine->validationExtensions.array;
        
        VK_populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    
    MEMRE_ASSERT(vkCreateInstance(&createInfo, NULL, &f_engine->instance) != VK_SUCCESS,
                 "Failed to create a Vulkan instance\n");
    
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensionProperties = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, &extensionProperties[0]);
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
    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(f_device, &queueFamilyCount, &queueFamilies[0]);
    
    for(uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            result.graphicsFamily = (uint32_t*)malloc(sizeof(uint32_t));
            *result.graphicsFamily = i;
            
            VkBool32 presentSupport = MEMRE_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(f_device, i, f_surface, &presentSupport);
            if(presentSupport)
            {
                result.presentFamily = (uint32_t*)malloc(sizeof(uint32_t));
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
        result.formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(f_device, f_surface, &formatCount, &result.formats[0]);
    }
    
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(f_device, f_surface, &presentModeCount, NULL);
    result.presentModesArraySize = presentModeCount;
    if(presentModeCount != 0)
    {
        result.presentModes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(f_device, f_surface, &presentModeCount, &result.presentModes[0]);
    }
    
    return(result);
}

int
VK_checkDeviceExtensionSupport(VulkanEngine* f_engine, VkPhysicalDevice f_device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(f_device, NULL, &extensionCount, NULL);
    VkExtensionProperties* availableExtensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateDeviceExtensionProperties(f_device, NULL, &extensionCount, &availableExtensions[0]);
    
    for(uint32_t i = 0;i < ARRAY_SIZE(f_engine->swapchainExtensions.array); i++)
    {
        for(uint32_t j = 0; j < extensionCount; j++)
        {
            if(compareTwoStrings(f_engine->swapchainExtensions.array[i], availableExtensions[j].extensionName))
            {
                return(MEMRE_TRUE);
            }
        }
    }
    return(MEMRE_FALSE);
}

int
VK_isDeviceSuitable(VulkanEngine* f_engine, VkPhysicalDevice f_device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(f_device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(f_device, &deviceFeatures);
    
    QueueFamilyIndices indices = VK_findQueueFamilies(f_device, f_engine->surface);
    
    int extensionsSupported = VK_checkDeviceExtensionSupport(f_engine, f_device);
    
    int swapChainAdequte = MEMRE_FALSE;
    SwapChainSupportDetails swapChainSupport = VK_querySwapchainSupport(f_device, f_engine->surface);
    swapChainAdequte = (swapChainSupport.formatsArraySize != 0) && (swapChainSupport.presentModesArraySize != 0);
    
    return((indices.graphicsFamily != NULL) &&
           (extensionsSupported) &&
           (swapChainAdequte));
}

VkSurfaceFormatKHR
VK_chooseSwapSurfaceFormat(VkSurfaceFormatKHR* f_availableFormats, uint32_t f_formatsArraySize)
{
    for(uint32_t i = 0; i < f_formatsArraySize; i++)
    {
        if(f_availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
           f_availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return(f_availableFormats[i]);
        }
    }
    
    return(f_availableFormats[0]);
}

VkExtent2D
VK_chooseSwapExtent(VkSurfaceCapabilitiesKHR* f_capabilities, uint32_t f_windowWidth, uint32_t f_windowHeight)
{
    if(f_capabilities->currentExtent.width != UINT32_MAX)
    {
        return(f_capabilities->currentExtent);
    }
    else
    {
        VkExtent2D actualExtent = {f_windowWidth, f_windowHeight};
        
        actualExtent.width =
        (actualExtent.width < f_capabilities->minImageExtent.width) ? f_capabilities->maxImageExtent.width : actualExtent.width;
        actualExtent.height =
        (actualExtent.height < f_capabilities->minImageExtent.height) ? f_capabilities->maxImageExtent.height : actualExtent.height;
        
        return(actualExtent);
    }
}

void
VK_createSurface(VulkanEngine* f_engine)
{
    VkWin32SurfaceCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = *f_engine->mainWindowHandle;
    createInfo.hinstance = GetModuleHandle(NULL);
    
    MEMRE_ASSERT(vkCreateWin32SurfaceKHR(f_engine->instance, &createInfo, NULL, &f_engine->surface) != VK_SUCCESS,
                 "Failed to create window surface\n");
}

void
VK_pickPhysicalDevice(VulkanEngine* f_engine)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(f_engine->instance, &deviceCount, NULL);
    MEMRE_ASSERT(!deviceCount, "Failed to find vulkan compatible GPUs\n");
    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(f_engine->instance, &deviceCount, &devices[0]);
    
    for(uint32_t i = 0; i < deviceCount; i++)
    {
        if(VK_isDeviceSuitable(f_engine, devices[i]))
        {
            f_engine->physicalDevice = devices[i];
            break;
        }
    }
    MEMRE_ASSERT(f_engine->physicalDevice == VK_NULL_HANDLE, "Failed to find a suitable GPU\n");
}

void
VK_createLogicalDevice(VulkanEngine* f_engine)
{
    QueueFamilyIndices indices = VK_findQueueFamilies(f_engine->physicalDevice, f_engine->surface);
    
    uint32_t* copiedIndicesArray = (uint32_t*)malloc(sizeof(uint32_t) * ARRAY_SIZE(indices.array));
    for(uint32_t i = 0; i < ARRAY_SIZE(indices.array); i++) copiedIndicesArray[i] = *indices.array[i];
    
    UniqueIntegerArray uniqueIndices = createUniqueIntegerArray(copiedIndicesArray, ARRAY_SIZE(indices.array));
    
    VkDeviceQueueCreateInfo* queueCreateInfos =
    (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo) * ARRAY_SIZE(indices.array));
    
    float queuePriority = 1.f;
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
    
    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32_t)uniqueIndices.size;
    createInfo.pQueueCreateInfos = &queueCreateInfos[0];
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = (uint32_t)ARRAY_SIZE(f_engine->swapchainExtensions.array);
    createInfo.ppEnabledExtensionNames = &f_engine->swapchainExtensions.array[0];
    
    // TODO: check if the GPU i'm currently using requires this
    if(globalEnableValidationLayers)
    {
        createInfo.enabledLayerCount = (uint32_t)ARRAY_SIZE(f_engine->validationExtensions.array);
        createInfo.ppEnabledLayerNames = &f_engine->validationExtensions.array[0];
    }
    
    MEMRE_ASSERT(vkCreateDevice(f_engine->physicalDevice, &createInfo, NULL, &f_engine->device) != VK_SUCCESS,
                 "Failed to create logical device\n");
    
    vkGetDeviceQueue(f_engine->device, *indices.graphicsFamily, 0, &f_engine->graphicsQueue);
    vkGetDeviceQueue(f_engine->device, *indices.presentFamily, 0, &f_engine->presentQueue);
}

void
VK_createSwapchain(VulkanEngine* f_engine, uint32_t f_windowWidth, uint32_t f_windowHeight)
{
    SwapChainSupportDetails swapchainSupport = VK_querySwapchainSupport(f_engine->physicalDevice, f_engine->surface);
    
    VkSurfaceFormatKHR surfaceFormat = VK_chooseSwapSurfaceFormat(swapchainSupport.formats, swapchainSupport.formatsArraySize);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D extent = VK_chooseSwapExtent(&swapchainSupport.capabilities, f_windowWidth, f_windowHeight);
    
    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if((swapchainSupport.capabilities.maxImageCount > 0) && (imageCount > swapchainSupport.capabilities.maxImageCount))
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = f_engine->surface;
    
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = VK_findQueueFamilies(f_engine->physicalDevice, f_engine->surface);
    uint32_t queueFamilyIndices[2] = {*indices.graphicsFamily, *indices.presentFamily};
    
    if(*indices.graphicsFamily != *indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    MEMRE_ASSERT(vkCreateSwapchainKHR(f_engine->device, &createInfo, NULL, &f_engine->swapchain) != VK_SUCCESS,
                 "Failed to create the swapchain");
    
    vkGetSwapchainImagesKHR(f_engine->device, f_engine->swapchain, &imageCount, NULL);
    f_engine->swapchainImages = (VkImage*)malloc(sizeof(VkImage)*imageCount);
    vkGetSwapchainImagesKHR(f_engine->device, f_engine->swapchain, &imageCount, &f_engine->swapchainImages[0]);
    
    f_engine->swapchainImageFormat = surfaceFormat.format;
    f_engine->swapchainExtent = extent;
}

void
VK_initialize(VulkanEngine* f_engine, HWND* f_mainWindowHandle)
{
    // Create an instance
	f_engine->instance = 0;
    
    // Surface extensions
	f_engine->surfaceExtensions.extSurface = VK_KHR_SURFACE_EXTENSION_NAME;
	f_engine->surfaceExtensions.extWin32Surface = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    
    // Validation layer extensions
    f_engine->validationExtensions.extValidation = "VK_LAYER_KHRONOS_validation";
    
    // Take windows's handle address
    f_engine->mainWindowHandle = f_mainWindowHandle;
    
    // Swapchain extension(s?)
    f_engine->swapchainExtensions.extSwapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    
    // Initialization stage
    VK_createInstance(f_engine);
    VK_setupDebugMessenger(f_engine);
    VK_createSurface(f_engine);
    VK_pickPhysicalDevice(f_engine);
    VK_createLogicalDevice(f_engine);
    VK_createSwapchain(f_engine, globalWidth, globalHeight);
}

void
VK_cleanup(VulkanEngine* f_engine)
{
    vkDestroySwapchainKHR(f_engine->device, f_engine->swapchain, NULL);
    vkDestroyDevice(f_engine->device, NULL);
    if(globalEnableValidationLayers)
    {
        VK_destroyDebugUtilsMessengerEXT(f_engine, NULL);
    }
    vkDestroySurfaceKHR(f_engine->instance, f_engine->surface, NULL);
    vkDestroyInstance(f_engine->instance, NULL);
}