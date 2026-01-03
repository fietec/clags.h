/*
  Example 1: Basic
      This is a basic, POSIX-style, example of how to use clags
*/

#include <stdio.h>
#include <stdbool.h>

// This includes the function implemenetations and only has to be done once per translation unit
#define CLAGS_IMPLEMENTATION
#include "../clags.h"

// Declare argument variables with optional default values
const char *input_file = NULL;
const char *output_file = "a.out";
bool warnings = false;
bool help = false;

// Declare all expected arguments
clags_arg_t args[] = {
    // Required (positional) arguments are parsed in the order they are defined here
    clags_required(&input_file, "input_file", "the input file"),
    
    // Optional arguments support both short and long flags
    // For long flags, both the `--output <file>` and `--output=<file>` syntaxes are supported
    clags_optional('o', "output", &output_file, "FILE", "the output file", .value_type=Clags_Subcmd),


    // Use flags set boolean values on occurrence
    // Short flags can be standalone, or combined into multi-flags, e.g.: -abc
    clags_flag('w', "warnings", &warnings, "print warnings"),
    // This is a neat short-hand since the `--help` flags are so common
    clags_flag_help(&help),
};

clags_config_t config = clags_config(args);

int main(int argc, char **argv)
{
    // Parse the arguments using the previously defined rules, returns the config on error
    if (clags_parse(argc, argv, &config) != NULL){
        // Print an automatic usage, based on the defined config
        clags_usage(argv[0], &config);
        return 1;
    }
    // You can now use the set argument variables
    if (help){
        clags_usage(argv[0], &config);
        return 0;
    }
    printf("input: %s, output: %s, warnings:%s\n", input_file, output_file, warnings?"true":"false");
    return 0;
}
