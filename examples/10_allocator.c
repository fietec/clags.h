/*
  Example 10: Custom Allocators
      This example shows how to use custom allocators and redefine allocation functions.
*/

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// We use Tsoding's `arena` as the exemplary custom allocator
#define ARENA_IMPLEMENTATION
#include "10_arena.h"

static Arena arena = {0};

// Redefine the internal allocation functions
#define CLAGS_CALLOC(count, size) arena_zalloc(&arena, count*size)
#define CLAGS_REALLOC(ptr, oldsz, newsz) arena_realloc(&arena, ptr, oldsz, newsz)
// The arena implementation used here does not have support of indiviual deallocation
#define CLAGS_FREE(ptr, sz)
#define CLAGS_IMPLEMENTATION
#include "../clags.h"

clags_list_t strings = clags_list(Clags_String);
clags_list_t nums    = clags_list(Clags_UInt32);
bool help = false;

clags_arg_t args[] = {
    clags_positional(&strings, "strings", "a list of strings", .is_list=true),
    clags_positional(&nums, "nums", "a list of nums", .is_list=true, .value_type=Clags_UInt32),
    clags_flag_help(&help),
};

clags_config_t config = clags_config(args, .list_terminator="::");

int main(int argc, char *argv[])
{
    int result = 0;
    
    const char *program_name = argv[0];
    clags_config_t *failed = clags_parse(argc, argv, &config);
    if (failed){
        clags_usage(program_name, failed);
        clags_return_defer(1);
    }
    if (help){
        clags_usage(program_name, &config);
        clags_return_defer(0);
    }

    printf("Strings:\n");
    for (size_t i=0; i<strings.count; ++i){
        printf("%s\n", clags_list_element(strings, char *, i));
    }
    printf("\nNums:\n");
    for (size_t i=0; i<nums.count; ++i){
        printf("%"PRIu32"\n", clags_list_element(nums, uint32_t, i));
    }
    
defer:
    // Calling `clags_list_free` would do nothing here, since we disabled individual deallocation
    arena_free(&arena);
    return result;
}
