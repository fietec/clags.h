/*
  Example 3.2: Multiple Lists
     This example shows how to work with multiple lists.
     To terminate a list, use the custom `list_terminator` feature
*/


#include <stdio.h>
#include <inttypes.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

clags_list_t string_list = clags_list();

// lists can also be typed
clags_list_t int_list = clags_int32_list();
clags_list_t file_list = clags_file_list();
clags_list_t test_list = clags_list();

bool help = false;

clags_arg_t args[] = {
    clags_positional(&string_list, "strings", "A list of strings", .is_list=true),
    clags_positional(&int_list, "ints", "A list of integers", .value_type=Clags_Int32, .is_list=true),
    clags_positional(&test_list, "test", "a test argument", .optional=true, .is_list=true),
    clags_option('f', "file", &file_list, "FILE", "A list of files", .value_type=Clags_File, .is_list=true),
    clags_flag_help(&help),
};

// Having multiple lists or, in fact, any other required arguments directly after a list,
// requires for a delimeter that terminates the list.
// When `.duplicate_strings` is true, clags duplicates all strings. 
// This is necessary if the original argument strings may go out of scope before the parsed values are used.
clags_config_t config = clags_config(args, .list_terminator="::", .ignore_prefix="!", .allow_option_parsing_toggle=true, .duplicate_strings=true);

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
    printf("\nTests (%zu):\n", test_list.count);
    for (size_t i=0; i<test_list.count; ++i){
        printf("  %zu: %s\n", i, clags_list_element(test_list, char*, i));
    }

    
defer:
    clags_list_free(&string_list);
    clags_list_free(&int_list);
    clags_list_free(&file_list);

    // the config stores all the duplicated strings. You can free them like this
    clags_config_free_allocs(&config);

    return result;
}
        
