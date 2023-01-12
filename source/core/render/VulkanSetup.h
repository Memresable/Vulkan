#ifndef _VULKAN_SETUP_H
#define _VULKAN_SETUP_H

#include "../utilities/general_utilities.h"
#include "../utilities/types.h"

#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"

#define APPLICATION_NAME "MemreClub"
#define ENGINE_NAME "VulkanIsFunny"

const int globalEnableValidationLayers = MEMRE_TRUE;

typedef struct
{
    string_t* extensions;
    uint32_t size;
} VulkanExtensionsData;

VulkanExtensionsData
VK_getRequiredExtensions(string_t* f_surfaceExtensionsArray, uint32_t f_surfaceExtensionsSize)
{
    /* total size: 3 */
    uint32_t requiredExtensionsSize = 
        f_surfaceExtensionsSize + /* (size: 2) | rank 1-2  */
        1; /* extDebugUtils (size: 1) | rank 3 */
    
    string_t* array = ALLOCATE_MEMORY(string_t, requiredExtensionsSize);
    
    for(uint32_t i = 0; i < requiredExtensionsSize; i++)
    {
        if(i < 2)
        {
            array[i] = f_surfaceExtensionsArray[i];
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
VK_validationSupport(string_t* f_validationExtensionsArray, uint32_t f_validationExtensionsSize)
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties* availableLayers = ALLOCATE_MEMORY(VkLayerProperties, layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, &availableLayers[0]);
    
    for(uint32_t i = 0;i < f_validationExtensionsSize; i++)
    {
        for(uint32_t j = 0; j < layerCount; j++)
        {
            if(compareTwoStrings(f_validationExtensionsArray[i], availableLayers[j].layerName))
            {
                RELEASE_MEMORY(availableLayers);
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
    OutputDebugStringA("[Validation layer] ");
    OutputDebugStringA(pCallbackData->pMessage);
    OutputDebugStringA("\n");
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
VK_destroyDebugUtilsMessengerEXT(VkInstance f_instance, VkDebugUtilsMessengerEXT f_debugMessenger,
                                 const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT function =
    (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(f_instance, "vkDestroyDebugUtilsMessengerEXT");
    if(function != NULL)
    {
        function(f_instance, f_debugMessenger, pAllocator);
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
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = VK_debugCallback;
}

void
VK_setupDebugMessenger(VkInstance f_instance, VkDebugUtilsMessengerEXT* f_debugMessenger)
{
    if(!globalEnableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    VK_populateDebugMessengerCreateInfo(&createInfo);
    
    MEMRE_ASSERT(VK_createDebugUtilsMessengerEXT(f_instance, &createInfo, NULL, f_debugMessenger) != VK_SUCCESS,
                 "Failed to setup a debug messenger\n");
}

void
VK_createInstance(VkInstance* f_instance,
                  string_t* f_surfaceExtensionsArray, uint32_t f_surfaceExtensionsSize,
                  string_t* f_validationExtensionsArray, uint32_t f_validationExtensionsSize)
{
    MEMRE_ASSERT(!VK_validationSupport(f_validationExtensionsArray, f_validationExtensionsSize) && globalEnableValidationLayers,
                 "Validation Layers Requested, but not available\n");
    
    *f_instance = 0;
    
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
    
    VulkanExtensionsData requiredExtensions = VK_getRequiredExtensions(f_surfaceExtensionsArray,
                                                                       f_surfaceExtensionsSize);
	createInfo.enabledExtensionCount = requiredExtensions.size;
	createInfo.ppEnabledExtensionNames = requiredExtensions.extensions;
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
    if(globalEnableValidationLayers)
    {
        createInfo.enabledLayerCount = f_validationExtensionsSize;
        createInfo.ppEnabledLayerNames = f_validationExtensionsArray;
        
        VK_populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    
    MEMRE_ASSERT(vkCreateInstance(&createInfo, NULL, f_instance) != VK_SUCCESS,
                 "Failed to create a Vulkan instance\n");
    
    /*
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensionProperties = ALLOCATE_MEMORY(VkExtensionProperties, extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, &extensionProperties[0]);
*/
}

#endif