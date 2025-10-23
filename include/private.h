#ifndef __PRIVATE_H__
#define __PRIVATE_H__

#include "defs.h"
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

typedef struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    uint32_t selectedQueueFamily;
    VkQueue  queue;
    VkPhysicalDeviceMemoryProperties mprops;
} VulkanContext;
#endif