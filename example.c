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

const char *choice_values[] = {"FIFO", "LIFO"};
clags_choice_t choice = clags_choice(choice_values);

clags_arg_t args[] = {
    clags_required(&input, "input_file", "the input file"),
    clags_optional_choice("-a", "--algorithm", &choice, "ALG", "the algorithm to use"),

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
    printf("input: %s, algorithm: %s\n", input, clags_choice_value(choice));
    printf("output: %s\n", output);
    printf("quality: %"PRId8"\n", quality);
    return 0;
}
