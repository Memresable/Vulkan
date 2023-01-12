#ifndef _VULKAN_MEMORY_MANAGEMENT_H
#define _VULKAN_MEMORY_MANAGEMENT_H

#include "../math/Vector.h"

#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"

/*
* TODO: Think about this quote from Vulkan Tutorial:
* "It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory for every
*  individual buffer. The maximum number of simultaneous memory allocations
*  is limited by the maxMemoryAllocationCount physical device limit, which may be as low as 4096
*  even on high end hardware like an NVIDIA GTX 1080. The right way to allocate memory for a large number
*  of objects at the same time is to create a custom allocator that splits up a single allocation
*  among many different objects by using the offset parameters that we've seen in many functions."
*/

typedef struct
{
    Vector2f position;
    Vector3f color;
} Vertex;

/*
const Vertex globalVertices[3] =
{
    {
        {0.0f, -0.5f}, // position
        {1.0f, 0.0f, 0.0f} // color
    },
    {
        {0.5f, 0.5f},
        {0.0f, 1.0f, 0.0f}
    },
    {
        {-0.5f, 0.5f},
        {0.0f, 0.0f, 1.0f}
    }
};
*/

const Vertex globalVertices[4] =
{
    { // top left
        {-0.5f, -0.5f}, // position
        {1.0f, 0.0f, 0.0f} // color
    },
    { // top right
        {0.5f, -0.5f},
        {0.0f, 1.0f, 0.0f}
    },
    { // bottom right
        {0.5f, 0.5f},
        {0.0f, 0.0f, 1.0f}
    },
    { // bottom left
        {-0.5f, 0.5f},
        {1.0f, 1.0f, 1.0f}
    }
};
const uint16_t globalIndices[6] =
{
    0, 1, 2, 2, 3, 0
};

static VkVertexInputBindingDescription
Vertex_getBindingDescription()
{
    VkVertexInputBindingDescription result = {0};
    result.binding = 0;
    result.stride = sizeof(Vertex);
    result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return(result);
}

typedef struct
{
    VkVertexInputAttributeDescription* data;
    uint32_t size;
} VertexInputAttributeDescriptionArray;

VertexInputAttributeDescriptionArray
Vertex_getAttributeDescriptions()
{
    VertexInputAttributeDescriptionArray result = {ALLOCATE_MEMORY(VkVertexInputAttributeDescription, 2), 2};
    
    // position
    result.data[0].binding = 0;
    result.data[0].location = 0;
    result.data[0].format = VK_FORMAT_R32G32_SFLOAT;
    result.data[0].offset = offsetof(Vertex, position);
    // color
    result.data[1].binding = 0;
    result.data[1].location = 1;
    result.data[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    result.data[1].offset = offsetof(Vertex, color);
    
    return(result);
}

uint32_t
findMemoryType(VkPhysicalDevice f_physicalDevice, uint32_t f_typeFilter, VkMemoryPropertyFlags f_properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(f_physicalDevice, &memProperties);
    
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if((f_typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & f_properties) == f_properties)
        {
            return(i);
        }
    }
    MEMRE_ASSERT(MEMRE_TRUE, "Failed to find any suitable memory type");
}

void
VK_createBuffer(VkDevice f_device, VkPhysicalDevice f_physicalDevice,
                VkDeviceSize f_size, VkBufferUsageFlags f_usage, VkMemoryPropertyFlags f_properties,
                VkBuffer* f_vertexBuffer, VkDeviceMemory* f_vertexBufferMemory)
{
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = f_size;
    bufferInfo.usage = f_usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    MEMRE_ASSERT(vkCreateBuffer(f_device, &bufferInfo, NULL, f_vertexBuffer) != VK_SUCCESS,
                 "Failed to create a vertex buffer");
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(f_device, *f_vertexBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(f_physicalDevice, memRequirements.memoryTypeBits, f_properties);
    MEMRE_ASSERT(vkAllocateMemory(f_device, &allocInfo, NULL, f_vertexBufferMemory) != VK_SUCCESS,
                 "Failed to allocate vertex buffer memory");
    
    vkBindBufferMemory(f_device, *f_vertexBuffer, *f_vertexBufferMemory, 0);
}

void
VK_copyBuffer(VkDevice f_device, VkCommandPool f_commandPool, VkQueue f_graphicsQueue,
              VkBuffer f_srcBuffer, VkBuffer f_dstBuffer, VkDeviceSize f_size)
{
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = f_commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(f_device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkBufferCopy copyRegion = {0};
    copyRegion.size = f_size;
    vkCmdCopyBuffer(commandBuffer, f_srcBuffer, f_dstBuffer, 1, &copyRegion);
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(f_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(f_graphicsQueue);
    
    vkFreeCommandBuffers(f_device, f_commandPool, 1, &commandBuffer);
}

void
VK_createVertexBuffer(VkDevice f_device, VkPhysicalDevice f_physicalDevice, VkCommandPool f_commandPool, VkQueue f_graphicsQueue,
                      VkBuffer* f_vertexBuffer, VkDeviceMemory* f_vertexBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(globalVertices[0]) * ARRAY_SIZE(globalVertices);
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VK_createBuffer(f_device, f_physicalDevice,
                    bufferSize, // size
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // usage
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // properties
                    &stagingBuffer, &stagingBufferMemory);
    
    void* data = NULL;
    vkMapMemory(f_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    CopyMemory(data, globalVertices, (size_t)bufferSize);
    vkUnmapMemory(f_device, stagingBufferMemory);
    
    VK_createBuffer(f_device, f_physicalDevice,
                    bufferSize, // size
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // usage
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // properties
                    f_vertexBuffer, f_vertexBufferMemory);
    
    VK_copyBuffer(f_device, f_commandPool, f_graphicsQueue,
                  stagingBuffer, *f_vertexBuffer, bufferSize);
    
    vkDestroyBuffer(f_device, stagingBuffer, NULL);
    vkFreeMemory(f_device, stagingBufferMemory, NULL);
}

void
VK_createIndexBuffer(VkDevice f_device, VkPhysicalDevice f_physicalDevice, VkCommandPool f_commandPool, VkQueue f_graphicsQueue,
                     VkBuffer* f_indexBuffer, VkDeviceMemory* f_indexBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(globalIndices[0]) * ARRAY_SIZE(globalIndices);
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VK_createBuffer(f_device, f_physicalDevice,
                    bufferSize, // size
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // usage
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // properties
                    &stagingBuffer, &stagingBufferMemory);
    
    void* data = NULL;
    vkMapMemory(f_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    CopyMemory(data, globalIndices, (size_t)bufferSize);
    vkUnmapMemory(f_device, stagingBufferMemory);
    
    VK_createBuffer(f_device, f_physicalDevice,
                    bufferSize, // size
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, // usage
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // properties
                    f_indexBuffer, f_indexBufferMemory);
    
    VK_copyBuffer(f_device, f_commandPool, f_graphicsQueue,
                  stagingBuffer, *f_indexBuffer, bufferSize);
    
    vkDestroyBuffer(f_device, stagingBuffer, NULL);
    vkFreeMemory(f_device, stagingBufferMemory, NULL);
}

#endif