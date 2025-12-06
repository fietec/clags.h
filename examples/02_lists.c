#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

bool help = false;
clags_list_t list = clags_list();

clags_arg_t args[] = {
    clags_required_list(&list, "strings", "the strings to print"),
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
    for (size_t i=0; i<list.count; ++i){
	printf("String %zu: '%s'\n", i+1, ((char**)list.items)[i]);
    }

    clags_list_free(&list);
    return 0;
}

