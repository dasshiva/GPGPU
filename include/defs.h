#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdint.h>

enum Error {
    SUCCESS,
    VULKAN_NOT_FOUND,
    FAULTY_GPU_DRIVER,
    NO_DEVICE_AVAILABLE,
    NO_QUEUE_AVAILABLE,
    UNKNOWN_ERROR
};

typedef void* Handle;

typedef struct Vulkan {
    Handle    driver;
    char*     name;
    Handle    data;
    uint32_t  version;
} Vulkan; 

Vulkan* LoadVulkan(int* err);
void UnloadVulkan(Vulkan** vulkan);

int InitVulkan(Vulkan* vulkan);
void DestroyVulkan(Vulkan* vulkan);

#endif