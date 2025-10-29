#include <ctype.h>
#include <stdio.h>
#include <string.h>

int main(int argc, const char* argv[]) {
    if (argc < 4)
        return 1;

    FILE* file = fopen(argv[1], "r");
    if (!file)
        return 1;

    FILE* output = fopen(argv[2], "w");
    if (!output) {
        fclose(file);
        return 1;
    }

    FILE* pvt = fopen(argv[3], "w");

    fprintf(output, "#ifndef __GENERATED_H__\n");
    fprintf(output, "#define __GENERATED_H__\n");
    fprintf(output, "// AUTO-GENERATED: DO NOT EDIT\n");
    fprintf(output, "#define VK_NO_PROTOTYPES\n");
    fprintf(output, "#include <vulkan/vulkan.h>\n");
    while (1) {
        char function[100] = {0};
        if (!fgets(function, 100, file))
            break;

        // strip the newline
        int len = strlen(function);
        for (int i = 0; i < len; i++) {
            if (isspace(function[i])) {
                function[i] = '\0';
                break;
            }
        }

        fprintf(output, "extern PFN_%s ", function);
        fprintf(output, "%s;\n", function);
    }
    
    fprintf(output, "#endif\n");

    fclose(output);
    
    fseek(file, 0, SEEK_SET);

    fprintf(pvt, "#ifndef __GENERATED_PVT_H__\n");
    fprintf(pvt, "#define __GENERATED_PVT_H__\n");
    fprintf(pvt, "#define VK_NO_PROTOTYPES\n");
    fprintf(pvt, "#include <vulkan/vulkan.h>\n");
    fprintf(pvt, "// AUTO-GENERATED: DO NOT EDIT\n");
    fprintf(pvt, "struct LoadFunction {const char* name; void** sig;};\n");

     while (1) {
        char function[100] = {0};
        if (!fgets(function, 100, file))
            break;

        // strip the newline
        int len = strlen(function);
        for (int i = 0; i < len; i++) {
            if (isspace(function[i])) {
                function[i] = '\0';
                break;
            }
        }

        fprintf(pvt, "PFN_%s ", function);
        fprintf(pvt, "%s;\n", function);
    }

    fseek(file, 0, SEEK_SET);

    fprintf(pvt, "struct LoadFunction loadFunctions[] = {\n");
    while (1) {
        char function[100] = {0};
        if (!fgets(function, 100, file))
            break;

        // strip the newline
        int len = strlen(function);
        for (int i = 0; i < len; i++) {
            if (isspace(function[i])) {
                function[i] = '\0';
                break;
            }
        }

        fprintf(pvt, "{\"%s\", ", function);
        fprintf(pvt, " (void**)&%s},\n", function);
    }

    fprintf(pvt, "{0, 0}};\n");
    fseek(file, 0, SEEK_SET);

    fprintf(pvt, "#endif");
    fclose(pvt);
    fclose(file);
    return 0;
}