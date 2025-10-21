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
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef struct{
    const char *name;
    char **value;
    const char *description;
} clags_req_t;

typedef struct{
    const char *flag;
    char **value;
    const char *field_name;
    const char *description;
} clags_opt_t;

typedef struct{
    const char *flag;
    bool *value;
    const char *description;
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


#define clags_required(n, val, desc) (clags_arg_t) {.type=Clags_Required, .req=(clags_req_t){.name=(n), .value=(val), .description=(desc)}}
#define clags_optional(f, val, f_name, desc) (clags_arg_t) {.type=Clags_Optional, .opt=(clags_opt_t){.flag=(f), .value=(val), .description=(desc), .field_name=(f_name)}}
#define clags_flag(f, val, desc) (clags_arg_t) {.type=Clags_Flag, .flag=(clags_flag_t){.flag=(f), .value=(val), .description=(desc)}}

#define clags_arr_len(arr) (sizeof(arr)/sizeof(arr[0]))

#define clags_parse(argc, argv, args, help) _clags_parse((argc), (argv), (help), (args), clags_arr_len(args))
bool _clags_parse(int argc, char **argv, bool *help, clags_arg_t *args, size_t arg_count);

#define clags_usage(pn, args) _clags_usage((pn), (args), clags_arr_len(args))
void _clags_usage(const char *program_name, clags_arg_t *args, size_t arg_count);

#endif // CLAGS_H

#ifdef CLAGS_IMPLEMENTATION

void _clags_sort_args(clags_args_t *args, clags_arg_t *_args, size_t arg_count)
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

bool _clags_parse(int argc, char **argv, bool *help, clags_arg_t *_args, size_t arg_count)
{
    clags_req_t required[arg_count];
    clags_opt_t optional[arg_count];
    clags_flag_t flags[arg_count];

    clags_args_t args = {.required=required, .optional=optional, .flags=flags};

    _clags_sort_args(&args, _args, arg_count);
    
    size_t required_found = 0;
    for (size_t index=1; index<(size_t)argc; ++index){
        char *arg = argv[index];
        
        if (help != NULL && (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)){
            *help = true;
            return true;
        }
        for (size_t i=0; i<args.optional_count; ++i){
            clags_opt_t opt = args.optional[i];
            if (strcmp(arg, opt.flag) == 0){
                if (argc-index <= 1){
                    fprintf(stderr, "[ERROR] Optional flag %s requires argument!\n", arg);
                    return false;
                }
                if (opt.value != NULL) *opt.value = argv[++index];
                goto next_arg;
            }
        }
        for (size_t i=0; i<args.flag_count; ++i){
            clags_flag_t bol = args.flags[i];
            if (strcmp(arg, bol.flag) == 0){
                if (bol.value != NULL) *bol.value = true;
                goto next_arg;
            }
        }
        if (required_found >= args.required_count){
            fprintf(stderr, "[ERROR] Unknown additional argument: '%s'!\n", arg);
            return false;
        }
        clags_req_t req = args.required[required_found++];
        if (req.value != NULL) *req.value = arg;
    next_arg:
    }
    if (required_found != args.required_count){
        fprintf(stderr, "[ERROR] Missing required arguments:");
        for (size_t i=required_found; i<args.required_count; ++i){
            fprintf(stderr, " <%s>", args.required[i].name);
        }
        fprintf(stderr, "!\n");
        return false;
    }
    return true;
}

void _clags_usage(const char *program_name, clags_arg_t *_args, size_t arg_count)
{
    clags_req_t required[arg_count];
    clags_opt_t optional[arg_count];
    clags_flag_t flags[arg_count];

    clags_args_t args = {.required=required, .optional=optional, .flags=flags};
    
    _clags_sort_args(&args, _args, arg_count);

    printf("Usage: %s", program_name);
    if (args.optional_count) printf(" [OPTIONS]");
    printf(" [FLAGS]");
    for (size_t i=0; i<args.required_count; ++i){
        printf(" <%s>", args.required[i].name);
    }
    printf("\n");
    if (args.required_count){
        printf("  Arguments:\n");
        for (size_t i=0; i<args.required_count; ++i){
            clags_req_t req = args.required[i];
            printf("    %-16s : %s\n", req.name, req.description);
        }
    }
    if (args.optional_count){
        printf("  Options:\n");
        for (size_t i=0; i<args.optional_count; ++i){
            clags_opt_t opt = args.optional[i];
            size_t buf_size = strlen(opt.flag) + strlen(opt.field_name) + 4;
            char buf[buf_size];
            snprintf(buf, buf_size, "%s <%s>", opt.flag, opt.field_name);
            printf("    %-16s : %s\n", buf, opt.description);
        }
    }
    printf("  Flags:\n");
    for (size_t i=0; i<args.flag_count; ++i){
        clags_flag_t bol = args.flags[i];
        printf("    %-16s : %s\n", bol.flag, bol.description);
    }
    printf("    %-16s : %s\n", "-h / --help", "print this help dialog");
}
#endif // CLAGS_IMPLEMENTATION
