/*
  Example 7.1: Inherited Custom Verifiers
      This example shows how to create more complex custom types.
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

// A custom verfier that checks whether the input is a multiple of a base
bool multiple_of(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    int64_t value;
    // First, we need to verify that the argument is a number, so we can use the already built-in `Clags_Int` type verifiers.
    // Therefore, this custom types "inherits" functionality of that type and extends its functionality
    if (!clags_verify_int(config, arg_name, arg, &value, NULL)) return false;
    // Get the base defined by the custom type
    intptr_t base = (intptr_t) data;
    if (value % base != 0){
        clags_log(config, Clags_Error, "value is not a multiple of %"PRIdPTR" for argument '%s': %"PRIdPTR"!", base, arg_name, value);
        return false;
    }
    if (variable) *(int64_t*) variable = value;
    return true;
}

// The definition of two custom types with that share the same verification function, but differ in the type argument, here the `base`
clags_custom_t multiple_10 = {"multiple of 10", multiple_of, (void*) 10};
clags_custom_t multiple_2 = {"multiple of 2", multiple_of, (void*) 2};

int64_t mten = 0;
int64_t mtwo = 0;
bool help = false;

clags_arg_t args[] = {
    clags_positional(&mten, "10-multiple", "a multiple of ten", .value_type=Clags_Custom, .custom=&multiple_10),
    clags_positional(&mtwo, "2-multiple", "a multiple of two", .value_type=Clags_Custom, .custom=&multiple_2),
    clags_flag_help(&help),
};

clags_config_t config = clags_config(args);

int main(int argc, char *argv[])
{
    const char *program_name = argv[0];
    
    clags_config_t *failed = clags_parse(argc, argv, &config);
    if (failed){
        clags_usage(program_name, &config);
        return 1;
    }
    if (help){
        clags_usage(program_name, &config);
        return 0;
    }
    printf("mten : %"PRId64"\n", mten);
    printf("mtwo : %"PRId64"\n", mtwo);
    return 0;
}
