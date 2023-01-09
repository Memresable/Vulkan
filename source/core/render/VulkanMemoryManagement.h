#ifndef _VULKAN_MEMORY_MANAGEMENT_H
#define _VULKAN_MEMORY_MANAGEMENT_H

#include "../math/Vector.h"

#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan.h"
#include "C:\VulkanSDK\1.3.236.0\Include\vulkan\vulkan_win32.h"

typedef struct
{
    Vector2f position;
    Vector3f color;
} Vertex;

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
VK_createVertexBuffer(VkDevice f_device, VkPhysicalDevice f_physicalDevice, VkBuffer* f_vertexBuffer, VkDeviceMemory* f_vertexBufferMemory)
{
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(globalVertices[0]) * ARRAY_SIZE(globalVertices);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    MEMRE_ASSERT(vkCreateBuffer(f_device, &bufferInfo, NULL, f_vertexBuffer) != VK_SUCCESS,
                 "Failed to create a vertex buffer");
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(f_device, *f_vertexBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(f_physicalDevice, memRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    MEMRE_ASSERT(vkAllocateMemory(f_device, &allocInfo, NULL, f_vertexBufferMemory) != VK_SUCCESS,
                 "Failed to allocate vertex buffer memory");
    
    vkBindBufferMemory(f_device, *f_vertexBuffer, *f_vertexBufferMemory, 0);
    
    void* data;
    vkMapMemory(f_device, *f_vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    CopyMemory(data, globalVertices, (size_t)bufferInfo.size);
    vkUnmapMemory(f_device, *f_vertexBufferMemory);
}

#endif