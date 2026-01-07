/*
  clags.h - A simple declarative command line arguments parser for C

  Version: 0.3.2
  
  MIT License

  Copyright (c) 2026 Constantijn de Meer

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
#include <errno.h>
#include <float.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>

#include <sys/stat.h>
#ifdef _WIN32
#ifndef S_ISREG
#define S_ISREG(m) (((m) & _S_IFREG) != 0)
#endif // S_ISREG
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFDIR) != 0)
#endif // S_ISDIR
#define stat _stat
#endif // _WIN32

#ifndef CLAGS_FREE
#define CLAGS_FREE free
#endif // CLAGS_FREE

#ifndef CLAGS_CALLOC
#define CLAGS_CALLOC calloc
#endif // CLAGS_CALLOC

#ifndef CLAGS_REALLOC
#define CLAGS_REALLOC realloc
#endif // CLAGS_REALLOC

#ifndef CLAGS_LIST_INIT_CAPACITY
#define CLAGS_LIST_INIT_CAPACITY 8
#endif // CLAGS_LIST_INIT_CAPACITY

#ifndef CLAGS_USAGE_ALIGNMENT
#define CLAGS_USAGE_ALIGNMENT -24
#endif // CLAGS_USAGE_ALIGNMENT

// macro for enabling printf-like format checks in `clags_sb_appendf`
#if defined(__GNUC__) || defined(__clang__)
//   https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
#    ifdef __MINGW_PRINTF_FORMAT
#        define CLAGS__PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (__MINGW_PRINTF_FORMAT, STRING_INDEX, FIRST_TO_CHECK)))
#    else
#        define CLAGS__PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
#    endif // __MINGW_PRINTF_FORMAT
#else
#    define CLAGS__PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

typedef enum{
    Clags_Info,
    Clags_Warning,
    Clags_Error,
    Clags_ConfigWarning,
    Clags_ConfigError,
    Clags_NoLogs,        // disable all logs
} clags_log_level_t;

typedef struct clags_config_t clags_config_t;
typedef bool (*clags_custom_verify_func_t)(clags_config_t *config, const char *arg_name, const char *arg, void *variable);       // the function type for custom verifiers
typedef bool (clags_verify_func_t)(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *verify); 
typedef clags_verify_func_t *clags_verify_func_ptr_t;
typedef void (*clags_log_handler_t)(clags_log_level_t level, const char *format, va_list args);                                  // the function type of custom log handlers
typedef uint64_t clags_fsize_t;
typedef uint64_t clags_time_t;

// all available value verifiers
clags_verify_func_t clags__verify_string;
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

// the defintions of all supported value types. Format: (enum value, verification_function, type_name)
#define clags__types\
   X(Clags_String, clags__verify_string,  NULL      ) \
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
   X(Clags_Subcmd, NULL,                  "subcmd"  ) \

// an auto-generated enum of all supported value types
#define X(type, func, name) type,
typedef enum{
    clags__types
} clags_value_type_t;
#undef X

// the definition of clags's string builder
typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} clags_sb_t;

// the definition of a "generic" list
typedef struct{
    void *items;
    size_t item_size;  // set by the appropiate `clags_list_<type>` macro
    size_t count;
    size_t capacity;
} clags_list_t;

// the definition of a choice
typedef struct{
    const char *value;
    const char *description;
} clags_choice_t;

// a wrapper for choice definitions, construct with `clags_choices`
typedef struct{
    clags_choice_t *items;
    size_t count;
    bool print_no_details; // do not print the full choice descriptions in `clags_usage`, if possible
} clags_choices_t;

// the definition of a subcommand
typedef struct{
    const char *name;
    const char *description;
    clags_config_t *config;  // the config that should be used to parse the subcommand's arguments
} clags_subcmd_t;

// a wrapper for subcommand definitions, construct with `clags_subcmd`
typedef struct{
    clags_subcmd_t *items;
    size_t count;
} clags_subcmds_t;

// the definition of a optional argument, construct with `clags_required`
typedef struct{
    void *variable;
    const char *arg_name;
    const char *description;
    // options
    clags_value_type_t value_type;
    union{
        clags_custom_verify_func_t verify;
        clags_choices_t *choices;
        clags_subcmds_t *subcmds;
        void *_data;
    };
    bool is_list;
} clags_required_t;

// the definition of a optional argument, construct with `clags_optional`
typedef struct{
    char short_flag;
    const char *long_flag;
    void *variable;
    const char *arg_name;
    const char *description;
    // options
    clags_value_type_t value_type;
    union{
        clags_custom_verify_func_t verify;
        clags_choices_t *choices;
        void *_data;
    };
} clags_optional_t;

// the definition of a flag, construct with `clags_flag`
typedef struct{
    char short_flag;
    const char *long_flag;
    bool *variable;
    const char *description;
    // options
    bool exit;
} clags_flag_t;

// entirely internal
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
    Clags_Flag,
} clags_arg_type_t;

// a wrapper for all arg types
// construct with `clags_requried`, `clags_optional` and `clags_flag` macros
typedef struct{
    clags_arg_type_t type;
    union{
        clags_required_t req;
        clags_optional_t opt;
        clags_flag_t flag;
    };
} clags_arg_t;

// the available config options
typedef struct{
    const char *ignore_prefix;        // a custom prefix that instructs the parser to ignore the current argument
    const char *list_terminator;      // a custom list terminator that tells the parser that following required arguments do no longer belong to the current list
    bool print_no_notes;              // do not print the `Notes` section in the usage
    bool allow_option_parsing_toggle; // allow "--" to be used to toggle option and flag parsing
    bool duplicate_strings;           // duplicate all strings instead of setting variables to the content of argv, free the allocated memory via `clags_config_free_allocs`
    clags_log_handler_t log_handler;  // a custom log handler
    clags_log_level_t min_log_level;  // the minimal log level for which to print logs
} clags_options_t;

// a config for a single (sub-)command
// construct with `clags_config` macro
struct clags_config_t{
    clags_arg_t *args;
    size_t args_count;
    clags_options_t options;

    // internal, set automatically
    const char *name;
    clags_config_t *parent;
    bool invalid;
    clags_list_t allocs;
};

// helper macros
#define clags_arr_len(arr) ((arr)==NULL?0:(sizeof(arr)/sizeof(arr[0])))
#define clags_return_defer(value) do{result = (value); goto defer;}while(0)
#define clags_assert(expr, msg) do{if(!(expr)){fprintf(stderr, "%s:%d in %s: [FATAL] Assertion failed [%s] : %s\n", __FILE__, __LINE__, __func__, #expr, (msg)); fflush(stderr); abort();}}while(0)
#define clags_unreachable(msg) do{fprintf(stderr, "%s:%d in %s: [FATAL] Unreachable: %s\n", __FILE__, __LINE__, __func__, (msg)); abort();}while(0)

/* Custom Variable Types */

// list definitions
#define clags_list()            (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=sizeof(char*)}
#define clags_path_list()       clags_list()
#define clags_file_list()       clags_list()
#define clags_dir_list()        clags_list()
#define clags_custom_list(size) (clags_list_t) {.items = NULL, .count=0, .capacity=0, .item_size=(size)}  // specify the size of each element so the parser can handle memory correctly when adding items
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

// macros for easy value extraction from lists
#define clags_list_element(list, value_type, i) ((value_type*)(list).items)[i]
#define clags_list_choice_element(list, i) clags_list_element((list), (clags_choice_t*), (i))->value

// macro for choices definition
#define clags_choices(arr, ...) (clags_choices_t){.items=(arr), .count=clags_arr_len(arr), __VA_ARGS__}
// macro for getting the pointer to the index-th choice
#define clags_choice_value(choices, index) (&(choices)[index])

// macro for subcmds definition
#define clags_subcmds(subcmds) (clags_subcmds_t){.items=(subcmds), .count=clags_arr_len(subcmds)}

/* Argument Constructors */

// a required, positional argument, can be typed
#define clags_required(var, name, desc, ...) (clags_arg_t){.type=Clags_Required, .req=(clags_required_t){.variable=(var), .arg_name=(name), .description=(desc), __VA_ARGS__}}

// an optional argument, can be typed
#define clags_optional(sflag, lflag, var, name, desc, ...) (clags_arg_t){.type=Clags_Optional, .opt=(clags_optional_t){.short_flag=(sflag), .long_flag=(lflag), .variable=(var), .arg_name=(name), .description=(desc), __VA_ARGS__}}

// a boolean flag argument
#define clags_flag(sflag, lflag, var, desc, ...) (clags_arg_t) {.type=Clags_Flag, .flag=(clags_flag_t){.short_flag=(sflag), .long_flag=(lflag), .variable=(var), .description=(desc), __VA_ARGS__}}
#define clags_flag_help(val) clags_flag('h', "help", val, "print this help dialog", .exit=true)

/* Config Constructor */

// constructs a config from an array of clags_arg_t args
#define clags_config(arguments, ...) (clags_config_t){.args=(arguments), .args_count=clags_arr_len(arguments), .allocs=(clags_list_t){.item_size=sizeof(char*)}, .options=(clags_options_t){__VA_ARGS__}}

/* Core Functions */

/*
  Parse arguments based on the provided config.
  
  Argument:
    - argc          : the number of arguments
    - argv          : the array of arguments
    - config        : pointer to a config with argument definitions and other options
  Returns:
    clags_config_t* : pointer to the failed config
*/
clags_config_t* clags_parse(int argc, char **argv, clags_config_t *config);

/*
  Print a detailed usage based on the provided config.
  
  Arguments:
    - program_name  : the name of the program
    - config        : pointer to a config with argument definitions and other options
*/
void clags_usage(const char *program_name, clags_config_t *config);

/*
  Get the index of a selected subcommand in the provided subcommand array.

  Arguments:
    - subcmds       : pointer to a clags_subcmds_t containing the subcommand array
    - subcmd        : pointer to the selected clags_subcmd_t to find

  Returns:
    int             : the index of the selected subcommand in the array,
                      or -1 if the subcommand was not found or either argument is NULL
*/
static inline int clags_subcmd_index(clags_subcmds_t *subcmds, clags_subcmd_t *subcmd);

/*
  Get the index of a selected choice in the provided choice array.

  Arguments:
    - choices       : pointer to clags_choices_t containing the choice array
    - choice        : pointer to the selected clags_choice_t to find

  Returns:
    int             : the index of the selected choice in the array,
                      or -1 if the choice was not found or either argument is NULL
*/
static inline int clags_choice_index(clags_choices_t *choices, clags_choice_t *choice);

/*
  Free all memory allocated for strings duplicated during parsing.
  This only applies if `.duplicate_strings` was enabled in the config.
  
  Arguments:
    - config        : pointer to the clags_config_t whose duplicated strings should be freed
*/
void clags_config_free_allocs(clags_config_t *config);

/*
  Free all memory associated with a `clags_list_t` instance.
  
  Arguments:
    - list          : a pointer to the list to free
*/
void clags_list_free(clags_list_t *list);

/* Logging */
static inline void clags_sb_appendf(clags_sb_t *sb, const char *format, ...);
static inline void clags_sb_append_null(clags_sb_t *sb);
static inline void clags_sb_free(clags_sb_t *sb);
void clags_log(clags_config_t *config, clags_log_level_t level, const char *format, ...) CLAGS__PRINTF_FORMAT(3, 4);
void clags_log_sb(clags_config_t *config, clags_log_level_t level, clags_sb_t *sb);

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

static inline char* clags__strdup(const char *string)
{
    if (!string) return NULL;
    size_t length = strlen(string);
    char *new_string = CLAGS_CALLOC(length+1, sizeof(char));
    clags_assert(new_string != NULL, "Out of memory!");
    return strcpy(new_string, string);
}

static inline void clags__sb_reserve(clags_sb_t *sb, size_t capacity)
{
    if (sb->capacity >= capacity) return;
    if (sb->capacity == 0) sb->capacity = CLAGS_LIST_INIT_CAPACITY;
    while (capacity > sb->capacity){
        sb->capacity *= 2;
    }
    sb->items = CLAGS_REALLOC(sb->items, sb->capacity*sizeof(*sb->items));
    clags_assert(sb->items != NULL, "Out of memory!");
}

static inline void clags_sb_appendf(clags_sb_t *sb, const char *format, ...)
{
    va_list args, args_copy;
    
    va_start(args, format);
    va_copy(args_copy, args);

    int n = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    clags__sb_reserve(sb, sb->count + n + 1);
    char *start = sb->items + sb->count;

    vsnprintf(start, n+1, format, args);
    va_end(args);

    sb->count += n;
}

static inline void clags_sb_append_null(clags_sb_t *sb)
{
    clags__sb_reserve(sb, sb->count+1);
    sb->items[sb->count++] = '\0';
}

static inline void clags_sb_free(clags_sb_t *sb)
{
    if (!sb) return;
    CLAGS_FREE(sb->items);
    sb->items = NULL;
    sb->count = sb->capacity = 0;
}

void clags__default_log_handler(clags_log_level_t level, const char *format, va_list args)
{
    switch(level){
        case Clags_Info:{
            fprintf(stderr, "[INFO] ");
        }break;
        case Clags_Warning:{
            fprintf(stderr, "[WARNING] ");
        }break;
        case Clags_Error:{
            fprintf(stderr, "[ERROR] ");
        }break;
        case Clags_ConfigWarning:{
            fprintf(stderr, "[CONFIG_WARNING] ");
        }break;
        case Clags_ConfigError:{
            fprintf(stderr, "[CONFIG_ERROR] ");
        }break;
        case Clags_NoLogs: return;
        default:{
            clags_unreachable("Invalid clags_log_level_t!");
        }
    }
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

void clags_log(clags_config_t *config, clags_log_level_t level, const char *format, ...)
{
    if (config && config->options.min_log_level > level) return;
    va_list args;
    va_start(args, format);
    clags_log_handler_t handler = (config && config->options.log_handler)? config->options.log_handler : clags__default_log_handler;
    handler(level, format, args);
    va_end(args);
}

void clags_log_sb(clags_config_t *config, clags_log_level_t level, clags_sb_t *sb)
{
    clags_log(config, level, "%s", sb->items);
}

bool clags__verify_string(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    (void) arg_name;
    (void) config;
    if (pvalue){
        char *string;
        if (config->options.duplicate_strings){
            string = clags__strdup(arg);
            clags_assert(string != NULL, "Out of memory!");

            clags_list_t *allocs = &config->allocs;
            if (allocs->count >= allocs->capacity){
                size_t new_capacity = allocs->capacity ? allocs->capacity*2 : CLAGS_LIST_INIT_CAPACITY;
                allocs->items = CLAGS_REALLOC(allocs->items, allocs->item_size*new_capacity);
                clags_assert(allocs->items != NULL, "Out of memory!");
                allocs->capacity = new_capacity;
            }
            ((char**)allocs->items)[allocs->count++] = string;
        } else{
            // caller must guarantee lifetime
            string = (char*) arg;
        }
        *(char**)pvalue = string;
    }
    return true;
}

bool clags__verify_bool(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    if (strcmp(arg, "true") == 0 || strcmp(arg, "True") == 0) {
        if (pvalue) *(bool*)pvalue = true;
        return true;
    } else if (strcmp(arg, "false") == 0 || strcmp(arg, "False") == 0) {
        if (pvalue) *(bool*)pvalue = false;
        return true;
    }
    clags_log(config, Clags_Error, "Invalid boolean value for argument '%s': '%s'!", arg_name, arg);
    return false;
}

bool clags__verify_int8(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    long value = strtol(arg, &endptr, 0);

    if (*endptr != '\0') {
        clags_log(config, Clags_Error, "Invalid int8 value for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value < INT8_MIN || value > INT8_MAX) {
        clags_log(config, Clags_Error, "int8 value out of range (%"PRId8" to %"PRId8") for argument '%s': '%s'!", INT8_MIN, INT8_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(int8_t*)pvalue = (int8_t)value;
    return true;
}

bool clags__verify_uint8(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    unsigned long value = strtoul(arg, &endptr, 0);

    if (*endptr != '\0') {
        clags_log(config, Clags_Error, "Invalid uint8 value for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > UINT8_MAX || *arg == '-') {
        clags_log(config, Clags_Error, "uint8 value out of range (0 to %"PRIu8") for argument '%s': '%s'!", UINT8_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(uint8_t*)pvalue = (uint8_t)value;
    return true;
}

bool clags__verify_int32(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    long value = strtol(arg, &endptr, 0);

    if (*endptr != '\0') {
        clags_log(config, Clags_Error, "Invalid int32 value for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value < INT32_MIN || value > INT32_MAX) {
        clags_log(config, Clags_Error, "int32 value out of range (%"PRId32" to %"PRId32") for argument '%s': '%s'!", INT32_MIN, INT32_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(int32_t*)pvalue = (int32_t)value;
    return true;
}

bool clags__verify_uint32(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    unsigned long value = strtoul(arg, &endptr, 0);

    if (*endptr != '\0') {
        clags_log(config, Clags_Error, "Invalid uint32 value for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > UINT32_MAX || *arg == '-') {
        clags_log(config, Clags_Error, "uint32 value out of range (0 to %"PRIu32") for argument '%s': '%s'!", UINT32_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(uint32_t*)pvalue = (uint32_t)value;
    return true;
}

bool clags__verify_int64(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    long long value = strtoll(arg, &endptr, 0);

    if (*endptr != '\0') {
        clags_log(config, Clags_Error, "Invalid int64 value for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value < INT64_MIN || value > INT64_MAX) {
        clags_log(config, Clags_Error, "int64 value out of range (%"PRId64" to %"PRId64") for argument '%s': '%s'!", INT64_MIN, INT64_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(int64_t*)pvalue = (int64_t)value;
    return true;
}

bool clags__verify_uint64(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    unsigned long long value = strtoull(arg, &endptr, 0);

    if (*endptr != '\0') {
        clags_log(config, Clags_Error, "Invalid uint64 value for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > UINT64_MAX || *arg == '-') {
        clags_log(config, Clags_Error, "uint64 value out of range (0 to %"PRIu64") for argument '%s': '%s'!", UINT64_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(uint64_t*)pvalue = (uint64_t)value;
    return true;
}

bool clags__verify_double(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    double value = strtod(arg, &endptr);

    if (*endptr != '\0') {
        clags_log(config, Clags_Error, "Invalid double value for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    if (errno == ERANGE || value > DBL_MAX || value < -DBL_MAX) {
        clags_log(config, Clags_Error, "double value out of range (%lf to %lf) for argument '%s': '%s'!", DBL_MAX, -DBL_MAX, arg_name, arg);
        return false;
    }

    if (pvalue) *(double*)pvalue = value;
    return true;
}

bool clags__verify_choice(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    if (pvalue == NULL) return false;
    clags_choice_t  **pchoice = (clags_choice_t**) pvalue;
    clags_choices_t  *choices = (clags_choices_t*) data;
    for (size_t i=0; i<choices->count; ++i){
        clags_choice_t *choice = choices->items + i;
        if (strcmp(choice->value, arg) == 0){
            if (pchoice) *pchoice = choice;
            return true;
        }
    }
    clags_log(config, Clags_Error, "Invalid choice for argument '%s': '%s'!", arg_name, arg);
    return false;
}

bool clags__verify_path(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    struct stat attr;
    if (stat(arg, &attr) == -1){
        clags_log(config, Clags_Error, "Invalid path for argument '%s': '%s' : %s!", arg_name, arg, strerror(errno));
        return false;
    }
    if (pvalue) *(char**)pvalue = (char*) arg;
    return true;
}

bool clags__verify_file(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    struct stat attr;
    if (stat(arg, &attr) == -1){
        clags_log(config, Clags_Error, "Invalid path for argument '%s': '%s' : %s!", arg_name, arg, strerror(errno));
        return false;
    }
    if (!S_ISREG(attr.st_mode)){
        clags_log(config, Clags_Error, "Path for arguments '%s' is not a file: '%s'!", arg_name, arg);
        return false;
    }
    if (pvalue) *(char**)pvalue = (char*) arg;
    return true;
}

bool clags__verify_dir(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    struct stat attr;
    if (stat(arg, &attr) == -1){
        clags_log(config, Clags_Error, "Invalid path for argument '%s': '%s' : %s!", arg_name, arg, strerror(errno));
        return false;
    }
    if (!S_ISDIR(attr.st_mode)){
        clags_log(config, Clags_Error, "Path for arguments '%s' is not a dir: '%s'!", arg_name, arg);
        return false;
    }
    if (pvalue) *(char**)pvalue = (char*) arg;
    return true;
}

bool clags__verify_size(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    unsigned long long value = strtoull(arg, &endptr, 10);

    if (endptr == arg){
        clags_log(config, Clags_Error, "No leading number in size argument '%s': '%s'!", arg_name, arg);
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
        clags_log(config, Clags_Error, "Invalid size unit for argument '%s': '%s'!", arg_name, endptr);
        return false;
    }
    
    if (errno == ERANGE || value > UINT64_MAX/factor || *arg == '-') {
        clags_log(config, Clags_Error, "clags_fsize_t value out of range (0 to %"PRIu64") for argument '%s': '%s'!", UINT64_MAX, arg_name, arg);
        return false;
    }
    if (pvalue) *(clags_fsize_t*)pvalue = (clags_fsize_t)value * factor;
    return true;
}

bool clags__verify_time_s(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    double value = strtod(arg, &endptr);
    if (endptr == arg){
        clags_log(config, Clags_Error, "No leading number in time argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    clags_time_t factor;
    if (*endptr == '\0' || strcmp(endptr, "s") == 0)  factor =       1;
    else if (strcmp(endptr, "m")  == 0)               factor =      60;
    else if (strcmp(endptr, "h")  == 0)               factor =    3600;
    else if (strcmp(endptr, "d")  == 0)               factor = 24*3600;
    else {
        clags_log(config, Clags_Error, "Invalid time unit for argument '%s': '%s'!", arg_name, endptr);
        return false;
    }
    if (errno == ERANGE || value > UINT64_MAX/factor || value < 0){
        clags_log(config, Clags_Error, "clags_time_t value out of range (0s to %"PRIu64"s) for argument '%s': '%s'!", UINT64_MAX, arg_name, arg);
        return false;
    }
    if (pvalue) *(clags_time_t*)pvalue = (clags_time_t)(value * factor);
    return true;
}

bool clags__verify_time_ns(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    (void) data;
    char *endptr;
    errno = 0;
    double value = strtod(arg, &endptr);
    if (endptr == arg){
        clags_log(config, Clags_Error, "No leading number in time argument '%s': '%s'!", arg_name, arg);
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
        clags_log(config, Clags_Error, "Invalid time unit for argument '%s': '%s'!", arg_name, endptr);
        return false;
    }
    if (errno == ERANGE || value > UINT64_MAX/factor || value < 0){
        clags_log(config, Clags_Error, "clags_time_t value out of range (0ns to %"PRIu64"ns) for argument '%s': '%s'!", UINT64_MAX, arg_name, arg);
        return false;
    }
    if (pvalue) *(clags_time_t*)pvalue = (clags_time_t)(value * factor);
    return true;
}

bool clags__verify_custom(clags_config_t *config, const char *arg_name, const char *arg, void *pvalue, void *data)
{
    clags_custom_verify_func_t value_func = (clags_custom_verify_func_t) data;
    if (!value_func(config, arg_name, (char*)arg, pvalue)) {
        clags_log(config, Clags_Error, "Value for argument '%s' does not match custom criteria: '%s'!", arg_name, arg);
        return false;
    }
    return true;
}

bool clags__append_to_list(clags_config_t *config, clags_required_t req, const char *arg)
{
    clags_list_t *list = (clags_list_t*) req.variable;
    size_t item_size = list->item_size;
    if (list->count >= list->capacity){
        size_t new_capacity = list->capacity==0? CLAGS_LIST_INIT_CAPACITY:list->capacity*2;
        list->items = CLAGS_REALLOC(list->items, new_capacity*item_size);
        clags_assert(list->items != NULL, "Out of memory!");
        list->capacity = new_capacity;
    }
    char *ptr = (char*) list->items;
    if (clags__verify_funcs[req.value_type](config, req.arg_name, arg, ptr+item_size*list->count, req._data)){
        list->count++;
        return true;
    }
    return false;
}

bool clags__validate_required(clags_config_t *config, clags_required_t req)
{
    switch (req.value_type){
        case Clags_Subcmd:{
            if (req.subcmds == NULL){
                clags_log(config, Clags_ConfigError, "incomplete subcommand definition for argument '%s'! Define `.subcmds` for subcommand verification!", req.arg_name);
                return false;
            }
        } break;
        case Clags_Choice:{
            if (req.choices == NULL){
                clags_log(config, Clags_ConfigError, "incomplete choice definition for argument '%s'! Define `.choices` for choice verification!", req.arg_name);
                return false;
            }
        } break;
        case Clags_Custom:{
            if (req.verify == NULL){
                clags_log(config, Clags_ConfigError, "incomplete custom verifier definition for argument '%s'! Define `.verify` for custom verification!", req.arg_name);
                return false;
            }
        } break;
        default: break;
    }
    return true;
}

bool clags__validate_optional(clags_config_t *config, clags_optional_t opt)
{
    char buf[3] = {'-', '\0', '\0'};
    const char *name = opt.long_flag ? opt.long_flag
                      : (opt.short_flag ? (buf[1] = opt.short_flag, buf) : "(unnamed)");
    if (opt.short_flag == '\0' && opt.long_flag == NULL){
        clags_log(config, Clags_ConfigWarning, "optional argument is unreachable. Define at least one of `short_flag` and `long_flag`.");
    }
    if (opt.long_flag && strncmp(opt.long_flag, "--", 2) == 0){
        clags_log(config, Clags_ConfigWarning,
                  "optional long flag '%s' should not start with '--'. "
                  "The parser automatically handles leading '--' for long flags, "
                  "so including it in the config may cause incorrect parsing.",
                  opt.long_flag);
    }
    switch (opt.value_type){
        case Clags_Subcmd:{
            clags_log(config, Clags_ConfigError, "optional argument '%s' may not be a subcommand!", name);
            return false;
        } break;
        case Clags_Choice:{
            if (opt.choices == NULL){
                clags_log(config, Clags_ConfigError, "incomplete choice definition for argument '%s'! Define `.choices` for choice verification!", name);
                return false;
            }
        } break;
        case Clags_Custom:{
            if (opt.verify == NULL){
                clags_log(config, Clags_ConfigError, "incomplete custom verifier definition for argument '%s'! Define `.verify` for custom verification!", name);
                return false;
            }
        } break;
        default: break;
    }
    return true;
}

bool clags__validate_flag(clags_config_t *config, clags_flag_t flag)
{
    if (flag.short_flag == '\0' && flag.long_flag == NULL){
        clags_log(config, Clags_ConfigWarning, "flag argument is unreachable. Define at least one of `short_flag` and `long_flag`.");
    }
    if (flag.long_flag && strncmp(flag.long_flag, "--", 2) == 0){
        clags_log(config, Clags_ConfigWarning,
                  "long flag '%s' should not start with '--'. "
                  "The parser automatically handles leading '--' for long flags, "
                  "so including it in the config may cause incorrect parsing.",
                  flag.long_flag);
    }
    return true;
}

bool clags__validate_config(clags_config_t *config)
{
    // validate options
    if (config->options.list_terminator && strcmp(config->options.list_terminator, "--") == 0){
        clags_log(config, Clags_ConfigError,"'.list_terminator' may not be '--' because '--' is reserved for toggling option and flag parsing!");
        return false;
    }
    if (config->options.ignore_prefix && strcmp(config->options.ignore_prefix, "--") == 0){
        clags_log(config, Clags_ConfigError, "'.ignore_prefix' may not be '--' since this conflicts with the long option and flag prefix!");
        return false;
    }

    // validate args

    bool last_was_list = false;
    bool subcmd_found = false;
    const char *last_req_name = NULL;
    for (size_t i=0; i<config->args_count; ++i){
        switch (config->args[i].type){
            case Clags_Required:{
                clags_required_t req = config->args[i].req;
                if (!clags__validate_required(config, req)) return false;
                if (req.value_type == Clags_Subcmd){
                    subcmd_found = true;
                    if (last_req_name != NULL){
                        clags_log(config, Clags_ConfigError, "subcommand '%s' must be the only required argument in its config!", req.arg_name);
                        return false;
                    }
                } else if (subcmd_found){
                    clags_log(config, Clags_ConfigError, "trailing required argument after subcommand: '%s'!", req.arg_name);
                    return false;
                }
                if (last_was_list && config->options.list_terminator == NULL){
                    clags_sb_t sb = {0};
                    clags_sb_appendf(&sb, "required argument '%s' is unreachable after list '%s'! Define '.list_terminator' in 'clags_config' to separate them", req.arg_name, last_req_name);
                    if (!req.is_list){
                        clags_sb_appendf(&sb, " or make '%s' optional", req.arg_name);
                    }
                    clags_sb_appendf(&sb, ".");
                    clags_log_sb(config, Clags_ConfigError, &sb);
                    clags_sb_free(&sb);
                    return false;
                }
                last_was_list = req.is_list;
                last_req_name = req.arg_name;
            } break;
            case Clags_Optional:{
                last_was_list = false;
                if (!clags__validate_optional(config, config->args[i].opt)) return false;
            } break;
            case Clags_Flag:{
                last_was_list = false;
                if (!clags__validate_flag(config, config->args[i].flag)) return false;
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
                clags_assert(false, "Invalid clags_arg_type_t");
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

void clags__subcmd_usage(clags_subcmds_t *subcmds)
{
    printf(" (%s)\n      Subcommands:\n", clags__type_names[Clags_Subcmd]);
    for (size_t i=0; i<subcmds->count; ++i){
        clags_subcmd_t subcmd = subcmds->items[i];
        printf("        - %*s : %s\n", CLAGS_USAGE_ALIGNMENT+6, subcmd.name, subcmd.description);
    }
}

void clags__type_usage(clags_value_type_t type, void *data, bool is_list)
{
    switch (type){
        case Clags_Choice:{
            clags__choice_usage((clags_choices_t *)data, is_list);
        }break;
        case Clags_Subcmd:{
            clags__subcmd_usage((clags_subcmds_t*) data);
        }break;
        case Clags_String:{
            if (is_list) printf(" ([])");
        }break;
        default:{
            printf(" (%s%s)", clags__type_names[type], is_list?"[]":"");
        }
    }
    printf("\n");
}

void clags__subcommand_path_usage(const char *program_name, clags_config_t *config)
{
    if (config->parent){
        clags__subcommand_path_usage(program_name, config->parent);
        printf(" %s", config->name);
    } else{
        printf("Usage: %s", program_name);
    }
}

clags_config_t* clags_parse(int argc, char **argv, clags_config_t *config)
{
    if (config == NULL || config->args == NULL) return NULL;
    // validate the configuration, exit and mark config as invalid on fatal error
    if (!clags__validate_config(config)){
        config->invalid = true;
        return config;
    }

    clags_config_t *result = NULL;

    // sort arguments by type
    clags_required_t *required = CLAGS_CALLOC(config->args_count, sizeof(*required));
    clags_optional_t *optional = CLAGS_CALLOC(config->args_count, sizeof(*optional));
    clags_flag_t     *flags    = CLAGS_CALLOC(config->args_count, sizeof(*flags));

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
                clags_log(config, Clags_Error, "Missing flag or option name: '--%s'!", arg);
                clags_return_defer(config);
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
                                clags_log(config, Clags_Error, "Optional flag %s requires argument!", arg);
                                clags_return_defer(config);
                            }
                            value = argv[++index];
                            if (!ignore_prefix || strncmp(value, ignore_prefix, ignore_prefix_len) != 0) break;
                            arguments_ignored = true;
                        }
                    } else if (*value++ == '='){
                        if (*value == '\0'){
                            clags_log(config, Clags_Error, "Designated option assignment may not have an empty value: '%s'!", arg);
                            clags_return_defer(config);
                        }
                    } else {
                        continue;
                    }
                    if (!clags__verify_funcs[opt.value_type](config, arg, value, opt.variable, opt.verify)) clags_return_defer(config);
                    goto next;
                }
            }
            // parse long flags
            for (size_t i=0; i<args.flag_count; ++i){
                clags_flag_t flag = args.flags[i];
                if (flag.long_flag && strcmp(arg, flag.long_flag) == 0){
                    if (flag.variable != NULL) *flag.variable = true;
                    if (flag.exit) clags_return_defer(NULL);
                    goto next;
                }
            }
            clags_log(config, Clags_Error, "Unknown long flag or option: '--%s'!", arg);
            clags_return_defer(config);
        } else if (accept_options && *arg == '-' && !isdigit((unsigned char)arg[1])){
            // parse short flag or option
            arg += 1;
            size_t flag_len = strlen(arg);
            if (flag_len == 0){
                clags_log(config, Clags_Error, "Missing flag or option name: '-'!");
                clags_return_defer(config);
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
                                    clags_log(config, Clags_Error, "Optional flag %s requires argument!", arg);
                                    clags_return_defer(config);
                                }
                                value = argv[++index];
                                if (!ignore_prefix || strncmp(value, ignore_prefix, ignore_prefix_len) != 0) break;
                                arguments_ignored = true;
                            }
                        }
                        if (!clags__verify_funcs[opt.value_type](config, arg, value, opt.variable, opt.verify)) clags_return_defer(config);
                        goto next;
                    }
                }
                bool matched = false;
                for (size_t i=0; i<args.flag_count; ++i){
                    clags_flag_t flag = args.flags[i];
                    if (*c == flag.short_flag){
                        if (flag.variable) *flag.variable = true;
                        if (flag.exit) clags_return_defer(NULL);
                        matched = true;
                    }
                }
                if (!matched){
                    if (flag_len > 1){
                        clags_log(config, Clags_Error, "Unknown short flag '-%c' in combination '-%s'!", *c, arg);
                    } else{
                        clags_log(config, Clags_Error, "Unknown short flag '-%c'!", *c);
                    }
                    clags_return_defer(config);
                }
            }
        } else {
            // parse required argument
            if (required_count >= args.required_count){
                clags_log(config, Clags_Error, "Unknown additional argument (%zu/%zu): '%s'!", required_count+1, args.required_count, arg);
                clags_return_defer(config);
            }

            // verify and write argument
            clags_required_t req = args.required[required_count];

            // parse subcommands
            if (req.value_type == Clags_Subcmd){
                clags_subcmds_t *subcmds = req.subcmds;
                for (size_t i=0; i<subcmds->count; ++i){
                    clags_subcmd_t subcmd = subcmds->items[i];
                    if (strcmp(subcmd.name, arg) == 0){
                        // found matching subcommand, descend into recursion
                        clags_config_t *child_config = subcmd.config;
                        if (req.variable) *(clags_subcmd_t**)req.variable = &subcmds->items[i];
                        if (child_config == NULL) clags_return_defer(NULL);

                        child_config->parent = config;
                        child_config->name = arg;
                        clags_return_defer(clags_parse((int) argc-index, argv+index, child_config));
                    }
                }
                clags_log(config, Clags_Error, "unknown subcommand '%s' for argument '%s'!", arg, req.arg_name);
                clags_return_defer(config);
            }
            if (req.is_list){
                in_list = true;
                if (!clags__append_to_list(config, req, arg)) clags_return_defer(config);
            } else{
                required_count += 1;
                if (!clags__verify_funcs[req.value_type](config, req.arg_name, arg, req.variable, req.verify)) clags_return_defer(config);
            }
        }
    next:
    }
    if (in_list) required_count += 1;
    if (arguments_ignored) clags_log(config, Clags_Warning, "Arguments were ignored because they were prefixed with '%s'", ignore_prefix);

    // report missing required arguments
    if (required_count != args.required_count){
        clags_sb_t sb = {0};
        clags_sb_appendf(&sb, "Missing required arguments:");
        for (size_t i=required_count; i<args.required_count; ++i){
            clags_sb_appendf(&sb, " <%s>", args.required[i].arg_name);
        }
        clags_sb_appendf(&sb, "!");
        clags_log_sb(config, Clags_Error, &sb);
        clags_sb_free(&sb);
        clags_return_defer(config);
    }

    clags_return_defer(NULL);

defer:
    // cleanup memory of sorted args
    CLAGS_FREE(required);
    CLAGS_FREE(optional);
    CLAGS_FREE(flags);
    return result;
}

void clags_usage(const char *program_name, clags_config_t *config)
{
    if (config->args == NULL || config->invalid) return;

    clags_required_t *required = CLAGS_CALLOC(config->args_count, sizeof(*required));
    clags_optional_t *optional = CLAGS_CALLOC(config->args_count, sizeof(*optional));
    clags_flag_t *flags = CLAGS_CALLOC(config->args_count, sizeof(*flags));

    clags_args_t args = {.required=required, .optional=optional, .flags=flags};

    clags__sort_args(&args, config);

    clags_options_t options = config->options;
        
    clags__subcommand_path_usage(program_name, config);

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
            clags__type_usage(req.value_type, req._data, req.is_list);
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
                    printf("    -%*c: %s", CLAGS_USAGE_ALIGNMENT, opt.short_flag, opt.description);
                }
                clags__type_usage(opt.value_type, opt._data, false);
            }else if (opt.long_flag){
                size_t buf_size = strlen(opt.long_flag) + (opt.arg_name? strlen(opt.arg_name):0) + 6;
                char buf[buf_size];
                snprintf(buf, buf_size, "--%s(=)%s", opt.long_flag, opt.arg_name);
                printf("    %*s : %s", CLAGS_USAGE_ALIGNMENT, buf, opt.description);
                clags__type_usage(opt.value_type, opt._data, false);
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
                    printf("    -%*c: %s", CLAGS_USAGE_ALIGNMENT, flag.short_flag, flag.description);
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
        if (config->parent != NULL){
            printf("    Options and flags of parent subcommands that are not listed above, are not supported. Place them before the beginning of this subcommand.\n");
        }
    }

    CLAGS_FREE(args.required);
    CLAGS_FREE(args.optional);
    CLAGS_FREE(args.flags);
}

static inline int clags_subcmd_index(clags_subcmds_t *subcmds, clags_subcmd_t *subcmd)
{
    if (!subcmds || !subcmd) return -1;
    for (size_t i=0; i<subcmds->count; ++i){
        if (&subcmds->items[i] == subcmd) return (int) i;
    }
    return -1;
}

static inline int clags_choice_index(clags_choices_t *choices, clags_choice_t *choice)
{
    if (!choices || !choice) return -1;
    for (size_t i=0; i<choices->count; ++i){
        if (&choices->items[i] == choice) return (int) i;
    }
    return -1;
}

void clags_list_free(clags_list_t *list)
{
    if (list == NULL) return;
    CLAGS_FREE(list->items);
    list->items = NULL;
    list->count = list->capacity = 0;
}

void clags_config_free_allocs(clags_config_t *config)
{
    if (config == NULL) return;
    clags_list_t *allocs = &config->allocs;
    for (size_t i=0; i<allocs->count; ++i){
        CLAGS_FREE(((char**) allocs->items)[i]);
    }
    CLAGS_FREE(allocs->items);
    allocs->items = NULL;
    allocs->count = allocs->capacity = 0;
}

#endif // CLAGS_IMPLEMENTATION
