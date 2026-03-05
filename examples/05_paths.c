/*
  Example 5: Paths
      This example demonstrates how to verify file paths
*/

#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

const char *input_file = NULL;
// `clags_fsize_t` is just a typedef of uint64_t
clags_fsize_t size = 0;
const char *output_file = NULL;
bool warnings = false;
bool help = false;

clags_arg_t args[] = {
    // Force the provided string to be a valid path to a file
    clags_positional(&input_file, "input_file", "the input file", .value_type=Clags_File),

    // Verify that the provided string is a valid path, no matter whether regular file, dir or other
    clags_option('o', "output", &output_file, "FILE", "the output file or directory", .value_type=Clags_Path),
    // Verify a size string (like '1.4MB', '10B', '10') and convert to the number of bytes in `clags_fsize_t`
    clags_option('s', "size", &size, "SIZE", "the amount of bytes to read", .value_type=Clags_Size, .default_input="1KB"),
    clags_flag_help(&help),
};

clags_config_t config = clags_config(args);

int main(int argc, char **argv)
{
    if (clags_parse(argc, argv, &config) != NULL){
        clags_usage(argv[0], &config);
        return 1;
    }
    if (help){
        clags_usage(argv[0], &config);
        return 0;
    }
    // Since `output_file` was only checked for existance, you don't know of what type the path is
    clags_path_type_t type = clags_path_type(output_file);
    const char *type_name = clags_path_type_name(type);

    printf("input: %s, size: %"PRIu64", output: %s (%s)\n", input_file, size, output_file, type_name);
    return 0;
}
