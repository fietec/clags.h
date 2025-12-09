/*
  Example 4: Choices
      Choices allow the user to define a set of an 'enum-like' type which only accepts self-defined literals.
*/

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

char *input = NULL;
char *algorithm = NULL;
char *output = "output.pdf";
bool warnings = false;
bool help = false;

uint8_t quality = 0;

clags_choice_t choice_values[] = {
    {"LIFO", "last-in first-out"},
    {"FIFO", "first-in first_out"},
    {"RANDOM", "random order"}
};

clags_choices_t choices = clags_choice(choice_values, true);
clags_choice_t *choice = &choice_values[0];

clags_arg_t args[] = {
    clags_required(&input, "input_file", "the input file"),
    
    clags_optional("-o", "--output", &output, "FILE", "the output file"),
    clags_optional("-a", "--algorithm", &choice, "ALG", "the algorithm to use", .value_type=Clags_Choice, .verify=&choices),
    clags_optional("-q", "--quality", &quality, "LEVEL", "the sample quality", .value_type=Clags_UInt8),

    clags_flag("-w", NULL, &warnings, "print warnings"),
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
    printf("input: %s, algorithm: %s\n", input, choice->value);
    printf("output: %s\n", output);
    printf("quality: %"PRId8"\n", quality);
    return 0;
}
