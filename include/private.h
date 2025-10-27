#ifndef __PRIVATE_H__
#define __PRIVATE_H__

#include "defs.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

void* GetFunction(Vulkan* vulkan, const char* name);

extern PFN_vkGetInstanceProcAddr VkGetInstanceProcAddr;
extern PFN_vkCreateInstance VkCreateInstance;
extern PFN_vkEnumerateInstanceLayerProperties VkEnumerateInstanceLayerProperties;
extern PFN_vkEnumerateInstanceExtensionProperties VkEnumerateInstanceExtensionProperties;
extern PFN_vkEnumerateInstanceVersion VkEnumerateInstanceVersion;
extern PFN_vkDestroyInstance VkDestroyInstance;

extern PFN_vkEnumeratePhysicalDevices VkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceProperties VkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties VkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkGetDeviceQueue VkGetDeviceQueue;
extern PFN_vkCreateDevice VkCreateDevice;
extern PFN_vkDestroyDevice VkDestroyDevice;

extern PFN_vkGetPhysicalDeviceMemoryProperties VkGetPhysicalDeviceMemoryProperties;
extern PFN_vkCreateBuffer VkCreateBuffer;
extern PFN_vkGetBufferMemoryRequirements VkGetBufferMemoryRequirements;
extern PFN_vkAllocateMemory VkAllocateMemory;
extern PFN_vkMapMemory VkMapMemory; 
extern PFN_vkUnmapMemory VkUnmapMemory;
extern PFN_vkBindBufferMemory VkBindBufferMemory;
extern PFN_vkFlushMappedMemoryRanges VkFlushMappedMemoryRanges;
extern PFN_vkDestroyBuffer VkDestroyBuffer;
extern PFN_vkFreeMemory VkFreeMemory;
extern PFN_vkCreateDescriptorPool VkCreateDescriptorPool;
extern PFN_vkCreateDescriptorSetLayout VkCreateDescriptorSetLayout;
extern PFN_vkAllocateDescriptorSets VkAllocateDescriptorSets;
extern PFN_vkUpdateDescriptorSets VkUpdateDescriptorSets;
extern PFN_vkCreatePipelineLayout VkCreatePipelineLayout;

extern PFN_vkGetDeviceQueue VkGetDeviceQueue;
extern PFN_vkCreateCommandPool VkCreateCommandPool;
extern PFN_vkDestroyCommandPool VkDestroyCommandPool;
extern PFN_vkDestroyPipelineLayout VkDestroyPipelineLayout;
extern PFN_vkDestroyDescriptorSetLayout VkDestroyDescriptorSetLayout;
extern PFN_vkDestroyDescriptorPool VkDestroyDescriptorPool;

extern PFN_vkCreateShaderModule VkCreateShaderModule;
extern PFN_vkCreateComputePipelines VkCreateComputePipelines;
extern PFN_vkDestroyPipeline VkDestroyPipeline;
extern PFN_vkDestroyShaderModule VkDestroyShaderModule;

extern PFN_vkAllocateCommandBuffers VkAllocateCommandBuffers;
extern PFN_vkCreateFence VkCreateFence;

extern PFN_vkBeginCommandBuffer VkBeginCommandBuffer;
extern PFN_vkEndCommandBuffer VkEndCommandBuffer;
extern PFN_vkCmdBindPipeline VkCmdBindPipeline;
extern PFN_vkCmdBindDescriptorSets VkCmdBindDescriptorSets;
extern PFN_vkCmdDispatch VkCmdDispatch;
extern PFN_vkQueueSubmit VkQueueSubmit;
extern PFN_vkWaitForFences VkWaitForFences;
extern PFN_vkQueueWaitIdle VkQueueWaitIdle;
extern PFN_vkDestroyFence VkDestroyFence;

enum Flags {
    ALLOCATE_DIRECT = (1 << 0),
    ALLOCATE_INDIRECT = (1 << 1)
};

typedef struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    uint32_t selectedQueueFamily;
    VkQueue  queue;
    VkPhysicalDeviceMemoryProperties mprops;
    uint32_t flags;
    uint32_t memoryHeaps;
} VulkanContext;

typedef struct VulkanResource {
    VkBuffer buffer;
    uint64_t size;
    VkDeviceMemory backingMemory;
} VulkanResource; 

typedef struct VulkanJob {
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descSetLayout;
    VkDescriptorSet descSet;
    VkDescriptorPool descPool;
    VkPipeline pipeline;
    VkQueue queue;
    VkCommandPool cpool;
    VkShaderModule shader;
} VulkanJob;

#endif