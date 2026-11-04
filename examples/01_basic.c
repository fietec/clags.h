/*
  Example 1: Basic
      This is a basic, POSIX-style, example of how to use clags
*/

#include <stdio.h>
#include <stdbool.h>

// This includes the function implementations and only has to be done once per translation unit
#define CLAGS_IMPLEMENTATION
#include <clags.h>

// Note: Many struct creation macros support optional arguments via the designated initializer syntax (see the following examples).
//       This makes for very readable and extensible definitions since you don't have to change the entire macro to add a new field.
//       The only downside is that technically, standard C before ISO-C23 does not support empty variadic arguments in macros (sigh).
//       Although basically every compiler has an extension for this, if you really want to compile to the standard
//       you have to add a trailing comma to the macro. This will fix all warnings.
//           clags_positional(&input_file, "input_file", "the file to read",),
//                                                                         |
//                                                                   trailing comma
//       In these examples we apply this technique to demonstrate ISO-c99 compatability, though in most cases leaving the comma out
//       works just fine and is asthetically much cleaner.
//       Alternatively, you can always construct the structs manually, of course.

int main(int argc, char **argv)
{
    // Declare argument variables with optional default values
    char *input_file = NULL;
    char *output_file = "a.out";
    bool warnings = false;
    bool help = false;

    // Declare all expected arguments
    clags_arg_t args[] = {
        // Positional arguments are parsed in the order they are defined here
        clags_positional(&input_file, "input_file", "the input file",),
    
        // Option arguments support both short and long flags
        // For long flags, both the `--output <file>` and `--output=<file>` syntaxes are supported
        clags_option('o', "output", &output_file, "FILE", "the output file",),

        // Use flags to set boolean values on occurrence
        // Short flags can be standalone, or combined into multi-flags, e.g.: -abc
        clags_flag('w', "warnings", &warnings, "print warnings",),
        // This is a neat short-hand since the `--help` flag is so common
        // Note: When this flag is encountered, parsing exits immediately
        //       If you don't want this behavior then define the flag yourself
        clags_flag_help(&help),
    };

    // A config is the description of a single (sub)command
    // It contains the previously defined argument rules as well as other settings.
    // See later examples and `clags_options_t` for more details
    clags_config_t config = clags_config(args,);

    // Parse the arguments using the previously defined rules
    clags_error_t error = clags_parse(argc, argv, &config);
    if (error.type != Clags_Error_Ok){
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
