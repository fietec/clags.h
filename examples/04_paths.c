#include <stdio.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

char *path = NULL;
bool help = false;

clags_arg_t args[] = {
    clags_required_path(&path, "path", "the path to open", Clags_Path_All),
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
    printf("provided path: '%s'\n", path);
    return 0;
}
