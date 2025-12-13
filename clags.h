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

#ifndef CLAGS_IGNORE_PREFIX          // argument prefix to ignore, disabled by default
#define CLAGS_IGNORE_PREFIX NULL
#endif // CLAGS_IGNORE_PREFIX

typedef bool (*clags_value_func_t)(const char *arg_name, const char *arg, void *variable);
typedef bool (*clags_value_verify_t) (const char *arg_name, const char *arg, void *pvalue, void *func);
typedef uint64_t clags_fsize_t;

bool clags__verify_none   (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_custom (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_bool   (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_int8   (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_uint8  (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_int32  (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_uint32 (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_int64  (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_uint64 (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_double (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_choice (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_path   (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_file   (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_dir    (const char *arg_name, const char *arg, void *pvalue, void *func);
bool clags__verify_size   (const char *arg_name, const char *arg, void *pvalue, void *func);

#define clags__types\
    X(Clags_None,   clags__verify_none,    NULL   )\
    X(Clags_Custom, clags__verify_custom, "custom")\
    X(Clags_Bool,   clags__verify_bool,   "bool"  )\
    X(Clags_Int8,   clags__verify_int8,   "int8"  )\
    X(Clags_UInt8,  clags__verify_uint8,  "uint8" )\
    X(Clags_Int32,  clags__verify_int32,  "int32" )\
    X(Clags_UInt32, clags__verify_uint32, "uint32")\
    X(Clags_Int64,  clags__verify_int64,  "int64" )\
    X(Clags_UInt64, clags__verify_uint64, "uint64")\
    X(Clags_Double, clags__verify_double, "double")\
    X(Clags_Choice, clags__verify_choice, "choice")\
    X(Clags_Path,   clags__verify_path,   "path"  )\
    X(Clags_File,   clags__verify_file,   "file"  )\
    X(Clags_Dir,    clags__verify_dir,    "dir"   )\
    X(Clags_Size,   clags__verify_size,   "size"  )\

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
    bool print_details;
} clags_choices_t;

typedef struct{
    void *variable;
    const char *arg_name;
    const char *description;
    clags_value_type_t value_type;
    void *verify;
    bool is_list;
} clags_req_t;

typedef struct{
    const char *short_flag;
    const char *long_flag;
    void *variable;
    const char *arg_name;
    const char *description;
    clags_value_type_t value_type;
    void *verify;
} clags_opt_t;

typedef struct{
    const char *short_flag;
    const char *long_flag;
    bool *variable;
    const char *description;
    bool exit;
} clags_flag_t;

typedef struct{
    clags_req_t *required;
    size_t required_count;
    clags_opt_t *optional;
    size_t optional_count;
    clags_flag_t *flags;
    size_t flag_count;
} clags_args_t;

typedef enum{
    Clags_Required,
    Clags_Optional,
    Clags_Flag
} clags_arg_type_t;

typedef struct{
    clags_arg_type_t type;
    union{
        clags_req_t req;
        clags_opt_t opt;
        clags_flag_t flag;
    };
} clags_arg_t;

#define CLAGS_USAGE_ALIGNMENT -24

#define clags_required(var, name, desc, ...) (clags_arg_t){.type=Clags_Required, .req=(clags_req_t){.variable=(var), .arg_name=(name), .description=(desc), __VA_ARGS__}}

#define clags_optional(sflag, lflag, var, name, desc, ...) (clags_arg_t){.type=Clags_Optional, .opt=(clags_opt_t){.short_flag=(sflag), .long_flag=(lflag), .variable=(var), .arg_name=(name), .description=(desc), __VA_ARGS__}}

#define clags_flag(sflag, lflag, var, desc, ...) (clags_arg_t) {.type=Clags_Flag, .flag=(clags_flag_t){.short_flag=(sflag), .long_flag=(lflag), .variable=(var), .description=(desc), __VA_ARGS__}}
#define clags_flag_help(val) clags_flag("-h", "--help", val, "print this help dialog", .exit=true)

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
#define clags_choice_list()     (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(clags_choice_t*)}

#define clags_list_element(list, value_type, i) ((value_type*)(list).items)[i]
#define clags_list_choice_element(list, i) clags_list_element((list), (clags_choice_t*), (i))->value

#define clags_choice(arr, details) (clags_choices_t){.items=(arr), .count=clags_arr_len(arr), .print_details=(details)}

#define clags_arr_len(arr) ((arr)==NULL?0:(sizeof(arr)/sizeof(arr[0])))

#define clags_parse(argc, argv, args) clags__parse((argc), (argv), (args), clags_arr_len(args))
bool clags__parse(int argc, char **argv, clags_arg_t *args, size_t arg_count);

#define clags_usage(pn, args) clags__usage((pn), (args), clags_arr_len(args))
void clags__usage(const char *program_name, clags_arg_t *args, size_t arg_count);
void clags__choice_usage(clags_choices_t *choices, bool is_list);
void clags__type_usage(clags_value_type_t type, void *func, bool is_list);

void clags_list_free(clags_list_t *list);

#endif // CLAGS_H

#ifdef CLAGS_IMPLEMENTATION

static char* clags__ignore_prefix = CLAGS_IGNORE_PREFIX;

#define X(type, func, name) [type] = func,
static clags_value_verify_t clags__verify_funcs[] = {
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
        fprintf(stderr, "[ERROR] Invalid size suffix for argument '%s': '%s'!\n", arg_name, endptr);
        return false;
    }
    
    if (errno == ERANGE || value > UINT64_MAX/factor || *arg == '-') {
        fprintf(stderr, "[ERROR] clags_fsize_t value out of range (0 to %lu) for argument '%s': '%s'!\n", UINT64_MAX, arg_name, arg);
        return false;
    }
    if (pvalue) *(clags_fsize_t*)pvalue = (clags_fsize_t)value * factor;
    return true;
}

bool clags__verify_custom(const char *arg_name, const char *arg, void *pvalue, void *func)
{
    clags_value_func_t value_func = (clags_value_func_t) func;
    if (!value_func(arg_name, (char*)arg, pvalue)) {
        fprintf(stderr, "[ERROR] Value for argument '%s' does not match custom criteria: '%s'!\n", arg_name, arg);
        return false;
    }
    return true;
}

bool clags__append_to_list(clags_req_t req, const char *arg)
{
    clags_list_t *list = (clags_list_t*) req.variable;
    size_t item_size = list->item_size;
    if (list->count >= list->capacity){
        size_t new_capacity = list->capacity==0? 8:list->capacity*2;
        list->items = realloc(list->items, new_capacity*item_size);
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

void clags__sort_args(clags_args_t *args, clags_arg_t *_args, size_t arg_count)
{
    for (size_t i=0; i<arg_count; ++i){
        switch(_args[i].type){
            case Clags_Required:{
                args->required[args->required_count++] = _args[i].req;
            } break;
            case Clags_Optional:{
                args->optional[args->optional_count++] = _args[i].opt;
            } break;
            case Clags_Flag:{
                args->flags[args->flag_count++] = _args[i].flag;
            } break;
            default: {
                assert(0 && "Unreachable");
            }
        }
    }
}

bool clags__parse(int argc, char **argv, clags_arg_t *_args, size_t arg_count)
{
    if (_args == NULL) return true;
    clags_req_t required[arg_count];
    clags_opt_t optional[arg_count];
    clags_flag_t flags[arg_count];
    bool in_list = false;

    clags_args_t args = {.required=required, .optional=optional, .flags=flags};

    clags__sort_args(&args, _args, arg_count);

    size_t required_found = 0;
    bool ignored = false;
    for (size_t index=1; index<(size_t)argc; ++index){
        char *arg = argv[index];
        if (clags__ignore_prefix && strncmp(clags__ignore_prefix, arg, strlen(clags__ignore_prefix)) == 0){
            ignored = true;
            continue;
        }
        if (strcmp(arg, "--") == 0){
            if (in_list){
                in_list = false;
                required_found++;
            }
            continue;
        }
        for (size_t i=0; i<args.optional_count; ++i){
            clags_opt_t opt = args.optional[i];
            if ((opt.short_flag != NULL && strcmp(arg, opt.short_flag) == 0) || (opt.long_flag != NULL && strcmp(arg, opt.long_flag) == 0)){
                char *result = NULL;
                while (true){
                    if (argc-index <= 1){
                        fprintf(stderr, "[ERROR] Optional flag %s requires argument!\n", arg);
                        return false;
                    }
                    result = argv[++index];
                    if (!clags__ignore_prefix || !strncmp(clags__ignore_prefix, result, strlen(clags__ignore_prefix)) == 0) break;
                    ignored = true;
                }
                if (!clags__verify_funcs[opt.value_type](arg, result, opt.variable, opt.verify)) return false;
                goto next_arg;
            } else if (opt.long_flag != NULL && strncmp(arg, opt.long_flag, strlen(opt.long_flag)) == 0){
                char *value = arg+strlen(opt.long_flag);
                if (*value++ == '='){
                    if (*value == '\0'){
                        fprintf(stderr, "[ERROR] Designated option assignment may not have an empty value: '%s'!\n", arg);
                        return false;
                    }
                    if (!clags__verify_funcs[opt.value_type](opt.long_flag, value, opt.variable, opt.verify)) return false;
                    goto next_arg;
                }
            }
        }

        for (size_t i=0; i<args.flag_count; ++i){
            clags_flag_t flag = args.flags[i];
            if ((flag.short_flag != NULL && strcmp(arg, flag.short_flag) == 0) || (flag.long_flag != NULL && strcmp(arg, flag.long_flag) == 0)){
                if (flag.variable != NULL) *flag.variable = true;
                if (flag.exit) return true;
                goto next_arg;
            }
        }
        if (arg[0] == '-' && !isdigit(arg[1])){
            size_t arg_len = strlen(arg);
            if (arg_len == 1){
                fprintf(stderr, "[ERROR] Missing flag name: '%s'!\n", arg);
                return false;
            }
            if (arg[1] == '-'){
                fprintf(stderr, "[ERROR] Unknown long flag: '%s'!\n", arg);
                return false;
            }
            for (size_t c = 1; c < arg_len; ++c) {
                char short_flag_str[3] = { '-', arg[c], '\0' };
                bool matched = false;
                for (size_t i = 0; i < args.flag_count; ++i) {
                    clags_flag_t flag = args.flags[i];
                    if (flag.short_flag && (strcmp(flag.short_flag, short_flag_str) == 0 || (*flag.short_flag!='-' && strcmp(flag.short_flag, short_flag_str+1) == 0))) {
                        if (flag.variable) *flag.variable = true;
                        if (flag.exit) return true;
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    if (arg_len > 2){
                        fprintf(stderr, "[ERROR] Unknown short flag in combination '%s': '%s'\n", arg, short_flag_str);
                    } else{
                        fprintf(stderr, "[ERROR] Unknown short flag: '%s'!\n", short_flag_str);
                    }
                    return false;
                }
            }
            goto next_arg;
        }

        if (required_found >= args.required_count){
            fprintf(stderr, "[ERROR] Unknown additional argument (%zu/%zu): '%s'!\n", required_found+1, args.required_count, arg);
            return false;
        }
        clags_req_t current_req = args.required[required_found];
        if (current_req.is_list){
            in_list = true;
            if (!clags__append_to_list(current_req, arg)) return false;
            continue;
        } else{
            clags_req_t req = args.required[required_found++];
            if (!clags__verify_funcs[req.value_type](req.arg_name, arg, req.variable, req.verify)) return false;
        }
    next_arg:
        if (in_list){
            required_found++;
            in_list = false;
        }
    }
    if (in_list) required_found++;
    if (ignored) printf("[WARNING] Arguments were ignored because they were prefixed with CLAGS_IGNORE_PREFIX ('%s')\n", clags__ignore_prefix);
    if (required_found != args.required_count){
        fprintf(stderr, "[ERROR] Missing required arguments:");
        for (size_t i=required_found; i<args.required_count; ++i){
            fprintf(stderr, " <%s>", args.required[i].arg_name);
        }
        fprintf(stderr, "!\n");
        return false;
    }
    return true;
}

void clags__usage(const char *program_name, clags_arg_t *_args, size_t arg_count)
{
    if (_args == NULL) return;
    clags_req_t required[arg_count];
    clags_opt_t optional[arg_count];
    clags_flag_t flags[arg_count];

    clags_args_t args = {.required=required, .optional=optional, .flags=flags};

    clags__sort_args(&args, _args, arg_count);

    printf("Usage: %s", program_name);
    if (args.optional_count) printf(" [OPTIONS]");
    if (args.flag_count) printf(" [FLAGS]");
    for (size_t i=0; i<args.required_count; ++i){
        printf(" <%s%s>", args.required[i].arg_name, args.required[i].is_list?"..":"");
    }
    printf("\n");

    if (args.required_count){
        printf("  Arguments:\n");
        for (size_t i=0; i<args.required_count; ++i){
            clags_req_t req = args.required[i];
            printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, req.arg_name, req.description);
            clags__type_usage(req.value_type, req.verify, req.is_list);
        }
    }
    if (args.optional_count){
        printf("  Options:\n");
        for (size_t i=0; i<args.optional_count; ++i){
            clags_opt_t opt = args.optional[i];
            if (opt.short_flag){
                if (opt.long_flag){
                    size_t buf_size = strlen(opt.short_flag) + strlen(opt.long_flag) + (opt.arg_name? strlen(opt.arg_name):0) + 6;
                    char buf[buf_size];
                    snprintf(buf, buf_size, "%s, %s(=)%s>", opt.short_flag, opt.long_flag, opt.arg_name);
                    printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, buf, opt.description);
                } else{
                    printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, opt.short_flag, opt.description);
                }
                clags__type_usage(opt.value_type, opt.verify, false);
            }else if (opt.long_flag){
                size_t buf_size = strlen(opt.long_flag) + (opt.arg_name? strlen(opt.arg_name):0) + 4;
                char buf[buf_size];
                snprintf(buf, buf_size, "%s(=)%s", opt.long_flag, opt.arg_name);
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
                    size_t buf_size = strlen(flag.short_flag) + strlen(flag.long_flag) + 12;
                    char buf[buf_size];
                    snprintf(buf, buf_size, "%s, %s", flag.short_flag, flag.long_flag);
                    printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, buf, flag.description);
                } else{
                    printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, flag.short_flag, flag.description);
                }
            } else if (flag.long_flag){
                printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, flag.long_flag, flag.description);
            } else{
                continue;
            }
            printf("%s\n", flag.exit?" and exit":"");
        }
    }
}

void clags__choice_usage(clags_choices_t *choices, bool is_list)
{
    if (choices->print_details || choices->count >= 6){
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
        printf(" ([])");
    } else{
        printf(" (%s%s)", clags__type_names[type], is_list?"[]":"");
    }
    printf("\n");
}

void clags_list_free(clags_list_t *list)
{
    free(list->items);
    list->items = NULL;
    list->count = list->capacity = 0;
}

#endif // CLAGS_IMPLEMENTATION
