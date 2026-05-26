# clags.h

A declarative command line argument parser for C99. Single-header, no dependencies.

Instead of writing manual `while(getopt(...))` loops and parsing strings with `strtol`, `clags.h` uses C99 designated initializers to define your CLI schema in arrays.
It handles type validation, bounds checking, variadic lists, and subcommands internally.

## Features

- **Declarative API:** Define arguments as static arrays of structs.
- **Config Validation:** Performs an upfront validation step to catch schema definition errors before parsing begins.
- **Built-in Type Validation:** Automatically parses and validates `int`, `uint`, `double`, `bool`, `size` (e.g., "10KiB"), and `time` (e.g., "50ms").
- **Filesystem Checks:** `Clags_File` and `Clags_Dir` types automatically verify existence via `stat()` before parsing succeeds.
- **Restricted Choices:** Bind inputs to a specific set of strings (like an enum).
- **Lists and Terminators:** Consume multiple positional arguments into dynamic arrays, with support for custom list terminators (e.g., `::`).
- **Subcommands:** Native routing to child configurations.
- **Auto-generated Usage:** Formats and aligns help menus based on your config.
- **Memory Tracking:** Tracks its own string duplications and list allocations for a single-call cleanup.

## Integration

`clags.h` uses the standard [stb-style](https://github.com/nothings/stb#how-do-i-use-these-libraries) implementation pattern. Include the header wherever you need the types, but define `CLAGS_IMPLEMENTATION` in exactly one `.c` file before including it to compile the definitions.

```c
#define CLAGS_IMPLEMENTATION
#include "clags.h"
```
## Example

This example demonstrates lists, bounds-checked integers, choices, and memory cleanup.  
See the `examples/` directory for advanced examples including multi-level subcommands and variadic list terminators.
```c
#include <stdio.h>
#include <stdint.h>

#define CLAGS_IMPLEMENTATION
#include "clags.h"

int main(int argc, char **argv)
{
    // 1. Declare variables to hold the parsed state
    clags_list_t inputs = clags_list(Clags_File);
    int64_t optimization = 0;
    clags_choice_t *format = NULL;
    bool verbose = false;
    bool help = false;

    // 2. Define constraints
    clags_range_t opt_range = clags_int_range(0, 3);
    clags_choice_t format_choices[] = {{"elf", "an executable file"}, {"bin", "a binary blob"}, {"hex", "binary values as hex strings"}};
    clags_choices_t formats = clags_choices(format_choices);

    // 3. Define the argument schema
    clags_arg_t args[] = {
        clags_positional(&inputs, "files", "input files to process", .is_list=true, .value_type=Clags_File),
        clags_option('O', "opt", &optimization, "LEVEL", "optimization level", .value_type=Clags_Int, .range=&opt_range, .default_input="1"),
        clags_option('f', "format", &format, "FMT", "output format", .value_type=Clags_Choice, .choices=&formats, .default_input="elf"),
        clags_flag('v', "verbose", &verbose, "enable verbose output"),
        clags_flag_help(&help),
    };

    // 4. Initialize the config (with optional parser behaviors)
    clags_config_t config = clags_config(args, .ignore_prefix="!");

    // 5. Parse
    if (clags_parse(argc, argv, &config) != NULL) {
        // clags_parse returns the failed config on error
        clags_usage(argv[0], &config);
        clags_config_free(&config);
        return 1;
    }

    if (help) {
        clags_usage(argv[0], &config);
        clags_config_free(&config);
        return 0;
    }

    // 6. Use the validated data
    printf("Format: %s | Opt Level: %ld | Verbose: %d\n", format->value, optimization, verbose);
    printf("Valid files found: %zu\n", inputs.count);

    for (size_t i = 0; i < inputs.count; i++) {
        printf("  - %s\n", clags_list_element(inputs, char*, i));
    }

    // 7. Free lists and duplicated strings
    clags_config_free(&config);
    return 0;
}
```

When run with ./build --help, clags generates the following:
```Plaintext
Usage: ./build [OPTIONS] [FLAGS] <files..>
  Arguments:
    files                            : input files to process (file[])
  Options:
    -O, --opt(=)LEVEL                : optimization level (int, 0-3) (default: 1)
    -f, --format(=)FMT               : output format (choice) (default: elf)
        Choices:
          - elf                      : an executable file
          - bin                      : a binary blob
          - hex                      : binary values as hex strings
  Flags:
    -v, --verbose                    : enable verbose output
    -h, --help                       : print this help dialog and exit

  Notes:
    Arguments prefixed with '!' are ignored.

```
## Memory Management

By default, clags points char* variables directly to the argv strings to avoid unnecessary allocations.

If you enable `.duplicate_strings = true` in your `clags_options_t`, the parser will calloc duplicates. It tracks all internal allocations (including variadic lists) in the `clags_config_t` context.

Calling `clags_config_free(&config)` at the end of your program will safely clean up all tracked memory. You can override the default allocators by defining `CLAGS_CALLOC`, `CLAGS_REALLOC`, and `CLAGS_FREE` before including the implementation.
