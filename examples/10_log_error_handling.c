/*
  Example 10: Custom Log and Error Handling
      This example demonstrates:
        1. Using a custom log handler to format warnings and errors.
        2. Handling config error types
        3. Using the minimum log level
*/

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdarg.h>

#define CLAGS_IMPLEMENTATION
#include <clags.h>

/*
  A custom log handler, called by `clags_log` internally

  Arguments:
    - level  : the level of error (clags_log_level_t)
    - format : a printf-style format string
    - args   : the variadic arguments for the format printing
*/
void custom_log_handler(clags_config_t *config, clags_log_level_t level, const char *format, va_list args)
{
    (void) config;
    switch(level){
        case Clags_Info:
            printf("INFO: ");
            vprintf(format, args);
            printf("\n");
            break;
        case Clags_Warning:
            fprintf(stderr, "WARNING: ");
            vfprintf(stderr, format, args);
            fprintf(stderr, "\n");
            break;
        case Clags_Error:
            fprintf(stderr, "ERROR: ");
            vfprintf(stderr, format, args);
            fprintf(stderr, "\n");
            break;
        case Clags_ConfigWarning:
            fprintf(stderr, "CONFIG WARNING: ");
            vfprintf(stderr, format, args);
            fprintf(stderr, "\n");
            break;
        case Clags_ConfigError:
            fprintf(stderr, "CONFIG ERROR: ");
            vfprintf(stderr, format, args);
            fprintf(stderr, "\n");
            break;
        case Clags_NoLogs: break;
        default:
            clags_unreachable("Invalid log level!");
    }
}

int32_t lhs = 0;
int32_t rhs = 0;
bool help = false;

clags_arg_t args[] = {
    clags_positional(&lhs, "lhs", "left-hand side (integer)", .value_type=Clags_Int32),
    clags_positional(&rhs, "rhs", "right-hand side (integer)", .value_type=Clags_Int32),
    clags_flag_help(&help),
};

clags_config_t config = clags_config(
    args,
    .log_handler = custom_log_handler,   // instruct clags to use the custom log handler for logging
    .min_log_level = Clags_NoLogs,       // do not print any logs of a lower level, here: disable all logging
);

int main(int argc, char **argv)
{
    const char *program_name = argv[0];

    clags_error_t error = clags_parse(argc, argv, &config);
    if (error.type != Clags_Error_Ok){
        // Re-enable logging
        error.config->options.min_log_level = Clags_Info;

        // Act based on error type
        if (error.type == Clags_Error_InvalidValue){
            clags_log(error.config, Clags_Info, "Invalid number detected. Using defaults (lhs=0, rhs=0).");
            lhs = 0;
            rhs = 0;
        } else {
            // fatal: print detailed usage and exit
            // Check whether the error struct contains information about a specifc failed argument.
            // This depends on the error type. See the `CLAGS__ERROR_TYPES` macro for a complete list.
            if (error.has_arg){
                clags_log(error.config, Clags_Error, "error in '%s' while parsing argument %d of subcommand '%s': '%s'!",
                     error.config->name,
                     clags_arg_index(error.arg.config, error.arg.pointer), // this retrives the index within the configs `clags_arg_t` array
                     error.arg.config->name,
                     clags_error_type_name(error.type)
                );
            } else{
                clags_log(error.config, Clags_Error, "error in '%s': '%s'!",
                     error.config->name,
                     clags_error_type_name(error.type)
                );
            }
            clags_usage(program_name, error.config);
            return 1;
        }
    }

    if (help){
        clags_usage(program_name, &config);
        return 0;
    }

    printf("%"PRId32" + %"PRId32" = %"PRId32"\n", lhs, rhs, lhs + rhs);
    return 0;
}
