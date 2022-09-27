#include "vulkan.h"

void VulkanEngine::initWindow()
{
    if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_SCR_WIDTH, m_SCR_HEIGHT, "valaska_v", NULL, NULL);
    if (!m_window) throw std::exception("Failed to create a Window");

    glfwMakeContextCurrent(m_window);
}

std::vector<const char*> VulkanEngine::getRequiredExtensions()
{
    uint32_t glfwExtensionCount{};
    const char** glfwExtensions{};
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (m_isDEBUG) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void VulkanEngine::createInstance()
{
    if (m_isDEBUG && (checkValidationSupport(m_validationLayers) == VK_FALSE))
        throw std::exception("Validation layers requested, but not available");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "memrekom";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 2, 0);
    appInfo.pEngineName = "memrekom";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 2, 0);
    appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0);

    auto extensions = getRequiredExtensions();

    VkInstanceCreateInfo createInstance{};
    createInstance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInstance.pApplicationInfo = &appInfo;
    createInstance.enabledExtensionCount = (uint32_t)extensions.size();
    createInstance.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_isDEBUG)
    {
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInstance.enabledLayerCount = (uint32_t)m_validationLayers.size();
        createInstance.ppEnabledLayerNames = m_validationLayers.data();
        createInstance.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

    if (vkCreateInstance(&createInstance, VK_NULL_HANDLE, &m_instance) != VK_SUCCESS)
        throw std::exception("Failed to create an Instance");
}

// More information:
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFamilyProperties.html
VulkanEngine::QueueFamilies VulkanEngine::findQueueFamilies(const VkPhysicalDevice& device)
{
    QueueFamilies indices;

    uint32_t queueFamilyIndicesCount{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyIndicesCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyIndicesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyIndicesCount, queueFamilyProperties.data());
    
    int i = 0;
    for (const auto& queue : queueFamilyProperties)
    {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.ms_graphicsFamily = i;
        if (indices.hasValue()) return indices;
        i++;
    }

    return indices;
}

bool VulkanEngine::isDeviceSuitable(const VkPhysicalDevice& device)
{
    // More Information:
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceProperties.html
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // More Information:
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
    VkPhysicalDeviceFeatures deviceFeatures{};
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilies indices = findQueueFamilies(device);

    return
        (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
        indices.hasValue();
}

void VulkanEngine::pickPhysicalDevice()
{
    uint32_t deviceCount{};
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, VK_NULL_HANDLE);
    if (!deviceCount) throw std::exception("Failed to find Vulkan compatible GPUs");
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    for (auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
        throw std::exception("GPU Failed to meet the requirements(not suitable)");
}

VulkanEngine::VulkanEngine()
{
    initWindow();
    createInstance();
    setupDebugMessenger(m_instance, m_debugMessenger, m_isDEBUG);
    pickPhysicalDevice();
}

void VulkanEngine::run()
{
    mainLoop();
}

void VulkanEngine::mainLoop()
{
/*
    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
*/
}

VulkanEngine::~VulkanEngine()
{
// VULKAN
    DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, VK_NULL_HANDLE);
    vkDestroyInstance(m_instance, VK_NULL_HANDLE);

// GLFW
    glfwDestroyWindow(m_window);
    glfwTerminate();
}