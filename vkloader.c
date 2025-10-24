#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "include/defs.h"
#include "include/alloc.h"
#include "include/private.h"

Vulkan* LoadVulkan(int* err) {
	char* driver = getenv("VULKAN_DRIVER");
    if (!driver) 
        driver = "libvulkan.so";
    
    void* handle = dlopen(driver, RTLD_LAZY);
    if (!handle) {
		*err = VULKAN_NOT_FOUND;
		return NULL;
	}

	Vulkan* ret = Alloc(sizeof(Vulkan));
	ret->driver = handle;
	ret->name = driver;
	ret->version = 0; // Vulkan is not yet initalized

	*err = SUCCESS;
	return ret;
}

void UnloadVulkan(Vulkan** vulkan) {
	if (!vulkan) {
		puts("UnloadVulkan(): vulkan == NULL");
		abort();
	}

	dlclose((*vulkan)->driver);
	Free(*vulkan);
}

void* GetFunction(Vulkan* vulkan, const char* name) {
    if (!name) {
        puts("GetFunction(): name == NULL");
        abort();
    }

    void* pfn = dlsym(vulkan->driver, name);
    if (!pfn) {
        printf("GetFunction(): %s function not found\n", name);
        abort();
    }

    return pfn;
}

void DestroyVulkan(Vulkan* vulkan) {
    VulkanContext* ctx = vulkan->data;
    VkDestroyDevice(ctx->logicalDevice, NULL);
    VkDestroyInstance(ctx->instance, NULL);
}

static int DeviceScore(VkPhysicalDeviceProperties* prop) {
    int score = 0;
    switch (prop->deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: score += 300; break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: score += 500; break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: score += 75; break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU: score += 25; break;
        default: score += 0;
    }

    VkPhysicalDeviceLimits limits = prop->limits;

    // prefer a device with a larger push constant size
    score += limits.maxPushConstantsSize * 10;
    // prefer a device with larger memory allocation counts
    score += limits.maxMemoryAllocationCount * 8;
    // prefer a device with more compute shared memory size
    score += limits.maxComputeSharedMemorySize * 7;
    // prefer a device with more compute work group invocations
    score += limits.maxComputeWorkGroupInvocations * 4;
    // prefer a device with larger compute work group sizes
    score += limits.maxComputeWorkGroupSize[0] * 12  + // along X
        limits.maxComputeWorkGroupSize[1] * 11 + // along Y
        limits.maxComputeWorkGroupSize[2] * 7; // along Z

    // prefer a device with larger storage buffers capacity
    score += limits.maxStorageBufferRange * 10;
    return score;
}

static int ConfigureVulkanDevice(Vulkan* vulkan) {
    VulkanContext* ctx = vulkan->data;

    VkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)
        VkGetInstanceProcAddr(ctx->instance, "vkEnumeratePhysicalDevices");
    if (!VkEnumeratePhysicalDevices)
        return FAULTY_GPU_DRIVER;

    uint32_t devices = 0;
    VkEnumeratePhysicalDevices(ctx->instance, &devices, NULL);
    if (!devices) 
        return NO_DEVICE_AVAILABLE;

    VkPhysicalDevice* physDevices = Alloc(devices * sizeof(VkPhysicalDevice));
    VkEnumeratePhysicalDevices(ctx->instance, &devices, physDevices);

    VkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)
        VkGetInstanceProcAddr(ctx->instance, "vkGetPhysicalDeviceProperties");
    if (!VkGetPhysicalDeviceProperties)
        return FAULTY_GPU_DRIVER;

    VkPhysicalDeviceProperties selectedDeviceProps;
    uint32_t selectedDevice = 0;
    uint32_t score = 0;
    
    for (uint32_t i = 0; i < devices; i++) {
        VkPhysicalDeviceProperties devProps;
        VkGetPhysicalDeviceProperties(physDevices[i], &devProps);
        printf("Device detected: %s\n", devProps.deviceName);
        uint32_t devScore = DeviceScore(&devProps);
        if (devScore > score) {
            selectedDevice = i;
            memcpy(&selectedDeviceProps, &devProps, sizeof(VkPhysicalDeviceProperties));
        }
    }

    ctx->physicalDevice = physDevices[selectedDevice];

    uint32_t qfcnt = 0;
    VkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)
        VkGetInstanceProcAddr(ctx->instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    if (!VkGetPhysicalDeviceQueueFamilyProperties)
        return FAULTY_GPU_DRIVER;

    VkGetPhysicalDeviceQueueFamilyProperties(physDevices[selectedDevice], &qfcnt, NULL);
    if (!qfcnt) 
        return NO_QUEUE_AVAILABLE;

    VkQueueFamilyProperties* qfp = Alloc(sizeof(VkQueueFamilyProperties) * qfcnt);

    VkGetPhysicalDeviceQueueFamilyProperties(physDevices[selectedDevice], &qfcnt, qfp);
    uint32_t selectedQueueFamily = 0;

    for (uint32_t i = 0; i < qfcnt; i++) {
        VkQueueFamilyProperties qf = qfp[i];
        if (qf.queueFlags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)) {
            selectedQueueFamily = i;
            break;
        }
    }

    VkCreateDevice = (PFN_vkCreateDevice) VkGetInstanceProcAddr(ctx->instance, "vkCreateDevice");
    VkDestroyDevice = (PFN_vkDestroyDevice) VkGetInstanceProcAddr(ctx->instance, "vkDestroyDevice");
    if (!VkCreateDevice || !VkDestroyDevice)
        return FAULTY_GPU_DRIVER;

    float prio = 1.0f;

    VkDeviceQueueCreateInfo dqCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueFamilyIndex = selectedQueueFamily,
        .queueCount = 1,
        .pQueuePriorities = &prio
    };

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &dqCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = NULL,
        .pEnabledFeatures = NULL
    };

    if (VkCreateDevice(physDevices[selectedDevice], &deviceCreateInfo
        , NULL, &ctx->logicalDevice) != VK_SUCCESS)
        return FAULTY_GPU_DRIVER;
    
    VkGetDeviceQueue = (PFN_vkGetDeviceQueue) 
        VkGetInstanceProcAddr(ctx->instance, "vkGetDeviceQueue");
    if (!VkGetDeviceQueue)
        return FAULTY_GPU_DRIVER;

    VkGetDeviceQueue(ctx->logicalDevice, selectedQueueFamily, 0, &ctx->queue);
    ctx->selectedQueueFamily = selectedQueueFamily;

    VkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)
        VkGetInstanceProcAddr(ctx->instance, "vkGetPhysicalDeviceMemoryProperties");
    if (!VkGetPhysicalDeviceMemoryProperties)
        return FAULTY_GPU_DRIVER;

    VkGetPhysicalDeviceMemoryProperties(ctx->physicalDevice, &ctx->mprops);
    return SUCCESS;
}

static void ChooseMemoryAllocationStrategy(Vulkan* vulkan) {
    VulkanContext* ctx = (VulkanContext*) vulkan->data;
    
    /* We need to find two distinct memory types, one with VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 
     * and one with VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT. That is, there are two distinct 
     * memory regions, one that is device local and one that is device visible. These two
     * flags should not be together, for indirect allocation. Otherwise, go ahead with
     * direct allocation as most likely this is an integrated GPU or even a CPU and there
     * is no point in allocating two separate buffers in the same kind of memory 
     */
    uint32_t deviceLocal = UINT_MAX, hostLocal = UINT_MAX;

    for (uint32_t idx = 0; idx < ctx->mprops.memoryTypeCount; idx++) {
        VkMemoryType mtype = ctx->mprops.memoryTypes[idx];
        if ((mtype.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && 
            !(mtype.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            deviceLocal = idx;
        }
        else if ((mtype.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            !(mtype.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            hostLocal = idx;
        }
    }

    if ((deviceLocal == UINT_MAX) || (hostLocal == UINT_MAX)) {
        ctx->flags |= ALLOCATE_DIRECT;
        for (uint32_t idx = 0; idx < ctx->mprops.memoryTypeCount; idx++) {
            VkMemoryType mtype = ctx->mprops.memoryTypes[idx];
            if ((mtype.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && 
                (mtype.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
                    ctx->memoryHeaps = (0xFFFFU << 16) | (idx);
        }

        printf("%d\n", ctx->memoryHeaps & 0xFFFF);
    }
    else {
        ctx->flags |= ALLOCATE_INDIRECT;
        ctx->memoryHeaps = (deviceLocal << 16) | (hostLocal);
        printf("%d %d\n", ctx->memoryHeaps >> 16, ctx->memoryHeaps & 0xFFFF);
    }
}

int InitVulkan(Vulkan* vulkan) {
    VkGetInstanceProcAddr = GetFunction(vulkan, "vkGetInstanceProcAddr");
    VkCreateInstance = (PFN_vkCreateInstance) 
        VkGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (!VkCreateInstance)
        return FAULTY_GPU_DRIVER;

    VkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion) 
        VkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");
    if (!VkEnumerateInstanceVersion)
        return FAULTY_GPU_DRIVER;

    VkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)
        VkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
    if (!VkEnumerateInstanceExtensionProperties)
        return FAULTY_GPU_DRIVER;

    VkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)
        VkGetInstanceProcAddr(NULL, "vkEnumerateInstanceLayerProperties");
    if (!VkEnumerateInstanceLayerProperties)
        return FAULTY_GPU_DRIVER;

    if (VkEnumerateInstanceVersion(&vulkan->version) != VK_SUCCESS) 
        return UNKNOWN_ERROR;

    uint32_t layers = 0;
    VkEnumerateInstanceLayerProperties(&layers, NULL);
    VkLayerProperties* lprops = Alloc(layers * sizeof(VkLayerProperties));
    const char* validation = "VK_LAYER_KHRONOS_validation";
    int enable_layers = 0;

    if (layers) {
        VkEnumerateInstanceLayerProperties(&layers, lprops);
        for (int i = 0; i < layers; i++) {
            printf("Layer %s - %s\n", lprops[i].layerName, lprops[i].description);
            if (strcmp(validation, lprops[i].layerName) == 0) {
                enable_layers = 1;
            }
        }
    }

    vulkan->data = Alloc(sizeof(VulkanContext));
    VulkanContext* ctx = vulkan->data;

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = "blue",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "blue-engine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = vulkan->version
    };

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = enable_layers,
        .ppEnabledLayerNames = &validation,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = NULL
    };

    if (VkCreateInstance(&instanceCreateInfo, NULL, &ctx->instance) != VK_SUCCESS) 
        return UNKNOWN_ERROR;

    VkDestroyInstance = (PFN_vkDestroyInstance) 
        VkGetInstanceProcAddr(ctx->instance, "vkDestroyInstance");
    if (!VkDestroyInstance)
        return FAULTY_GPU_DRIVER;

    int ret;

    ret = ConfigureVulkanDevice(vulkan);
    if (ret)
        return ret;

    ChooseMemoryAllocationStrategy(vulkan);

    VkCreateBuffer = (PFN_vkCreateBuffer) VkGetInstanceProcAddr(ctx->instance, "vkCreateBuffer");
    if (!VkCreateBuffer) 
        return FAULTY_GPU_DRIVER;

    return SUCCESS;
}

