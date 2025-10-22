# clags.h
A lightweight command line argument parser.

## How to use
`clags.h` is an stb-style library, which means a single header file
and headers and implementations seperated by the `CLAGS_IMPLEMENTATION` header guard.  
See an easy example below:
```c
#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "clags.h"

char *input = NULL;
char *output = NULL;
char *algorithm = NULL;
char *quality = NULL;
bool help = false;
bool warnings = false;

clags_arg_t args[] = {
    clags_required(&input, "input_file", "the input file"),
    clags_required(&algorithm, "algorithm", "the algorithm to use"),
    
    clags_optional("-o", "--output", &output, "FILE", "the output file"),
    clags_optional("-q", "--quality", &quality, "LEVEL", "the sample quality"),

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
    printf("input: %s, output: %s\n", input, output);
    return 0;
}
```
