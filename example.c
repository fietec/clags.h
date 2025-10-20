#include <stdio.h>
#include <stdbool.h>

char *input = NULL;
char *output = NULL;
char *algorithm = NULL;
bool help = false;
bool line = false;

#define clags_required\
    clags_arg("input_file", &input, "the input file")\
    clags_arg("algorithm", &algorithm, "the algorithm to use")

#define clags_optional\
    clags_arg("-o", &output, "output_file", "the output file")\
    clags_arg("-q", NULL, "quality", "the sample quality")

#define clags_flags\
    clags_arg("-w", &line, "print warnings")

#define CLAGS_IMPLEMENTATION
#include "clags.h"

int main(int argc, char **argv)
{
    if (!clags_parse(argc, argv, &help)){
        clags_usage(argv[0]);
        return 1;
    }
    if (help) {
        clags_usage(argv[0]);
        return 0;
    }
    printf("input: %s, output: %s\n", input, output);
    return 0;
}
