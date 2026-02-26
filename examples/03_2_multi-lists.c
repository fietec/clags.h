/*
  Example 3.2: Multiple Lists
     This example shows how to work with multiple lists.
     To terminate a list, use the custom `.list_terminator` feature
*/

#include <stdio.h>
#include <inttypes.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

clags_list_t string_list = clags_list(Clags_String);
clags_list_t int_list = clags_list(Clags_Int32);
clags_list_t file_list = clags_list(Clags_File);
clags_list_t extra_list = clags_list(Clags_String);
clags_list_t ignored_list = clags_list(Clags_String);

bool help = false;

clags_arg_t args[] = {
    // You can simply combine the known `.value_type` and `.is_list` fields to achieve typed lists
    clags_positional(&string_list, "strings", "a list of strings", .is_list=true),
    clags_positional(&int_list, "ints", "a list of integers", .value_type=Clags_Int32, .is_list=true),
    // Positional arguments may also be optional, so the parser does not throw an error if no argument is provided
    clags_positional(&extra_list, "extras", "extra arguments", .optional=true, .is_list=true),

    // Options can also be lists
    // Every time the user provides the flag together with an argument that argument is added to the list
    clags_option('f', "file", &file_list, "FILE", "a list of files", .value_type=Clags_File, .is_list=true),
    clags_flag_help(&help),
};

// Having multiple lists or, in fact, any other required arguments directly after a list,
// requires for a delimeter that terminates the list.
// When `.duplicate_strings` is true, clags duplicates all strings. 
// This is necessary if the original argument strings may go out of scope before the parsed values are used.
//
// Set `.ignored_args` to a list pointer to instruct clags to collect all ignored arguments
// in the provided list. The `ignore_prefix` is removed from all arguments.
clags_config_t config = clags_config(
    args,
    .list_terminator="::",
    .ignore_prefix="!",
    .ignored_args=&ignored_list,
    .allow_option_parsing_toggle=true,
    .duplicate_strings=true,
);

int main(int argc, char **argv)
{
    int result = 0;
    const char *program_name = argv[0];

    if (clags_parse(argc, argv, &config) != NULL){
        clags_usage(program_name, &config);
        // this makes working with lists way more convenient
        clags_return_defer(1);
    }
    if (help){
        clags_usage(program_name, &config);
        clags_return_defer(0);
    }

    printf("Strings (%zu):\n", string_list.count);
    for (size_t i=0; i<string_list.count; ++i){
        printf("  %zu: %s\n", i, clags_list_element(string_list, char*, i));
    }
    printf("\nInts (%zu):\n", int_list.count);
    for (size_t i=0; i<int_list.count; ++i){
        printf("  %zu: %"PRId32"\n", i, clags_list_element(int_list, int32_t, i));
    }
    printf("\nFiles (%zu):\n", file_list.count);
    for (size_t i=0; i<file_list.count; ++i){
        printf("  %zu: %s\n", i, clags_list_element(file_list, char*, i));
    }
    printf("\nExtras (%zu):\n", extra_list.count);
    for (size_t i=0; i<extra_list.count; ++i){
        printf("  %zu: %s\n", i, clags_list_element(extra_list, char*, i));
    }
    printf("\n");

    // Use the previously defined `.ignored_args` list to access all ignored arguments
    if (ignored_list.count){
        printf("These arguments were ignored:\n");
        for (size_t i=0; i<ignored_list.count; ++i){
            printf("%s\n", clags_list_element(ignored_list, char*, i));
        }
    } else{
        printf("No arguments were ignored.\n");
    }

defer:
#if 1
    // manually free each list explicitly
    clags_list_free(&string_list);
    clags_list_free(&int_list);
    clags_list_free(&extra_list);
    clags_list_free(&file_list);
    clags_list_free(&ignored_list);

    // the config stores all the duplicated strings. You can free them like this
    clags_config_free_allocs(&config);
#else
    // alternatively, all this can be done automatically
    clags_config_free(&config);
#endif
    return result;
}
        
