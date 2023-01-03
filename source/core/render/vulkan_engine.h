#pragma once

// TODO: replace malloc with VirtualAlloc & remove any non-platform library
#include <stdlib.h>

#include "../utilities/general_utilities.h"
#include "../utilities/unique_array.h"

#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"
#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan_win32.h"

#define APPLICATION_NAME "MemreVK1"
#define ENGINE_NAME "MemrEngine"

const int globalEnableValidationLayers = MEMRE_TRUE;

typedef struct
{
    HWND* handle;
    uint32_t* width;
    uint32_t* height;
} WindowInfo;

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
    
    WindowInfo window;
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
    VkImageView* swapchainImageViews;
    
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
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
    char buffer[8192]; // i wonder what's the biggest message vulkan could output?
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
    
    MEMRE_ASSERT(VK_createDebugUtilsMessengerEXT(f_engine->instance, &createInfo, NULL, &f_engine->debugMessenger) != VK_SUCCESS,
                 "Failed to setup a debug messenger\n");
}

void
VK_createInstance(VulkanEngine* f_engine)
{
    MEMRE_ASSERT(!VK_validationSupport(f_engine) && globalEnableValidationLayers,
                 "Validation Layers Requested, but not available\n");
    
    f_engine->instance = 0;
    
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
    createInfo.hwnd = *f_engine->window.handle;
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
    
    uint32_t* copiedIndicesArray = (uint32_t*)malloc(sizeof(uint32_t) * (ARRAY_SIZE(indices.array)));
    for(uint32_t i = 0; i < ARRAY_SIZE(indices.array); i++) copiedIndicesArray[i] = *indices.array[i];
    
    UniqueIntegerArray uniqueIndices = createUniqueIntegerArray(copiedIndicesArray, ARRAY_SIZE(indices.array));
    
    VkDeviceQueueCreateInfo* queueCreateInfos =
    (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo) * (ARRAY_SIZE(indices.array)));
    
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
VK_createSwapchain(VulkanEngine* f_engine)
{
    SwapChainSupportDetails swapchainSupport = VK_querySwapchainSupport(f_engine->physicalDevice, f_engine->surface);
    
    VkSurfaceFormatKHR surfaceFormat = VK_chooseSwapSurfaceFormat(swapchainSupport.formats, swapchainSupport.formatsArraySize);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D extent = VK_chooseSwapExtent(&swapchainSupport.capabilities, *f_engine->window.width, *f_engine->window.height);
    
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
VK_createImageViews(VulkanEngine* f_engine)
{
    uint32_t swapchainImagesSize = f_engine->swapchainImagesArraySize;
    f_engine->swapchainImageViews = (VkImageView*)malloc(sizeof(VkImageView)*swapchainImagesSize);
    for(uint32_t i = 0; i < swapchainImagesSize; i++)
    {
        VkImageViewCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = f_engine->swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = f_engine->swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        MEMRE_ASSERT(vkCreateImageView(f_engine->device, &createInfo, NULL, &f_engine->swapchainImageViews[i]) != VK_SUCCESS,
                     "Failed to create image views");
    }
}

void
VK_createRenderpass(VulkanEngine* f_engine)
{
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = f_engine->swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    
    MEMRE_ASSERT(vkCreateRenderPass(f_engine->device, &renderPassInfo, NULL, &f_engine->renderPass) != VK_SUCCESS,
                 "failed to create render pass!");
}

typedef struct
{
    char* binaryData;
    uint32_t binarySize;
} VulkanShaderBinaryData;

// TODO: move the reading functionality to platform specific header file
VulkanShaderBinaryData*
VK_readShaderFile(LPCWSTR f_binaryShaderFile)
{
    HANDLE hFile;
    hFile = CreateFile((LPCWSTR)f_binaryShaderFile,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                       NULL);
    if(hFile == INVALID_HANDLE_VALUE) 
    { 
        OutputDebugStringA("Unable to open the file");
        return(NULL);
    }
    
    int64_t binarySize = 0;
    GetFileSizeEx(hFile, (PLARGE_INTEGER)&binarySize);
    char* readBuffer = (char*)malloc(sizeof(char)*binarySize);
    OVERLAPPED ol = {0};
    if(ReadFileEx(hFile, readBuffer, binarySize, &ol, NULL) == FALSE)
    {
        OutputDebugStringA("Reading the binary shader file was interrupted");
        CloseHandle(hFile);
        return(NULL);
    }
    
    CloseHandle(hFile);
    
    VulkanShaderBinaryData* result = (VulkanShaderBinaryData*)malloc(sizeof(VulkanShaderBinaryData));
    result->binaryData = readBuffer;
    result->binarySize = binarySize;
    return(result);
}

VkShaderModule
VK_createShaderModule(VulkanEngine* f_engine, VulkanShaderBinaryData* f_shaderBinary)
{
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = f_shaderBinary->binarySize;
    createInfo.pCode = (const uint32_t*)f_shaderBinary->binaryData;
    
    VkShaderModule shaderModule;
    MEMRE_ASSERT(vkCreateShaderModule(f_engine->device, &createInfo, NULL, &shaderModule) != VK_SUCCESS, "Failed to create a shader module");
    return(shaderModule);
}

void
VK_createGraphicsPipeline(VulkanEngine* f_engine)
{
    VulkanShaderBinaryData* vertexData =
        VK_readShaderFile(L"C:\\Users\\Memresable\\Desktop\\MemreVK1\\source\\core\\render\\shaders\\vertex.spv");
    VulkanShaderBinaryData* fragmentData =
        VK_readShaderFile(L"C:\\Users\\Memresable\\Desktop\\MemreVK1\\source\\core\\render\\shaders\\fragment.spv");
    MEMRE_ASSERT((!vertexData->binaryData) || (!fragmentData->binaryData), "Unable to read the binary shader files");
    
    VkShaderModule vertexShaderModule = VK_createShaderModule(f_engine, vertexData);
    VkShaderModule fragmentShaderModule = VK_createShaderModule(f_engine, fragmentData);
    
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {0};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {0};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[2] = {vertexShaderStageInfo, fragmentShaderStageInfo};
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    uint32_t dynamicStatesSize = 2;
    VkDynamicState dynamicStates[2];
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
    
    VkPipelineDynamicStateCreateInfo dynamicState = {0};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (uint32_t)dynamicStatesSize;
    dynamicState.pDynamicStates = &dynamicStates[0];
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float32_t)f_engine->swapchainExtent.width;
    viewport.height = (float32_t)f_engine->swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = f_engine->swapchainExtent;
    
    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    MEMRE_ASSERT(vkCreatePipelineLayout(f_engine->device, &pipelineLayoutInfo, NULL, &f_engine->pipelineLayout) != VK_SUCCESS,
                 "failed to create pipeline layout!");
    
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = &shaderStages[0];
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = f_engine->pipelineLayout;
    pipelineInfo.renderPass = f_engine->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    MEMRE_ASSERT(vkCreateGraphicsPipelines(f_engine->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &f_engine->graphicsPipeline) != VK_SUCCESS,
                 "failed to create graphics pipeline!");
    
    vkDestroyShaderModule(f_engine->device, vertexShaderModule, NULL);
    vkDestroyShaderModule(f_engine->device, fragmentShaderModule, NULL);
}

void
VK_initialize(VulkanEngine* f_engine, HWND* f_mainWindowHandle, uint32_t* f_mainWindowWidth, uint32_t* f_mainWindowHeight)
{
    // Window info
    f_engine->window.handle = f_mainWindowHandle;
    f_engine->window.width = f_mainWindowWidth;
    f_engine->window.height = f_mainWindowHeight;
    
    // Surface extensions
    f_engine->surfaceExtensions.extSurface = VK_KHR_SURFACE_EXTENSION_NAME;
    f_engine->surfaceExtensions.extWin32Surface = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    
    // Validation layer extensions
    f_engine->validationExtensions.extValidation = "VK_LAYER_KHRONOS_validation";
    
    // Swapchain extension(s?)
    f_engine->swapchainExtensions.extSwapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    
    // Initialization stage
    VK_createInstance(f_engine);
    VK_setupDebugMessenger(f_engine);
    VK_createSurface(f_engine);
    VK_pickPhysicalDevice(f_engine);
    VK_createLogicalDevice(f_engine);
    VK_createSwapchain(f_engine);
    VK_createImageViews(f_engine);
    VK_createRenderpass(f_engine);
    VK_createGraphicsPipeline(f_engine);
}
void
VK_cleanup(VulkanEngine* f_engine)
{
    vkDestroyPipeline(f_engine->device, f_engine->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(f_engine->device, f_engine->pipelineLayout, NULL);
    vkDestroyRenderPass(f_engine->device, f_engine->renderPass, NULL);
    for(uint32_t i = 0; i < f_engine->swapchainImagesArraySize; i++)
    {
        vkDestroyImageView(f_engine->device, f_engine->swapchainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(f_engine->device, f_engine->swapchain, NULL);
    vkDestroyDevice(f_engine->device, NULL);
    if(globalEnableValidationLayers)
    {
        VK_destroyDebugUtilsMessengerEXT(f_engine, NULL);
    }
    vkDestroySurfaceKHR(f_engine->instance, f_engine->surface, NULL);
    vkDestroyInstance(f_engine->instance, NULL);
}