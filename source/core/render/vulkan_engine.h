#pragma once

#include <stdlib.h>

#include "../utilities/general_utilities.h"

#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"
#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan_win32.h"

typedef const char* string_t;

typedef float float32_t;
typedef double float64_t;

typedef unsigned int uint32_t;

#define APPLICATION_NAME "MemreVK1"
#define ENGINE_NAME "MemrEngine"

const uint32_t globalWidth = 800;
const uint32_t globalHeight = 600;

#ifdef NDEBUG
const bool globalEnableValidationLayers = false;
#else
const bool globalEnableValidationLayers = true;
#endif

// NOTE: use this struct more often
struct VulkanExtensionsData
{
    string_t* extensions;
    uint32_t size;
};

struct VulkanSurface
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
};

struct VulkanValidationLayers
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
};

struct VulkanEngine
{
	VkInstance instance;
    
    VulkanValidationLayers validationExtensions;
    VkDebugUtilsMessengerEXT debugMessenger;
    
    HWND* windowHandle;
    VulkanSurface surfaceExtensions;
    VkSurfaceKHR surface;
    
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

// TODO: make this better
VulkanExtensionsData
VK_getRequiredExtensions(VulkanEngine* f_engine)
{
    string_t* result;
    
    /* total size: 3 */
    uint32_t requiredExtensionsSize = 
        STRING_STRUCT_SIZE(f_engine->surfaceExtensions) /* (size: 2) | rank 1-2  */ +
        1 /* extDebugUtils (size: 1) | rank 3 */;
    
    result = (string_t*)malloc(sizeof(string_t) * requiredExtensionsSize);
    
    for(uint32_t i = 0; i < requiredExtensionsSize; i++)
    {
        if(i < 2)
        {
            result[i] = f_engine->surfaceExtensions.array[i];
        }
        if(i == 2)
        {
            result[i] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }
    }
    
    return{result, requiredExtensionsSize};
}

bool
VK_validationSupport(VulkanEngine* f_engine)
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, &availableLayers[0]);
    
    for(uint32_t i = 0;i < STRING_STRUCT_SIZE(f_engine->validationExtensions); i++)
    {
        for(uint32_t j = 0; j < layerCount; j++)
        {
            // TODO: convert the following string comparison to it's own function
            for(uint32_t k = 0;
                f_engine->validationExtensions.array[i][k] != '\0' && availableLayers[j].layerName[k] != '\0';
                k++)
            {
                if(f_engine->validationExtensions.array[i][k] == availableLayers[j].layerName[k])
                {
                    if((f_engine->validationExtensions.array[i][k+1] == '\0') &&
                       (availableLayers[j].layerName[k+1] == '\0'))
                    {
                        return(true);
                    }
                    continue;
                }
            }
        }
    }
    
    return(false);
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
    if(function != nullptr)
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
    if(function != nullptr)
    {
        function(f_engine->instance, f_engine->debugMessenger, pAllocator);
    }
}

void
VK_populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = VK_debugCallback;
}

void
VK_setupDebugMessenger(VulkanEngine* f_engine)
{
    if(!globalEnableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    VK_populateDebugMessengerCreateInfo(createInfo);
    
    MEMRE_ASSERT(VK_createDebugUtilsMessengerEXT(f_engine->instance, &createInfo, nullptr, &f_engine->debugMessenger) != VK_SUCCESS,
                 "Failed to setup a debug messenger\n");
}

void
VK_createInstance(VulkanEngine* f_engine)
{
    MEMRE_ASSERT(!VK_validationSupport(f_engine) && globalEnableValidationLayers,
                 "Validation Layers Requested, but not available\n");
    
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = APPLICATION_NAME;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;
    
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
    
    VulkanExtensionsData requiredExtensions = VK_getRequiredExtensions(f_engine);
	createInfo.enabledExtensionCount = requiredExtensions.size;
	createInfo.ppEnabledExtensionNames = requiredExtensions.extensions;
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if(globalEnableValidationLayers)
    {
        createInfo.enabledLayerCount = (uint32_t)STRING_STRUCT_SIZE(f_engine->validationExtensions);
        createInfo.ppEnabledLayerNames = f_engine->validationExtensions.array;
        
        VK_populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    
    MEMRE_ASSERT(vkCreateInstance(&createInfo, nullptr, &f_engine->instance) != VK_SUCCESS,
                 "Failed to create a Vulkan instance\n");
    
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    VkExtensionProperties* extensionProperties = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &extensionProperties[0]);
}

struct QueueFamilyIndices
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
};

QueueFamilyIndices
VK_findQueueFamilies(VkPhysicalDevice f_device, VkSurfaceKHR f_surface)
{
    QueueFamilyIndices result = {};
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(f_device, &queueFamilyCount, nullptr);
    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(f_device, &queueFamilyCount, &queueFamilies[0]);
    
    for(uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            result.graphicsFamily = (uint32_t*)malloc(sizeof(uint32_t));
            *result.graphicsFamily = i;
            
            VkBool32 presentSupport = false;
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

bool
VK_isDeviceSuitable(VkPhysicalDevice f_device, VkSurfaceKHR f_surface)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(f_device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(f_device, &deviceFeatures);
    
    QueueFamilyIndices indices = VK_findQueueFamilies(f_device, f_surface);
    
    return((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
           (indices.graphicsFamily != nullptr));
}

void
VK_createSurface(VulkanEngine* f_engine)
{
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = *f_engine->windowHandle;
    createInfo.hinstance = GetModuleHandle(nullptr);
    
    MEMRE_ASSERT(vkCreateWin32SurfaceKHR(f_engine->instance, &createInfo, nullptr, &f_engine->surface) != VK_SUCCESS,
                 "Failed to create window surface\n");
}

void
VK_pickPhysicalDevice(VulkanEngine* f_engine)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(f_engine->instance, &deviceCount, nullptr);
    MEMRE_ASSERT(!deviceCount, "Failed to find vulkan compatible GPUs\n");
    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(f_engine->instance, &deviceCount, &devices[0]);
    
    for(uint32_t i = 0; i < deviceCount; i++)
    {
        if(VK_isDeviceSuitable(devices[i], f_engine->surface))
        {
            f_engine->physicalDevice = devices[i];
            break;
        }
    }
    MEMRE_ASSERT(f_engine->physicalDevice == VK_NULL_HANDLE, "Failed to find a suitable GPU\n");
}

// An alternative to std::set in C++
template<typename T>
struct UniqueArray
{
    T* array;
    uint32_t size;
};

template<typename T>
bool
checkForArrayDuplicates(T* f_array, T* f_arrayLookup, uint32_t f_arraySize, uint32_t f_currentIndex)
{
    for(uint32_t j = 0; j < f_arraySize; j++)
    {
        for(uint32_t i = 0; i < f_arraySize; i++)
        {
            if(f_array[j] == f_arrayLookup[i])
            {
                return(false);
            }
        }
    }
    f_array[f_currentIndex] = f_arrayLookup[f_currentIndex];
    return(true);
}

// TODO: remove the template and replace it with something like size_t if possible
template<typename T>
UniqueArray<T>
removeDuplicateArrayValues(T* f_array, uint32_t f_arraySize)
{
    UniqueArray<T> result = {};
    result.array = (T*)malloc(sizeof(T) * f_arraySize);
    for(uint32_t i = 0; i < f_arraySize; i++)
    {
        if(checkForArrayDuplicates<T>(result.array, f_array, ARRAY_SIZE(f_array), i) == true)
        {
            result.size++;
        }
    }
    return(result);
}

void
VK_createLogicalDevice(VulkanEngine* f_engine)
{
    QueueFamilyIndices indices = VK_findQueueFamilies(f_engine->physicalDevice, f_engine->surface);
    UniqueArray<uint32_t> uniqueIndices = removeDuplicateArrayValues<uint32_t>(*indices.array, ARRAY_SIZE(indices.array));
    
    VkDeviceQueueCreateInfo* queueCreateInfos =
    (VkDeviceQueueCreateInfo*)malloc(sizeof(VkDeviceQueueCreateInfo) * ARRAY_SIZE(indices.array));
    
    float queuePriority = 1.f;
    for(uint32_t i = 0; i < uniqueIndices.size; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = (uint32_t)uniqueIndices.array[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }
    
    VkPhysicalDeviceFeatures deviceFeatures = {};
    
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32_t)uniqueIndices.size;
    createInfo.pQueueCreateInfos = &queueCreateInfos[0];
    createInfo.pEnabledFeatures = &deviceFeatures;
    
    // TODO: check if the GPU i'm currently using requires
    if(globalEnableValidationLayers)
    {
        createInfo.enabledLayerCount = (uint32_t)STRING_STRUCT_SIZE(f_engine->validationExtensions);
        createInfo.ppEnabledLayerNames = &f_engine->validationExtensions.array[0];
    }
    
    MEMRE_ASSERT(vkCreateDevice(f_engine->physicalDevice, &createInfo, nullptr, &f_engine->device) != VK_SUCCESS,
                 "Failed to create logical device\n");
    
    vkGetDeviceQueue(f_engine->device, *indices.graphicsFamily, 0, &f_engine->graphicsQueue);
    vkGetDeviceQueue(f_engine->device, *indices.presentFamily, 0, &f_engine->presentQueue);
}

void
VK_initialize(VulkanEngine* f_engine, HWND* f_windowHandle)
{
    // Create an instance
	f_engine->instance = {};
    
    // Surface extensions
    f_engine->surfaceExtensions = {};
	f_engine->surfaceExtensions.extSurface = VK_KHR_SURFACE_EXTENSION_NAME;
	f_engine->surfaceExtensions.extWin32Surface = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    
    // Validation layer extensions
    f_engine->validationExtensions = {};
    f_engine->validationExtensions.extValidation = "VK_LAYER_KHRONOS_validation";
    
    // Take windows's handle address
    f_engine->windowHandle = f_windowHandle;
    
    // Initialization stage
    VK_createInstance(f_engine);
    VK_setupDebugMessenger(f_engine);
    VK_createSurface(f_engine);
    VK_pickPhysicalDevice(f_engine);
    VK_createLogicalDevice(f_engine);
}

void
VK_cleanup(VulkanEngine* f_engine)
{
    vkDestroyDevice(f_engine->device, nullptr);
    if(globalEnableValidationLayers)
    {
        VK_destroyDebugUtilsMessengerEXT(f_engine, nullptr);
    }
    vkDestroySurfaceKHR(f_engine->instance, f_engine->surface, nullptr);
    vkDestroyInstance(f_engine->instance, nullptr);
}