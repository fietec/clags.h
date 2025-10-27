#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "clags.h"

char *input = NULL;
char *algorithm = NULL;
char *output = "output.pdf";
bool warnings = false;
bool help = false;

uint8_t quality = 0;

clags_list_t list = clags_custom_list(sizeof(char*));

bool check_algorithm(const char *arg_name, const char *arg, void *pvalue)
{
    if (strcmp(arg, "FIFO") == 0 || strcmp(arg, "LIFO") == 0){
        *(char**)pvalue = (char*) arg;
        return true;
    }
    fprintf(stderr, "Unknown %s: '%s'!\n", arg_name, arg);
    return false;
}

clags_arg_t args[] = {
    clags_required(&input, "input_file", "the input file"),
    clags_required_custom_list(&list, "algorithm", "the algorithm to use", &check_algorithm),
    
    clags_optional("-o", "--output", &output, "FILE", "the output file"),
    clags_optional_uint8("-q", "--quality", &quality, "LEVEL", "the sample quality"),

    clags_flag("-w", NULL, &warnings, "print warnings", false),
    clags_flag_help(&help),
};

int main(int argc, char **argv)
{
    if (!clags_parse(argc, argv, args)){
        clags_usage(argv[0], args);
        return 1;
    }
    if (help){
        clags_usage(argv[0], args);
        return 0;
    }
    printf("input: %s, output: %s, algorithm: %s\n", input, output, algorithm);
    printf("quality: %"PRId8"\n", quality);
    printf("list with %zu items:\n", list.count);
    for (size_t i=0; i<list.count; ++i){
        printf("'%s'\n", ((char**) list.items)[i]);
    }
    free(list.items);
    return 0;
}
