/*
  Example 6: Custom Verifiers
      This example shows how to use custom verification functions.
*/

#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

// a custom verification function of type `clags_value_func_t`
bool verify_lower_case(const char *arg_name, char *arg, void *variable)
{
    (void) arg_name;
    if (arg && islower(*arg)){
        *(char**)variable = arg;
        return true;
    }
    fprintf(stderr, "[ERROR] String is not lower case: '%s'!\n", arg);
    return false;
}

clags_list_t list = clags_custom_list(sizeof(char*));
bool help = false;

clags_arg_t args[] = {
    clags_required(&list, "values", "lower case strings", .value_type=Clags_Custom, .verify=verify_lower_case, .is_list=true),
    clags_flag_help(&help),
};

int main(int argc, char **argv)
{
    const char *program_name = argv[0];
    if (!clags_parse(argc, argv, args)){
        clags_usage(program_name, args);
        return 1;
    }
    if (help){
        clags_usage(program_name, args);
        return 0;
    }
    printf("The lower case inputs are:\n");
    for (size_t i=0; i<list.count; ++i){
        printf("%3zu: '%s'\n", i, ((char**)list.items)[i]);
    }
    clags_list_free(&list);
    return 0;
}
