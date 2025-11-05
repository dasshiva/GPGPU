#include "../include/kc.h"

#include "../include/vulkan/private.h"
#include "../include/alloc.h"
#include <stdlib.h>

int KC_Init(uint64_t driver, Handle* handle) {
    if (!handle)
        return REQUIRED_PARAMETER_NULL;

    if (driver == KC_USE_VULKAN) {
        int err = SUCCESS;
        Vulkan* vulkan = LoadVulkan(&err);
        if (err) 
            return err;

        err = InitVulkan(vulkan);
        if (err)
            return err;

        Context* ctx = Alloc(sizeof(Context));
        ctx->driver = driver;
        ctx->handle = vulkan;
        ctx->CreateVulkanJob = CreateVulkanJob;
        ctx->DestroyVulkanJob = DestroyVulkanJob;
        ctx->SubmitVulkanJob = SubmitVulkanJob;
        ctx->CreateVulkanResource = CreateVulkanResource;
        ctx->FreeVulkanResource = FreeVulkanResource;

        *handle = ctx;
        return SUCCESS;
    }
    
    else if (driver == KC_USE_OPENGL) {
        *handle = NULL;
        return UNIMPLEMENTED;
    }

    return UNKNOWN_DRIVER;
}

int KC_Destroy(Handle handle) {
    if (!handle)
        return REQUIRED_PARAMETER_NULL;

    Context* ctx = handle;
    if (ctx->driver == KC_USE_VULKAN) {
        Vulkan* vulkan = ctx->handle;
        if (!vulkan)
            return STRUCT_OVERWRITE;

        DestroyVulkan(vulkan);
        UnloadVulkan(&vulkan);
        Free(ctx);
        return SUCCESS;
    } 
    else if (ctx->driver == KC_USE_OPENGL) 
        return UNIMPLEMENTED;

    return UNKNOWN_DRIVER;
}

int CreateResource(Handle handle, void* data, uint64_t size, Resource* ret) {
    if (!handle || !ret)
        return REQUIRED_PARAMETER_NULL;

    Context* ctx = handle;
    if (!ctx->handle)
        return STRUCT_OVERWRITE;

    if (ctx->driver == KC_USE_VULKAN) {
        Vulkan* vulkan = ctx->handle;

        int err = SUCCESS;
        Resource val = ctx->CreateVulkanResource(vulkan, data, size, &err);
        if (err != SUCCESS)
            return err;

        *ret = val;
        return SUCCESS;
    } 
    else if (ctx->driver == KC_USE_OPENGL) 
        return UNIMPLEMENTED;

    
    return UNKNOWN_DRIVER;
}

int FreeResource(Handle handle, Resource resource) {
    if (!handle || !resource)
        return REQUIRED_PARAMETER_NULL;

    Context* ctx = handle;
    if (!ctx->handle)
        return STRUCT_OVERWRITE;

    if (ctx->driver == KC_USE_VULKAN) {
        Vulkan* vulkan = ctx->handle;
        ctx->FreeVulkanResource(vulkan, resource);

        return SUCCESS;
    }
    else if (ctx->driver == KC_USE_OPENGL)
        return UNIMPLEMENTED;

    return UNKNOWN_DRIVER;
}

int CreateJob(Handle handle, Resource* res, Job* job) {
    if (!handle || !res || !job)
        return REQUIRED_PARAMETER_NULL;

    Context* ctx = handle;
    if (!ctx->handle)
        return STRUCT_OVERWRITE;

    if (ctx->driver == KC_USE_VULKAN) {
        Vulkan* vulkan = ctx->handle;
        int err = SUCCESS;
        Job ret = ctx->CreateVulkanJob(vulkan, res, &err);

        if (err != SUCCESS)
            return err;

        *job = ret;
        return SUCCESS;

    }
    else if (ctx->driver == KC_USE_OPENGL)
        return UNIMPLEMENTED;

    return UNKNOWN_DRIVER;

}

int SubmitJob(Handle handle, Job job, Resource* inputs) {
    if (!handle || !job || !inputs)
        return REQUIRED_PARAMETER_NULL;

    Context* ctx = handle;
    if (!ctx->handle)
        return STRUCT_OVERWRITE;

    if (ctx->driver == KC_USE_VULKAN) {
        Vulkan* vulkan = ctx->handle;
        int err = ctx->SubmitVulkanJob(vulkan, job, inputs);
        if (err != SUCCESS)
            return err;

        return SUCCESS;
    }
    else if (ctx->driver == KC_USE_OPENGL) 
        return UNIMPLEMENTED;

    return UNKNOWN_DRIVER;
}

int DestroyJob(Handle handle, Job job) {
    if (!handle || !job)
        return REQUIRED_PARAMETER_NULL;

    Context* ctx = handle;
    if (!ctx->handle)
        return STRUCT_OVERWRITE;

    if (ctx->driver == KC_USE_VULKAN) {
        Vulkan* vulkan = ctx->handle;
        ctx->DestroyVulkanJob(vulkan, job);
        return SUCCESS;
    }
    else if (ctx->driver == KC_USE_OPENGL)
        return UNIMPLEMENTED;

    return UNKNOWN_DRIVER;
}