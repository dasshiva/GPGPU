#include "include/defs.h"
#include "include/alloc.h"
#include "include/private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

Resource CreateResource(Vulkan* vulkan, void* data, uint64_t size, int* err) {
    if (!size)
        return NULL;

    if (!vulkan) {
        puts("CreateResource(): vulkan == NULL");
        abort();
    }

    VulkanContext* ctx = vulkan->data;

    if (ctx->flags & ALLOCATE_DIRECT) {
        uint32_t memoryIndex = ctx->memoryHeaps & 0xFFFF;
        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .size  = size,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = &ctx->selectedQueueFamily
        };

        VulkanResource* resource = Alloc(sizeof(VulkanResource));
        if (VkCreateBuffer(ctx->logicalDevice, &bufferCreateInfo, NULL, &resource->buffer)
            != VK_SUCCESS) {
                *err = OUT_OF_MEMORY;
                return NULL;
        }

        VkMemoryRequirements memReqs;
        VkGetBufferMemoryRequirements(ctx->logicalDevice, resource->buffer, &memReqs);
        resource->size = memReqs.size;

        VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = NULL,
            .allocationSize = resource->size,
            .memoryTypeIndex = memoryIndex
        };

        if (VkAllocateMemory(ctx->logicalDevice, &memoryAllocateInfo, NULL, &resource->backingMemory)
                != VK_SUCCESS) {
            *err = OUT_OF_GPU_MEMORY;
            return NULL;
        }

        void* mapped;
        if (VkMapMemory(ctx->logicalDevice, resource->backingMemory, 0, 
                    resource->size, 0, &mapped) != VK_SUCCESS) {
            *err = UNKNOWN_ERROR;
            return NULL;
        }

        memcpy(mapped, data, size);

        if (!(ctx->mprops.memoryTypes[memoryIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            VkMappedMemoryRange mappedMemoryRange = {
                .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                .pNext = NULL,
                .memory = resource->backingMemory,
                .offset = 0,
                .size = resource->size
            };

            VkFlushMappedMemoryRanges(ctx->logicalDevice, 1, &mappedMemoryRange);
        }

        VkUnmapMemory(ctx->logicalDevice, resource->backingMemory);
        VkBindBufferMemory(ctx->logicalDevice, resource->buffer, resource->backingMemory, 0);

        return resource;

    } else {
        // TODO: Implement Indirect GPU allocation
        return NULL;
    }

}

void FreeResource(Vulkan* vulkan, Resource resource) {
    VulkanResource* res = resource;
    if (!res) {
        puts("FreeResource(): resource == NULL");
        abort();
    }

    VulkanContext* ctx = vulkan->data;

    if (ctx->flags & ALLOCATE_DIRECT) {
        VkFreeMemory(ctx->logicalDevice, res->backingMemory, NULL);
        VkDestroyBuffer(ctx->logicalDevice, res->buffer, NULL);
    }
    else {
        // Implement GPU indirect de-allocation
    }
}