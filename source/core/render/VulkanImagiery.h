#ifndef _VULKAN_IMAGIERY_H
#define _VULKAN_IMAGIERY_H

#include "VulkanMemoryManagement.h"

#include <windows.h>

#include "../utilities/general_utilities.h"
#include "../math/Vector.h"

#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"

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
    if((f_capabilities->currentExtent.width != UINT32_MAX) || (f_capabilities->currentExtent.height != UINT32_MAX))
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
VK_createSwapchain(VkDevice f_device, VkSwapchainKHR* f_swapchain,
                   VkPhysicalDevice f_physicalDevice, VkSurfaceKHR f_surface, WindowInfo* f_window,
                   VkImage** f_swapchainImages, uint32_t* f_swapchainImagesArraySize,
                   VkFormat* f_swapchainImageFormat, VkExtent2D* f_swapchainExtent)
{
    SwapChainSupportDetails swapchainSupport = VK_querySwapchainSupport(f_physicalDevice, f_surface);
    
    VkSurfaceFormatKHR surfaceFormat = VK_chooseSwapSurfaceFormat(swapchainSupport.formats, swapchainSupport.formatsArraySize);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D extent = VK_chooseSwapExtent(&swapchainSupport.capabilities, *f_window->width, *f_window->height);
    
    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if((swapchainSupport.capabilities.maxImageCount > 0) && (imageCount > swapchainSupport.capabilities.maxImageCount))
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }
    *f_swapchainImagesArraySize = imageCount;
    
    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = f_surface;
    
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = VK_findQueueFamilies(f_physicalDevice, f_surface);
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
    
    MEMRE_ASSERT(vkCreateSwapchainKHR(f_device, &createInfo, NULL, f_swapchain) != VK_SUCCESS,
                 "Failed to create the swapchain\n");
    
    vkGetSwapchainImagesKHR(f_device, *f_swapchain, &imageCount, NULL);
    *f_swapchainImages = ALLOCATE_MEMORY(VkImage, imageCount);
    vkGetSwapchainImagesKHR(f_device, *f_swapchain, &imageCount, f_swapchainImages[0]);
    
    *f_swapchainImageFormat = surfaceFormat.format;
    *f_swapchainExtent = extent;
}

void
VK_createImageViews(VkDevice f_device, VkImageView** f_swapchainImageViews, uint32_t f_swapchainImagesArraySize,
                    VkImage* f_swapchainImages, VkFormat f_swapchainImageFormat)
{
    *f_swapchainImageViews = ALLOCATE_MEMORY(VkImageView, f_swapchainImagesArraySize);
    for(uint32_t i = 0; i < f_swapchainImagesArraySize; i++)
    {
        VkImageViewCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = f_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = f_swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        MEMRE_ASSERT(vkCreateImageView(f_device, &createInfo, NULL, &f_swapchainImageViews[0][i]) != VK_SUCCESS,
                     "Failed to create image views\n");
    }
}

void
VK_createRenderPass(VkDevice f_device, VkRenderPass* f_renderPass, VkFormat f_swapchainImageFormat)
{
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = f_swapchainImageFormat;
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
    
    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    MEMRE_ASSERT(vkCreateRenderPass(f_device, &renderPassInfo, NULL, f_renderPass) != VK_SUCCESS,
                 "Failed to create render pass\n");
}

typedef struct
{
    char* binaryData;
    uint32_t binarySize;
} VulkanShaderBinaryData;

VulkanShaderBinaryData*
VK_readShaderFile(LPCWSTR f_binaryShaderFile)
{
    HANDLE hFile = CreateFile((LPCWSTR)f_binaryShaderFile,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                              NULL);
    
    if(hFile == INVALID_HANDLE_VALUE) 
    { 
        OutputDebugStringA("Unable to open the file\n");
        return(NULL);
    }
    
    int64_t binarySize = 0;
    GetFileSizeEx(hFile, (PLARGE_INTEGER)&binarySize);
    char* readBuffer = ALLOCATE_MEMORY(char, binarySize);
    OVERLAPPED ol = {0};
    if(ReadFileEx(hFile, readBuffer, binarySize, &ol, NULL) == FALSE)
    {
        OutputDebugStringA("Reading the binary shader file was interrupted\n");
        CloseHandle(hFile);
        return(NULL);
    }
    
    CloseHandle(hFile);
    
    VulkanShaderBinaryData* result = ALLOCATE_MEMORY(VulkanShaderBinaryData, 1);
    result->binaryData = readBuffer;
    result->binarySize = binarySize;
    return(result);
}

VkShaderModule
VK_createShaderModule(VkDevice f_device, VulkanShaderBinaryData* f_shaderBinary)
{
    VkShaderModule shaderModule = {0};
    
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = f_shaderBinary->binarySize;
    createInfo.pCode = (const uint32_t*)f_shaderBinary->binaryData;
    
    MEMRE_ASSERT(vkCreateShaderModule(f_device, &createInfo, NULL, &shaderModule) != VK_SUCCESS,
                 "Failed to create a shader module\n");
    return(shaderModule);
}

void
VK_createGraphicsPipeline(VkDevice f_device, VkPipelineLayout* f_pipelineLayout, VkPipeline* f_graphicsPipeline,
                          VkExtent2D f_swapchainExtent, VkRenderPass f_renderPass)
{
    VulkanShaderBinaryData* vertexData =
        VK_readShaderFile(L"..\\source\\core\\render\\shaders\\vertex.spv");
    VulkanShaderBinaryData* fragmentData =
        VK_readShaderFile(L"..\\source\\core\\render\\shaders\\fragment.spv");
    MEMRE_ASSERT((!vertexData->binaryData) || (!fragmentData->binaryData), "Unable to read the binary shader files\n");
    
    VkShaderModule vertexShaderModule = VK_createShaderModule(f_device, vertexData);
    VkShaderModule fragmentShaderModule = VK_createShaderModule(f_device, fragmentData);
    
    RELEASE_MEMORY(vertexData);
    RELEASE_MEMORY(fragmentData);
    
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
    
    VkVertexInputBindingDescription bindingDescription = Vertex_getBindingDescription();
    VertexInputAttributeDescriptionArray attributeDescriptions = Vertex_getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data;
    
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
    viewport.width = (float32_t)f_swapchainExtent.width;
    viewport.height = (float32_t)f_swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = f_swapchainExtent;
    
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
    
    MEMRE_ASSERT(vkCreatePipelineLayout(f_device, &pipelineLayoutInfo, NULL, f_pipelineLayout) != VK_SUCCESS,
                 "Failed to create pipeline layout\n");
    
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
    pipelineInfo.layout = *f_pipelineLayout;
    pipelineInfo.renderPass = f_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    MEMRE_ASSERT(vkCreateGraphicsPipelines(f_device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, f_graphicsPipeline) != VK_SUCCESS,
                 "Failed to create graphics pipeline\n");
    
    vkDestroyShaderModule(f_device, vertexShaderModule, NULL);
    vkDestroyShaderModule(f_device, fragmentShaderModule, NULL);
}

void
VK_createFramebuffers(VkDevice f_device, VkFramebuffer** f_swapchainFramebuffers,
                      VkImageView** f_swapchainImageViews, uint32_t f_swapchainImagesSize,
                      VkRenderPass f_renderPass, VkExtent2D f_swapchainExtent)
{
    *f_swapchainFramebuffers = ALLOCATE_MEMORY(VkFramebuffer, f_swapchainImagesSize);
    for(uint32_t i = 0; i < f_swapchainImagesSize; i++)
    {
        VkImageView attachments[1] = {f_swapchainImageViews[0][i]};
        
        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = f_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = f_swapchainExtent.width;
        framebufferInfo.height = f_swapchainExtent.height;
        framebufferInfo.layers = 1;
        
        MEMRE_ASSERT(vkCreateFramebuffer(f_device, &framebufferInfo, NULL, &f_swapchainFramebuffers[0][i]) != VK_SUCCESS,
                     "Failed to create framebuffers\n");
    }
}

#endif