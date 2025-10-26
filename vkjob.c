#include "include/defs.h"
#include "include/alloc.h"
#include "include/private.h"
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>


Job CreateJob(Vulkan* vulkan, Resource* inputs, int* err) {
    if (!vulkan) {
        puts("CreateJob(): vulkan == NULL");
        abort();
    }

    VulkanContext* ctx = vulkan->data;
    int n = 0;
    Resource* copy = inputs;
    while (*copy) {
        n++;
        copy++;
    }

    VkDescriptorPoolSize poolSize = {
        .descriptorCount = n,
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    };

    VkDescriptorPoolCreateInfo descPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = n,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    VkDescriptorPool pool;
    if (VkCreateDescriptorPool(ctx->logicalDevice, &descPoolCreateInfo, NULL, &pool) != VK_SUCCESS) {
        *err = OUT_OF_GPU_MEMORY;
        return NULL;
    }


    VkDescriptorSetLayoutBinding* descSetLayoutBinding = Alloc(sizeof(VkDescriptorSetLayoutBinding) * n);
    for (int i = 0; i < n; i++) {
        descSetLayoutBinding[i].binding = 0;
        descSetLayoutBinding[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descSetLayoutBinding[i].descriptorCount = 1;
        descSetLayoutBinding[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        descSetLayoutBinding[i].pImmutableSamplers = NULL;
    }

    VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .bindingCount = n,
        .pBindings = descSetLayoutBinding
    };

    VkDescriptorSetLayout descSetLayout;
    if (VkCreateDescriptorSetLayout(ctx->logicalDevice, &descSetLayoutCreateInfo, NULL, 
        &descSetLayout) != VK_SUCCESS) {
        *err = OUT_OF_GPU_MEMORY;
        return NULL;
    }

    VkDescriptorSet descSet;
    VkDescriptorSetAllocateInfo descSetAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = NULL,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descSetLayout
    };

    if (VkAllocateDescriptorSets(ctx->logicalDevice, &descSetAllocateInfo, &descSet) != VK_SUCCESS) {
        *err = OUT_OF_GPU_MEMORY;
        return NULL;
    }

    VkDescriptorBufferInfo* descBuffers = Alloc(n * sizeof(VkDescriptorBufferInfo));
    for (int i = 0; i < n; i++) {
        VulkanResource* res = inputs[i];
        descBuffers[i].buffer = res->buffer;
        descBuffers[i].offset = 0;
        descBuffers[i].range = VK_WHOLE_SIZE;
    }

    VkWriteDescriptorSet writeSet = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = NULL,
        .dstSet = descSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = n,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = NULL,
        .pBufferInfo = descBuffers,
        .pTexelBufferView = NULL
    };

    VkUpdateDescriptorSets(ctx->logicalDevice, 1, &writeSet, 0, NULL);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &descSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };

    VkPipelineLayout pipelineLayout;
    if (VkCreatePipelineLayout(ctx->logicalDevice, &pipelineLayoutCreateInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        *err = UNKNOWN_ERROR;
        return NULL;
    }

    Job ret = Alloc(sizeof(VulkanJob));
    VulkanJob* vjob = ret;
    
    VkGetDeviceQueue(ctx->logicalDevice, ctx->selectedQueueFamily, 0, &vjob->queue);

    VkCommandPoolCreateInfo cpoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueFamilyIndex = ctx->selectedQueueFamily
    };

    if (VkCreateCommandPool(ctx->logicalDevice, &cpoolCreateInfo, NULL, &vjob->cpool) != VK_SUCCESS) {
        *err = UNKNOWN_ERROR;
        return NULL;
    }

    return ret;
}

void DestroyJob(Vulkan* vulkan, Job job) {
    VulkanJob* vjob = job;
    VulkanContext* ctx = vulkan->data;
    VkDestroyCommandPool(ctx->logicalDevice, vjob->cpool, NULL);
}