/*
  clags.h - A simple command line arguments parser for C

  MIT License

  Copyright (c) 2025 Constantijn de Meer

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef CLAGS_H
#define CLAGS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <ctype.h>
#include <sys/stat.h>
#include <inttypes.h>

#ifndef CLAGS_FREE
#define CLAGS_FREE free
#endif // CLAGS_FREE

#ifndef CLAGS_REALLOC
#define CLAGS_REALLOC realloc
#endif // CLAGS_REALLOC

typedef bool (*clags_custom_verify_func_t)(const char *arg_name, const char *arg, void *variable);
typedef bool (clags_verify_func_t)(const char *arg_name, const char *arg, void *variable, void *verify);
typedef clags_verify_func_t *clags_verify_func_ptr_t;
typedef uint64_t clags_fsize_t;
typedef uint64_t clags_time_t;

clags_verify_func_t clags__verify_none;
clags_verify_func_t clags__verify_custom;
clags_verify_func_t clags__verify_bool;
clags_verify_func_t clags__verify_int8;
clags_verify_func_t clags__verify_uint8;
clags_verify_func_t clags__verify_int32;
clags_verify_func_t clags__verify_uint32;
clags_verify_func_t clags__verify_int64;
clags_verify_func_t clags__verify_uint64;
clags_verify_func_t clags__verify_double;
clags_verify_func_t clags__verify_choice;
clags_verify_func_t clags__verify_path;
clags_verify_func_t clags__verify_file;
clags_verify_func_t clags__verify_dir;
clags_verify_func_t clags__verify_size;
clags_verify_func_t clags__verify_time_s;
clags_verify_func_t clags__verify_time_ns;

// the defintions of all supported value types. Format: (EnumValue, verification_function, type_name)
#define clags__types\
   X(Clags_None,   clags__verify_none,    NULL      ) \
   X(Clags_Custom, clags__verify_custom,  "custom"  ) \
   X(Clags_Bool,   clags__verify_bool,    "bool"    ) \
   X(Clags_Int8,   clags__verify_int8,    "int8"    ) \
   X(Clags_UInt8,  clags__verify_uint8,   "uint8"   ) \
   X(Clags_Int32,  clags__verify_int32,   "int32"   ) \
   X(Clags_UInt32, clags__verify_uint32,  "uint32"  ) \
   X(Clags_Int64,  clags__verify_int64,   "int64"   ) \
   X(Clags_UInt64, clags__verify_uint64,  "uint64"  ) \
   X(Clags_Double, clags__verify_double,  "double"  ) \
   X(Clags_Choice, clags__verify_choice,  "choice"  ) \
   X(Clags_Path,   clags__verify_path,    "path"    ) \
   X(Clags_File,   clags__verify_file,    "file"    ) \
   X(Clags_Dir,    clags__verify_dir,     "dir"     ) \
   X(Clags_Size,   clags__verify_size,    "size"    ) \
   X(Clags_TimeS,  clags__verify_time_s,  "time_s"  ) \
   X(Clags_TimeNS, clags__verify_time_ns, "time_ns" ) \

#define X(type, func, name) type,
typedef enum{
    clags__types
} clags_value_type_t;
#undef X

typedef struct{
    void *items;
    size_t item_size;
    size_t count;
    size_t capacity;
} clags_list_t;

typedef struct{
    const char *value;
    const char *description;
} clags_choice_t;

typedef struct{
    clags_choice_t *items;
    size_t count;
    bool print_no_details;
} clags_choices_t;

typedef struct{
    void *variable;
    const char *arg_name;
    const char *description;
    // options
    clags_value_type_t value_type;
    void *verify;
    bool is_list;
} clags_required_t;

typedef struct{
    char short_flag;
    const char *long_flag;
    void *variable;
    const char *arg_name;
    const char *description;
    // options
    clags_value_type_t value_type;
    void *verify;
} clags_optional_t;

typedef struct{
    char short_flag;
    const char *long_flag;
    bool *variable;
    const char *description;
    // options
    bool exit;
} clags_flag_t;

typedef struct{
    clags_required_t *required;
    size_t required_count;
    clags_optional_t *optional;
    size_t optional_count;
    clags_flag_t *flags;
    size_t flag_count;
} clags_args_t;

typedef enum{
    Clags_Required,
    Clags_Optional,
    Clags_Flag
} clags_arg_type_t;

// construct with `clags_requried`, `clags_optional` and `clags_flag` macros
typedef struct{
    clags_arg_type_t type;
    union{
        clags_required_t req;
        clags_optional_t opt;
        clags_flag_t flag;
    };
} clags_arg_t;

typedef struct{
    const char *ignore_prefix;
    const char *list_terminator;
    bool print_no_notes;
    bool allow_option_parsing_toggle;
} clags_options_t;

// construct with `clags_config` macro
typedef struct{
    clags_arg_t *args;
    size_t args_count;
    clags_options_t options;
    bool invalid;
} clags_config_t;

#define CLAGS_USAGE_ALIGNMENT -24

// constructs a config from an array of clags_arg_t args
#define clags_config(args, ...) (clags_config_t){.args=(args), .args_count=clags_arr_len(args), .options=(clags_options_t){__VA_ARGS__}}

// a required, positional argument, can be typed
#define clags_required(var, name, desc, ...) (clags_arg_t){.type=Clags_Required, .req=(clags_required_t){.variable=(var), .arg_name=(name), .description=(desc), __VA_ARGS__}}

// an optional argument, can be typed
#define clags_optional(sflag, lflag, var, name, desc, ...) (clags_arg_t){.type=Clags_Optional, .opt=(clags_optional_t){.short_flag=(sflag), .long_flag=(lflag), .variable=(var), .arg_name=(name), .description=(desc), __VA_ARGS__}}

// a boolean flag argument
#define clags_flag(sflag, lflag, var, desc, ...) (clags_arg_t) {.type=Clags_Flag, .flag=(clags_flag_t){.short_flag=(sflag), .long_flag=(lflag), .variable=(var), .description=(desc), __VA_ARGS__}}
#define clags_flag_help(val) clags_flag('h', "help", val, "print this help dialog", .exit=true)

#define clags_list()            (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(char*)}
#define clags_path_list()       clags_list()
#define clags_file_list()       clags_list()
#define clags_dir_list()        clags_list()
#define clags_custom_list(size) (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=(size)}
#define clags_bool_list()       (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(bool)}
#define clags_int8_list()       (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(int8_t)}
#define clags_uint8_list()      (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(uint8_t)}
#define clags_int32_list()      (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(int32_t)}
#define clags_uint32_list()     (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(uint32_t)}
#define clags_int64_list()      (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(int64_t)}
#define clags_uint64_list()     (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(uint64_t)}
#define clags_double_list()     (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(double)}
#define clags_size_list()       (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(clags_fsize_t)}
#define clags_choice_list()     (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(clags_choice_t*)}

#define clags_list_element(list, value_type, i) ((value_type*)(list).items)[i]
#define clags_list_choice_element(list, i) clags_list_element((list), (clags_choice_t*), (i))->value

#define clags_choice(arr, ...) (clags_choices_t){.items=(arr), .count=clags_arr_len(arr), __VA_ARGS__}
#define clags_choice_default(choices, index) (&(choices)[index])

#define clags_arr_len(arr) ((arr)==NULL?0:(sizeof(arr)/sizeof(arr[0])))

// parse arguments according to the given configuration.
// returns true if parsing succeeded, false otherwise.
bool clags_parse(int argc, char **argv, clags_config_t *config);

// print usage information for the program, based on the given configuration.
void clags_usage(const char *program_name, clags_config_t *config);

// free all memory associated with a clags_list_t instance.
void clags_list_free(clags_list_t *list);

#endif // CLAGS_H

#ifdef CLAGS_IMPLEMENTATION

#define X(type, func, name) [type] = func,
static clags_verify_func_ptr_t clags__verify_funcs[] = {
    clags__types
};
#undef X

#define X(type, func, name) [type] = name,
static const char *clags__type_names[] = {
    clags__types
};
#undef X

bool clags__verify_none(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    (void) arg_name;
    if (pvalue) *(char**)pvalue = (char*)arg;
    return true;
}

bool clags__verify_bool(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    if (strcmp(arg, "true") == 0 || strcmp(arg, "True") == 0) {
        if (pvalue) *(bool*)pvalue = true;
        return true;
    } else if (strcmp(arg, "false") == 0 || strcmp(arg, "False") == 0) {
        if (pvalue) *(bool*)pvalue = false;
        return true;
    }
    fprintf(stderr, "[ERROR] Invalid boolean value for argument '%s': '%s'!\n", arg_name, arg);
    return false;
}

bool clags__verify_int8(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    char *endptr;
    errno = 0;
    long value = strtol(arg, &endptr, 0);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid int8 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value < INT8_MIN || value > INT8_MAX) {
        fprintf(stderr, "[ERROR] int8 value out of range (%d to %d) for argument '%s': '%s'!\n", INT8_MIN, INT8_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(int8_t*)pvalue = (int8_t)value;
    return true;
}

bool clags__verify_uint8(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    char *endptr;
    errno = 0;
    unsigned long value = strtoul(arg, &endptr, 0);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid uint8 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > UINT8_MAX || *arg == '-') {
        fprintf(stderr, "[ERROR] uint8 value out of range (0 to %u) for argument '%s': '%s'!\n", UINT8_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(uint8_t*)pvalue = (uint8_t)value;
    return true;
}

bool clags__verify_int32(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    char *endptr;
    errno = 0;
    long value = strtol(arg, &endptr, 0);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid int32 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value < INT32_MIN || value > INT32_MAX) {
        fprintf(stderr, "[ERROR] int32 value out of range (%d to %d) for argument '%s': '%s'!\n", INT32_MIN, INT32_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(int32_t*)pvalue = (int32_t)value;
    return true;
}

bool clags__verify_uint32(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    char *endptr;
    errno = 0;
    unsigned long value = strtoul(arg, &endptr, 0);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid uint32 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > UINT32_MAX || *arg == '-') {
        fprintf(stderr, "[ERROR] uint32 value out of range (0 to %u) for argument '%s': '%s'!\n", UINT32_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(uint32_t*)pvalue = (uint32_t)value;
    return true;
}

bool clags__verify_int64(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    char *endptr;
    errno = 0;
    long long value = strtoll(arg, &endptr, 0);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid int64 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value < INT64_MIN || value > INT64_MAX) {
        fprintf(stderr, "[ERROR] int64 value out of range (%ld to %ld) for argument '%s': '%s'!\n", INT64_MIN, INT64_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(int64_t*)pvalue = (int64_t)value;
    return true;
}

bool clags__verify_uint64(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    char *endptr;
    errno = 0;
    unsigned long long value = strtoull(arg, &endptr, 0);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid uint64 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > UINT64_MAX || *arg == '-') {
        fprintf(stderr, "[ERROR] uint64 value out of range (0 to %lu) for argument '%s': '%s'!\n", UINT64_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(uint64_t*)pvalue = (uint64_t)value;
    return true;
}

bool clags__verify_double(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void)func;
    char *endptr;
    errno = 0;
    double value = strtod(arg, &endptr);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid double value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > DBL_MAX || value < -DBL_MAX) {
        fprintf(stderr, "[ERROR] double value out of range for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }

    if (pvalue) *(double*)pvalue = value;
    return true;
}

bool clags__verify_choice(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void) func;
    if (pvalue == NULL) return false;
    clags_choice_t  **pchoice = (clags_choice_t**) pvalue;
    clags_choices_t  *choices = (clags_choices_t*) func;
    for (size_t i=0; i<choices->count; ++i){
        clags_choice_t *choice = choices->items + i;
        if (strcmp(choice->value, arg) == 0){
            if (pchoice) *pchoice = choice;
            return true;
        }
    }
    fprintf(stderr, "[ERROR] Invalid choice for argument '%s': '%s'!\n", arg_name, arg);
    return false;
}

bool clags__verify_path(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void) func;
    struct stat attr;
    if (stat(arg, &attr) == -1){
        fprintf(stderr, "[ERROR] Invalid path for argument '%s': '%s' : %s!\n", arg_name, arg, strerror(errno));
        return false;
    }
    if (pvalue) *(char**)pvalue = (char*) arg;
    return true;
}

bool clags__verify_file(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void) func;
    struct stat attr;
    if (stat(arg, &attr) == -1){
        fprintf(stderr, "[ERROR] Invalid path for argument '%s': '%s' : %s!\n", arg_name, arg, strerror(errno));
        return false;
    }
    if (!S_ISREG(attr.st_mode)){
        fprintf(stderr, "[ERROR] Path for arguments '%s' is not a file: '%s'!\n", arg_name, arg);
        return false;
    }
    if (pvalue) *(char**)pvalue = (char*) arg;
    return true;
}

bool clags__verify_dir(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void) func;
    struct stat attr;
    if (stat(arg, &attr) == -1){
        fprintf(stderr, "[ERROR] Invalid path for argument '%s': '%s' : %s!\n", arg_name, arg, strerror(errno));
        return false;
    }
    if (!S_ISDIR(attr.st_mode)){
        fprintf(stderr, "[ERROR] Path for arguments '%s' is not a dir: '%s'!\n", arg_name, arg);
        return false;
    }
    if (pvalue) *(char**)pvalue = (char*) arg;
    return true;
}

bool clags__verify_size(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void) func;
    char *endptr;
    errno = 0;
    unsigned long long value = strtoull(arg, &endptr, 10);

    if (endptr == arg){
        fprintf(stderr, "[ERROR] No leading number in size argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    clags_fsize_t factor;
    if (*endptr == '\0' || strcmp(endptr, "B") == 0) factor = 1;
    else if (strcmp(endptr, "KiB") == 0)             factor = 1ULL << 10;
    else if (strcmp(endptr, "KB")  == 0)             factor = 1000;
    else if (strcmp(endptr, "MiB") == 0)             factor = 1ULL << 20;
    else if (strcmp(endptr, "MB")  == 0)             factor = 1000000;
    else if (strcmp(endptr, "GiB") == 0)             factor = 1ULL << 30;
    else if (strcmp(endptr, "GB")  == 0)             factor = 1000000000;
    else if (strcmp(endptr, "TiB") == 0)             factor = 1ULL << 40;
    else if (strcmp(endptr, "TB")  == 0)             factor = 1000000000000;
    else {
        fprintf(stderr, "[ERROR] Invalid size unit for argument '%s': '%s'!\n", arg_name, endptr);
        return false;
    }
    
    if (errno == ERANGE || value > UINT64_MAX/factor || *arg == '-') {
        fprintf(stderr, "[ERROR] clags_fsize_t value out of range (0 to %lu) for argument '%s': '%s'!\n", UINT64_MAX, arg_name, arg);
        return false;
    }
    if (pvalue) *(clags_fsize_t*)pvalue = (clags_fsize_t)value * factor;
    return true;
}

bool clags__verify_time_s(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void) func;
    char *endptr;
    errno = 0;
    double value = strtod(arg, &endptr);
    if (endptr == arg){
        fprintf(stderr, "[ERROR] No leading number in time argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    clags_time_t factor;
    if (*endptr == '\0' || strcmp(endptr, "s") == 0)  factor =       1;
    else if (strcmp(endptr, "m")  == 0)               factor =      60;
    else if (strcmp(endptr, "h")  == 0)               factor =    3600;
    else if (strcmp(endptr, "d")  == 0)               factor = 24*3600;
    else {
        fprintf(stderr, "[ERROR] Invalid time unit for argument '%s': '%s'!\n", arg_name, endptr);
        return false;
    }
    if (errno == ERANGE || value > UINT64_MAX/factor || value < 0){
        fprintf(stderr, "[ERROR] clags_time_t value out of range (0s to %"PRIu64"s) for argument '%s': '%s'!\n", UINT64_MAX, arg_name, arg);
        return false;
    }
    if (pvalue) *(clags_time_t*)pvalue = (clags_time_t)(value * factor);
    return true;
}

bool clags__verify_time_ns(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    (void) func;
    char *endptr;
    errno = 0;
    double value = strtod(arg, &endptr);
    if (endptr == arg){
        fprintf(stderr, "[ERROR] No leading number in time argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    clags_time_t factor;
    if (*endptr == '\0' || strcmp(endptr, "ns") == 0) factor =           1;
    else if (strcmp(endptr, "us") == 0)               factor =         1e3;
    else if (strcmp(endptr, "ms") == 0)               factor =         1e6;
    else if (strcmp(endptr, "s")  == 0)               factor =         1e9;
    else if (strcmp(endptr, "m")  == 0)               factor =      60*1e9;
    else if (strcmp(endptr, "h")  == 0)               factor =    3600*1e9;
    else if (strcmp(endptr, "d")  == 0)               factor = 24*3600*1e9;
    else {
        fprintf(stderr, "[ERROR] Invalid time unit for argument '%s': '%s'!\n", arg_name, endptr);
        return false;
    }
    if (errno == ERANGE || value > UINT64_MAX/factor || value < 0){
        fprintf(stderr, "[ERROR] clags_time_t value out of range (0ns to %"PRIu64"ns) for argument '%s': '%s'!\n", UINT64_MAX, arg_name, arg);
        return false;
    }
    if (pvalue) *(clags_time_t*)pvalue = (clags_time_t)(value * factor);
    return true;
}

bool clags__verify_custom(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    clags_custom_verify_func_t value_func = (clags_custom_verify_func_t) func;
    if (!value_func(arg_name, (char*)arg, pvalue)) {
        fprintf(stderr, "[ERROR] Value for argument '%s' does not match custom criteria: '%s'!\n", arg_name, arg);
        return false;
    }
    return true;
}

bool clags__append_to_list(clags_required_t req, const char *arg)
{
    clags_list_t *list = (clags_list_t*) req.variable;
    size_t item_size = list->item_size;
    if (list->count >= list->capacity){
        size_t new_capacity = list->capacity==0? 8:list->capacity*2;
        list->items = CLAGS_REALLOC(list->items, new_capacity*item_size);
        list->capacity = new_capacity;
        assert(list->items && "Buy more RAM lol");
    }
    char *ptr = (char*) list->items;
    if (clags__verify_funcs[req.value_type](req.arg_name, arg, ptr+item_size*list->count, req.verify)){
        list->count++;
        return true;
    }
    return false;
}

bool clags__validate_config(clags_config_t *config)
{
    if (config->options.list_terminator && strcmp(config->options.list_terminator, "--") == 0){
        fprintf(stderr, "[CONFIG_ERROR] '.list_terminator' may not be '--' because '--' is reserved for toggling option and flag parsing!\n");
        return false;
    }
    if (config->options.ignore_prefix && strcmp(config->options.ignore_prefix, "--") == 0){
        fprintf(stderr, "[CONFIG_ERROR] '.ignore_prefix' may not be '--' since this conflicts with the long option and flag prefix!\n");
        return false;
    }
    bool last_was_list = false;
    const char *last_req_name = NULL;
    for (size_t i=0; i<config->args_count; ++i){
        switch (config->args[i].type){
            case Clags_Required:{
                clags_required_t req = config->args[i].req;
                if (last_was_list && config->options.list_terminator == NULL){
                    fprintf(stderr, "[CONFIG_ERROR] required argument '%s' is unreachable after list '%s'! Define '.list_terminator' in 'clags_config' to separate them", req.arg_name, last_req_name);
                    if (!req.is_list) {
                        fprintf(stderr, " or make '%s' optional", req.arg_name);
                    }
                    printf(".\n");
                    return false;
                }
                last_was_list = req.is_list;
                last_req_name = req.arg_name;
            } break;
            case Clags_Optional:{
                last_was_list = false;
                clags_optional_t opt = config->args[i].opt;
                if (opt.short_flag == '\0' && opt.long_flag == NULL){
                    fprintf(stderr, "[CONFIG_WARNING] optional argument is unreachable. Define at least one of `short_flag` and `long_flag`.\n");
                }
                if (opt.long_flag && strncmp(opt.long_flag, "--", 2) == 0){
                    fprintf(stderr,
                            "[CONFIG_WARNING] optional long flag '%s' should not start with '--'. "
                            "The parser automatically handles leading '--' for long flags, "
                            "so including it in the config may cause incorrect parsing.\n",
                            opt.long_flag);

                }
            } break;
            case Clags_Flag:{
                last_was_list = false;
                clags_flag_t flag = config->args[i].flag;
                if (flag.short_flag == '\0' && flag.long_flag == NULL){
                    fprintf(stderr, "[CONFIG_WARNING] flag argument is unreachable. Define at least one of `short_flag` and `long_flag`.\n");
                }
                if (flag.long_flag && strncmp(flag.long_flag, "--", 2) == 0){
                    fprintf(stderr,
                            "[CONFIG_WARNING] long flag '%s' should not start with '--'. "
                            "The parser automatically handles leading '--' for long flags, "
                            "so including it in the config may cause incorrect parsing.\n",
                            flag.long_flag);

                }
            } break;
        }
    }
    return true;
}

void clags__sort_args(clags_args_t *args, clags_config_t *config)
{
    for (size_t i=0; i<config->args_count; ++i){
        switch(config->args[i].type){
            case Clags_Required:{
                args->required[args->required_count++] = config->args[i].req;
            } break;
            case Clags_Optional:{
                args->optional[args->optional_count++] = config->args[i].opt;
            } break;
            case Clags_Flag:{
                args->flags[args->flag_count++] = config->args[i].flag;
            } break;
            default: {
                assert(0 && "Unreachable");
            }
        }
    }
}

void clags__choice_usage(clags_choices_t *choices, bool is_list)
{
    if (!choices->print_no_details || choices->count >= 6){
        printf(" (%s%s)\n        Choices:\n", clags__type_names[Clags_Choice], is_list?"[]":"");
        for (size_t j=0; j<choices->count; ++j){
            clags_choice_t choice = choices->items[j];
            printf("          - %*s : %s\n", CLAGS_USAGE_ALIGNMENT+8, choice.value, choice.description);
        }
    } else{
        printf(" (%s%s:", clags__type_names[Clags_Choice], is_list?"[]":"");
        for (size_t j=0; j<choices->count; ++j){
            printf("%s%s", j>0?" | ":" ", choices->items[j].value);
        }
        printf(")");
    }
}

void clags__type_usage(clags_value_type_t type, void *func, bool is_list)
{
    if (type == Clags_Choice){
        clags__choice_usage((clags_choices_t *)func, is_list);
    }else if (type == Clags_None){
        if (is_list) printf(" ([])");
    } else{
        printf(" (%s%s)", clags__type_names[type], is_list?"[]":"");
    }
    printf("\n");
}

bool clags_parse(int argc, char **argv, clags_config_t *config)
{
    if (config->args == NULL) return true;
    // validate the configuration, exit and mark config as invalid on fatal error
    if (!clags__validate_config(config)){
        config->invalid = true;
        return false;
    }
    // sort arguments by type
    clags_required_t required[config->args_count];
    clags_optional_t optional[config->args_count];
    clags_flag_t flags[config->args_count];
    clags_args_t args = {.required=required, .optional=optional, .flags=flags};
    clags__sort_args(&args, config);

    const char *ignore_prefix = config->options.ignore_prefix;
    size_t ignore_prefix_len = ignore_prefix?strlen(ignore_prefix):0;
    const char *list_term = config->options.list_terminator;

    // parse arguments
    bool arguments_ignored = false;
    bool in_list = false;
    bool accept_options = true;
    size_t required_count = 0;
    for (size_t index=1; index<(size_t) argc; ++index){
        char *arg = argv[index];

        // toggle option and flag parsing based on '--'
        if (strcmp(arg, "--") == 0){
            if (accept_options || config->options.allow_option_parsing_toggle){
                accept_options = !accept_options;
                continue;
            }
        }
        
        // ignore arguments prefixed with `ignore_prefix`
        if (ignore_prefix && strncmp(arg, ignore_prefix, ignore_prefix_len) == 0){
            arguments_ignored = true;
            continue;
        }

        // detect list terminator
        if (list_term && strcmp(arg, list_term) == 0){
            if (in_list){
                in_list = false;
                required_count += 1;
            }
            continue;
        }
        if (accept_options && strncmp(arg, "--", 2) == 0){
            // parse long flag or option
            arg += 2;
            if (*arg == '\0'){
                fprintf(stderr, "[ERROR] Missing flag or option name: '--%s'!\n", arg);
                return false;
            }

            // parse long option
            for (size_t i=0; i<args.optional_count; ++i){
                clags_optional_t opt = args.optional[i];
                if (opt.long_flag == NULL) continue;
                size_t long_flag_len = strlen(opt.long_flag);
                if (strncmp(arg, opt.long_flag, long_flag_len) == 0){
                    char *value = arg + long_flag_len;
                    if (*value == '\0'){
                        // get value from the next not-ignored argument
                        while (true){
                            if (argc-index <= 1){
                                fprintf(stderr, "[ERROR] Optional flag %s requires argument!\n", arg);
                                return false;
                            }
                            value = argv[++index];
                            if (!ignore_prefix || strncmp(value, ignore_prefix, ignore_prefix_len) != 0) break;
                            arguments_ignored = true;
                        }
                    } else if (*value++ == '='){
                        if (*value == '\0'){
                            fprintf(stderr, "[ERROR] Designated option assignment may not have an empty value: '%s'!\n", arg);
                            return false;
                        }
                    } else {
                        continue;
                    }
                    if (!clags__verify_funcs[opt.value_type](arg, value, opt.variable, opt.verify)) return false;
                    goto next;
                }
            }
            // parse long flags
            for (size_t i=0; i<args.flag_count; ++i){
                clags_flag_t flag = args.flags[i];
                if (flag.long_flag && strcmp(arg, flag.long_flag) == 0){
                    if (flag.variable != NULL) *flag.variable = true;
                    if (flag.exit) return true;
                    goto next;
                }
            }
            fprintf(stderr, "[ERROR] Unknown long flag or option: '--%s'!\n", arg);
            return false;
        } else if (accept_options && *arg == '-' && !isdigit((unsigned char)arg[1])){
            // parse short flag or option
            arg += 1;
            size_t flag_len = strlen(arg);
            if (flag_len == 0){
                fprintf(stderr, "[ERROR] Missing flag or option name: '-'!\n");
                return false;                
            }
            for (char* c=arg; c<arg+flag_len; ++c){
                // check for short option
                for (size_t i=0; i<args.optional_count; ++i){
                    clags_optional_t opt = args.optional[i];
                    if (*c == opt.short_flag){
                        char *value = c+1;
                        if (*value == '\0'){
                            while (true){
                                if (argc-index <= 1){
                                    fprintf(stderr, "[ERROR] Optional flag %s requires argument!\n", arg);
                                    return false;
                                }
                                value = argv[++index];
                                if (!ignore_prefix || strncmp(value, ignore_prefix, ignore_prefix_len) != 0) break;
                                arguments_ignored = true;
                            }
                        }
                        if (!clags__verify_funcs[opt.value_type](arg, value, opt.variable, opt.verify)) return false;
                        goto next;
                    }
                }
                bool matched = false;
                for (size_t i=0; i<args.flag_count; ++i){
                    clags_flag_t flag = args.flags[i];
                    if (*c == flag.short_flag){
                        if (flag.variable) *flag.variable = true;
                        if (flag.exit) return true;
                        matched = true;
                    }
                }
                if (!matched){
                    if (flag_len > 1){
                        fprintf(stderr, "[ERROR] Unknown short flag '-%c' in combination '-%s'!\n", *c, arg);
                    } else{
                        fprintf(stderr, "[ERROR] Unknown short flag '-%c'!\n", *c);
                    }
                    return false;
                }
            }
        } else {
            // parse required argument
            if (required_count >= args.required_count){
                fprintf(stderr, "[ERROR] Unknown additional argument (%zu/%zu): '%s'!\n", required_count+1, args.required_count, arg);
                return false;
            }

            // verify and write argument
            clags_required_t req = args.required[required_count];
            if (req.is_list){
                in_list = true;
                if (!clags__append_to_list(req, arg)) return false;
            } else{
                required_count += 1;
                if (!clags__verify_funcs[req.value_type](req.arg_name, arg, req.variable, req.verify)) return false;
            }
        }
    next:
    }
    if (in_list) required_count += 1;
    if (arguments_ignored) printf("[WARNING] Arguments were ignored because they were prefixed with '%s'\n", ignore_prefix);

    // report missing required arguments
    if (required_count != args.required_count){
        fprintf(stderr, "[ERROR] Missing required arguments:");
        for (size_t i=required_count; i<args.required_count; ++i){
            fprintf(stderr, " <%s>", args.required[i].arg_name);
        }
        fprintf(stderr, "!\n");
        return false;
    }
    return true;
}

void clags_usage(const char *program_name, clags_config_t *config)
{
    if (config->args == NULL || config->invalid) return;
    clags_required_t required[config->args_count];
    clags_optional_t optional[config->args_count];
    clags_flag_t flags[config->args_count];

    clags_args_t args = {.required=required, .optional=optional, .flags=flags};

    clags__sort_args(&args, config);

    clags_options_t options = config->options;
        
    printf("Usage: %s", program_name);
    if (args.optional_count) printf(" [OPTIONS]");
    if (args.flag_count) printf(" [FLAGS]");
    bool last_was_list = false;
    for (size_t i=0; i<args.required_count; ++i){
        if (last_was_list) {
            printf(" %s", options.list_terminator);
            last_was_list = false;
        }
        if (args.required[i].is_list){
            printf(" <%s..>", args.required[i].arg_name);
            last_was_list = true;
        } else{
            printf(" <%s>", args.required[i].arg_name);
        }
    }
    printf("\n");

    if (args.required_count){
        printf("  Arguments:\n");
        for (size_t i=0; i<args.required_count; ++i){
            clags_required_t req = args.required[i];
            printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, req.arg_name, req.description);
            clags__type_usage(req.value_type, req.verify, req.is_list);
        }
    }
    if (args.optional_count){
        printf("  Options:\n");
        for (size_t i=0; i<args.optional_count; ++i){
            clags_optional_t opt = args.optional[i];
            if (opt.short_flag){
                if (opt.long_flag){
                    size_t buf_size = strlen(opt.long_flag) + (opt.arg_name? strlen(opt.arg_name):0) + 10;
                    char buf[buf_size];
                    snprintf(buf, buf_size, "-%c, --%s(=)%s>", opt.short_flag, opt.long_flag, opt.arg_name);
                    printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, buf, opt.description);
                } else{
                    printf("    -%*c : %s", CLAGS_USAGE_ALIGNMENT, opt.short_flag, opt.description);
                }
                clags__type_usage(opt.value_type, opt.verify, false);
            }else if (opt.long_flag){
                size_t buf_size = strlen(opt.long_flag) + (opt.arg_name? strlen(opt.arg_name):0) + 6;
                char buf[buf_size];
                snprintf(buf, buf_size, "--%s(=)%s", opt.long_flag, opt.arg_name);
                printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, buf, opt.description);
                clags__type_usage(opt.value_type, opt.verify, false);
            }
        }
    }
    if (args.flag_count){
        printf("  Flags:\n");
        for (size_t i=0; i<args.flag_count; ++i){
            clags_flag_t flag = args.flags[i];
            if (flag.short_flag){
                if (flag.long_flag){
                    size_t buf_size = + strlen(flag.long_flag) + 16;
                    char buf[buf_size];
                    snprintf(buf, buf_size, "-%c, --%s", flag.short_flag, flag.long_flag);
                    printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, buf, flag.description);
                } else{
                    printf("    -%*c : %s", CLAGS_USAGE_ALIGNMENT, flag.short_flag, flag.description);
                }
            } else if (flag.long_flag){
                printf("    --%*s : %s", CLAGS_USAGE_ALIGNMENT, flag.long_flag, flag.description);
            } else{
                continue;
            }
            printf("%s\n", flag.exit?" and exit":"");
        }
    }
    if (!options.print_no_notes && (options.list_terminator || options.ignore_prefix || options.allow_option_parsing_toggle)){
        printf("\n  Notes:\n");
        if (options.allow_option_parsing_toggle){
            printf("    '--' toggles option and flag parsing and can re-enable parsing when provided again.\n");
        }
        if (options.list_terminator){
            printf("    '%s' terminates a list argument when followed by another argument.\n", options.list_terminator);
        }
        if (options.ignore_prefix){
            printf("    Arguments prefixed with '%s' are ignored.\n", options.ignore_prefix);
        }

    }
}

void clags_list_free(clags_list_t *list)
{
    CLAGS_FREE(list->items);
    list->items = NULL;
    list->count = list->capacity = 0;
}

#endif // CLAGS_IMPLEMENTATION
