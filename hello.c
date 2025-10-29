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

	int memory[] = {1, 2, 3, 4, 5, 6, 7, 8};
	Resource res = CreateResource(vulkan, memory, 32, &ret);
	if (!res) {
		printf("Could not allocate resource on GPU = %d\n", ret);
		return 1;
	}

	Resource inputs[] = {res, NULL};

	Job job = CreateJob(vulkan, inputs, &err);
	if (SubmitJob(vulkan, job, inputs) != SUCCESS) {
		printf("Compute Work failed\n");
		return 0;
	}
	
	DestroyJob(vulkan, job);

	FreeResource(vulkan, res);
	DestroyVulkan(vulkan);
	UnloadVulkan(&vulkan);

    return 0;
}
