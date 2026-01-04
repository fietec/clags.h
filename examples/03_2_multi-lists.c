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

bool help = false;

clags_arg_t args[] = {
    clags_required(&string_list, "strings", "A list of strings", .is_list=true),
    clags_required(&int_list, "ints", "A list of integers", .value_type=Clags_Int32, .is_list=true),
    clags_flag_help(&help),
};

// Having multiple lists or, in fact, any other required arguments directly after a list,
// requires for a delimeter that terminates the list.
// Since this is not really standard, a custom terminator must be defined manually here.
clags_config_t config = clags_config(args, .list_terminator="::", .ignore_prefix="!", .allow_option_parsing_toggle=true);

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

    printf("Strings:\n");
    for (size_t i=0; i<string_list.count; ++i){
        printf("  %zu: %s\n", i, clags_list_element(string_list, char*, i));
    }
    printf("\nInts:\n");
    for (size_t i=0; i<int_list.count; ++i){
        printf("  %zu: %"PRId32"\n", i, clags_list_element(int_list, int32_t, i));
    }
    
defer:
    clags_list_free(&string_list);
    clags_list_free(&int_list);
    return result;
}
        
