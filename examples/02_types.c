#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

char *input_file = NULL;
char *output_file = "a.out";
uint8_t quality = 100;
bool help = false;
bool warnings = false;
bool version = false;

clags_arg_t args[] = {
    clags_required(&input_file, "input_file", "the file to read", .value_type=Clags_File),
    clags_optional("-o", "--output", &output_file, "FILE", "the file to write"),
    clags_optional("-q", "--quality", &quality, "NUM", "the quality of the output image", .value_type=Clags_UInt8),
    clags_flag("-w", "--warnings", &warnings, "print warnings"),
    clags_flag("-v", "--version", &version, "print the version", .exit=true),
    clags_flag_help(&help),
};

int main(int argc, char **argv)
{
    const char *program_name = argv[0];
    if (!clags_parse(argc, argv, args)){
        clags_usage(program_name, args);
        return 1;
    }
    if (help){
        clags_usage(program_name, args);
        return 0;
    }
    if (version){
        printf("02_types: v1.0.0\n");
        return 0;
    }
    printf("Reading: '%s', Writing: '%s', Quality: %"PRIu8"\n", input_file, output_file, quality);
    return 0;
}
