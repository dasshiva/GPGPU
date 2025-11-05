#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdint.h>

enum Error {
    SUCCESS,
    VULKAN_NOT_FOUND,
    FAULTY_GPU_DRIVER,
    NO_DEVICE_AVAILABLE,
    NO_QUEUE_AVAILABLE,
    UNKNOWN_ERROR,
    OUT_OF_MEMORY,
    OUT_OF_GPU_MEMORY,
    UNIMPLEMENTED,
    REQUIRED_PARAMETER_NULL,
    UNKNOWN_DRIVER,
    STRUCT_OVERWRITE
};

typedef void* Handle;

typedef struct Vulkan {
    Handle    driver;
    char*     name;
    Handle    data;
    uint32_t  version;
} Vulkan;

typedef Handle Resource;
Resource CreateResource(Vulkan* vulkan, void* data, uint64_t size, int* err);
void FreeResource(Vulkan* vulkan, Resource resource);

typedef Handle Job;
Job CreateJob(Vulkan* vulkan, Resource* inputs, int* err);
int SubmitJob(Vulkan* vulkan, Job job, Resource* inputs);
void DestroyJob(Vulkan* vulkan, Job job);

#define KC_USE_VULKAN 0x56554C4B414E0000UL // "VULKAN\0\0"
#define KC_USE_OPENGL 0x4F50454E47470000UL // "OPENGL\0\0"

int KC_Init(uint64_t driver, Handle* handle);
int KC_Destroy(Handle handle);

#endif