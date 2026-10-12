/*
  Example 2: Types
      This example demonstrates how to add type verification to arguments
*/

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define CLAGS_IMPLEMENTATION
#include <clags.h>

// Here all declarations are done statically and globally.
// This improves the readability of the `main` function but is a matter of taste.

char *input_file = NULL;
uint64_t compression = 0;
char *output_file;
uint8_t quality = 100;
float scaling;
bool help = false;
bool warnings = false;
bool version = false;

// You can define custom ranges for the `Clags_Int`, `Clags_UInt` and `Clags_Real` types (use the corresponding `clags_<type>_range` macros)
clags_range_t compression_range = clags_uint_range(0, 100);

// Note: As shown below, many struct creation macros support optional arguments via the designated initializer syntax.
//       This makes for very readable and extensible definitions since you don't have to change the entire macro to add a new field.
//       The only downside is that technically, standard C before ISO-C23 does not support empty variadic arguments in macros (sigh).
//       Although basically every compiler has an extension for this, if you really want to compile to the standard
//       you have to add a trailing comma to the macro. This will fix all warnings.
//           clags_positional(&input_file, "input_file", "the file to read",),
//                                                                         |
//                                                                   trailing comma
//       Alternatively, you can always construct the structs manually, of course.
clags_arg_t args[] = {
    clags_positional(&input_file, "input_file", "the file to read"),

    // You can provide a string that will be used as the argument's value if the user
    // does not supply it on the command line. This is specified via the `.default_input` field.
    // The advantages of this over just directly initializing the variable are that `clags_usage`
    // is now able to print this default value making it more obvious to the user what the default value is
    // and also that this default input is run through the same verfiers that normal user input would
    // which makes sure that the variable will always have a valid state after parsing.
    clags_option('o', "output", &output_file, "FILE", "the file to write", .default_input="a.out"),

    // To add type verification to an argument, simply set the `value_type` field
    // For a list of all available types, see `CLAGS__VALUE_TYPES` in clags.h
    // Notes:
    //  - It is the user's responsibility to provide a variable pointer matching the type specified
    //       so here, &quality is expected to be uint8_t*
    //  - The default input is always a string and will be parsed according to the argument's
    //    `value_type` (e.g., converted to uint8_t, double, etc.).
    //  - For lists, default inputs are **not supported**, because each list element is
    //    appended individually from user input.
    //  - An invalid default input will raise a `Clags_ConfigError` during validation.
    clags_option('q', "quality", &quality, "NUM", "the quality of the output image", .value_type=Clags_UInt8, .default_input="50"),
    clags_option('s', "scaling", &scaling, "FACT", "the factor by which to scale the image", .value_type=Clags_Float, .default_input="1.0"),

    // Some types allow/require you to provide additional data via the `.using` field.
    // For example, use `Clags_Int` together with the `.using.range` field for custom number ranges.
    clags_option('c', "compression", &compression, "NUM", "compression value of the output file", .value_type=Clags_UInt, .using.range=&compression_range, .default_input="0"),

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

    // Use the variables set to their converted values
    printf("Reading: '%s', Writing: '%s', Quality: %"PRIu8", Compression Level: %"PRIu64", Scaling: %f, Warnings: %d\n", input_file, output_file, quality, compression, scaling, warnings);
    return 0;
}
