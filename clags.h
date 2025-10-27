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

typedef bool (*clags_value_func_t)(const char *arg_name, const char *arg, void *pvalue);
typedef bool (*clags_value_verify_t) (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);

bool clags__verify_none   (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);
bool clags__verify_custom (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);
bool clags__verify_bool   (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);
bool clags__verify_int8   (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);
bool clags__verify_uint8  (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);
bool clags__verify_int32  (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);
bool clags__verify_uint32 (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);
bool clags__verify_double (const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func);

#define clags__types\
    X(Clags_None,   clags__verify_none,    NULL)    \
    X(Clags_Custom, clags__verify_custom, "custom") \
    X(Clags_Bool,   clags__verify_bool,   "bool")   \
    X(Clags_Int8,   clags__verify_int8,   "int8")   \
    X(Clags_UInt8,  clags__verify_uint8,  "uint8")  \
    X(Clags_Int32,  clags__verify_int32,  "int32")  \
    X(Clags_UInt32, clags__verify_uint32, "uint32") \
    X(Clags_Double, clags__verify_double, "double") \


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
    const char *name;
    clags_value_type_t value_type;
    void *value;
    clags_value_func_t value_func;
    const char *description;
    bool is_list;
} clags_req_t;

typedef struct{
    const char *short_flag;
    const char *long_flag;
    clags_value_type_t value_type;
    void *value;
    clags_value_func_t value_func;
    const char *field_name;
    const char *description;
} clags_opt_t;

typedef struct{
    const char *short_flag;
    const char *long_flag;
    bool *value;
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

#define clags_required(val, n, desc)               (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_None,.value_func=NULL,.is_list=false}}
#define clags_required_custom(val, n, desc, vfunc) (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Custom,.value_func=(vfunc),.is_list=false}}
#define clags_required_bool(val, n, desc)          (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Bool,.value_func=NULL,.is_list=false}}
#define clags_required_int8(val, n, desc)          (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Int8,.value_func=NULL,.is_list=false}}
#define clags_required_uint8(val, n, desc)         (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_UInt8,.value_func=NULL,.is_list=false}}
#define clags_required_int32(val, n, desc)         (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Int32,.value_func=NULL,.is_list=false}}
#define clags_required_uint32(val, n, desc)        (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_UInt32,.value_func=NULL,.is_list=false}}
#define clags_required_double(val, n, desc)        (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Double,.value_func=NULL,.is_list=false}}

#define clags_required_list(val, n, desc)               (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_None,.value_func=NULL,.is_list=true}}
#define clags_required_custom_list(val, n, desc, vfunc) (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Custom,.value_func=(vfunc),.is_list=true}}
#define clags_required_bool_list(val, n, desc)          (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Bool,.value_func=NULL,.is_list=true}}
#define clags_required_int8_list(val, n, desc)          (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Int8,.value_func=NULL,.is_list=true}}
#define clags_required_uint8_list(val, n, desc)         (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_UInt8,.value_func=NULL,.is_list=true}}
#define clags_required_int32_list(val, n, desc)         (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Int32,.value_func=NULL,.is_list=true}}
#define clags_required_uint32_list(val, n, desc)        (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_UInt32,.value_func=NULL,.is_list=true}}
#define clags_required_double_list(val, n, desc)        (clags_arg_t){.type=Clags_Required,.req=(clags_req_t){.name=(n),.value=(val),.description=(desc),.value_type=Clags_Double,.value_func=NULL,.is_list=true}}

#define clags_optional(sf, lf, val, f_name, desc)               (clags_arg_t){.type=Clags_Optional,.opt=(clags_opt_t){.short_flag=(sf),.long_flag=(lf),.value=(val),.description=(desc),.field_name=(f_name),.value_type=Clags_None,.value_func=NULL}}
#define clags_optional_custom(sf, lf, val, f_name, desc, vfunc) (clags_arg_t){.type=Clags_Optional,.opt=(clags_opt_t){.short_flag=(sf),.long_flag=(lf),.value=(val),.description=(desc),.field_name=(f_name),.value_type=Clags_Custom,.value_func=(vfunc)}}
#define clags_optional_bool(sf, lf, val, f_name, desc)          (clags_arg_t){.type=Clags_Optional,.opt=(clags_opt_t){.short_flag=(sf),.long_flag=(lf),.value=(val),.description=(desc),.field_name=(f_name),.value_type=Clags_Bool,.value_func=NULL}}
#define clags_optional_int8(sf, lf, val, f_name, desc)          (clags_arg_t){.type=Clags_Optional,.opt=(clags_opt_t){.short_flag=(sf),.long_flag=(lf),.value=(val),.description=(desc),.field_name=(f_name),.value_type=Clags_Int8,.value_func=NULL}}
#define clags_optional_uint8(sf, lf, val, f_name, desc)         (clags_arg_t){.type=Clags_Optional,.opt=(clags_opt_t){.short_flag=(sf),.long_flag=(lf),.value=(val),.description=(desc),.field_name=(f_name),.value_type=Clags_UInt8,.value_func=NULL}}
#define clags_optional_int32(sf, lf, val, f_name, desc)         (clags_arg_t){.type=Clags_Optional,.opt=(clags_opt_t){.short_flag=(sf),.long_flag=(lf),.value=(val),.description=(desc),.field_name=(f_name),.value_type=Clags_Int32,.value_func=NULL}}
#define clags_optional_uint32(sf, lf, val, f_name, desc)        (clags_arg_t){.type=Clags_Optional,.opt=(clags_opt_t){.short_flag=(sf),.long_flag=(lf),.value=(val),.description=(desc),.field_name=(f_name),.value_type=Clags_UInt32,.value_func=NULL}}
#define clags_optional_double(sf, lf, val, f_name, desc)        (clags_arg_t){.type=Clags_Optional,.opt=(clags_opt_t){.short_flag=(sf),.long_flag=(lf),.value=(val),.description=(desc),.field_name=(f_name),.value_type=Clags_Double,.value_func=NULL}}

#define clags_flag(sf, lf, val, desc, ex) (clags_arg_t) {.type=Clags_Flag, .flag=(clags_flag_t){.short_flag=(sf), .long_flag=(lf), .value=(val), .description=(desc), .exit=(ex)}}
#define clags_flag_help(val) clags_flag("-h", "--help", val, "print this help dialog", true)

#define clags_list              (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(char*)}
#define clags_custom_list(size) (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=(size)}
#define clags_bool_list         (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(bool)}
#define clags_int8_list         (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(int8_t)}
#define clags_uint8_list        (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(uint8_t)}
#define clags_int32_list        (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(int32_t)}
#define clags_uint32_list       (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(uint32_t)}
#define clags_double_list       (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(double)}

#define clags_arr_len(arr) (sizeof(arr)/sizeof(arr[0]))

#define clags_parse(argc, argv, args) clags__parse((argc), (argv), (args), clags_arr_len(args))
bool clags__parse(int argc, char **argv, clags_arg_t *args, size_t arg_count);

#define clags_usage(pn, args) clags__usage((pn), (args), clags_arr_len(args))
void clags__usage(const char *program_name, clags_arg_t *args, size_t arg_count);

#endif // CLAGS_H

#ifdef CLAGS_IMPLEMENTATION

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

bool clags__verify_none(const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func) {
    (void)func;
    (void) arg_name;
    if (pvalue) *(char**)pvalue = (char*)arg;
    return true;
}

bool clags__verify_bool(const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func) {
    (void)func;
    if (strcmp(arg, "true") == 0) {
        if (pvalue) *(bool*)pvalue = true;
        return true;
    } else if (strcmp(arg, "false") == 0) {
        if (pvalue) *(bool*)pvalue = false;
        return true;
    }
    fprintf(stderr, "[ERROR] Invalid boolean value for argument '%s': '%s'!\n", arg_name, arg);
    return false;
}


bool clags__verify_int8(const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func) {
    (void)func;
    char *endptr;
    errno = 0;
    long value = strtol(arg, &endptr, 10);

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

bool clags__verify_uint8(const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func) {
    (void)func;
    char *endptr;
    errno = 0;
    unsigned long value = strtoul(arg, &endptr, 10);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid uint8 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > UINT8_MAX) {
        fprintf(stderr, "[ERROR] uint8 value out of range (0 to %u) for argument '%s': '%s'!\n", UINT8_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(uint8_t*)pvalue = (uint8_t)value;
    return true;
}

bool clags__verify_int32(const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func) {
    (void)func;
    char *endptr;
    errno = 0;
    long value = strtol(arg, &endptr, 10);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid int32 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value < INT32_MIN || value > INT32_MAX) {
        return false;
        fprintf(stderr, "[ERROR] int32 value out of range (%d to %d) for argument '%s': '%s'!\n", INT32_MIN, INT32_MAX, arg_name, arg);
    }

    if (pvalue) *(int32_t*)pvalue = (int32_t)value;
    return true;
}

bool clags__verify_uint32(const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func) {
    (void)func;
    char *endptr;
    errno = 0;
    unsigned long value = strtoul(arg, &endptr, 10);

    if (*endptr != '\0') {
        fprintf(stderr, "[ERROR] Invalid uint32 value for argument '%s': '%s'!\n", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > UINT32_MAX) {
        fprintf(stderr, "[ERROR] uint32 value out of range (0 to %u) for argument '%s': '%s'!\n", UINT32_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(uint32_t*)pvalue = (uint32_t)value;
    return true;
}

bool clags__verify_double(const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func) {
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

bool clags__verify_custom(const char *arg_name, const char *arg, void *pvalue, clags_value_func_t func) {
    if (!func(arg_name, (char*)arg, pvalue)) {
        fprintf(stderr, "[ERROR] Value for argument '%s' does not match custom criteria: '%s'!\n", arg_name, arg);
        return false;
    }
    return true;
}

bool clags__append_to_list(clags_req_t req, const char *arg)
{
    // TODO: - reserve one element of size type
    //       - depending on value_type, write value using `_verify` functions
    clags_list_t *list = (clags_list_t*) req.value;
    size_t item_size = list->item_size;
    if (list->count >= list->capacity){
        size_t new_capacity = list->capacity==0? 8:list->capacity*2;
        list->items = realloc(list->items, new_capacity*item_size);
        list->capacity = new_capacity;
        assert(list->items && "Buy more RAM lol");
    }
    return clags__verify_funcs[req.value_type](req.name, arg, list->items+item_size*list->count++, req.value_func);
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
    clags_req_t required[arg_count];
    clags_opt_t optional[arg_count];
    clags_flag_t flags[arg_count];
    bool in_list = false;

    clags_args_t args = {.required=required, .optional=optional, .flags=flags};
    
    clags__sort_args(&args, _args, arg_count);
    
    size_t required_found = 0;
    for (size_t index=1; index<(size_t)argc; ++index){
        char *arg = argv[index];
        
        for (size_t i=0; i<args.optional_count; ++i){
            clags_opt_t opt = args.optional[i];
            if ((opt.short_flag != NULL && strcmp(arg, opt.short_flag) == 0) || (opt.long_flag != NULL && strcmp(arg, opt.long_flag) == 0)){
                if (argc-index <= 1){
                    fprintf(stderr, "[ERROR] Optional flag %s requires argument!\n", arg);
                    return false;
                }
                char *result = argv[++index];
                if (!clags__verify_funcs[opt.value_type](arg, result, opt.value, opt.value_func)) return false;
                goto next_arg;
            } else if (opt.long_flag != NULL && strncmp(arg, opt.long_flag, strlen(opt.long_flag)) == 0){
                char *value = arg+strlen(opt.long_flag);
                if (*value++ == '='){
                    if (*value == '\0'){
                        fprintf(stderr, "[ERROR] Designated option assignment may not have an empty value: '%s'!\n", arg);
                        return false;
                    }
                    if (!clags__verify_funcs[opt.value_type](opt.long_flag, value, opt.value, opt.value_func)) return false;
                    goto next_arg;
                }
            }
        }
        
        for (size_t i=0; i<args.flag_count; ++i){
            clags_flag_t flag = args.flags[i];
            if ((flag.short_flag != NULL && strcmp(arg, flag.short_flag) == 0) || (flag.long_flag != NULL && strcmp(arg, flag.long_flag) == 0)){
                if (flag.value != NULL) *flag.value = true;
                if (flag.exit) return true;
                goto next_arg;
            }
        }

        if (*arg == '-'){
            fprintf(stderr, "[ERROR] Unknown option: '%s'!\n", arg);
            return false;
        }

        if (required_found >= args.required_count){
            fprintf(stderr, "[ERROR] Unknown additional argument (%zu/%zu): '%s'!\n", required_found+1, args.required_count, arg);
            return false;
        }
        // TODO: support lists
        clags_req_t current_req = args.required[required_found];
        if (current_req.is_list){
            in_list = true;
            if (!clags__append_to_list(current_req, arg)) return false;
            continue;
        } else{
            clags_req_t req = args.required[required_found++];
            if (!clags__verify_funcs[req.value_type](req.name, arg, req.value, req.value_func)) return false;
        }
    next_arg:
        if (in_list){
            required_found++;
            in_list = false;
        }
    }
    if (required_found != args.required_count && !args.required[required_found].is_list){
        fprintf(stderr, "[ERROR] Missing required arguments:");
        for (size_t i=required_found; i<args.required_count; ++i){
            fprintf(stderr, " <%s>", args.required[i].name);
        }
        fprintf(stderr, "!\n");
        return false;
    }
    return true;
}

void clags__usage(const char *program_name, clags_arg_t *_args, size_t arg_count)
{
    clags_req_t required[arg_count];
    clags_opt_t optional[arg_count];
    clags_flag_t flags[arg_count];

    clags_args_t args = {.required=required, .optional=optional, .flags=flags};
    
    clags__sort_args(&args, _args, arg_count);

    printf("Usage: %s", program_name);
    if (args.optional_count) printf(" [OPTIONS]");
    if (args.flag_count) printf(" [FLAGS]");
    for (size_t i=0; i<args.required_count; ++i){
        printf(" <%s%s>", args.required[i].name, args.required[i].is_list?"..":"");
    }
    printf("\n");
    
    if (args.required_count){
        printf("  Arguments:\n");
        for (size_t i=0; i<args.required_count; ++i){
            clags_req_t req = args.required[i];
            printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, req.name, req.description);
            if (req.value_type != Clags_None) printf(" (%s%s)", clags__type_names[req.value_type], req.is_list?"[]":"");
            printf("\n");
        }
    }
    if (args.optional_count){
        printf("  Options:\n");
        for (size_t i=0; i<args.optional_count; ++i){
            clags_opt_t opt = args.optional[i];
            if (opt.short_flag){
                if (opt.long_flag){
                    size_t buf_size = strlen(opt.short_flag) + strlen(opt.long_flag) + (opt.field_name? strlen(opt.field_name):0) + 6;
                    char buf[buf_size];
                    snprintf(buf, buf_size, "%s, %s(=)%s>", opt.short_flag, opt.long_flag, opt.field_name);
                    printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, buf, opt.description);
                } else{
                    printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, opt.short_flag, opt.description);
                }
                if (opt.value_type != Clags_None) printf(" (%s)", clags__type_names[opt.value_type]);
                printf("\n");
            }else if (opt.long_flag){
                size_t buf_size = strlen(opt.long_flag) + (opt.field_name? strlen(opt.field_name):0) + 4;
                char buf[buf_size];
                snprintf(buf, buf_size, "%s(=)%s", opt.long_flag, opt.field_name);
                printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, buf, opt.description);
                if (opt.value_type != Clags_None) printf(" (%s)", clags__type_names[opt.value_type]);
                printf("\n");
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
                    printf("    %*s : %s\n", CLAGS_USAGE_ALIGNMENT, buf, flag.description);
                } else{
                    printf("    %*s : %s\n", CLAGS_USAGE_ALIGNMENT, flag.short_flag, flag.description);
                }
            } else if (flag.long_flag){
                printf("    %*s : %s\n", CLAGS_USAGE_ALIGNMENT, flag.long_flag, flag.description);
            }
        }
    }
}
#endif // CLAGS_IMPLEMENTATION
