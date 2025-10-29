#ifndef __PRIVATE_H__
#define __PRIVATE_H__

#include "defs.h"
#include <stdint.h>
#include "generated/public.h"

void* GetFunction(Vulkan* vulkan, const char* name);

extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
extern PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
extern PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
extern PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
extern PFN_vkCreateInstance vkCreateInstance;

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

int InitVulkanPrivate(VkInstance instance);

#endif