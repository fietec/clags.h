/*
  Example 2: Types
      This example demonstrates how to add type verification to arguments
*/

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

char *input_file = NULL;
int64_t compression = 0;
char *output_file = "a.out";
uint8_t quality = 100;
bool help = false;
bool warnings = false;
bool version = false;

clags_range_t compression_range = {0, 100};

clags_arg_t args[] = {
    clags_positional(&input_file, "input_file", "the file to read"),
    // You can provide a string that will be used as the argument's value if the user
    // does not supply it on the command line. This is specified via the `.default_input` field.
    clags_option('o', "output", &output_file, "FILE", "the file to write", .default_input="a.out"),

    // To add type verification to an argument, simply set the `value_type` field
    // For a list of all available types, see `clags__types` in clags.h
    // Notes:
    //  - It is the user's responsibility to provide a variable pointer matching the type specified
    //       so here, &quality is expected to be uint8_t*
    //  - The default input is always a string and will be parsed according to the argument's
    //    `value_type` (e.g., converted to uint8_t, double, etc.).
    //  - For lists, default inputs are **not supported**, because each list element is
    //    appended individually from user input.
    //  - It is the caller's responsibility to ensure the string represents a valid value
    //    for the argument's type, as it will be processed through the same verification as
    //    user-provided values.
    clags_option('q', "quality", &quality, "NUM", "the quality of the output image", .value_type=Clags_UInt8, .default_input="50"),

    // Use `Clags_Number` together with the `range` field for custom number ranges
    clags_option('c', "compression", &compression, "NUM", "compression value of the output file", .value_type=Clags_Number, .range=&compression_range, .default_input="0"),

    clags_flag('w', "warnings", &warnings, "print warnings"),
    
    // This is how you create a flag that exits the parsing on occurrence, just like with `clags_flag_help`
    clags_flag('v', "version", &version, "print the version", .exit=true),
    clags_flag_help(&help),
};

clags_config_t config = clags_config(args);

int main(int argc, char **argv)
{
    const char *program_name = argv[0];
    if (clags_parse(argc, argv, &config) != NULL){
        clags_usage(program_name, &config);
        return 1;
    }
    if (help){
        clags_usage(program_name, &config);
        return 0;
    }
    if (version){
        printf("02_types: v1.0.0\n");
        return 0;
    }
    printf("Reading: '%s', Writing: '%s', Quality: %"PRIu8", Compression Level: %"PRId64", Warnings: %d\n", input_file, output_file, quality, compression, warnings);
    return 0;
}
