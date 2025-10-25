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

#endif