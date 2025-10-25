#include "include/defs.h"
#include "include/alloc.h"
#include "include/private.h"

#include <stdio.h>
#include <stdlib.h>
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

        
        
        resource->size = size;
        
        return resource;

    } else {
        // TODO: Implement Indirect GPU allocation
        return NULL;
    }

}