# clags.h
A lightweight command line argument parser.

## How to use
`clags.h` is an stb-style library, which means a single header file
and headers and implementations seperated by the `CLAGS_IMPLEMENTATION` header guard.  

### Example
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

## Documentation

The main two main functions provided by **clags** are:
```c
bool clags_parse(int argc, char **argv, clags_arg_t args[]);
void clags_usage(const char *program_name, clags_arg_t args[]);
```
### Required arguments

You can create an required argument by calling:
```c
clags_arg_t clags_required(char *output_variable, const char *argument_name, const char *argument_description);
```
Additionally, there are also required argument parsers for certain basic data types:
- `bool`: `clags_required_bool(bool *output_variable, const char *argument_name, const char *argument_description);`
- `int8`: `clags_required_int8(int8_t *output_variable, const char *argument_name, const char *argument_description);`
- `uint8`: `clags_required_uint8(uint8_t *output_variable, const char *argument_name, const char *argument_description);`
- `int32`: `clags_required_int32(int32_t *output_variable, const char *argument_name, const char *argument_description);`
- `uint32`: `clags_required_uint32(uint32_t *output_variable, const char *argument_name, const char *argument_description);`
- `double`: `clags_required_double(double *output_variable, const char *argument_name, const char *argument_description);`
- `custom`: `clags_required_custom(void *output_variable, const char *argument_name, const char *argument_description, clags_value_func_t custom_value_func);`
for implementing parse-time custom data types.  

#### Lists
You can create an argument list by calling:
```c
clags_arg_t clags_required_<type>_list(clags_list_t *list, const char *argument_name, const char *argument_description);
```
For this, you have to define a list:
```c
clags_list_t my_list = clags_<type>_list;
```
where the last part initializes the list with the correct item size for each type.  
For the `custom` type, you have to provide your custom item's size in bytes:
```c
clags_list_t my_custom_list = clags_custom_list(size_t size_in_bytes);
```
Lists have to be manually de-allocated after you are done using them:
```c
void clags_list_free(clags_list_t *list);
```
### Optional arguments

You can create an optional argument by calling:
```c
clags_arg_t clags_optional(const char *short_flag, const char *long_flag, char *output_variable, const char *argument_name, const char *argument_description);
```
Similarly to the required arguments, there are also typed versions for all supported types:
```c
clags_arg_t clags_optional_<type>(const char *short_flag, const char *long_flag, <type> *output_variable, const char *argument_name, const char *argument_description);
```
All these support the `-o <file>`, `--output <file>` and `--output=<file>` syntaxes.

### Flags

```c
clags_arg_t clags_flag(const char *short_flag, const char *long_flag, bool *output_variable, const char *flag_description, bool exit_on_occurence);
```
For convenience, **clags** also provides a short hand for the common `-h` and `--help` flags:
```c
clags_arg_t clags_flag_help(bool *help_flag_variable);
```
