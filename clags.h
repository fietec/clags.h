#ifndef CLAGS_H
#define CLAGS_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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

bool clags_parse(int argc, char **argv, bool *help);
void clags_usage(const char *program_name);
#endif // CLAGS_H

#ifdef CLAGS_IMPLEMENTATION

#ifndef clags_required
#define clags_required
#endif // clags_required

#ifndef clags_optional
#define clags_optional
#endif // clags_optional

#ifndef clags_flags
#define clags_flags
#endif // clags_flags

static clags_args_t args = {
#define clags_arg(n, val, desc) (clags_req_t){.name=(n), .value=(val), .description=(desc)},
    .required = (clags_req_t[]){
        clags_required
    },
#undef clags_arg
#define clags_arg(f, val, fname, desc) (clags_opt_t){.value=(val), .description=(desc), .flag=(f), .field_name=(fname)},
    .optional = (clags_opt_t[]){
        clags_optional
    },
#undef clags_arg
#define clags_arg(f, val, desc) (clags_flag_t){.flag=(f), .value=(val), .description=(desc)},
    .flags = (clags_flag_t[]){
        clags_flags
    },
#undef clags_arg
#define clags_arg(...) 1+
    .required_count = clags_required 0,
    .optional_count = clags_optional 0,
    .flag_count  = clags_flags  0,
#undef clags_arg
};

bool clags_parse(int argc, char **argv, bool *help)
{
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

void clags_usage(const char *program_name)
{
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
    printf("\n");
}
#endif // CLAGS_IMPLEMENTATION
