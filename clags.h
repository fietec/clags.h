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
    const char *short_flag;
    const char *long_flag;
    char **value;
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


#define clags_required(val, n, desc) (clags_arg_t) {.type=Clags_Required, .req=(clags_req_t){.name=(n), .value=(val), .description=(desc)}}
#define clags_optional(sf, lf, val, f_name, desc) (clags_arg_t) {.type=Clags_Optional, .opt=(clags_opt_t){.short_flag=(sf), .long_flag=(lf), .value=(val), .description=(desc), .field_name=(f_name)}}
#define clags_flag(sf, lf, val, desc, ex) (clags_arg_t) {.type=Clags_Flag, .flag=(clags_flag_t){.short_flag=(sf), .long_flag=(lf), .value=(val), .description=(desc), .exit=(ex)}}
#define clags_flag_help(val) clags_flag("-h", "--help", val, "print this help dialog", true)

#define clags_arr_len(arr) (sizeof(arr)/sizeof(arr[0]))

#define clags_parse(argc, argv, args) clags__parse((argc), (argv), (args), clags_arr_len(args))
bool clags__parse(int argc, char **argv, clags_arg_t *args, size_t arg_count);

#define clags_usage(pn, args) clags__usage((pn), (args), clags_arr_len(args))
void clags__usage(const char *program_name, clags_arg_t *args, size_t arg_count);

#endif // CLAGS_H

#ifdef CLAGS_IMPLEMENTATION

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
                if (opt.value != NULL) *opt.value = argv[++index];
                goto next_arg;
            } else if (opt.long_flag != NULL && strncmp(arg, opt.long_flag, strlen(opt.long_flag)) == 0){
                char *value = arg+strlen(opt.long_flag);
                if (*value++ == '='){
                    if (*value == '\0'){
                        fprintf(stderr, "[ERROR] Designated option assignment may not have an empty value: '%s'!\n", arg);
                        return false;
                    }
                    if (opt.value != NULL) *opt.value = value;
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

        if (required_found >= args.required_count){
            fprintf(stderr, "[ERROR] Unknown additional argument: '%s'!\n", arg);
            return false;
        }
        // TODO: support lists
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
        printf(" <%s>", args.required[i].name);
    }
    printf("\n");
    
    if (args.required_count){
        printf("  Arguments:\n");
        for (size_t i=0; i<args.required_count; ++i){
            clags_req_t req = args.required[i];
            printf("    %*s : %s\n", CLAGS_USAGE_ALIGNMENT, req.name, req.description);
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
                    printf("    %*s : %s\n", CLAGS_USAGE_ALIGNMENT, buf, opt.description);
                } else{
                    printf("    %*s : %s\n", CLAGS_USAGE_ALIGNMENT, opt.short_flag, opt.description);
                }
            }else if (opt.long_flag){
                size_t buf_size = strlen(opt.long_flag) + (opt.field_name? strlen(opt.field_name):0) + 4;
                char buf[buf_size];
                snprintf(buf, buf_size, "%s(=)%s", opt.long_flag, opt.field_name);
                printf("    %*s : %s\n", CLAGS_USAGE_ALIGNMENT, buf, opt.description);
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
