#include <stdio.h>
#define CLAGS_IMPLEMENTATION
#include "../clags.h"

char *path = NULL;
bool help = false;

clags_list_t paths = clags_path_list();

clags_arg_t args[] = {
    clags_required_path_list(&paths, "paths", "the paths to open", Clags_Path_Dir),
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
    for (size_t i=0; i<paths.count; ++i){
        printf("%zu: %s\n", i, ((char**)paths.items)[i]);
    }
    clags_list_free(&paths);
    return 0;
}
