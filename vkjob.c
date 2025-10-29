#include "include/defs.h"
#include "include/alloc.h"
#include "include/private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
    if (vkCreateDescriptorPool(ctx->logicalDevice, &descPoolCreateInfo, NULL, &pool) != VK_SUCCESS) {
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
    if (vkCreateDescriptorSetLayout(ctx->logicalDevice, &descSetLayoutCreateInfo, NULL, 
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

    if (vkAllocateDescriptorSets(ctx->logicalDevice, &descSetAllocateInfo, &descSet) != VK_SUCCESS) {
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

    vkUpdateDescriptorSets(ctx->logicalDevice, 1, &writeSet, 0, NULL);

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
    if (vkCreatePipelineLayout(ctx->logicalDevice, &pipelineLayoutCreateInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        *err = UNKNOWN_ERROR;
        return NULL;
    }

    Job ret = Alloc(sizeof(VulkanJob));
    VulkanJob* vjob = ret;
    
    vkGetDeviceQueue(ctx->logicalDevice, ctx->selectedQueueFamily, 0, &vjob->queue);

    VkCommandPoolCreateInfo cpoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueFamilyIndex = ctx->selectedQueueFamily
    };

    if (vkCreateCommandPool(ctx->logicalDevice, &cpoolCreateInfo, NULL, &vjob->cpool) != VK_SUCCESS) {
        *err = UNKNOWN_ERROR;
        return NULL;
    }

    vjob->descPool = pool;
    vjob->descSet = descSet;
    vjob->descSetLayout = descSetLayout;
    vjob->pipelineLayout = pipelineLayout;

    FILE* file = fopen("shader.spv", "rb");
    void* shader = Alloc(10000 * sizeof(uint8_t));
    int size = fread(shader, 1, 10000, file);

    VkShaderModule module;
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = size,
        .pCode = shader
    };
    
    vkCreateShaderModule(ctx->logicalDevice, &shaderModuleCreateInfo, NULL, &module);

    VkPipelineShaderStageCreateInfo shaderCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = module,
        .pName = "main",
        .pSpecializationInfo = NULL
    };

    VkComputePipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = shaderCreateInfo,
        .layout = pipelineLayout,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    if (vkCreateComputePipelines(ctx->logicalDevice, VK_NULL_HANDLE, 1, 
        &pipelineCreateInfo, NULL, &vjob->pipeline) != VK_SUCCESS) {
        *err = FAULTY_GPU_DRIVER;
        return NULL;
    }

    vjob->shader = module;
    return ret;
}

int SubmitJob(Vulkan* vulkan, Job job, Resource* inputs) {
    VulkanJob* vjob = job;
    VulkanContext* ctx = vulkan->data;

    VkCommandBufferAllocateInfo commandBuffer = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = vjob->cpool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer cmdBuffer;
    if (vkAllocateCommandBuffers(ctx->logicalDevice, &commandBuffer, &cmdBuffer) 
            != VK_SUCCESS) {
        return OUT_OF_MEMORY;
    }

    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };

    if (vkCreateFence(ctx->logicalDevice, &fenceCreateInfo, NULL, &fence) != VK_SUCCESS) 
        return UNKNOWN_ERROR;

    VkCommandBufferBeginInfo cmdBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = 0,
        .pInheritanceInfo = NULL
    };

    if (vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo) != VK_SUCCESS) 
        return UNKNOWN_ERROR;

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vjob->pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
        vjob->pipelineLayout, 0, 1, &vjob->descSet, 0, 0);
    vkCmdDispatch(cmdBuffer, 8, 1, 1);
    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = NULL,
        .pWaitDstStageMask = NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = NULL
    };

    if (vkQueueSubmit(ctx->queue, 1, &submitInfo, fence) != VK_SUCCESS)
        return FAULTY_GPU_DRIVER;

    vkWaitForFences(ctx->logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);
    vkQueueWaitIdle(ctx->queue);

    VulkanResource* res = inputs[0];
    void* out;
    vkMapMemory(ctx->logicalDevice, res->backingMemory, 0, VK_WHOLE_SIZE, 0, &out);

    int* output = out;
    printf("Output: ");
    for (int i = 0; i < 8; i++) {
        printf("%d ", output[i]);
    }

    vkUnmapMemory(ctx->logicalDevice, res->backingMemory);
    vkDestroyFence(ctx->logicalDevice, fence, NULL);
    
    return SUCCESS;
}

void DestroyJob(Vulkan* vulkan, Job job) {
    VulkanJob* vjob = job;
    VulkanContext* ctx = vulkan->data;
    vkDestroyCommandPool(ctx->logicalDevice, vjob->cpool, NULL);
    vkDestroyPipelineLayout(ctx->logicalDevice, vjob->pipelineLayout, NULL);
    vkDestroyDescriptorPool(ctx->logicalDevice, vjob->descPool, NULL);
    vkDestroyDescriptorSetLayout(ctx->logicalDevice, vjob->descSetLayout, NULL);
    vkDestroyPipeline(ctx->logicalDevice, vjob->pipeline, NULL);
    vkDestroyShaderModule(ctx->logicalDevice, vjob->shader, NULL);
}