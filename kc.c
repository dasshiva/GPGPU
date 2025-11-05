#include "include/defs.h"
#include "include/private.h"
#include "include/alloc.h"
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
        return SUCCESS;
    } 
    else if (ctx->driver == KC_USE_OPENGL) 
        return UNIMPLEMENTED;

    return UNKNOWN_DRIVER;
}