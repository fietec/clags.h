# clags.h

A declarative command-line argument parser for C99. Implemented as a single-header library with zero external dependencies.

clags.h allows you to define a CLI schema statically using C99 designated initializers, binding command-line inputs directly to the memory addresses of application variables.

## Features
- Flexible CLI Syntax:
  - Positional arguments (required and optional).
  - Options (flags with values) supporting multiple syntaxes: `--foo=bar`, `--foo bar`, `-O3`.
  - Flags (boolean switches) with combined flags: `-abc`.
  - Variadic argument collection into positional or option lists
  - Double-dash (`--`) argument terminator for disabling further option parsing (can be re-enabled if thus configured).
  - List terminators to explicitly end variadic argument collection.
  - Ignore prefix to skip arguments matching a specified prefix (e.g., `!ignored`).
- Direct Memory Binding: Arguments are mapped directly to variable pointers. Type conversion and value assignments are handled internally by the parser during execution.
- Two-Phase Validation:
  - Schema Validation: Inspects the developer's configuration array before input processing to detect structural errors (e.g., duplicate flags, required positional arguments placed after optional ones, etc.).
  - Input Verification: Performs runtime checks on user input, ensuring values conform to type boundaries, enum choices, or physical criteria.
	- Extensive Primitive & Derived Types: Integrated parsing for standard types (`int`, `uint`, `float`, `double`, `bool`, etc.)
    - Size units (e.g., 10KiB, 4GB).
    - Time units (e.g., 50ms, 2s, 1h).
    - Filesystem Paths: `Clags_File` and `Clags_Dir` types physically verify existence and attribute types via system stat checks.
	- Custom Types: support for parameterized custom type implementations that can also extend already existing ones
- Hierarchical Subcommands: Native support for deeply nested subcommands with optional flag and option inheritance down the command tree.
- Contextual Memory Tracking: Tracks internal dynamic arrays (lists) and string duplications within the configuration context, enabling full cleanups via a single function call.

## Integration

clags.h uses the [stb-style](https://github.com/nothings/stb#how-do-i-use-these-libraries) single-header implementation pattern. Include the header normally to access types and function declarations. In exactly one C source file, define `CLAGS_IMPLEMENTATION` before including the file to compile the function implementations.
```c
#define CLAGS_IMPLEMENTATION
#include "clags.h"
```
## Usage

For a detailed showcase of clags's features, see the `examples` directory containing multiple incrementally more advanced examples and documentation.

### Example

This basic example demonstrates handling variadic file lists, range-constrained integers, restricted string choices, and automated help generation.  
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

    // 2. Define the argument schema
    clags_range_t opt_range = clags_int_range(0, 3);
    clags_choice_t format_choices[] = {
        {"elf", "an executable file"}, 
        {"bin", "a binary blob"}, 
        {"hex", "binary values as hex strings"}
    };
    clags_choices_t formats = clags_choices(format_choices);

    clags_arg_t args[] = {
        clags_positional(&inputs, "files", "input files to process", .is_list=true, .value_type=Clags_File),
        clags_option('O', "opt", &optimization, "LEVEL", "optimization level", .value_type=Clags_Int, .range=&opt_range, .default_input="1"),
        clags_option('f', "format", &format, "FMT", "output format", .value_type=Clags_Choice, .choices=&formats, .default_input="elf"),
        clags_flag('v', "verbose", &verbose, "enable verbose output"),
        clags_flag_help(&help),
    };

    // 3. Initialize the config context
    clags_config_t config = clags_config(args, .ignore_prefix="!");

    // 4. Parse execution inputs
    clags_config_t* failed_config = clags_parse(argc, argv, &config);
    if (failed_config != NULL) {
        // Returns a pointer to the specific subcommand configuration context that encountered the error
        clags_usage(argv[0], failed_config);
        clags_config_free(&config);
        return 1;
    }

    // 5. Use the populated variables
    if (help) {
        clags_usage(argv[0], &config);
        clags_config_free(&config);
        return 0;
    }

    printf("Format: %s | Opt Level: %ld | Verbose: %d\n", format->value, optimization, verbose);
    printf("Valid files found: %zu\n", inputs.count);

    for (size_t i = 0; i < inputs.count; i++) {
        printf("  - %s\n", clags_list_element(inputs, char*, i));
    }

    // 6. Free list structures and allocations
    clags_config_free(&config);
    return 0;
}
```
As you can see, parsing arguments with clags.h follows a simple 6-step sequence:

1. Declare the native variables that will hold the final parsed state.

2. Populate an array of `clags_arg_t` structures using provided convenience macros (`clags_positional`, `clags_option`, `clags_flag`).

3. Wrap the schema array in a `clags_config_t` structure along with global parser parameters.

4. Pass argc and argv to `clags_parse`. This checks for user syntax errors and populates the bound variables.

5. Use the populated variables
6. Free dynamically populated lists and other internal allocations. This is only necessary when lists were defined in the argument schema or string duplication was enabled. See the [Memory Allocation Model](#memory-allocation-model) section for more info.

### Generated Usage

Executing the resulting program with `--help` formats the following layout dynamically based on constraints, types, and defaults:
```plaintext
Usage: ./example [OPTIONS] [FLAGS] <files..>
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

## Memory Allocation Model

By default, clags.h operates zero-allocation on string parameters by assigning `char*` targets to direct slices inside the environment's argv array.

If explicit ownership of string data is required, setting `.duplicate_strings = true` inside `clags_options_t` directs the parser to duplicate string values using standard allocations. All tracking pointers and list instances are cataloged internally within the respective `clags_config_t` object.

Calling `clags_config_free(&config)` frees all allocations bound to that config.

## Custom Allocator Overrides

To integrate with custom allocators (such as arena or pool allocators), override the fallback macros prior to compiling the implementation block. Because low-overhead systems often require explicitly sized free operations, the size of the allocated size is passed to both `CLAGS_REALLOC` and `CLAGS_FREE`:
```c
#define CLAGS_CALLOC(count, size)        my_arena_calloc(count, size)
#define CLAGS_REALLOC(ptr, oldsz, newsz) my_arena_realloc(ptr, oldsz, newsz)
#define CLAGS_FREE(ptr, sz)              my_arena_free(ptr, sz)

#define CLAGS_IMPLEMENTATION
#include "clags.h"
```
