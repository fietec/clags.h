/*
  Example 3.1: Lists
     This example shows how to work with lists.
     Lists can also be typed, similarily to normal arguments, as can be seen in example 3.2
*/

#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include <clags.h>

bool help = false;

// Create a list to store multiple values of a specific type
// Use the generic `clags_list(TYPE)` macro for any supported type
// or one of the explicit initializers, e.g. `clags_string_list()`
clags_list_t list = clags_list(Clags_String);

clags_arg_t args[] = {
    // Set `.is_list` to tell the parser that the variable is a list storing values of type `.value_type`, `Clags_String` is default
    clags_positional(&list, "strings", "the strings to print", .is_list=true),
    clags_flag_help(&help),
};

// Optional argument-ignore feature.
// Set `.ignore_prefix` to enable ignoring of arguments that begin with the
// specified prefix. Any argument whose string starts with this prefix will be
// skipped by the parser and not added to the argument list.
//
// This can be useful when repeatedly invoking the same command and temporarily
// disabling specific arguments without removing them.
clags_config_t config = clags_config(args, .ignore_prefix="!");

int main(int argc, char **argv)
{
    const char *program_name = argv[0];
    if (clags_parse(argc, argv, &config) != NULL){
        clags_usage(program_name, &config);
        return 1;
    }
    if (help){
        clags_usage(program_name, &config);
        clags_list_free(&list);
        return 0;
    }
    for (size_t i=0; i<list.count; ++i){
        // Since lists are "generic" it is not possible to directly access the list.items
#if 1
        // You can use this convenient macro
        // Arguments:
        //   - the list to index (not a pointer)
        //   - the type of the stored value
        //   - the index to read
        char *item = clags_list_element(list, char*, i);
#else
        // Alternatively, this is the manual way
        char *item = ((char**)list.items)[i];
#endif
        printf("String %zu: '%s'\n", i+1, item);
    }

    // This is mandatory if you don't want to leak all the list's precious memory
    clags_list_free(&list);
    return 0;
}

