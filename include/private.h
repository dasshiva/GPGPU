#ifndef __PRIVATE_H__
#define __PRIVATE_H__

#include "defs.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

void* GetFunction(Vulkan* vulkan, const char* name);

PFN_vkGetInstanceProcAddr VkGetInstanceProcAddr;
PFN_vkCreateInstance VkCreateInstance;
PFN_vkEnumerateInstanceLayerProperties VkEnumerateInstanceLayerProperties;
PFN_vkEnumerateInstanceExtensionProperties VkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateInstanceVersion VkEnumerateInstanceVersion;
PFN_vkDestroyInstance VkDestroyInstance;

PFN_vkEnumeratePhysicalDevices VkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceProperties VkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties VkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetDeviceQueue VkGetDeviceQueue;
PFN_vkCreateDevice VkCreateDevice;
PFN_vkDestroyDevice VkDestroyDevice;

PFN_vkGetPhysicalDeviceMemoryProperties VkGetPhysicalDeviceMemoryProperties;
PFN_vkCreateBuffer VkCreateBuffer;
PFN_vkGetBufferMemoryRequirements VkGetBufferMemoryRequirements;
PFN_vkAllocateMemory VkAllocateMemory;
PFN_vkMapMemory VkMapMemory; 
PFN_vkUnmapMemory VkUnmapMemory;
PFN_vkBindBufferMemory VkBindBufferMemory;

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