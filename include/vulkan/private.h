#ifndef __PRIVATE_H__
#define __PRIVATE_H__

#include "../kc.h"
#include <stdint.h>
#include "generated/public.h"

typedef struct Vulkan {
    Handle    driver;
    char*     name;
    Handle    data;
    uint32_t  version;
} Vulkan;

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

typedef Resource (*createVulkanResource)(Vulkan*, void*, uint64_t, int*);
typedef void (*freeVulkanResource)(Vulkan*, Resource);
Resource CreateVulkanResource(Vulkan* vulkan, void* data, uint64_t size, int* err);
void FreeVulkanResource(Vulkan* vulkan, Resource resource);

typedef Job (*createVulkanJob)(Vulkan*, Resource*, int*);
typedef int (*submitVulkanJob)(Vulkan*, Job, Resource*);
typedef void (*destroyVulkanJob)(Vulkan*, Job);
Job CreateVulkanJob(Vulkan* vulkan, Resource* inputs, int* err);
int SubmitVulkanJob(Vulkan* vulkan, Job job, Resource* inputs);
void DestroyVulkanJob(Vulkan* vulkan, Job job);

Vulkan* LoadVulkan(int* err);
void UnloadVulkan(Vulkan** vulkan);

int InitVulkan(Vulkan* vulkan);
void DestroyVulkan(Vulkan* vulkan);

typedef struct Context {
    uint64_t driver;
    Handle   handle;
    createVulkanResource CreateVulkanResource;
    freeVulkanResource FreeVulkanResource;
    createVulkanJob CreateVulkanJob;
    submitVulkanJob SubmitVulkanJob;
    destroyVulkanJob DestroyVulkanJob;
} Context; 

#endif