#include "vulkan.h"

void VulkanEngine::initWindow()
{
    ASSERT_VULKAN(!glfwInit(), "Failed to initialize GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_SCR_WIDTH, m_SCR_HEIGHT, "valaska_v", NULL, NULL);
    ASSERT_VULKAN(!m_window, "Failed to create a window");

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

// TODO: values are not passed to the struct in the correct way
    if(true)
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

    ASSERT_VULKAN(m_physicalDevice == VK_NULL_HANDLE, "GPU Failed meeting the requirements");
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

// TODO: Read more about these
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    m_swapchainImageFormat = swapchainInfo.ms_formats[0].format;
    m_swapchainExtent = swapchainInfo.ms_extent;
}

void VulkanEngine::createImageViews()
{
    m_swapchainImageViews.resize(m_swapchainImages.size());
	for (uint32_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// NOTE: while adding the mipmapping feature, don't forget about this:
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		ASSERT_VULKAN(vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]), "Failed to create an ImageView");
	}
}

void VulkanEngine::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

// pretty much it's this: layout (location = 0) out vec4 outColor, it's cool isn't it?
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &colorAttachment;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;

	ASSERT_VULKAN(vkCreateRenderPass(m_device, &createInfo, nullptr, &m_renderPass) != VK_SUCCESS, "Failed to create Render Pass");
}

std::vector<char> VulkanEngine::loadSPRV(const std::string& fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);
	ASSERT_VULKAN(!file.is_open(), "Failed to read SPR-V Binaries");
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

VkShaderModule VulkanEngine::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	createInfo.codeSize = code.size();
	VkShaderModule shaderModule;
	ASSERT_VULKAN(vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS, "Failed to create Shader Module");
	return shaderModule;
}

void VulkanEngine::createGraphicsPipeline()
{
	auto vertexShader = loadSPRV("SPIR_V_COMPILER\\vertex.sprv");
	auto fragmentShader = loadSPRV("SPIR_V_COMPILER\\fragment.sprv");

	VkShaderModule vertexModule = createShaderModule(vertexShader);
	VkShaderModule fragmentModule = createShaderModule(fragmentShader);

// Programmable Stages
  // Shaders
	VkPipelineShaderStageCreateInfo vertexStageInfo{};
	vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageInfo.module = vertexModule;
	vertexStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentStageInfo{};
	fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStageInfo.module = fragmentModule;
	fragmentStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexStageInfo, fragmentStageInfo };

  // Dynamic State
	std::vector<VkDynamicState> dynamicStates
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateStageInfo{};
	dynamicStateStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateStageInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateStageInfo.pDynamicStates = dynamicStates.data();

// Fixed Stages
  // Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStageInfo{};
	inputAssemblyStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStageInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStageInfo.primitiveRestartEnable = VK_FALSE;

	// Vertex Input
	// TODO: Get back to the Vertex Input stage when starting to use vertex buffers
	// For more information:
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineVertexInputStateCreateInfo.html
	VkPipelineVertexInputStateCreateInfo vertexInputStageInfo{};
	vertexInputStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  // Viewport and Scissors
	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(m_swapchainExtent.width);
	viewport.height= static_cast<float>(m_swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 0.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	viewportInfo.scissorCount = 1;

  // Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizerStageInfo{};
	rasterizerStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerStageInfo.depthClampEnable = VK_FALSE;
	rasterizerStageInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerStageInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerStageInfo.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
	rasterizerStageInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerStageInfo.depthBiasEnable = VK_FALSE;
	rasterizerStageInfo.lineWidth = 1.0f;

  // Color Blending
	// NOTE: FOR TRANSPARENT IMAGES
	VkPipelineColorBlendAttachmentState colorBlendingAttachment{};
	colorBlendingAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendingAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
	colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendingCreateInfo.attachmentCount = 1;
	colorBlendingCreateInfo.pAttachments = &colorBlendingAttachment;

  // Pipeline Layout
	VkPipelineLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	ASSERT_VULKAN
	(
		vkCreatePipelineLayout(m_device, &layoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS,
		"Failed to create the Pipeline Layout"
	);

	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = 2;
	createInfo.pStages = shaderStages;
	createInfo.pVertexInputState = &vertexInputStageInfo;
	createInfo.pInputAssemblyState = &inputAssemblyStageInfo;
	createInfo.pViewportState = &viewportInfo;
	createInfo.pRasterizationState = &rasterizerStageInfo;
	createInfo.pColorBlendState = &colorBlendingCreateInfo;
	createInfo.pDynamicState = &dynamicStateStageInfo;
	createInfo.layout = m_pipelineLayout;
	createInfo.renderPass = m_renderPass;
	createInfo.subpass = 0;

	ASSERT_VULKAN
	(
		vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS,
		"Failed to create the Graphics Pipeline"
	);

	vkDestroyShaderModule(m_device, vertexModule, nullptr);
	vkDestroyShaderModule(m_device, fragmentModule, nullptr);
}

VulkanEngine::VulkanEngine()
{
    initWindow();
    createInstance();

    // Create window surface
    ASSERT_VULKAN
	(
		glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS,
		"Failed to create a surface window"
	);

    setupDebugMessenger(m_instance, m_debugMessenger, m_isDEBUG);
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
}

void VulkanEngine::run() { mainLoop(); }

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
	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	for (auto imageView : m_swapchainImageViews) vkDestroyImageView(m_device, imageView, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroyDevice(m_device, nullptr);
    DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, VK_NULL_HANDLE);

// GLFW
	glfwDestroyWindow(m_window);
    glfwTerminate();
}