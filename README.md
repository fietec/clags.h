# clags.h

A single-header C99 library for parsing and validating command-line arguments.

## Features

- **Flexible CLI syntax**
  - Positional arguments (required and optional)
  - Options with values using `--foo=bar`, `--foo bar`, `-O3` or `-o a.out` syntax
  - Boolean flags, including combined short flags (`-abc`)
  - Variadic positional and option arguments
  - `--` terminator to disable option parsing (optionally reversible)
  - Configurable list terminators for variadic arguments
  - Ignore prefixes to skip matching arguments (e.g. `!ignored`)

- **Direct memory binding**
  - Parses values directly into application variables
  - Performs type conversion and assignment automatically

- **Schema validation**
  - Detects configuration errors before parsing user input
  - Examples include duplicate flags, invalid positional ordering, and conflicting definitions

- **Input validation**
  - Verifies parsed values against type constraints and validation rules
  - Built-in support for:
    - Primitive types (`int`, `uint`, `float`, `double`, `bool`, etc.)
    - Numeric ranges
    - Enumerated choices
    - Size units (e.g. `10KiB`, `4GB`)
    - Time units (e.g. `50ms`, `2s`, `1d4h15s`)
    - Filesystem paths with existence and type checks
    - Parameterized custom types, with possible extension of existing types

- **Hierarchical subcommands**
  - Supports deeply nested subcommands
  - Optional inheritance of flags and options throughout the command tree

- **Automatic memory tracking**
  - Tracks dynamically allocated lists and duplicated strings
  - Releases all internally managed allocations with a single cleanup call (see the [Memory Model](#memory-model))
  
## Integration

clags.h uses the [stb-style](https://github.com/nothings/stb#how-do-i-use-these-libraries) single-header implementation pattern. Include the header normally to access types and function declarations. In exactly one C source file, define `CLAGS_IMPLEMENTATION` before including the file to compile the function implementations.
```c
#define CLAGS_IMPLEMENTATION
#include "clags.h"
```
If you still want to pre-compile the library and link with it later, you can create the `.o` file like this:
```plaintext
cc -xc -DCLAGS_IMPLEMENTATION -c clags.h
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
    char *output;
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
        clags_option('o', "output", &output, "FILE", "the file to output", .default_input="a.out"),
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
        // Returns a pointer to the specific subcommand config context that encountered the error
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

    printf("Output File: %s | Format: %s | Opt Level: %ld | Verbose: %d\n", output, format->value, optimization, verbose);
    printf("Input files: %zu\n", inputs.count);

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
6. Free dynamically populated lists and other internal allocations. This is only necessary when lists were defined in the argument schema or string duplication was enabled. See the [Memory Model](#memory-model) section for more info.

### Generated Usage

Executing the program above with `--help` formats the following layout dynamically based on constraints, types, and defaults:
```plaintext
Usage: ./example [OPTIONS] [FLAGS] <files..>
  Arguments:
    files                            : input files to process (file[])
  Options:
    -o, --output(=)FILE              : the file to output (default: a.out)
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

### Error Handling

Invalid input to the program, such as:

```bash
./example -O4 main.c
```

produces the following descriptive error message:

```plaintext
[ERROR] int out of range [0-3] for argument '-O4': '4'!
```

When an error occurs, `clags_parse()` returns a pointer to the config where the error occurred,
with the `.error` field indicating the type of failure.
This allows error handling and usage reporting to be performed at the appropriate subcommand level.

## Memory Model

`clags_parse()` performs no persistent allocations beyond those required for explicitly requested features.

### Lists

When parsing into a `clags_list_t`, the parser allocates storage for the collected elements. Ownership of that storage is transferred to the application and remains valid until explicitly released.

Free an individual list with:

```c
clags_list_free(&list);
```

### Strings

By default, string arguments are **not** copied. Instead, `char *` targets are assigned pointers directly into the program's `argv` array, resulting in zero allocations for string values.

If independent ownership is required, enable string duplication:

```c
clags_config_t config = clags_config(args,
    .duplicate_strings = true
);
```

Duplicated strings are tracked by the owning `clags_config_t` and can be released with:

```c
clags_config_free_allocs(&config);
```

### Convenience Cleanup

Instead of freeing resources individually, you can use the provided cleanup helpers:

- `clags_config_free(&config)` frees all duplicated strings and lists owned by the specified config.
- `clags_free(&config)` performs the same cleanup recursively for the config and all of its child subcommands.

### Custom Allocator Overrides

To integrate with custom allocators (such as arena or pool allocators), override the fallback macros prior to compiling the implementation block. Because low-overhead systems often require explicitly sized free operations, the allocation size is passed to both `CLAGS_REALLOC` and `CLAGS_FREE`:
```c
#define CLAGS_CALLOC(count, size)        my_arena_calloc(count, size)
#define CLAGS_REALLOC(ptr, oldsz, newsz) my_arena_realloc(ptr, oldsz, newsz)
#define CLAGS_FREE(ptr, sz)              my_arena_free(ptr, sz)

#define CLAGS_IMPLEMENTATION
#include "clags.h"
```
