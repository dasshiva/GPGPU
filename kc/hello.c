#include <stdio.h>
#include "../include/kc.h"

int main() {
	Handle handle = NULL;
	int err = KC_Init(KC_USE_VULKAN, &handle);

	if (err != SUCCESS) {
		printf("Could not initialize KC = %d\n", err);
		return 1;
	}

	int memory[] = {1, 2, 3, 4, 5, 6, 7, 8};
	Resource res;
	
	err = CreateResource(handle, memory, 32, &res);
	if (err != SUCCESS) {
		printf("Could not allocate resource on GPU = %d\n", err);
		return 1;
	}

	Resource inputs[] = {res, NULL};
	Job job;
	err = CreateJob(handle, inputs, &job);

	if (SubmitJob(handle, job, inputs) != SUCCESS) {
		printf("Compute Work failed\n");
		return 0;
	}
	
	DestroyJob(handle, job);
	FreeResource(handle, res);
	KC_Destroy(handle);

    return 0;
}
