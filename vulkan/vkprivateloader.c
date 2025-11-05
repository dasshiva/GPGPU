#include "../include/vulkan/generated/private.h"
#include "../include/vulkan/private.h"

int InitVulkanPrivate(VkInstance instance) {
    int i = 0;
    while (1) {
        struct LoadFunction lf = loadFunctions[i];
        if (!lf.name)
            break;
        *lf.sig = vkGetInstanceProcAddr(instance, lf.name);
        if (!*lf.sig)
            return FAULTY_GPU_DRIVER;
        i++;
    }

    return SUCCESS;
}