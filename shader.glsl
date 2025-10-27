#version 450

// Define the local workgroup size.
// A 1D workgroup of size 64 is a common default.
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

// Define the storage buffer (SSBO) at binding 0.
// We use 'uint' for 32-bit unsigned integers.
// 'int' (signed) could also be used, but sqrt on negative numbers
// is undefined, so 'uint' is safer.
layout(std430, binding = 0) buffer DataBuffer {
    uint data[];
} buf;

void main() {
    // Get the unique ID for this shader invocation.
    uint index = gl_GlobalInvocationID.x;

    // Bounds check: Ensure we don't try to access data outside the buffer's range.
    // This is important if the number of dispatched threads isn't an exact
    // multiple of the workgroup size.
    if (index >= buf.data.length()) {
        return;
    }

    // Read the original 32-bit integer.
    uint original_value = buf.data[index];

    // Calculate the square root.
    // 1. Cast the integer to a float to use the sqrt() function.
    // 2. Calculate the square root.
    // 3. Cast the resulting float back to an unsigned integer.
    uint result = uint(sqrt(float(original_value)));

    // Write the result back into the buffer at the same position.
    buf.data[index] = result;
}
