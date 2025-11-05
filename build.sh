set -e
gcc vulkan/imports-gen.c -o aux/imports-gen
mkdir -p include/vulkan/generated
aux/imports-gen vulkan/vulkan.txt include/vulkan/generated/public.h include/vulkan/generated/private.h
gcc kc/*.c vulkan/vk*.c -o bin/blue 
