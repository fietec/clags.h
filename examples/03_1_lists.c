/*
  Example 3.1: Lists
     This example shows how to work with lists.
     Lists are only supported as required arguments and can also be typed, similarily to normal arguments
*/

#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

bool help = false;
// use a typed initializer here if required
clags_list_t list = clags_list();

clags_arg_t args[] = {
    clags_required(&list, "strings", "the strings to print", .is_list=true),
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
    if (!clags_parse(argc, argv, config)){
        clags_usage(program_name, config);
        return 1;
    }
    if (help){
        clags_usage(program_name, config);
        clags_list_free(&list);
        return 0;
    }
    for (size_t i=0; i<list.count; ++i){
        printf("String %zu: '%s'\n", i+1, clags_list_element(list, char*, i));
    }

    // this is mandatory if you don't want to leek all the list's precious memory
    clags_list_free(&list);
    return 0;
}

