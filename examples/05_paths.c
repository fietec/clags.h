/*
  Example 5: Paths
      This example demonstrates how to verify file paths
*/

#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

const char *input_file = NULL;
clags_fsize_t size = 1000;
const char *output_file = "a.out";
bool warnings = false;
bool help = false;

clags_arg_t args[] = {
    // force the provided string to be a valid path to a file
    clags_positional(&input_file, "input_file", "the input file", .value_type=Clags_File),

    // verify that the provided string is a valid path, either to a dir or file
    clags_option('o', "output", &output_file, "FILE", "the output file or directory", .value_type=Clags_Path),
    // verify a size string (like '1.4MB', '10B', '10') and convert to the number of bytes in `clags_fsize_t`
    clags_option('s', "size", &size, "SIZE", "the amount of bytes to read", .value_type=Clags_Size),
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
    printf("input: %s, size: %"PRIu64", output: %s\n", input_file, size, output_file);
    return 0;
}
