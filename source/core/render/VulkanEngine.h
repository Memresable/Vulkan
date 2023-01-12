#ifndef _VULKAN_ENGINE_H
#define _VULKAN_ENGINE_H

#include "VulkanSetup.h"
#include "VulkanCompatibility.h"
#include "VulkanImagiery.h"

#include <windows.h>

#include "../utilities/general_utilities.h"
#include "../utilities/unique_array.h"

#include "../math/vector.h"

// figure out a better way to handle this
#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"
#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan_win32.h"

#define MAX_FRAMES_IN_FLIGHT 2

uint32_t currentFrame = 0;

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
    VkFramebuffer* swapchainFramebuffers;
    
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;
    uint32_t sizeOfCommandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
} VulkanEngine;

void
VK_cleanupSwapchain(VulkanEngine* f_engine)
{
    for(uint32_t i = 0; i < f_engine->swapchainImagesArraySize; i++)
    {
        vkDestroyFramebuffer(f_engine->device, f_engine->swapchainFramebuffers[i], NULL);
        vkDestroyImageView(f_engine->device, f_engine->swapchainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(f_engine->device, f_engine->swapchain, NULL);
}

/*
* TODO: Think about this approach from Vulkan Tutorial:
* "That's all it takes to recreate the swap chain! However, the disadvantage of this approach is that we need to
* stop all rendering *  before creating the new swap chain. It is possible to create a new swap chain while
* drawing commands on an image from the old *swap chain are still in-flight.
* You need to pass the previous swap chain to the oldSwapChain field in the
* VkSwapchainCreateInfoKHR *struct and destroy the old swap chain as soon as you've finished using it. "
*/
void
VK_recreateSwapchain(VulkanEngine* f_engine)
{
    // just trying to get the functionality working for now
    // will cleanup later and try the quote above
    MSG msg = {0};
    VkSurfaceCapabilitiesKHR capabilities = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(f_engine->physicalDevice, f_engine->surface, &capabilities);
    while((capabilities.currentExtent.width == 0) || (capabilities.currentExtent.height == 0))
    {
        Sleep(10); // fixes the high CPU usage when minimized
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(f_engine->physicalDevice, f_engine->surface, &capabilities);
        *f_engine->window.width = capabilities.currentExtent.width;
        *f_engine->window.height = capabilities.currentExtent.height;
        while(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    *f_engine->window.width = capabilities.currentExtent.width;
    *f_engine->window.height = capabilities.currentExtent.height;
    
    vkDeviceWaitIdle(f_engine->device);
    
    VK_cleanupSwapchain(f_engine);
    
    VK_createSwapchain(f_engine->device, &f_engine->swapchain,
                       f_engine->physicalDevice, f_engine->surface, &f_engine->window,
                       &f_engine->swapchainImages, &f_engine->swapchainImagesArraySize,
                       &f_engine->swapchainImageFormat, &f_engine->swapchainExtent);
    VK_createImageViews(f_engine->device, &f_engine->swapchainImageViews, f_engine->swapchainImagesArraySize,
                        f_engine->swapchainImages, f_engine->swapchainImageFormat);
    VK_createFramebuffers(f_engine->device, &f_engine->swapchainFramebuffers,
                          &f_engine->swapchainImageViews, f_engine->swapchainImagesArraySize,
                          f_engine->renderPass, f_engine->swapchainExtent);
}

void
VK_createCommandPool(VulkanEngine* f_engine)
{
    QueueFamilyIndices queueFamilyIndices = VK_findQueueFamilies(f_engine->physicalDevice, f_engine->surface);
    
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = *queueFamilyIndices.graphicsFamily;
    
    MEMRE_ASSERT(vkCreateCommandPool(f_engine->device, &poolInfo, NULL, &f_engine->commandPool) != VK_SUCCESS,
                 "Failed to create command pool\n");
}

void
recordCommandBuffer(VulkanEngine* f_engine, VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    MEMRE_ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS,
                 "Failed to begin recording command buffer\n");
    
    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = f_engine->renderPass;
    renderPassInfo.framebuffer = f_engine->swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = f_engine->swapchainExtent;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // BACKGROUND COLOR
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, f_engine->graphicsPipeline);
    
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float32_t)f_engine->swapchainExtent.width;
    viewport.height = (float32_t)f_engine->swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = f_engine->swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    
    VkBuffer vertexBuffers[1] = {f_engine->vertexBuffer};
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, f_engine->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    
    vkCmdDrawIndexed(commandBuffer, (uint32_t)ARRAY_SIZE(globalIndices), 1, 0, 0, 0);
    
    vkCmdEndRenderPass(commandBuffer);
    
    MEMRE_ASSERT(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS, "Failed to record command buffer\n");
}

void
VK_createCommandBuffers(VulkanEngine* f_engine)
{
    f_engine->sizeOfCommandBuffers = MAX_FRAMES_IN_FLIGHT;
    f_engine->commandBuffers = ALLOCATE_MEMORY(VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);
    
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = f_engine->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = f_engine->sizeOfCommandBuffers;
    
    MEMRE_ASSERT(vkAllocateCommandBuffers(f_engine->device, &allocInfo, &f_engine->commandBuffers[0]) != VK_SUCCESS,
                 "Failed to allocate command buffers\n");
}

void
VK_createSyncObjects(VulkanEngine* f_engine)
{
    f_engine->imageAvailableSemaphores = ALLOCATE_MEMORY(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    f_engine->renderFinishedSemaphores = ALLOCATE_MEMORY(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    f_engine->inFlightFences = ALLOCATE_MEMORY(VkFence, MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        MEMRE_ASSERT(vkCreateSemaphore(f_engine->device, &semaphoreInfo, NULL, &f_engine->imageAvailableSemaphores[i]) != VK_SUCCESS ||
                     vkCreateSemaphore(f_engine->device, &semaphoreInfo, NULL, &f_engine->renderFinishedSemaphores[i]) != VK_SUCCESS ||
                     vkCreateFence(f_engine->device, &fenceInfo, NULL, &f_engine->inFlightFences[i]) != VK_SUCCESS,
                     "Failed to create semaphores\n");
    }
}

void
VK_drawFrame(VulkanEngine* f_engine)
{
    vkWaitForFences(f_engine->device, 1, &f_engine->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(f_engine->device, f_engine->swapchain, UINT64_MAX,
                                            f_engine->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        VK_recreateSwapchain(f_engine);
        return;
    }
    else MEMRE_ASSERT((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR), "Failed to acquire swapchain image\n");
    
    vkResetFences(f_engine->device, 1, &f_engine->inFlightFences[currentFrame]);
    
    vkResetCommandBuffer(f_engine->commandBuffers[currentFrame], 0);
    recordCommandBuffer(f_engine, f_engine->commandBuffers[currentFrame], imageIndex);
    
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[1] = {f_engine->imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &f_engine->commandBuffers[currentFrame];
    VkSemaphore signalSemaphores[1] = {f_engine->renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    MEMRE_ASSERT(vkQueueSubmit(f_engine->graphicsQueue, 1, &submitInfo, f_engine->inFlightFences[currentFrame]) != VK_SUCCESS,
                 "Failed to submit draw command buffer\n");
    
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapchains[1] = {f_engine->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    
    result = vkQueuePresentKHR(f_engine->presentQueue, &presentInfo);
    
    if((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
    {
        VK_recreateSwapchain(f_engine);
    }
    else MEMRE_ASSERT((result != VK_SUCCESS), "Failed to present swapchain image\n");
    
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void
VK_run(VulkanEngine* f_engine, int* f_appIsRunning)
{
    MSG msg = {0};
    while(*f_appIsRunning)
    {
        while(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        VK_drawFrame(f_engine);
    }
    vkDeviceWaitIdle(f_engine->device);
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
    VK_createInstance(&f_engine->instance,
                      f_engine->surfaceExtensions.array, (uint32_t)ARRAY_SIZE(f_engine->surfaceExtensions.array),
                      f_engine->validationExtensions.array, (uint32_t)ARRAY_SIZE(f_engine->validationExtensions.array));
    VK_setupDebugMessenger(f_engine->instance, &f_engine->debugMessenger);
    VK_createSurface(f_engine->instance, &f_engine->surface, &f_engine->window);
    VK_pickPhysicalDevice(f_engine->instance, f_engine->surface,
                          &f_engine->swapchainExtensions,
                          &f_engine->physicalDevice);
    VK_createLogicalDevice(&f_engine->device, f_engine->physicalDevice, f_engine->surface,
                           &f_engine->graphicsQueue, &f_engine->presentQueue,
                           &f_engine->swapchainExtensions,
                           f_engine->validationExtensions.array, ARRAY_SIZE(f_engine->validationExtensions.array));
    VK_createSwapchain(f_engine->device, &f_engine->swapchain,
                       f_engine->physicalDevice, f_engine->surface, &f_engine->window,
                       &f_engine->swapchainImages, &f_engine->swapchainImagesArraySize,
                       &f_engine->swapchainImageFormat, &f_engine->swapchainExtent);
    VK_createImageViews(f_engine->device, &f_engine->swapchainImageViews, f_engine->swapchainImagesArraySize,
                        f_engine->swapchainImages, f_engine->swapchainImageFormat);
    VK_createRenderPass(f_engine->device, &f_engine->renderPass, f_engine->swapchainImageFormat);
    VK_createGraphicsPipeline(f_engine->device, &f_engine->pipelineLayout, &f_engine->graphicsPipeline,
                              f_engine->swapchainExtent, f_engine->renderPass);
    VK_createFramebuffers(f_engine->device, &f_engine->swapchainFramebuffers,
                          &f_engine->swapchainImageViews, f_engine->swapchainImagesArraySize,
                          f_engine->renderPass, f_engine->swapchainExtent);
    VK_createCommandPool(f_engine);
    VK_createVertexBuffer(f_engine->device, f_engine->physicalDevice, f_engine->commandPool, f_engine->graphicsQueue,
                          &f_engine->vertexBuffer, &f_engine->vertexBufferMemory);
    VK_createIndexBuffer(f_engine->device, f_engine->physicalDevice, f_engine->commandPool, f_engine->graphicsQueue,
                         &f_engine->indexBuffer, &f_engine->indexBufferMemory);
    VK_createCommandBuffers(f_engine);
    VK_createSyncObjects(f_engine);
}

void
VK_cleanup(VulkanEngine* f_engine)
{
    VK_cleanupSwapchain(f_engine);
    
    vkDestroyBuffer(f_engine->device, f_engine->indexBuffer, NULL);
    vkFreeMemory(f_engine->device, f_engine->indexBufferMemory, NULL);
    vkDestroyBuffer(f_engine->device, f_engine->vertexBuffer, NULL);
    vkFreeMemory(f_engine->device, f_engine->vertexBufferMemory, NULL);
    
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(f_engine->device, f_engine->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(f_engine->device, f_engine->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(f_engine->device, f_engine->inFlightFences[i], NULL);
    }
    vkDestroyCommandPool(f_engine->device, f_engine->commandPool, NULL);
    vkDestroyPipeline(f_engine->device, f_engine->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(f_engine->device, f_engine->pipelineLayout, NULL);
    vkDestroyRenderPass(f_engine->device, f_engine->renderPass, NULL);
    
    vkDestroyDevice(f_engine->device, NULL);
    if(globalEnableValidationLayers)
    {
        VK_destroyDebugUtilsMessengerEXT(f_engine->instance, f_engine->debugMessenger, NULL);
    }
    vkDestroySurfaceKHR(f_engine->instance, f_engine->surface, NULL);
    vkDestroyInstance(f_engine->instance, NULL);
}

#endif