#include <stdio.h>
#include "include/defs.h"

int main() {
    int err = SUCCESS;
	Vulkan* vulkan = LoadVulkan(&err);

	if (!vulkan) {
		printf("LoadVulkan() = %d\n", err);
		return 1;
	}

	int ret = InitVulkan(vulkan);
	if (ret) {
		printf("Could not initialize vulkan = %d\n", ret);
		return 1;
	}

	
	DestroyVulkan(vulkan);
	UnloadVulkan(&vulkan);
    return 0;
}
