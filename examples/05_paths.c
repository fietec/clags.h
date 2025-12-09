/*
  Example 5: Paths
      This is exactly the same example program as `01_basic`,
      only that here, we require a valid file input for the `input_file` argument 
*/

#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

const char *input_file = NULL;
const char *output_file = "a.out";
bool warnings = false;
bool help = false;

clags_arg_t args[] = {
    // Force the inputted string to be a valid path to a file
    clags_required(&input_file, "input_file", "the input file", .value_type=Clags_File),
    
    clags_optional("-o", "--output", &output_file, "FILE", "the output file"),
    clags_flag("-w", "--warnings", &warnings, "print warnings"),
    clags_flag_help(&help),
};

int main(int argc, char **argv)
{
    if (!clags_parse(argc, argv, args)){
        clags_usage(argv[0], args);
        return 1;
    }
    if (help){
        clags_usage(argv[0], args);
        return 0;
    }
    printf("input: %s, output: %s, warnings:%s\n", input_file, output_file, warnings?"true":"false");
    return 0;
}
