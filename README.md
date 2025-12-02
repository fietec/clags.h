# clags.h
A lightweight command line argument parser.

## Features
- Required and optional arguments
- Flags and help flag support
- Typed arguments: `bool`, `int8`, `uint8`, `int32`, `uint32`, `double`, `path`
- Choice arguments: restrict values to a fixed set (like an enum)
- Custom parsing functions for user-defined types

## How to use
`clags.h` is an stb-style library, which means a single header file
and headers and implementations separated by the `CLAGS_IMPLEMENTATION` header guard.  

### Example
```c
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
    clags_optional_choice("-a", "--algorithm", &choices, &choice, "ALG", "the algorithm to use"),
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
    printf("input: %s, algorithm: %s\n", input, choice->value);
    printf("output: %s\n", output);
    printf("quality: %"PRId8"\n", quality);
    return 0;
}
```
For an incorrect input, such as 
```
./example -a QUICK_SORT input.txt
```
**clags** will print the following error message and usage:
```
[ERROR] Invalid choice for argument '-a': 'QUICK_SORT'!
Usage: ./example [OPTIONS] [FLAGS] <input_file>
  Arguments:
    input_file               : the input file
  Options:
    -o, --output(=)FILE      : the output file
    -a, --algorithm(=)ALG    : the algorithm to use (choice)
        Choices:
          - LIFO             : last-in first-out
          - FIFO             : first-in first_out
          - RANDOM           : random order

    -q, --quality(=)LEVEL    : the sample quality (uint8)
  Flags:
    -w                       : print warnings
    -h, --help               : print this help dialog
```
