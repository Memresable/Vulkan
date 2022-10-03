#include "vulkan.h"

void VulkanEngine::initWindow()
{
    ASSERT_VULKAN(!glfwInit(), "Failed to initialize GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_SCR_WIDTH, m_SCR_HEIGHT, "valaska_v", NULL, NULL);
    ASSERT_VULKAN(!m_window, "Failed to create a Window");

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

VulkanEngine::SwapChainSupportDetails VulkanEngine::getSwapchainDetails
(
    const VkSurfaceCapabilitiesKHR& capabilities,
    const std::vector<VkSurfaceFormatKHR>& availableFormats,
    const std::vector<VkPresentModeKHR>& availablePresents
) {
    SwapChainSupportDetails shiny{};

    for (const auto& availableFormat : availableFormats)
    {
        if ((availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB) && (availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
        {
            shiny.ms_formats.push_back(availableFormat);
            break;
        }
    }

    for (const auto& availablePresent : availablePresents)
    {
        if (availablePresent == VK_PRESENT_MODE_FIFO_KHR)
        {
            shiny.ms_presentModes.push_back(availablePresent);
            break;
        }
    }

    //if (capabilities.currentExtent.width != std::numeric_limits<int>::max())
    //{
    //    shiny.ms_capabilities.currentExtent = capabilities.currentExtent;
    //}
    if(true) // TODO: FIX ME, VALUES ARE NOT PASSED TO THE STRUCT IN THE CORRECT WAY
    {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        VkExtent2D realExtent
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        realExtent.width = std::clamp(realExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        realExtent.height = std::clamp(realExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        shiny.ms_capabilities.currentExtent = realExtent;
        shiny.ms_extent = realExtent;
        shiny.ms_capabilities.currentTransform = capabilities.currentTransform;
        shiny.ms_capabilities.minImageCount = capabilities.minImageCount;
        shiny.ms_capabilities.maxImageCount = capabilities.maxImageCount;
    }

    return shiny;
}

VulkanEngine::SwapChainSupportDetails VulkanEngine::querySwapchainSupport(const VkPhysicalDevice& device)
{
    VulkanEngine::SwapChainSupportDetails result{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &result.ms_capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
    if (formatCount)
    {
        result.ms_formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, result.ms_formats.data());
    }

    uint32_t presentCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentCount, nullptr);
    if (presentCount)
    {
        result.ms_presentModes.resize(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentCount, result.ms_presentModes.data());
    }

    return result;
}

void VulkanEngine::createInstance()
{
    ASSERT_VULKAN(m_isDEBUG && (checkValidationSupport(m_validationLayers) == VK_FALSE), "Validation layers requested, but not available");

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
    createInstance.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInstance.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_isDEBUG)
    {
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInstance.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInstance.ppEnabledLayerNames = m_validationLayers.data();
        createInstance.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

    ASSERT_VULKAN(vkCreateInstance(&createInstance, VK_NULL_HANDLE, &m_instance) != VK_SUCCESS, "Failed to create an Instance");
}

// More information:
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFamilyProperties.html
VulkanEngine::QueueFamilies VulkanEngine::findQueueFamilies(const VkPhysicalDevice& device)
{
    QueueFamilies indices{};

    uint32_t queueFamilyIndicesCount{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyIndicesCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyIndicesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyIndicesCount, queueFamilyProperties.data());
    
    int i = 0;
    VkBool32 presentSupport = VK_FALSE;
    for (const auto& queue : queueFamilyProperties)
    {
        // Graphics queue
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.ms_graphicsFamily = i;

        // Present queue
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
        if (presentSupport) indices.ms_presentFamily = i;

        if (indices.isComplete()) return indices;
        i++;
    }

    return indices;
}

bool VulkanEngine::checkExtensionSupport(const VkPhysicalDevice& device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
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
    bool extensionSupported = checkExtensionSupport(device);

    bool swapchainAdequate = VK_FALSE;
    if (extensionSupported)
    {
        SwapChainSupportDetails swapchainSupport = querySwapchainSupport(device);
        swapchainAdequate = !swapchainSupport.ms_formats.empty() && !swapchainSupport.ms_presentModes.empty();
    }

    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        && indices.isComplete()
        && extensionSupported
        && swapchainAdequate;
}

void VulkanEngine::pickPhysicalDevice()
{
    uint32_t deviceCount{};
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, VK_NULL_HANDLE);
    ASSERT_VULKAN(!deviceCount, "Failed to find Vulkan compatible GPUs");
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

    ASSERT_VULKAN(m_physicalDevice == VK_NULL_HANDLE, "GPU Failed to meet the requirements(not suitable)");
}

void VulkanEngine::createLogicalDevice()
{
    QueueFamilies indices = findQueueFamilies(m_physicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies =
    {
        indices.ms_graphicsFamily.value(), indices.ms_presentFamily.value()
    };
    float queuePriority = 1.0f;

    for(uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

    if(m_isDEBUG) // For compatibility reasons
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    ASSERT_VULKAN(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS, "Failed to create the logical device")

    vkGetDeviceQueue(m_device, indices.ms_graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.ms_presentFamily.value(), 0, &m_presentQueue);
}

void VulkanEngine::createSwapchain()
{
    SwapChainSupportDetails swapchainSupport = querySwapchainSupport(m_physicalDevice);
    SwapChainSupportDetails swapchainInfo = getSwapchainDetails
    (
        swapchainSupport.ms_capabilities,
        swapchainSupport.ms_formats,
        swapchainSupport.ms_presentModes
    );

    uint32_t imageCount = swapchainInfo.ms_capabilities.minImageCount + 1;
    if ((swapchainInfo.ms_capabilities.maxImageCount > 0) && imageCount > swapchainInfo.ms_capabilities.maxImageCount)
        imageCount = swapchainInfo.ms_capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = swapchainInfo.ms_formats[0].format;
    createInfo.imageColorSpace = swapchainInfo.ms_formats[0].colorSpace;
    createInfo.imageExtent = swapchainInfo.ms_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilies indices = findQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.ms_graphicsFamily.value(), indices.ms_presentFamily.value() };
    if (indices.ms_graphicsFamily != indices.ms_presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    
    createInfo.preTransform = swapchainInfo.ms_capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = swapchainInfo.ms_presentModes[0];
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    ASSERT_VULKAN(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS, "Failed to create the swapchain");
}

VulkanEngine::VulkanEngine()
{
    initWindow();
    createInstance();

    // Create window surface
    ASSERT_VULKAN(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS, "Failed to create a surface window");

    setupDebugMessenger(m_instance, m_debugMessenger, m_isDEBUG);
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
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
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroyDevice(m_device, nullptr);
    DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, VK_NULL_HANDLE);

// GLFW
    glfwDestroyWindow(m_window);
    glfwTerminate();
}