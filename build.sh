gcc imports-gen.c -o imports-gen
mkdir -p include/generated
./imports-gen vulkan.txt include/generated/public.h include/generated/private.h
gcc hello.c alloc.c vkjob.c vkloader.c vkmemory.c vkprivateloader.c -o blue
