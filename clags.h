/*
  clags.h - Version 4.0.0 - https://github.com/fietec/clags.h

  A declarative single-header C99 library for parsing and validating command-line arguments.

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

/*
  Example
  =======

  ```
    #include <stdio.h>
    #include <stdbool.h>

    // This includes the function implementations and only has to be done once per translation unit
    #define CLAGS_IMPLEMENTATION
    #include <clags.h>

    int main(int argc, char **argv)
    {
        // Declare argument variables with optional default values
        const char *input_file;
        const char *output_file;
        bool warnings = false;
        bool help = false;

        // Describe the CLI
        clags_arg_t args[] = {
            // Positional arguments are parsed in the order they are defined here
            clags_positional(&input_file, "input_file", "the input file"),

            // Optional flag arguments that take an accompanying value
            clags_option('o', "output", &output_file, "FILE", "the output file", .default_input="a.out"),

            // Optional standalone flags to set a boolean value on occurrence
            clags_flag('w', "warnings", &warnings, "print warnings"),
            // Convenience helper for the standard '-h', '--help' flag
            clags_flag_help(&help),
        };
        // The parsing context for the (sub)command defined above
        clags_config_t config = clags_config(args);

        // Parse the arguments using the previously defined rules, returns the config on error
        clags_config_t *failed = clags_parse(argc, argv, &config);
        if (failed != NULL){
            // Print an automatic usage, based on the defined config
            clags_usage(argv[0], failed);
            return 1;
        }

        // You can now use the populated argument variables
        if (help){
            clags_usage(argv[0], &config);
            return 0;
        }
        printf("input: %s, output: %s, warnings:%s\n", input_file, output_file, warnings?"true":"false");
        return 0;
    }

  ```
*/

/*
  Syntax Reference
  ======================

  Positional arguments
  --------------------

  <arg>        required positional argument
  [arg]        optional positional argument

  <arg..>      positional list argument (one or more values)
  [arg..]      optional positional list argument


  Options
  -------

  Options may appear anywhere while option parsing is enabled.

  Long options:

  --opt VALUE
  --opt=VALUE

  Short options:

  -o VALUE  // separate argument
  -oValue   // attached argument


  Option Lists
  ------------

  If an option is configured as a list, each occurrence appends one value.

  --file a --file b --file c
  -f a -f b -f c

  Comma-separated values are NOT supported.


  Flags
  -----

  Flags do not take values.

  Short flag:

  -h

  Long flag:

  --help

  Combined short flags:

  -abc    ==  -a -b -c

  Note: If a short option that takes a value appears in a combined group,
        the rest of the string is treated as its argument:

        -wovalue    // -w is a flag, -o consumes 'value'


  Subcommands
  -----------

  Subcommands are positional arguments with their own config.

  program <subcmd> [SUBCMD_ARGS]

  Parsing continues in the selected subcommand's config.


  Option parsing toggle (`--`)
  ----------------------------

  `--` disables option/flag parsing from this point onward.

  If the config enables toggling, `--` can re-enable parsing.
  This is an add-on feature of clags, not POSIX.


  List Terminator
  ---------------

  If a list terminator is configured (e.g. "::"):

  <list1..> :: <list2..> :: [arg]

  Without a terminator, a positional list must be last.


  Ignored Arguments (`ignore_prefix`)
  -----------------------------------

  Arguments prefixed with the configured ignore prefix are ignored.

  For example with the ignore prefix defined as "!":

  !ignored  // argument is ignored


  Value Types
  -----------

  This table describes the supported value types and their expected syntax. See the `CLAGS__VALUE_TYPES` macro for more details.

  Value Type    | Syntax / Examples           | Notes
  --------------------------------------------------------------------------
  string        | "hello", world              | Any sequence of characters.
  bool          | true, false, yes, N         | Case-insensitive.
  int / uint    | 0, -128, 127, 0x1F, 0b1010  | Signed and unsigned integers of various sizes.
                |                             | Accepts decimal, hex (0x), octal (0), and binary (0b) notation.
                |                             | Values must fit in the type’s bounds ((u)int<8/16/32/64>_t) or the defined range.
  real          | 3.14, -0.001, 2e10          | Floating-point value.
  choice        | "red", "green", "blue"      | Must match one of the configured choices. Case-sensitive by default, can be disabled.
  path          | /home/user/docs, C:\Windows | Any filesystem path.
  file          | /etc/passwd, myfile.txt     | Must be a regular file.
  dir           | /tmp, C:\Users              | Must be a directory.
  size          | 1024, 10KiB, 2MB            | Supports decimal and binary suffixes. Case-insensitive.
  time_s        | 10s, 5m, 2h2s4m             | Time in seconds; supports s/m/h/d suffixes. Case-insensitive.
  time_ns       | 100ns, 1us, 5ms, 2s20ms     | Time in nanoseconds; supports ns/us/ms/s/m/h/d suffixes. Case-insensitive.
  custom        | depends on user function    | Validation done via a custom function pointer.
*/

#ifndef CLAGS_H
#define CLAGS_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>

// public api linkage / visibility
#ifndef CLAGS_API
#define CLAGS_API
#endif // CLAGS_API

// private implementation linkage
#ifndef CLAGS_PRIV
#define CLAGS_PRIV static
#endif // CLAGS_PRIV

// memory allocation functions; can be overridden
#ifndef CLAGS_CALLOC
/*
    CLAGS_CALLOC: Allocate zero-initialized memory for 'count' elements of 'size' bytes each.
    Parameters:
        count - the number of elements to allocate
        size  - the size in bytes of one element
*/
#include <stdlib.h>
#define CLAGS_CALLOC(count, size) calloc(count, size)
#endif // CLAGS_CALLOC

#ifndef CLAGS_REALLOC
/*
    CLAGS_REALLOC: Resize an existing memory block.
    Parameters:
        ptr   - pointer to the existing memory block (may be NULL)
        oldsz - previous size of the allocation in bytes (default realloc ignores this)
        newsz - new requested size in bytes
*/
#include <stdlib.h>
#define CLAGS_REALLOC(ptr, oldsz, newsz) realloc(ptr, newsz)
#endif // CLAGS_REALLOC

#ifndef CLAGS_FREE
/*
    CLAGS_FREE: Free a memory block.
    Parameters:
        ptr - pointer to memory to free
        sz  - size of the allocation in bytes (default free ignores this)
*/
#include <stdlib.h>
#define CLAGS_FREE(ptr, sz) free(ptr)
#endif // CLAGS_FREE

#if !defined(CLAGS_NO_REAL_TYPES) && !defined(CLAGS_STRTOD)
/*
    CLAGS_STRTOD: Convert a string to a double-precision floating-point number.
    The macro must expand to a function matching the signature:
        double CLAGS_STRTOD(const char *nptr, char **endptr);
*/
#include <stdlib.h>
#define CLAGS_STRTOD strtod
#endif // CLAGS_STRTOD

// terminates the program due to a unrecoverable fatal error.
// can be redefined for custom behavior (e.g., `#define CLAGS_PANIC exit(1)`).
#ifndef CLAGS_PANIC
#include <stdlib.h>
#define CLAGS_PANIC abort()
#endif // CLAGS_PANIC

// the initial capacity of lists
#ifndef CLAGS_LIST_INIT_CAPACITY
#define CLAGS_LIST_INIT_CAPACITY 16
#endif // CLAGS_LIST_INIT_CAPACITY

// the initial capacity of string builders
#ifndef CLAGS_SB_INIT_CAPACITY
#define CLAGS_SB_INIT_CAPACITY 32
#endif // CLAGS_SB_INIT_CAPACITY

/* Usage Alignment */

// the default minimum character column at which ':' appears in `clags_usage` output
// used when no `.min_usage_alignment` config option is specified
#ifndef CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT
#define CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT 32
#endif // CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT

// the default maximum character column at which ':' appears in `clags_usage` output
// used when no `.max_usage_alignment` config option is specified
#ifndef CLAGS_DEFAULT_MAX_USAGE_ALIGNMENT
#define CLAGS_DEFAULT_MAX_USAGE_ALIGNMENT 96
#endif // CLAGS_DEFAULT_MAX_USAGE_ALIGNMENT

#define CLAGS__MIN_USAGE_ALIGNMENT 24
#if CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT < CLAGS__MIN_USAGE_ALIGNMENT
#error CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT may not be smaller than CLAGS__MIN_USAGE_ALIGNMENT!
#endif

#if CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT > CLAGS_DEFAULT_MAX_USAGE_ALIGNMENT
#error CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT may not be greater than CLAGS_DEFAULT_MAX_USAGE_ALIGNMENT!
#endif

#define CLAGS__USAGE_PRINTF_ALIGNMENT(alignment) (int)-((alignment)-5)
#define CLAGS__USAGE_TEMP_BUFFER_SIZE(alignment) ((alignment)-4)
#define CLAGS__USAGE_ALIGNMENT_FROM_BUFFER_SIZE(size) ((size)+4)
#define CLAGS__USAGE_TRUNCATION_ELLIPSIS "..."

// the maximum amount of choices to print in the inline format, when `.print_no_details=true`
// if there are more than the defined maximum of choices the detailed output will be printed nevertheless
#ifndef CLAGS_MAX_INLINE_CHOICES
#define CLAGS_MAX_INLINE_CHOICES 5
#endif // CLAGS_MAX_INLINE_CHOICES

// macro for enabling printf-like format checks in `clags_sb_appendf`
#if defined(__GNUC__) || defined(__clang__)
#    ifdef __MINGW_PRINTF_FORMAT
#        define CLAGS__PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (__MINGW_PRINTF_FORMAT, STRING_INDEX, FIRST_TO_CHECK)))
#    else
#        define CLAGS__PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
#    endif // __MINGW_PRINTF_FORMAT
#else
#    define CLAGS__PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

// feature stripping macros
#ifdef CLAGS_NO_COMPLEX_TYPES
  #define CLAGS_NO_INT_TYPES
  #define CLAGS_NO_REAL_TYPES
  #define CLAGS_NO_PATH_TYPES
  #define CLAGS_NO_SIZE_TYPES
  #define CLAGS_NO_TIME_TYPES
#endif // CLAGS_NO_COMPLEX_TYPES

#define CLAGS__VTYPE(type, verify_func, name, value_type, validate_func, usage_func) X(type, verify_func, name, value_type, validate_func, usage_func)
#define CLAGS__IGNORE_VTYPE(type, verify_func, name, value_type, validate_func, usage_func)

#define CLAGS__BASIC_VTYPE CLAGS__VTYPE

#ifdef CLAGS_NO_INT_TYPES
  #define CLAGS__INT_VTYPE CLAGS__IGNORE_VTYPE
#else
  #define CLAGS__INT_VTYPE CLAGS__VTYPE
#endif // CLAGS_NO_INT_TYPES

#ifdef CLAGS_NO_REAL_TYPES
  #define CLAGS__REAL_VTYPE CLAGS__IGNORE_VTYPE
#else
  #define CLAGS__REAL_VTYPE CLAGS__VTYPE
#endif // CLAGS_NO_REAL_TYPES

#ifdef CLAGS_NO_PATH_TYPES
  #define CLAGS__PATH_VTYPE CLAGS__IGNORE_VTYPE
  #define CLAGS__PATH_TYPE(type, name, exist)
#else
  #define CLAGS__PATH_VTYPE CLAGS__VTYPE
  #define CLAGS__PATH_TYPE(type, name, exist) X(type, name, exist)
#endif // CLAGS_NO_PATH_TYPES

#ifdef CLAGS_NO_SIZE_TYPES
  #define CLAGS__SIZE_VTYPE CLAGS__IGNORE_VTYPE
#else
  #define CLAGS__SIZE_VTYPE CLAGS__VTYPE
#endif // CLAGS_NO_SIZE_TYPES

#ifdef CLAGS_NO_TIME_TYPES
  #define CLAGS__TIME_VTYPE CLAGS__IGNORE_VTYPE
#else
  #define CLAGS__TIME_VTYPE CLAGS__VTYPE
#endif // CLAGS_NO_TIME_TYPES

// the definition of all supported value types
// if an argument’s `value_type` is not explicitly set, `Clags_String` is used by default.
#define CLAGS__VALUE_TYPES                                                                                                                                                                                   \
 /* Category           Enum Value    Verification Func    Name       Var Type         Validation Func         Usage Func                                                                                  */ \
 /* ================== ============= ==================== ========== ================ ======================= ====================                                                                        */ \
    CLAGS__BASIC_VTYPE(Clags_String, clags_verify_string, "string",  char*,           clags__validate_none,   clags__default_usage ) /* string value                                                      */ \
    CLAGS__BASIC_VTYPE(Clags_Bool,   clags_verify_bool,   "bool",    bool,            clags__validate_none,   clags__default_usage ) /* boolean value                                                     */ \
    CLAGS__BASIC_VTYPE(Clags_Choice, clags_verify_choice, "choice",  clags_choice_t*, clags__validate_choice, clags__choice_usage  ) /* selects one value from a set of choices                           */ \
    CLAGS__BASIC_VTYPE(Clags_Subcmd, clags_verify_subcmd, "subcmd",  clags_subcmd_t*, clags__validate_subcmd, clags__subcmd_usage  ) /* subcommand                                                        */ \
    CLAGS__BASIC_VTYPE(Clags_Custom, clags_verify_custom, "custom",  void*,           clags__validate_custom, clags__custom_usage  ) /* custom type, defined via custom verification function             */ \
    CLAGS__INT_VTYPE  (Clags_Int,    clags_verify_int,    "int",     int64_t,         clags__validate_int,    clags__int_usage     ) /* a signed integer within user-defined limits                       */ \
    CLAGS__INT_VTYPE  (Clags_Int8,   clags_verify_int8,   "int8",    int8_t,          clags__validate_none,   clags__default_usage ) /* signed 8-bit integer                                              */ \
    CLAGS__INT_VTYPE  (Clags_Int16,  clags_verify_int16,  "int16",   int16_t,         clags__validate_none,   clags__default_usage ) /* signed 16-bit integer                                             */ \
    CLAGS__INT_VTYPE  (Clags_Int32,  clags_verify_int32,  "int32",   int32_t,         clags__validate_none,   clags__default_usage ) /* signed 32-bit integer                                             */ \
    CLAGS__INT_VTYPE  (Clags_Int64,  clags_verify_int64,  "int64",   int64_t,         clags__validate_none,   clags__default_usage ) /* signed 64-bit integer                                             */ \
    CLAGS__INT_VTYPE  (Clags_UInt,   clags_verify_uint,   "uint",    uint64_t,        clags__validate_uint,   clags__uint_usage    ) /* an unsigned integer within user-defined limits                    */ \
    CLAGS__INT_VTYPE  (Clags_UInt8,  clags_verify_uint8,  "uint8",   uint8_t,         clags__validate_none,   clags__default_usage ) /* unsigned 8-bit integer                                            */ \
    CLAGS__INT_VTYPE  (Clags_UInt16, clags_verify_uint16, "uint16",  uint16_t,        clags__validate_none,   clags__default_usage ) /* unsigned 16-bit integer                                           */ \
    CLAGS__INT_VTYPE  (Clags_UInt32, clags_verify_uint32, "uint32",  uint32_t,        clags__validate_none,   clags__default_usage ) /* unsigned 32-bit integer                                           */ \
    CLAGS__INT_VTYPE  (Clags_UInt64, clags_verify_uint64, "uint64",  uint64_t,        clags__validate_none,   clags__default_usage ) /* unsigned 64-bit integer                                           */ \
    CLAGS__REAL_VTYPE (Clags_Real,   clags_verify_real,   "real",    double,          clags__validate_real,   clags__real_usage    ) /* a fractional number within user-defined limits                    */ \
    CLAGS__REAL_VTYPE (Clags_Float,  clags_verify_float,  "float",   float,           clags__validate_none,   clags__default_usage ) /* a single-precision fractional number                              */ \
    CLAGS__REAL_VTYPE (Clags_Double, clags_verify_double, "double",  double,          clags__validate_none,   clags__default_usage ) /* a double-precision fractional number                              */ \
    CLAGS__PATH_VTYPE (Clags_Path,   clags_verify_path,   "path",    char*,           clags__validate_none,   clags__default_usage ) /* valid filesystem path                                             */ \
    CLAGS__PATH_VTYPE (Clags_File,   clags_verify_file,   "file",    char*,           clags__validate_none,   clags__default_usage ) /* path to a regular file                                            */ \
    CLAGS__PATH_VTYPE (Clags_Dir,    clags_verify_dir,    "dir",     char*,           clags__validate_none,   clags__default_usage ) /* path to a directory                                               */ \
    CLAGS__SIZE_VTYPE (Clags_Size,   clags_verify_size,   "size",    clags_fsize_t,   clags__validate_none,   clags__default_usage ) /* size in bytes (supports suffixes like KiB/MB)                     */ \
    CLAGS__TIME_VTYPE (Clags_TimeS,  clags_verify_time_s, "time",    clags_time_t,    clags__validate_none,   clags__default_usage ) /* time duration in seconds (supports suffixes s/m/h/d)              */ \
    CLAGS__TIME_VTYPE (Clags_TimeNS, clags_verify_time_ns,"time_ns", clags_time_t,    clags__validate_none,   clags__default_usage ) /* time duration in nanoseconds (supports suffixes ns/us/ms/s/m/h/d) */ \

// the definition of all flag types
#define CLAGS__FLAG_TYPES                                                                                                                       \
    X(Clags_BoolFlag,   clags__set_bool_flag   ) /* standard boolean flag (default); set to `true` when the flag occurs; variable type: bool */ \
    X(Clags_ConfigFlag, clags__set_config_flag ) /* stores a pointer to the config in which the flag was set; variable type: clags_config_t* */ \
    X(Clags_CountFlag,  clags__set_count_flag  ) /* tracks how many times the flag was encountered; variable type: size_t                    */ \

// the definition of all error types and their respective descriptions
#define CLAGS__ERROR_TYPES                                                                      \
    X(Clags_Error_Ok,               "no error"                                                ) \
    X(Clags_Error_InvalidConfig,    "configuration is invalid"                                ) \
    X(Clags_Error_InvalidDefault,   "failed to set a default value"                           ) \
    X(Clags_Error_InvalidValue,     "argument value does not match expected type or criteria" ) \
    X(Clags_Error_InvalidOption,    "unrecognized option or flag syntax"                      ) \
    X(Clags_Error_TooManyArguments, "too many positional arguments provided"                  ) \
    X(Clags_Error_TooFewArguments,  "required positional arguments missing"                   ) \
    X(Clags_Error_ListOverflow,     "a list reached its maximum capacity"                     ) \
    X(Clags_Error_ChildConfigError, "a child config encountered an error"                     ) \

// the definition of all path types
// Note: these are NOT value types but only used internally and together with the `clags_path_type` function
#define CLAGS__PATH_TYPES                                                                       \
    CLAGS__PATH_TYPE(Clags_Path_Invalid, "invalid", false ) /* invalid path                  */ \
    CLAGS__PATH_TYPE(Clags_Path_Dir,     "dir",     true  ) /* existing directory            */ \
    CLAGS__PATH_TYPE(Clags_Path_Reg,     "file",    true  ) /* existing regular file         */ \
    CLAGS__PATH_TYPE(Clags_Path_Other,   "other",   true  ) /* existing file of another type */ \

// an auto-generated enum of all supported value types
#define X(type, verify_func, name, vtype, validate_func, usage_func) type,
typedef enum{
    CLAGS__VALUE_TYPES
} clags_value_type_t;
#undef X

// an auto-generated enum of all error types
#define X(type, desc) type,
typedef enum{
    CLAGS__ERROR_TYPES
} clags_error_type_t;
#undef X

#ifndef CLAGS_NO_PATH_TYPES
// an auto-generated enum of all path types
#define X(type, desc, exist) type,
typedef enum{
    CLAGS__PATH_TYPES
} clags_path_type_t;
#undef X
#endif // !CLAGS_NO_PATH_TYPES

// an auto-genereated enum of all available flag types
#define X(type, setter) type,
typedef enum {
    CLAGS__FLAG_TYPES
} clags_flag_type_t;
#undef X

typedef struct clags_config_t clags_config_t;
typedef struct clags_choice_t clags_choice_t;
typedef struct clags_subcmd_t clags_subcmd_t;
typedef uint64_t clags_fsize_t;
typedef uint64_t clags_time_t;

/*
 The definition of the verification function type.
 These functions return the argument's validity for a specific type
 and write the converted value to the `variable` provided (if not NULL).

 Arguments:
   - config   : the config currently being parsed, mostly useful for logging
   - arg_name : the name of the argument currently being parsed, useful for logging
   - arg      : the argument currently being parsed
   - variable : the pointer of the variable to set
   - data     : additional custom data

 Returns:
   - bool     : success of the verification (even if no value could be written)
*/
typedef bool (clags_verify_func_def_t)(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data);
typedef clags_verify_func_def_t *clags_verify_func_t; // the function pointer type of verifiers

/*
 The definition of the validation function type.
 These functions check an argument declaration's validity for a specific type.

 Arguments:
   - config          : the config of the argument, mostly useful for logging
   - arg_name        : the name of the argument, useful for logging
   - data            : additional data belonging to the type to validate

 Returns:
   - clags_config_t* : NULL on success, the pointer to the invalid argument's config on error
*/
typedef clags_config_t* (clags_validate_func_def_t)(clags_config_t *config, const char *arg_name, void *data);
typedef clags_validate_func_def_t *clags_validate_func_t; // the function pointer type of validators

/*
 The definition of the usage function type.
 These functions are responsible for printing the type information usage for a specific type.
 Generally the format is: (<type-name> [constraints]) [(default: <default-input>],
 but certain types (e.g. choices, subcommands etc) might add further information.

 Arguments:
   - file          : the file to print to
   - config        : the current config
   - type          : the type of the current argument
   - is_list       : `true` if the current argument write to a list
   - default_value : the default value to print (can be NULL)
   - data          : additional data belonging to the type
*/
typedef void (clags_usage_func_def_t)(FILE *file, clags_config_t *config, clags_value_type_t type, bool is_list, const char *default_value, void *data);
typedef clags_usage_func_def_t *clags_usage_func_t; // the function pointer type of usage printers

/*
 The definition of the flag setter function type.
 These functions are responsible for setting a flag for a specific flag type.

 Arguments:
   - config        : the current config
   - variable      : the variable to set
*/
typedef clags_error_type_t (clags_flag_func_def_t)(clags_config_t *config, void *variable);
typedef clags_flag_func_def_t *clags_flag_func_t;   // the function pointer type of flag setters

// log levels used by clags. Levels are ordered by priority: higher enum values indicate higher severity.
typedef enum{
    Clags_Info,
    Clags_Warning,
    Clags_Error,
    Clags_ConfigWarning,
    Clags_ConfigError,
    Clags_NoLogs,        // disable all logs
} clags_log_level_t;

/*
  The definition of the log handler function type.
  These functions are responsible for logging a formatted message in respect to a given log level.
  For an example implementation, see `clags__default_log_handler`.

  Arguments:
    - config  : the current config
    - level   : the log level of the message
    - format  : a 'printf'-like format string
    - args    : the variadic arguments for the format string
*/
typedef void (*clags_log_handler_t)(clags_config_t *config, clags_log_level_t level, const char *format, va_list args);

// the definition of clags's string builder
typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} clags_sb_t;

// the definition of a "generic" list
typedef struct{
    // the fields of a common dynamic array
    void *items;
    size_t count;
    size_t capacity;

    // the fields set by the `clags_list` macros
    size_t item_size;
    clags_value_type_t value_type;
    bool custom_size_set;
} clags_list_t;

// the definition of a choice
struct clags_choice_t{
    const char *value;
    const char *description;
};

// a wrapper for choice definitions, construct with `clags_choices`
typedef struct{
    clags_choice_t *items;
    size_t count;
    bool print_no_details; // do not print the full choice descriptions in `clags_usage`, if possible
    bool case_insensitive; // match choices regardless of case
} clags_choices_t;

// the definition of a subcommand
struct clags_subcmd_t{
    const char *name;        // the name and identifier of a subcommand
    const char *description; // help text describing the subcommand
    clags_config_t *config;  // the config that should be used to parse the subcommand's arguments
};

// a wrapper for subcommand definitions, construct with `clags_subcmd`
typedef struct{
    clags_subcmd_t *items;
    size_t count;
} clags_subcmds_t;

// the definition of a range, both limits are inclusive, construct with `clags_<int/uint/real>_range` macros
typedef struct{
    clags_value_type_t type;
    union{
        int64_t as_int;
        uint64_t as_uint;
        double as_double;
    } min;
    union{
        int64_t as_int;
        uint64_t as_uint;
        double as_double;
    } max;
} clags_range_t;

// the definition of a custom value type
typedef struct{
    const char *name;                      // name of the custom type
    clags_verify_func_t verify;            // custom verfication to call
    clags_usage_func_t usage;
    clags_validate_func_t validate;
    void *data;                            // additional custom data to be passed to the custom verification function via the `data` argument
} clags_custom_t;

// the definition of a positional argument, construct with `clags_positional()`
typedef struct{
    void *variable;                        // pointer to store the parsed value at; type must match `value_type`, or `clags_list_t` of that type if `is_list` is set
    const char *arg_name;                  // name shown in usage for the positional argument
    const char *description;               // help text describing the positional argument
    // options
    clags_value_type_t value_type;         // type of the positional value. See `CLAGS__VALUE_TYPES` for a list of all types
    const char *default_input;             // fallback string parsed according to the argument's value_type if the user does not provide a value
    bool is_list;                          // if true, this positional consumes multiple values into a `clags_list_t` of the `value_type`
    bool optional;                         // if true, the positional argument is optional, meaning it may be omitted
    union{                                 // only one of these should be set
        clags_custom_t *custom;            // pointer to custom type definition, only if `value_type` == `Clags_Custom`
        clags_choices_t *choices;          // pointer to the choice wrapper, only if `value_type` == `Clags_Choice`
        clags_subcmds_t *subcmds;          // pointer to subcommand definitions, only if `value_type` == `Clags_Subcmd`
        clags_range_t *range;              // pointer to range definition, only if `value_type` is `Clags_Int`, `Clags_UInt` or `Clags_Real`
        void *_data;                       // internal, do not touch
    } using;
} clags_positional_t;

// the definition of an option argument, construct with `clags_option()`
typedef struct{
    char short_flag;                       // single-character flag (e.g. 'o' for -o), '\0' if none
    const char *long_flag;                 // full-length flag (e.g. "output" for --output), NULL if none
    void *variable;                        // pointer to store the parsed value at; type must match `value_type`, or `clags_list_t` of that type if `is_list` is set
    const char *arg_name;                  // name shown in usage for the option's value (e.g. "FILE")
    const char *description;               // help text describing the option
    // options
    clags_value_type_t value_type;         // type of the option's value. See `CLAGS__VALUE_TYPES` for a list of all types
    const char* default_input;             // fallback string parsed according to the argument's value_type if the user does not provide a value
    bool is_list;                          // if true, each occurrence appends one value to a clags_list_t
    bool inherit;                          // if true, each child config will have access to this option as well
    union{                                 // only one of these should be set
        clags_custom_t *custom;            // pointer to custom type definition, only if `value_type` == `Clags_Custom`
        clags_choices_t *choices;          // pointer to the choice wrapper, only if `value_type` == `Clags_Choice`
        clags_range_t *range;              // pointer to range definition, only if `value_type` is `Clags_Int`, `Clags_UInt` or `Clags_Real`
        void *_data;                       // internal, do not touch
    } using;
} clags_option_t;

// the definition of a flag argument, construct with `clags_flag()`
typedef struct{
    char short_flag;                       // single-character flag (e.g. 'h' for -h), '\0' if none
    const char *long_flag;                 // full-length flag (e.g. "help" for --help), NULL if none
    void *variable;                        // pointer to store the flag value; type depends on `.type`
    const char *description;               // help text describing the flag
    // options
    bool exit;                             // true if parsing should exit immediately when the flag occurs
    bool inherit;                          // if true, each child config will have access to this flag as well
    clags_flag_type_t type;                // behavior of the flag; see `clags_flag_type_t` (BoolFlag, CountFlag, ConfigFlag)
} clags_flag_t;

// the definition of the argument types
typedef enum{
    Clags_Positional, // a positional argument parsed in the order defined in the clags_arg_t array
    Clags_Option,     // an unpositional short ('-o') and long ('--output') flag with a value
    Clags_Flag,       // an unpositional short ('-o') and long ('--output') flag
} clags_arg_type_t;

// a wrapper for all arg types
// automatically construct with `clags_positional()`, `clags_option()` and `clags_flag()` macros
// a `clags_arg_t` array can then be passed to `clags_config` to define a (sub)command's arguments
typedef struct{
    clags_arg_type_t type;
    union{
        clags_positional_t pos;
        clags_option_t opt;
        clags_flag_t flag;
    } as;
} clags_arg_t;

// the available config options
typedef struct{
    const char *ignore_prefix;        // a custom prefix that instructs the parser to ignore arguments starting with this prefix
    clags_list_t *ignored_args;       // append all ignored arguments to this list without the `ignore_prefix`
    const char *list_terminator;      // a custom list terminator that tells the parser that following positional arguments do no longer belong to the current list
    bool print_no_notes;              // do not print the `Notes` section in the usage
    bool allow_option_parsing_toggle; // allow "--" to be used to toggle option and flag parsing, one-time disabling is always enabled
    bool duplicate_strings;           // duplicate all strings instead of setting variables to the content of argv, free the allocated memory via `clags_config_free_allocs`
    clags_log_handler_t log_handler;  // a custom log handler
    clags_log_level_t min_log_level;  // the minimal log level for which to print logs
    const char *description;          // a description of the current (sub)command
    bool no_inheritance;              // do not inherit any option or flags from parent configs
    size_t min_usage_alignment;       // the minimum column in which ':' appears in a usage line for this config, if not specified use `CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT`
    size_t max_usage_alignment;       // the maximum column in which ':' appears in a usage line for this config, if not specified use `CLAGS_DEFAULT_MAX_USAGE_ALIGNMENT`
    void *_reserved;
} clags_options_t;

// definition of a config's error
typedef struct{
    clags_error_type_t type;          // the error type that occured
    struct{                           // this is only populated certain error types. Check with `CLAGS_ERROR_HAS_ARG(type)`.
        clags_arg_t *pointer;         // a pointer to the arg that failed
        clags_config_t *config;       // a pointer to the config to which the arg belongs. Due to inheritance this may be different to the failed config returned by `clags_parse`
    } arg;
} clags_error_t;

// the definition of config states
typedef enum {
    Clags_Config_Unvalidated = 0,     // config has not been validated, yet
    Clags_Config_Valid,               // config has been successfully validated
    Clags_Config_Invalid,             // config is invalid
} clags_config_state_t;

// the parsing context for a single (sub)command.
// construct with the `clags_config()` macro.
struct clags_config_t{
    // set by the user
    clags_arg_t *args;                // the array of argument definitions
    size_t args_count;                // the amount of argument definitions
    clags_options_t options;          // additional settings

    // internal, set automatically by validation and parsing
    const char *name;                 // the program name or the name of the current subcommand
    clags_config_t *parent;           // pointer to the parent (sub)command's config
    clags_config_state_t state;       // the validation state of the config
    clags_list_t allocs;              // all duplicated strings allocated in this config's context, only if `options.duplicate_strings` is enabled
    clags_error_t error;              // the last error detected while parsing this config
    size_t inherit_options_count;     // the number of options that can be inherited by child configs
    size_t inherit_flags_count;       // the number of flags that can be inherited by child configs
    size_t usage_alignment;           // the column in which ':' appears in a usage line for this config
};

// helper macros
#define clags_arr_len(arr) ((arr) != NULL? sizeof(arr)/sizeof((arr)[0]) : 0)
#define clags_return_defer(value) do{result = (value); goto defer;}while(0) // set the `result` variable and jump to the `defer` label
#define clags_assert(expr, msg)  do{if(!(expr)){fprintf(stderr, "%s:%d: in %s: [FATAL] "msg"\n", __FILE__, __LINE__, __func__); fflush(stderr); CLAGS_PANIC;}}while(0)
#define clags_unreachable(msg) do{fprintf(stderr, "%s:%d in %s: [UNREACHABLE]: "msg"\n", __FILE__, __LINE__, __func__); fflush(stderr); CLAGS_PANIC;}while(0)

/* Struct Constructors */

/*
 NOTE: These convenience macros use variadic macros to allow optional
       designated initializers. ISO C99 does not support invoking variadic
       macros without a variadic argument, so when compiling with
       strict -pedantic diagnostics you must provide a trailing comma when
       no optional fields are specified.

       e.g. `clags_positional(&input_file, "input_file", "the input file",)`
                                                                         ^
                                                                         |
                                                             required comma

       Alternatively, the argument and configuration structs can always be
       constructed manually.
*/

// predefined list constructors for different types
// returns an empty `clags_list_t` with item_size set appropriately
// use `clags_custom_list(size)` to define a list of custom element size
#define clags__typed_list(vtype, vsize, cset) {.items=NULL, .count=0, .capacity=0, .value_type=(vtype), .item_size=(vsize), .custom_size_set=(cset)}
#define clags_list(type)        clags__typed_list((type), 0, false) // cannot be used with `Clags_Custom`
#define clags_custom_list(size) clags__typed_list(Clags_Custom, (size), true)

// macros for easy value extraction from lists
// `value_type` must match the type stored within the list
#define clags_list_element(list, value_type, index) ((value_type*)(list).items)[index]

// wrapper for a `clags_choice_t` array
// use designated initializers in the variadic arguments to set additional options.
// see `clags_choices_t` for all available fields
#define clags_choices(arr, ...) {.items=(arr), .count=clags_arr_len(arr), __VA_ARGS__}
// macro for getting the pointer to the index-th choice
#define clags_choice_value(choices, index) (&(choices)[index])

// wrapper for a `clags_subcmd_t` array
#define clags_subcmds(subcmds) {.items=(subcmds), .count=clags_arr_len(subcmds)}

#ifndef CLAGS_NO_INT_TYPES
  #define clags_int_range(from, to) {.type=Clags_Int, .min.as_int=(from), .max.as_int=(to)}
  #define clags_uint_range(from, to) {.type=Clags_UInt, .min.as_uint=(from), .max.as_uint=(to)}
#endif // !CLAGS_NO_INT_TYPES
#ifndef CLAGS_NO_REAL_TYPES
  #define clags_real_range(from, to) {.type=Clags_Real, .min.as_double=(from), .max.as_double=(to)}
#endif // !CLAGS_NO_REAL_TYPES

// macro for a `clags_custom_t` struct
#define clags_custom(type_name, verifier, ...) {.name=(type_name), .verify=(verifier), __VA_ARGS__}

// macro for a `clags_error_t` struct
#define clags_error(type, arg, config) (clags_error_t){(type), {(arg), (config)}}

/* Argument Constructors */

/*
  Define a positional argument.

  Parameters:
    var  : pointer to the variable that receives the parsed value, must be of a type matching the value type,
           or `clags_list_t` of that type if `is_list` is set, default: char*
    name : argument name shown in usage
    desc : description shown in help output

  Additional behavior (value type, lists, optionality, choices, custom verification, subcommands, etc.)
  is configured via designated initializers in the variadic arguments.
  See `clags_positional_t` for all available fields.
*/

#define clags_positional(var, name, desc, ...) {.type=Clags_Positional, .as.pos={.variable=(var), .arg_name=(name), .description=(desc), __VA_ARGS__}}

/*
  Define an option argument.

  Parameters:
    sflag : short option character (e.g. 'o' for -o), or '\0' if unused
    lflag : long option name (e.g. "output"), or NULL if unused
    var   : pointer to the variable that receives the parsed value, must be of a type matching the value type,
            or `clags_list_t` of that type if `is_list` is set, default: char*
    name  : value name shown in usage (e.g. "FILE")
    desc  : description shown in help output

  Additional behavior (value type, lists, choices, custom verification, etc.)
  is configured via designated initializers in the variadic arguments.
  See `clags_option_t` for all available fields.
*/
#define clags_option(sflag, lflag, var, name, desc, ...) {.type=Clags_Option, .as.opt={.short_flag=(sflag), .long_flag=(lflag), .variable=(var), .arg_name=(name), .description=(desc), __VA_ARGS__}}

/*
  Define a flag argument.

  Parameters:
    sflag : short flag character (e.g. 'h' for -h), or '\0' if unused
    lflag : long flag name (e.g. "help"), or NULL if unused
    var   : pointer to the variable that receives the flag value; type depends on `.type`, default: bool
    desc  : description shown in help output

  Additional behavior (exit-on-set, count flags, config pointer storage, etc.)
  is configured via designated initializers in the variadic arguments.
  See `clags_flag_t` for all available fields.
*/
#define clags_flag(sflag, lflag, var, desc, ...) {.type=Clags_Flag, .as.flag={.short_flag=(sflag), .long_flag=(lflag), .variable=(var), .description=(desc), __VA_ARGS__}}

// simple helpers for the common help flags
#define clags_flag_help(val)        clags_flag('h', "help", val, "print this help dialog", .exit=true, .inherit=true)
#define clags_flag_help_config(val) clags_flag('h', "help", val, "print the help dialog for this subcommand", .exit=true, .type=Clags_ConfigFlag, .inherit=true)

/* Config Constructors */

/*
  Construct a `clags_config_t` from an array of arguments.

  Parameters:
    arguments : array of `clags_arg_t` defining the positionals, options, and flags
    ...       : optional designated initializers for `clags_options_t` fields
                (e.g. .ignore_prefix="!", .list_terminator="::", .duplicate_strings=true, etc.),
                see `clags_options_t` for all available fields.
*/
#define clags_config(arguments, ...) {.args=(arguments), .args_count=clags_arr_len(arguments), .allocs={0}, .options={._reserved=NULL, __VA_ARGS__}}

/* Core Functions */

/*
  Parse arguments based on the provided config.

  Calling this on a root configuration automatically validates and initializes
  the config itself, links all nested sub-configs, and validates all default
  values across the entire tree.

  Arguments:
    - argc          : the number of arguments
    - argv          : the array of arguments
    - config        : pointer to the root config

  Returns:
    clags_config_t* : pointer to the failed config or `NULL` on success. If parsing
                      fails, the `.error` field will indicate the type.
*/
CLAGS_API clags_config_t* clags_parse(int argc, char *argv[], clags_config_t *config);

/*
  Print a detailed usage menu based on the provided config.

  Note: If you need to print the usage of a nested sub-command BEFORE the root
  configuration has been initialized (via `clags_parse`), you must first manually
  run `clags_validate` on the root config to establish the upward parent paths.

  Arguments:
    - program_name  : the name of the program
    - config        : pointer to the config to print usage for
*/
CLAGS_API void clags_usage(const char *program_name, clags_config_t *config);

/*
  Print a detailed usage menu based on the provided config to a specified
  output file stream.

  This behaves identically to `clags_usage` but allows explicit redirection of
  the menu output to streams such as `stdout`, `stderr`, or a custom file handle.

  Arguments:
    - file          : the target output file stream
    - program_name  : the name of the program
    - config        : pointer to the config to print usage for
*/
CLAGS_API void clags_usage_to_file(FILE *file, const char *program_name, clags_config_t *config);

/*
  Manually initialize and validate a configuration tree.

  This traverses the root config to link parent paths and verify that all
  schema rules (like default values matching bounds) are valid. You only need
  to call this manually if you intend to interact with a nested sub-config
  before the root config has been initialized by `clags_parse`.

  Arguments:
    - program_name  : the name of the program
    - config        : pointer to the root config to validate

  Returns:
    clags_config_t* : pointer to the failed config or `NULL` on success.
*/
CLAGS_API clags_config_t* clags_validate(const char *program_name, clags_config_t *config);

/*
  Get the index of a selected subcommand in the provided subcommand array.

  Arguments:
    - subcmds       : pointer to a clags_subcmds_t containing the subcommand array
    - subcmd        : pointer to the selected clags_subcmd_t to find

  Returns:
    int             : the index of the selected subcommand in the array,
                      or -1 if the subcommand was not found or either argument is NULL
*/
CLAGS_API int clags_subcmd_index(clags_subcmds_t *subcmds, clags_subcmd_t *subcmd);

/*
  Get the index of a selected choice in the provided choice array.

  Arguments:
    - choices       : pointer to clags_choices_t containing the choice array
    - choice        : pointer to the selected clags_choice_t to find

  Returns:
    int             : the index of the selected choice in the array,
                      or -1 if the choice was not found or either argument is NULL
*/
CLAGS_API int clags_choice_index(clags_choices_t *choices, clags_choice_t *choice);

/*
  Duplicate some sized data if string duplication is enabled in the config,
  otherwise return the original data. Usually these should still be string slices,
  but it will work with any data type.

  Arguments:
    - config  : pointer to the clags configuration
    - data    : the data to duplicate
    - size    : the size in bytes of the data

  Returns:
    void*     : pointer to the duplicated data if duplication is enabled,
                otherwise the original data. Memory allocated for duplicates
                is tracked internally within the config and will be freed when the config is cleaned up.
*/
CLAGS_API void* clags_config_duplicate(clags_config_t *config, const void *data, size_t size);

/*
  Duplicate a string if string duplication is enabled in the config,
  otherwise return the original string.

  Arguments:
    - config  : pointer to the clags configuration
    - string  : the string to duplicate

  Returns:
    char*     : pointer to the duplicated string if duplication is enabled,
                otherwise the original string. Memory allocated for duplicates
                is tracked internally within the config and will be freed when the config is cleaned up.
*/
CLAGS_API char* clags_config_duplicate_string(clags_config_t *config, const char *string);

/*
  Free all memory associated with a `clags_list_t` instance.

  Arguments:
    - list          : a pointer to the list to free
*/
CLAGS_API void clags_list_free(clags_list_t *list);

/*
  Free all memory allocated for strings duplicated during parsing.
  This only applies if `.duplicate_strings` was enabled in the config.

  Arguments:
    - config        : pointer to the clags_config_t whose duplicated strings should be freed
*/
CLAGS_API void clags_config_free_allocs(clags_config_t *config);

/*
  Free all lists and allocated strings of a config.
  The function does not propagate to child configs.

  Arguments:
    - config        : pointer to the config of which to free all strings and lists
*/
CLAGS_API void clags_config_free(clags_config_t *config);

/*
  Free all lists and allocated strings of a config and all its child configs.

  Arguments:
    - config        : pointer to the root config to free along with all its child configs
*/
CLAGS_API void clags_free(clags_config_t *config);

/*
  Return a description of the provided error type.

  Arguments:
    - error         : the error type

  Returns:
    const char*     : a string description of the provided error type
*/
CLAGS_API const char* clags_error_description(clags_error_type_t error);

#ifndef CLAGS_NO_PATH_TYPES

/*
  Return the type of a path.

  Arguments:
    - path            : the path to check

  Returns:
    clags_path_type_t : the type of the provided path. See `clags__path_types` for a list of all available types.
*/
CLAGS_API clags_path_type_t clags_path_type(const char *path);

/*
  Return the name of the provided path type.

  Arguments:
    - type          : the path type

  Returns:
    const char*     : the name of the provided type
*/
CLAGS_API const char* clags_path_type_name(clags_path_type_t type);

/*
  Returns whether a path exists.

  Arguments:
    - type          : the path type

  Returns:
    bool            : the existance of the path
*/
CLAGS_API bool clags_path_exists(const char *path);

/*
  Returns whether a path type means that the path exists.

  Arguments:
    - type          : the path type

  Returns:
    bool            : the existance of the path
*/
CLAGS_API bool clags_path_exists_by_type(clags_path_type_t type);

#endif // CLAGS_NO_PATH_TYPES

/* Logging */

/*
  Use a config's log handler to log a formatted message.

  Arguments:
    - config        : the config for which to log using its options
    - level         : the log level of the message, if less than the configs minimal log level, nothing will be logged
    - format        : the printf-style format of the message
    - ...           : the variadic format arguments
*/
CLAGS_API void clags_log(clags_config_t *config, clags_log_level_t level, const char *format, ...) CLAGS__PRINTF_FORMAT(3, 4);
// log the content of a string builder instead of a formatted message
CLAGS_API void clags_log_sb(clags_config_t *config, clags_log_level_t level, clags_sb_t *sb);

/*
  Set the log level of a config and all its children.

  Arguments:
    - config        : the root config for which to set the log level
    - level         : the log level to set
*/
CLAGS_API void clags_propagate_log_level(clags_config_t *config, clags_log_level_t level);

/* String Builder */
CLAGS_API int clags_sb_appendf(clags_sb_t *sb, const char *format, ...) CLAGS__PRINTF_FORMAT(2, 3);
CLAGS_API void clags_sb_append_null(clags_sb_t *sb);
CLAGS_API void clags_sb_free(clags_sb_t *sb);

/* String-to-Integer Conversion */
#if !defined(CLAGS_NO_INT_TYPES) || !defined(CLAGS_NO_SIZE_TYPES) || !defined(CLAGS_NO_STRTOINT)
// strtoint.h - Public Domain - https://github.com/fietec/strtoint.h

// These are the functions used internally to convert strings to integers.
// You can use these functions when the existing verifiers
// are too strict (e.g. forcing '\0' at the end) or too verbose since they
// use `clags_log` to log errors

// the definition of the result struct of integer parsing
typedef struct{
    const char *endptr;  // pointer to the first unparsed character
    bool negative;       // input had a leading '-' sign
    bool leading_spaces; // input had leading whitespaces

    // error flags
    bool no_digits;      // no valid digits were found
    bool out_of_range;   // value exceeded the representable range of target type
    bool invalid_params; // the supplied parameters were invalid (e.g. base not 0 or 2-36, min > max etc.)
} clags_strtoint_res_t;

// legacy functions: sets `errno` similar to `strtol` family functions
CLAGS_API int8_t  clags_strtoint8 (const char *str, char **endptr, int base);
CLAGS_API int16_t clags_strtoint16(const char *str, char **endptr, int base);
CLAGS_API int32_t clags_strtoint32(const char *str, char **endptr, int base);
CLAGS_API int64_t clags_strtoint64(const char *str, char **endptr, int base);
CLAGS_API int64_t clags_strtoint_custom(const char *str, char **endptr, int base, int64_t min, int64_t max);

CLAGS_API uint8_t  clags_strtouint8 (const char *str, char **endptr, int base);
CLAGS_API uint16_t clags_strtouint16(const char *str, char **endptr, int base);
CLAGS_API uint32_t clags_strtouint32(const char *str, char **endptr, int base);
CLAGS_API uint64_t clags_strtouint64(const char *str, char **endptr, int base);
CLAGS_API uint64_t clags_strtouint_custom(const char *str, char **endptr, int base, uint64_t min, uint64_t max);

// safe/structured functions: returns the parsing result via a `clags_strtoint_res_t` struct
CLAGS_API int8_t  clags_strtoint8_s (const char *str, clags_strtoint_res_t *res, int base);
CLAGS_API int16_t clags_strtoint16_s(const char *str, clags_strtoint_res_t *res, int base);
CLAGS_API int32_t clags_strtoint32_s(const char *str, clags_strtoint_res_t *res, int base);
CLAGS_API int64_t clags_strtoint64_s(const char *str, clags_strtoint_res_t *res, int base);
CLAGS_API int64_t clags_strtoint_custom_s(const char *str, clags_strtoint_res_t *res, int base, int64_t min, int64_t max);

CLAGS_API uint8_t  clags_strtouint8_s (const char *str, clags_strtoint_res_t *res, int base);
CLAGS_API uint16_t clags_strtouint16_s(const char *str, clags_strtoint_res_t *res, int base);
CLAGS_API uint32_t clags_strtouint32_s(const char *str, clags_strtoint_res_t *res, int base);
CLAGS_API uint64_t clags_strtouint64_s(const char *str, clags_strtoint_res_t *res, int base);
CLAGS_API uint64_t clags_strtouint_custom_s(const char *str, clags_strtoint_res_t *res, int base, uint64_t min, uint64_t max);

#endif // !CLAGS_NO_STRTOINT

#endif // CLAGS_H

/*
  The start of the implementation section.
  Define `CLAGS_IMPLEMENTATION` to access, otherwise this file will act as a regular header.
*/
#ifdef CLAGS_IMPLEMENTATION

#include <string.h>
#include <float.h>
#include <ctype.h>
#include <inttypes.h>

typedef struct{
    clags_verify_func_t verify;
    const char *name;
    size_t value_size;
    clags_validate_func_t validate;
    clags_usage_func_t usage;
} clags__type_t;

// Automatically generated declarations of all verifiers
#define X(type, verify_func, name, vtype, validate_func, usage_func) CLAGS_API clags_verify_func_def_t verify_func;
CLAGS__VALUE_TYPES
#undef X

// Automatically generated declarations of all validators
#define X(type, verify_func, name, vtype, validate_func, usage_func) CLAGS_PRIV clags_validate_func_def_t validate_func;
CLAGS__VALUE_TYPES
#undef X

// Automatically generated declarations of all usage printers
#define X(type, verify_func, name, vtype, validate_func, usage_func) CLAGS_PRIV clags_usage_func_def_t usage_func;
CLAGS__VALUE_TYPES
#undef X

// Automatically generated declaration of all flag setters
#define X(type, setter) CLAGS_PRIV clags_flag_func_def_t setter;
CLAGS__FLAG_TYPES
#undef X

/* Automatically generated lookup array for value types */
#define X(type, verify_func, name, vtype, validate_func, usage_func) [type] = {verify_func, name, sizeof(vtype), validate_func, usage_func},
static clags__type_t clags__types[] = {
    CLAGS__VALUE_TYPES
};
#undef X

/* String Utilities */

CLAGS_PRIV int clags__tolower(int c)
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

CLAGS_PRIV int clags__strcasecmp(const char *s1, const char *s2)
{
    if (s1 == s2) return 0;
    if (s1 == NULL) return -1;
    if (s2 == NULL) return 1;

    while (*s1 && *s2) {
        int c1 = clags__tolower((unsigned char)*s1);
        int c2 = clags__tolower((unsigned char)*s2);

        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return clags__tolower((unsigned char)*s1) - clags__tolower((unsigned char)*s2);
}

#ifndef CLAGS_NO_TIME_TYPES
CLAGS_PRIV int clags__strslice_casecmp(const char *slice, size_t slice_len, const char *exact)
{
    if (slice == exact) return 0;
    if (slice == NULL) return -1;
    if (exact == NULL) return 1;

    while (slice_len > 0) {
        if (*exact == '\0')  return clags__tolower((unsigned char)*slice) - '\0';
        if (*slice == '\0')  return '\0' - clags__tolower((unsigned char)*exact);

        int c1 = clags__tolower((unsigned char)*slice);
        int c2 = clags__tolower((unsigned char)*exact);

        if (c1 != c2) return c1 - c2;

        slice++;
        exact++;
        slice_len--;
    }

    return '\0' - clags__tolower((unsigned char)*exact);
}
#endif // !CLAGS_NO_TIME_TYPES

/* String Builder Functionality */

CLAGS_PRIV size_t clags__sb_reserve(clags_sb_t *sb, size_t nbytes)
{
    if (sb->count > SIZE_MAX - nbytes){
        nbytes = SIZE_MAX - sb->count;
    }
    size_t required_capacity = sb->count + nbytes;
    if (sb->capacity >= required_capacity) return nbytes;

    size_t old_capacity = sb->capacity;
    size_t new_capacity = (sb->capacity > 0)? sb->capacity : CLAGS_SB_INIT_CAPACITY;
    while (new_capacity < required_capacity){
        if (new_capacity > SIZE_MAX / 2){
            new_capacity = required_capacity;
            break;
        }
        new_capacity *= 2;
    }

    char *new_items = CLAGS_REALLOC(sb->items, old_capacity, new_capacity);
    clags_assert(new_items != NULL, "Out of memory!");

    sb->items = new_items;
    sb->capacity = new_capacity;
    (void) old_capacity; // to prevent `Wunused-variable`
    return nbytes;
}

CLAGS_API int clags_sb_appendf(clags_sb_t *sb, const char *format, ...)
{
    va_list args, args_copy;

    va_start(args, format);
    va_copy(args_copy, args);

    int n = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (sb != NULL && n >= 0){
        size_t reserved = clags__sb_reserve(sb, n + 1);
        if (reserved > 0){
            char *start = sb->items + sb->count;
            int w = vsnprintf(start, reserved, format, args);
            if (w > 0) sb->count += reserved - 1;
        }
    }
    va_end(args);
    return n;
}

CLAGS_API void clags_sb_append_null(clags_sb_t *sb)
{
    if (sb == NULL) return;
    if (sb->count > 0 && sb->items[sb->count - 1] == '\0') return;
    clags__sb_reserve(sb, 1);
    sb->items[sb->count++] = '\0';
}

CLAGS_API void clags_sb_free(clags_sb_t *sb)
{
    if (sb == NULL) return;
    CLAGS_FREE(sb->items, sb->capacity*sizeof(*sb->items));
    sb->items = NULL;
    sb->count = sb->capacity = 0;
}

/* Logging */

CLAGS_PRIV void clags__default_log_handler(clags_config_t *config, clags_log_level_t level, const char *format, va_list args)
{
    switch(level){
        case Clags_Info:{
            fprintf(stdout, "[INFO] ");
            vfprintf(stdout, format, args);
            fprintf(stdout, "\n");
            return;
        }break;
        case Clags_Warning:{
            fprintf(stderr, "[WARNING] ");
        }break;
        case Clags_Error:{
            fprintf(stderr, "[ERROR] ");
        }break;
        case Clags_ConfigWarning:{
            fprintf(stderr, "[CONFIG_WARNING] in '%s': ", config->name);
        }break;
        case Clags_ConfigError:{
            fprintf(stderr, "[CONFIG_ERROR] in '%s': ", config->name);
        }break;
        case Clags_NoLogs: return;
        default:{
            clags_unreachable("Invalid clags_log_level_t!");
        }
    }
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

CLAGS_API void clags_log(clags_config_t *config, clags_log_level_t level, const char *format, ...)
{
    // skip logging of message based on the set mininum log level
    if (config && config->options.min_log_level > level) return;

    va_list args;
    va_start(args, format);

    // call the correct log handler
    clags_log_handler_t handler = (config && config->options.log_handler)? config->options.log_handler : clags__default_log_handler;
    handler(config, level, format, args);

    va_end(args);
}

CLAGS_API void clags_log_sb(clags_config_t *config, clags_log_level_t level, clags_sb_t *sb)
{
    if (sb != NULL && sb->items != NULL){
        clags_log(config, level, "%.*s", (int) sb->count, sb->items);
    }
}

CLAGS_API void clags_propagate_log_level(clags_config_t *config, clags_log_level_t level)
{
    if (config == NULL) return;
    // set the log level
    config->options.min_log_level = level;

    // propagate to child configs
    for (size_t i=0; i<config->args_count; ++i){
        clags_arg_t arg = config->args[i];

        // find a positional subcommand definition
        if (arg.type != Clags_Positional) continue;
        clags_positional_t pos = arg.as.pos;
        if (pos.value_type != Clags_Subcmd || pos.using.subcmds == NULL) continue;

        clags_subcmds_t *subcmds = arg.as.pos.using.subcmds;
        for (size_t j=0; j<subcmds->count; ++j){
            // set the log level for each subcommand's config
            clags_propagate_log_level(subcmds->items[j].config, level);
        }
    }
}

/* Allocation Handling */

typedef struct{
    void *data;
    size_t size;
} clags__alloc_t;

CLAGS_PRIV clags__alloc_t clags__memdup(const void *data, size_t size)
{
    if (data == NULL || size == 0) return (clags__alloc_t){NULL, 0};
    void *new_data = CLAGS_CALLOC(size, 1);
    clags_assert(new_data != NULL, "Out of memory!");
    (void) memcpy(new_data, data, size);
    return (clags__alloc_t) {new_data, size};
}

CLAGS_PRIV bool clags__handle_list_resizing(clags_config_t *config, clags_list_t *list)
{
    if (list->count < list->capacity) return true;
    size_t item_size;
    if (list->value_type == Clags_Custom){
        item_size = list->item_size;
    } else{
        item_size = clags__types[list->value_type].value_size;
    }
    clags_assert(item_size > 0, "list with an item_size of 0");
    size_t max_capacity = SIZE_MAX / item_size;
    if (list->capacity >= max_capacity){
        clags_log(config, Clags_Error, "maximum list capacity %zu already reached. Unable to add further arguments to the list!", max_capacity);
        return false;
    }

    size_t new_capacity;
    if (list->capacity > 0){
        if (list->capacity > max_capacity / 2){
            new_capacity = max_capacity;
        } else{
            new_capacity = list->capacity * 2;
        }
    } else{
        new_capacity = CLAGS_LIST_INIT_CAPACITY;
        if (new_capacity > max_capacity) new_capacity = max_capacity;
    }

    size_t old_bytes = list->capacity*item_size;
    size_t new_bytes = new_capacity*item_size;
    void *new_items = CLAGS_REALLOC(list->items, old_bytes, new_bytes);
    clags_assert(new_items != NULL, "Out of memory!");

    list->items = new_items;
    list->capacity = new_capacity;
    (void) old_bytes;
    return true;
}

CLAGS_API void* clags_config_duplicate(clags_config_t *config, const void *data, size_t size)
{
    void *duplicate;
    if (config && config->options.duplicate_strings){
        // duplicate size-bytes
        clags__alloc_t alloc = clags__memdup(data, size);

        // register the allocation in the config's allocation list
        clags_list_t *allocs = &config->allocs;
        allocs->value_type = Clags_Custom;
        allocs->item_size = sizeof(clags__alloc_t);

        clags_assert(clags__handle_list_resizing(config, allocs), "allocation context overflown!");

        ((clags__alloc_t*) allocs->items)[allocs->count++] = alloc;
        // return the duplicated data instead
        duplicate = (void*) alloc.data;
    } else{
        // by default we simply return the same pointer back
        duplicate = (void*) data;
    }
    return duplicate;
}

CLAGS_API char* clags_config_duplicate_string(clags_config_t *config, const char *string)
{
    if (string == NULL) return NULL;
    return clags_config_duplicate(config, string, strlen(string) + 1);
}

/* Utility Functions */

#ifndef CLAGS_NO_PATH_TYPES

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/stat.h>
#endif

CLAGS_API clags_path_type_t clags_path_type(const char *path)
{
#ifdef _WIN32
    int wlen = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    if (wlen <= 0) return Clags_Path_Invalid;

    WCHAR stack_buf[512];
    WCHAR *wpath = stack_buf;
    int use_heap = (wlen > 512);
    if (use_heap){
        wpath = (WCHAR *)malloc(wlen * sizeof(WCHAR));
        if (wpath == NULL) return Clags_Path_Invalid;
    }

    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, wlen);

    DWORD attrs = GetFileAttributesW(wpath);

    if (use_heap) {
        free(wpath);
    }

    if (attrs == INVALID_FILE_ATTRIBUTES) return Clags_Path_Invalid;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) return Clags_Path_Dir;
    if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) return Clags_Path_Other;
    return Clags_Path_Reg;
#else
    struct stat attrs;
    if (stat(path, &attrs) == -1) return Clags_Path_Invalid;
    if (S_ISDIR(attrs.st_mode)) return Clags_Path_Dir;
    if (S_ISREG(attrs.st_mode)) return Clags_Path_Reg;
    return Clags_Path_Other;
#endif
}

CLAGS_API const char* clags_path_type_name(clags_path_type_t type)
{
    switch (type){
#define X(type, name, exist) case type: return name;
        CLAGS__PATH_TYPES
#undef X
        default: return "unknown file type";
    }
}

CLAGS_API bool clags_path_exists_by_type(clags_path_type_t type)
{
    switch (type){
#define X(type, name, exist) case type: return exist;
        CLAGS__PATH_TYPES
#undef X
        default: return false;
    }
}

CLAGS_API bool clags_path_exists(const char *path)
{
    return clags_path_exists_by_type(clags_path_type(path));
}

#endif // !CLAGS_NO_PATH_TYPES

CLAGS_API const char* clags_error_description(clags_error_type_t error)
{
    switch(error){
#define X(type, desc) case (type): return (desc);
        CLAGS__ERROR_TYPES
#undef X
        default: return "unknown error";
    }
}

CLAGS_PRIV int clags__array_index(void *array, size_t array_count, void *elem, size_t elem_size)
{
    if (!array || !elem || elem_size == 0) return -1;
    uintptr_t array_addr = (uintptr_t) array;
    uintptr_t elem_addr = (uintptr_t) elem;
    if (array_addr <= elem_addr && elem_addr < array_addr + array_count * elem_size){
        if ((elem_addr - array_addr) % elem_size == 0) {
            return (elem_addr - array_addr) / elem_size;
        }
    }
    return -1;
}

CLAGS_API int clags_subcmd_index(clags_subcmds_t *subcmds, clags_subcmd_t *subcmd)
{
    return clags__array_index(subcmds->items, subcmds->count, subcmd, sizeof(*subcmd));
}

CLAGS_API int clags_choice_index(clags_choices_t *choices, clags_choice_t *choice)
{
    return clags__array_index(choices->items, choices->count, choice, sizeof(*choice));
}

CLAGS_API int clags_arg_index(clags_config_t *config, clags_arg_t *arg)
{
    return clags__array_index(config->args, config->args_count, arg, sizeof(*arg));
}

/* String-to-Integer Conversion */
#if !defined(CLAGS_NO_INT_TYPES) || !defined(CLAGS_NO_SIZE_TYPES) || !defined(CLAGS_NO_STRTOINT)

CLAGS_PRIV int clags_strtoint__digit_value(char c, int base)
{
    int value = -1;
    if ('0' <= c && c <= '9') value = c - '0';
    else if ('A' <= c && c <= 'Z') value = c - 'A' + 10;
    else if ('a' <= c && c <= 'z') value = c - 'a' + 10;
    if (0 <= value && value < base) return value;
    return -1;
}

CLAGS_PRIV int clags_strtoint__handle_base(const char **str, int base)
{
    const char *s = *str;
    if ((base == 0 || base == 2) && s[0] == '0' && (s[1] == 'B' || s[1] == 'b')){
        if (clags_strtoint__digit_value(s[2], 2) != -1) {
            base = 2;
            *str += 2;
        } else if (base == 0) {
            base = 8;
        }
    } else if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'X' || s[1] == 'x')){
        if (clags_strtoint__digit_value(s[2], 16) != -1) {
            base = 16;
            *str += 2;
        } else if (base == 0) {
            base = 8;
        }
    } else if ((base == 0 || base == 8) && s[0] == '0'){
        base = 8;
    } else if (base == 0){
        base = 10;
    }
    return base;
}

CLAGS_API int64_t clags_strtoint_custom_s(const char *str, clags_strtoint_res_t *res, int base, int64_t min, int64_t max)
{
    if (!str || min > max || base < 0 || base == 1 || base > 36){
        if (res){
            res->endptr = str;
            res->negative = false;
            res->leading_spaces = false;
            res->no_digits = false;
            res->out_of_range = false;
            res->invalid_params = true;
        }
        return 0;
    }
    const char *start = str;
    while (isspace((unsigned char)*start)) start++;

    bool leading_spaces = (start != str);
    bool negative = false;
    if (*start == '-'){
        negative = true;
        start++;
    } else if (*start == '+'){
        start++;
    }

    base = clags_strtoint__handle_base(&start, base);

    bool out_of_range = false;
    int64_t cutoff = INT64_MIN / base;
    int64_t cutoff_digit = -(INT64_MIN % base);
    const char *pdigit = start;
    int digit;
    int64_t result = 0;
    while ((digit = clags_strtoint__digit_value((unsigned char)*pdigit, base)) != -1){
        pdigit++;
        if (out_of_range || (result == 0 && digit == 0)) continue;
        if (result < cutoff || (result == cutoff && digit > cutoff_digit)){
            result = negative? min : max;
            out_of_range = true;
            continue;
        }
        result = result * base - digit;
    }

    if (pdigit == start){
        if (res){
            res->endptr = str;
            res->negative = negative;
            res->leading_spaces = leading_spaces;
            res->no_digits = true;
            res->out_of_range = false;
            res->invalid_params = false;
        }
        return 0;
    }
    if (!out_of_range){
        if (!negative){
            if (result < -INT64_MAX){
                result = max;
                out_of_range = true;
            } else{
                result = -result;
            }
        }
        if (result > max){
            result = max;
            out_of_range = true;
        } else if (result < min){
            result = min;
            out_of_range = true;
        }
    }

    if (res){
        res->endptr = pdigit;
        res->negative = negative;
        res->leading_spaces = leading_spaces;
        res->no_digits = false;
        res->out_of_range = out_of_range;
        res->invalid_params = false;
    }
    return result;
}

CLAGS_API int64_t clags_strtoint_custom(const char *str, char **endptr, int base, int64_t min, int64_t max)
{
    clags_strtoint_res_t res;
    int64_t result = clags_strtoint_custom_s(str, &res, base, min, max);
    if (res.invalid_params) errno = EINVAL;
    else if (res.out_of_range) errno = ERANGE;
    if (endptr) *endptr = (char*) res.endptr;
    return result;
}

CLAGS_API int8_t clags_strtoint8(const char *str, char **endptr, int base)
{
    return (int8_t)clags_strtoint_custom(str, endptr, base, INT8_MIN, INT8_MAX);
}

CLAGS_API int8_t clags_strtoint8_s(const char *str, clags_strtoint_res_t *res, int base)
{
    return (int8_t)clags_strtoint_custom_s(str, res, base, INT8_MIN, INT8_MAX);
}

CLAGS_API int16_t clags_strtoint16(const char *str, char **endptr, int base)
{
    return (int16_t)clags_strtoint_custom(str, endptr, base,INT16_MIN, INT16_MAX);
}

CLAGS_API int16_t clags_strtoint16_s(const char *str, clags_strtoint_res_t *res, int base)
{
    return (int16_t)clags_strtoint_custom_s(str, res, base, INT16_MIN, INT16_MAX);
}

CLAGS_API int32_t clags_strtoint32(const char *str, char **endptr, int base)
{
    return (int32_t)clags_strtoint_custom(str, endptr, base, INT32_MIN, INT32_MAX);
}

CLAGS_API int32_t clags_strtoint32_s(const char *str, clags_strtoint_res_t *res, int base)
{
    return (int32_t)clags_strtoint_custom_s(str, res, base, INT32_MIN, INT32_MAX);
}

CLAGS_API int64_t clags_strtoint64(const char *str, char **endptr, int base)
{
    return clags_strtoint_custom(str, endptr, base, INT64_MIN, INT64_MAX);
}

CLAGS_API int64_t clags_strtoint64_s(const char *str, clags_strtoint_res_t *res, int base)
{
    return clags_strtoint_custom_s(str, res, base, INT64_MIN, INT64_MAX);
}

CLAGS_API uint64_t clags_strtouint_custom_s(const char *str, clags_strtoint_res_t *res, int base, uint64_t min, uint64_t max)
{
    if (!str || min > max || base < 0 || base == 1 || base > 36){
        if (res){
            res->endptr = str;
            res->negative = false;
            res->leading_spaces = false;
            res->no_digits = false;
            res->out_of_range = false;
            res->invalid_params = true;
        }
        return 0;
    }
    const char *start = str;
    while (isspace((unsigned char)*start)) start++;

    bool leading_spaces = (start != str);
    bool negative = false;
    if (*start == '-'){
        negative = true;
        start++;
    } else if (*start == '+'){
        start++;
    }

    base = clags_strtoint__handle_base(&start, base);

    bool out_of_range = false;
    const char *pdigit = start;
    uint64_t result = 0;
    uint64_t udigit;
    int digit;
    while ((digit = clags_strtoint__digit_value((unsigned char)*pdigit, base)) != -1){
        udigit = (uint64_t) digit;
        pdigit++;
        if (out_of_range || (result == 0 && udigit == 0)) continue;
        if (udigit > max || result > (max - udigit) / base){
            result = max;
            out_of_range = true;
            continue;
        }
        result = result * base + udigit;
    }
    if (pdigit == start){
        if (res){
            res->endptr = str;
            res->negative = negative;
            res->leading_spaces = leading_spaces;
            res->no_digits = true;
            res->out_of_range = false;
            res->invalid_params = false;
        }
        return 0;
    }

    if (!out_of_range){
        if (negative && result > 0) {
            if (max - result + 1 < min){
                result = max;
                out_of_range = true;
            } else{
                result = max - result + 1;
            }
        } else if (result < min){
            result = min;
            out_of_range = true;
        }
    }

    if (res){
        res->endptr = pdigit;
        res->negative = negative;
        res->leading_spaces = leading_spaces;
        res->no_digits = false;
        res->out_of_range = out_of_range;
        res->invalid_params = false;
    }
    return result;
}

CLAGS_API uint64_t clags_strtouint_custom(const char *str, char **endptr, int base, uint64_t min, uint64_t max)
{
    clags_strtoint_res_t res;
    uint64_t result = clags_strtouint_custom_s(str, &res, base, min, max);
    if (res.invalid_params) errno = EINVAL;
    else if (res.out_of_range) errno = ERANGE;
    if (endptr) *endptr = (char*) res.endptr;
    return result;
}

CLAGS_API uint8_t clags_strtouint8(const char *str, char **endptr, int base)
{
    return (uint8_t)clags_strtouint_custom(str, endptr, base, 0, UINT8_MAX);
}

CLAGS_API uint8_t clags_strtouint8_s(const char *str, clags_strtoint_res_t *res, int base)
{
    return (uint8_t)clags_strtouint_custom_s(str, res, base, 0, UINT8_MAX);
}

CLAGS_API uint16_t clags_strtouint16(const char *str, char **endptr, int base)
{
    return (uint16_t)clags_strtouint_custom(str, endptr, base, 0, UINT16_MAX);
}

CLAGS_API uint16_t clags_strtouint16_s(const char *str, clags_strtoint_res_t *res, int base)
{
    return (uint16_t)clags_strtouint_custom_s(str, res, base, 0, UINT16_MAX);
}

CLAGS_API uint32_t clags_strtouint32(const char *str, char **endptr, int base)
{
    return (uint32_t)clags_strtouint_custom(str, endptr, base, 0, UINT32_MAX);
}

CLAGS_API uint32_t clags_strtouint32_s(const char *str, clags_strtoint_res_t *res, int base)
{
    return (uint32_t)clags_strtouint_custom_s(str, res, base, 0, UINT32_MAX);
}

CLAGS_API uint64_t clags_strtouint64(const char *str, char **endptr, int base)
{
    return clags_strtouint_custom(str, endptr, base, 0, UINT64_MAX);
}

CLAGS_API uint64_t clags_strtouint64_s(const char *str, clags_strtoint_res_t *res, int base)
{
    return clags_strtouint_custom_s(str, res, base, 0, UINT64_MAX);
}

#endif // !CLAGS_NO_STRTOINT

/* Value Type Verifiers */

CLAGS_API bool clags_verify_string(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    (void) arg_name;
    if (variable) *(char**)variable = clags_config_duplicate_string(config, arg);
    return true;
}

CLAGS_API bool clags_verify_bool(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    if (clags__strcasecmp(arg, "true") == 0 || clags__strcasecmp(arg, "yes") == 0 || clags__strcasecmp(arg, "y") == 0){
        if (variable) *(bool*)variable = true;
        return true;
    } else if (clags__strcasecmp(arg, "false") == 0 || clags__strcasecmp(arg, "no") == 0 || clags__strcasecmp(arg, "n") == 0){
        if (variable) *(bool*)variable = false;
        return true;
    }
    clags_log(config, Clags_Error, "Invalid boolean value for argument '%s': '%s'!", arg_name, arg);
    return false;
}

#ifndef CLAGS_NO_INT_TYPES

CLAGS_PRIV bool clags__verify_signed_int(clags_config_t *config, clags_value_type_t type, clags_range_t *range, const char *arg_name, const char *arg, int64_t *variable)
{
    int64_t min, max;
    if (range != NULL){
        min = range->min.as_int;
        max = range->max.as_int;
    } else{
        min = INT64_MIN;
        max = INT64_MAX;
    }

    clags_strtoint_res_t res;
    int64_t value = clags_strtoint_custom_s(arg, &res, 0, min, max);
    if (res.no_digits){
        clags_log(config, Clags_Error, "Invalid %s value for argument '%s': '%s'!", clags__types[type].name, arg_name, arg);
        return false;
    }
    if (*res.endptr != '\0'){
        clags_log(config, Clags_Error, "trailing characters after valid digits for %s argument '%s': '%s': '%s'!", clags__types[type].name, arg_name, arg, res.endptr);
        return false;
    }
    if (res.out_of_range){
        clags_log(config, Clags_Error, "%s out of range [%"PRId64"-%"PRId64"] for argument '%s': '%s'!", clags__types[type].name, min, max, arg_name, arg);
        return false;
    }
    if (variable) *variable = value;
    return true;
}

CLAGS_API bool clags_verify_int(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    return clags__verify_signed_int(config, Clags_Int, data, arg_name, arg, variable);
}

CLAGS_API bool clags_verify_int8(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    int64_t var;
    clags_range_t range = clags_int_range(INT8_MIN, INT8_MAX);
    bool result = clags__verify_signed_int(config, Clags_Int8, &range, arg_name, arg, &var);
    if (result && variable) *(int8_t*)variable = (int8_t)var;
    return result;
}

CLAGS_API bool clags_verify_int16(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    int64_t var;
    clags_range_t range = clags_int_range(INT16_MIN, INT16_MAX);
    bool result = clags__verify_signed_int(config, Clags_Int16, &range, arg_name, arg, &var);
    if (result && variable) *(int16_t*)variable = (int16_t)var;
    return result;
}

CLAGS_API bool clags_verify_int32(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    int64_t var;
    clags_range_t range = clags_int_range(INT32_MIN, INT32_MAX);
    bool result = clags__verify_signed_int(config, Clags_Int32, &range, arg_name, arg, &var);
    if (result && variable) *(int32_t*)variable = (int32_t)var;
    return result;
}

CLAGS_API bool clags_verify_int64(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    int64_t var;
    clags_range_t range = clags_int_range(INT64_MIN, INT64_MAX);
    bool result = clags__verify_signed_int(config, Clags_Int64, &range, arg_name, arg, &var);
    if (result && variable) *(int64_t*)variable = (int64_t)var;
    return result;
}

CLAGS_PRIV bool clags__verify_unsigned_int(clags_config_t *config, clags_value_type_t type, clags_range_t *range, const char *arg_name, const char *arg, uint64_t *variable)
{
    uint64_t min, max;
    if (range){
        min = range->min.as_uint;
        max = range->max.as_uint;
    } else {
        min = 0;
        max = UINT64_MAX;
    }

    clags_strtoint_res_t res;
    uint64_t value = clags_strtouint_custom_s(arg, &res, 0, min, max);

    if (res.no_digits) {
        clags_log(config, Clags_Error, "Invalid %s value for argument '%s': '%s'!", clags__types[type].name, arg_name, arg);
        return false;
    }
    if (*res.endptr != '\0'){
        clags_log(config, Clags_Error, "trailing characters after valid digits for %s argument '%s': '%s': '%s'!", clags__types[type].name, arg_name, arg, res.endptr);
        return false;
    }
    if (res.out_of_range || res.negative) {
        clags_log(config, Clags_Error, "%s value out of range [%"PRIu64"-%"PRIu64"] for argument '%s': '%s'!", clags__types[type].name, min, max, arg_name, arg);
        return false;
    }

    if (variable) *variable = value;
    return true;
}

CLAGS_API bool clags_verify_uint(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    return clags__verify_unsigned_int(config, Clags_UInt, data, arg_name, arg, variable);
}

CLAGS_API bool clags_verify_uint8(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    uint64_t var;
    clags_range_t range = clags_uint_range(0, UINT8_MAX);
    bool result = clags__verify_unsigned_int(config, Clags_UInt8, &range, arg_name, arg, &var);
    if (result && variable) *(uint8_t*)variable = (uint8_t) var;
    return result;
}

CLAGS_API bool clags_verify_uint16(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    uint64_t var;
    clags_range_t range = clags_uint_range(0, UINT16_MAX);
    bool result = clags__verify_unsigned_int(config, Clags_UInt16, &range, arg_name, arg, &var);
    if (result && variable) *(uint16_t*)variable = (uint16_t) var;
    return result;
}

CLAGS_API bool clags_verify_uint32(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    uint64_t var;
    clags_range_t range = clags_uint_range(0, UINT32_MAX);
    bool result = clags__verify_unsigned_int(config, Clags_UInt32, &range, arg_name, arg, &var);
    if (result && variable) *(uint32_t*)variable = (uint32_t) var;
    return result;
}

CLAGS_API bool clags_verify_uint64(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    uint64_t var;
    clags_range_t range = clags_uint_range(0, UINT64_MAX);
    bool result = clags__verify_unsigned_int(config, Clags_UInt64, &range, arg_name, arg, &var);
    if (result && variable) *(uint64_t*)variable = (uint64_t) var;
    return result;
}

#endif // !CLAGS_NO_INT_TYPES

#ifndef CLAGS_NO_REAL_TYPES

CLAGS_PRIV bool clags__verify_real(clags_config_t *config, clags_value_type_t type, clags_range_t *range, const char *arg_name, const char *arg, void *variable)
{
    double min, max;
    if (range != NULL){
        min = range->min.as_double;
        max = range->max.as_double;
    } else{
        min = -DBL_MAX;
        max = DBL_MAX;
    }

    char *endptr;
    errno = 0;
    double value = CLAGS_STRTOD(arg, &endptr);

    if (*endptr != '\0') {
        clags_log(config, Clags_Error, "Invalid %s value for argument '%s': '%s'!", clags__types[type].name, arg_name, arg);
        return false;
    }

    if (errno == ERANGE || value > max || value < min || value != value) {
        clags_log(config, Clags_Error, "%s value out of range [%g-%g] for argument '%s': '%s'!", clags__types[type].name, min, max, arg_name, arg);
        return false;
    }

    if (variable) *(double*)variable = value;
    return true;
}

CLAGS_API bool clags_verify_real(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    return clags__verify_real(config, Clags_Real, data, arg_name, arg, variable);
}

CLAGS_API bool clags_verify_float(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    double var;
    clags_range_t range = clags_real_range(-FLT_MAX, FLT_MAX);
    bool result = clags__verify_real(config, Clags_Float, &range, arg_name, arg, &var);
    if (result && variable) *(float*)variable = (float) var;
    return result;
}

CLAGS_API bool clags_verify_double(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    double var;
    clags_range_t range = clags_real_range(-DBL_MAX, DBL_MAX);
    bool result = clags__verify_real(config, Clags_Double, &range, arg_name, arg, &var);
    if (result && variable) *(double*)variable = var;
    return result;
}

#endif // !CLAGS_NO_REAL_TYPES

#ifndef CLAGS_NO_PATH_TYPES

CLAGS_API bool clags_verify_path(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    clags_path_type_t type = clags_path_type(arg);
    if (type == Clags_Path_Invalid){
        clags_log(config, Clags_Error, "Invalid path for argument '%s': '%s' : %s!", arg_name, arg, strerror(errno));
        return false;
    }
    if (variable) *(char**)variable = clags_config_duplicate_string(config, arg);
    return true;
}

CLAGS_API bool clags_verify_file(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    clags_path_type_t type = clags_path_type(arg);
    switch (type){
        case Clags_Path_Invalid:{
            clags_log(config, Clags_Error, "Invalid path for argument '%s': '%s' : %s!", arg_name, arg, strerror(errno));
            return false;
        } break;
        case Clags_Path_Reg:{
            if (variable) *(char**)variable = clags_config_duplicate_string(config, arg);
            return true;
        }
        default:{
            clags_log(config, Clags_Error, "Path for argument '%s' is not a file: '%s'!", arg_name, arg);
            return false;
        }
    }
}

CLAGS_API bool clags_verify_dir(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    clags_path_type_t type = clags_path_type(arg);
    switch (type){
        case Clags_Path_Invalid:{
            clags_log(config, Clags_Error, "Invalid path for argument '%s': '%s' : %s!", arg_name, arg, strerror(errno));
            return false;
        } break;
        case Clags_Path_Dir:{
            if (variable) *(char**)variable = clags_config_duplicate_string(config, arg);
            return true;
        }
        default:{
            clags_log(config, Clags_Error, "Path for argument '%s' is not a dir: '%s'!", arg_name, arg);
            return false;
        }
    }
}

#endif // !CLAGS_NO_PATH_TYPES

#ifndef CLAGS_NO_SIZE_TYPES

CLAGS_API bool clags_verify_size(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;

    // parse the leading number string
    clags_strtoint_res_t res;
    clags_fsize_t value = clags_strtouint64_s(arg, &res, 10);
    if (res.no_digits){
        clags_log(config, Clags_Error, "No leading number in size argument '%s': '%s'!", arg_name, arg);
        return false;
    }

    // convert remaining unit string to a factor
    clags_fsize_t factor;
    if (*res.endptr == '\0' || clags__strcasecmp(res.endptr, "B") == 0) factor = 1;
    else if (clags__strcasecmp(res.endptr, "KiB") == 0)                 factor = 1ULL << 10;
    else if (clags__strcasecmp(res.endptr, "KB")  == 0)                 factor = 1000;
    else if (clags__strcasecmp(res.endptr, "MiB") == 0)                 factor = 1ULL << 20;
    else if (clags__strcasecmp(res.endptr, "MB")  == 0)                 factor = 1000000;
    else if (clags__strcasecmp(res.endptr, "GiB") == 0)                 factor = 1ULL << 30;
    else if (clags__strcasecmp(res.endptr, "GB")  == 0)                 factor = 1000000000;
    else if (clags__strcasecmp(res.endptr, "TiB") == 0)                 factor = 1ULL << 40;
    else if (clags__strcasecmp(res.endptr, "TB")  == 0)                 factor = 1000000000000;
    else {
        clags_log(config, Clags_Error, "Invalid size unit for argument '%s': '%s'!", arg_name, res.endptr);
        return false;
    }

    // range checks, reject negative inputs
    if (res.out_of_range || res.negative || value > UINT64_MAX/factor) {
        clags_log(config, Clags_Error, "clags_fsize_t value out of range (0 to %"PRIu64") for argument '%s': '%s'!", UINT64_MAX, arg_name, arg);
        return false;
    }
    if (variable) *(clags_fsize_t*)variable = (clags_fsize_t)value * factor;
    return true;
}

#endif // !CLAGS_NO_SIZE_TYPES

#ifndef CLAGS_NO_TIME_TYPES

CLAGS_API bool clags_verify_time_s(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    clags_strtoint_res_t res;
    const char *start = arg;

    // skip spaces
    while (isspace((unsigned char)*start)) start++;
    // parse one time string segment at a time and sum up the results
    clags_time_t result = 0;
    do{
        // convert leading number string to a double
        errno = 0;
        clags_time_t value = clags_strtouint64_s(start, &res, 10);
        if (res.no_digits){
            clags_log(config, Clags_Error, "No leading number in time argument '%s': '%s'!", arg_name, start);
            return false;
        }
        // find end of unit string
        const char *unit_end = res.endptr;
        while (isalpha((unsigned char) *unit_end)) unit_end++;

        // convert unit string to a factor
        clags_time_t factor;
        size_t unit_len = unit_end - res.endptr;
        if (unit_len == 0 || clags__strslice_casecmp(res.endptr, unit_len, "s") == 0) factor =       1;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "m")  == 0)            factor =      60;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "h")  == 0)            factor =    3600;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "d")  == 0)            factor = 24*3600;
        else {
            clags_log(config, Clags_Error, "Invalid time unit for argument '%s': '%.*s'!", arg_name, (int) unit_len, res.endptr);
            return false;
        }
        // range check for the factor
        if (res.out_of_range || value > UINT64_MAX/factor || res.negative){
            clags_log(config, Clags_Error, "clags_time_t value out of range (0s to %"PRIu64"s) for argument part '%s': '%.*s'!", UINT64_MAX, arg_name, (int)(unit_end-start), start);
            return false;
        }
        // range check for the temporary result
        clags_time_t value_in_s = value * factor;
        if (result > UINT64_MAX - value_in_s){
            clags_log(config, Clags_Error, "clags_time_t value out of range (0s to %"PRIu64"s) for argument part '%s': '%.*s'!", UINT64_MAX, arg_name, (int)(unit_end-arg), arg);
            return false;
        }

        // add to the result
        result += value_in_s;

        // advance reading pointer to next segment if existing
        start = unit_end;
        while (isspace((unsigned char)*start)) start++;
    } while(*start != '\0');

    if (variable) *(clags_time_t*)variable = result;
    return true;
}

CLAGS_API bool clags_verify_time_ns(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    clags_strtoint_res_t res;
    const char *start = arg;

    // skip spaces
    while (isspace((unsigned char)*start)) start++;
    // parse one time string segment at a time and sum up the results
    clags_time_t result = 0;
    do{
        // convert leading number string to a double
        errno = 0;
        clags_time_t value = clags_strtouint64_s(start, &res, 10);
        if (res.no_digits){
            clags_log(config, Clags_Error, "No leading number in time argument '%s': '%s'!", arg_name, start);
            return false;
        }
        // find end of unit string
        const char *unit_end = res.endptr;
        while (isalpha((unsigned char) *unit_end)) unit_end++;

        // convert unit string to a factor
        clags_time_t factor;
        size_t unit_len = unit_end - res.endptr;
        if (unit_len == 0 || clags__strslice_casecmp(res.endptr, unit_len, "ns") == 0)   factor = 1;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "us") == 0)               factor = 1000ULL;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "ms") == 0)               factor = 1000000ULL;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "s") == 0)                factor = 1000000000ULL;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "m") == 0)                factor = 60ULL * 1000000000ULL;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "h") == 0)                factor = 3600ULL * 1000000000ULL;
        else if (clags__strslice_casecmp(res.endptr, unit_len, "d") == 0)                factor = 24ULL * 3600ULL * 1000000000ULL;
        else {
            clags_log(config, Clags_Error, "Invalid time unit for argument '%s': '%.*s'!", arg_name, (int) unit_len, res.endptr);
            return false;
        }
        // range check for the factor
        if (res.out_of_range || value > UINT64_MAX/factor || res.negative){
            clags_log(config, Clags_Error, "clags_time_t value out of range (0s to %"PRIu64"ns) for argument part '%s': '%.*s'!", UINT64_MAX, arg_name, (int)(unit_end-start), start);
            return false;
        }
        // range check for the temporary result
        clags_time_t value_in_s = value * factor;
        if (result > UINT64_MAX - value_in_s){
            clags_log(config, Clags_Error, "clags_time_t value out of range (0s to %"PRIu64"ns) for argument '%s': '%.*s'!", UINT64_MAX, arg_name, (int)(unit_end-arg), arg);
            return false;
        }

        // add to the result
        result += value_in_s;

        // advance reading pointer to next segment if existing
        start = unit_end;
        while (isspace((unsigned char)*start)) start++;
    } while(*start != '\0');

    if (variable) *(clags_time_t*)variable = result;
    return true;
}

#endif // !CLAGS_NO_TIME_TYPES

CLAGS_API bool clags_verify_choice(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    clags_choices_t  *choices = (clags_choices_t*) data;
    for (size_t i=0; i<choices->count; ++i){
        clags_choice_t *choice = choices->items + i;
        if (choice->value == NULL) continue;
        if ((choices->case_insensitive && clags__strcasecmp(choice->value, arg) == 0) || (!choices->case_insensitive && strcmp(choice->value, arg) == 0)){
            if (variable) *(clags_choice_t**)variable = choice;
            return true;
        }
    }
    clags_log(config, Clags_Error, "Invalid choice for argument '%s': '%s'!", arg_name, arg);
    return false;
}

CLAGS_API bool clags_verify_subcmd(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    clags_subcmds_t *subcmds = (clags_subcmds_t*) data;
    for (size_t i=0; i<subcmds->count; ++i){
        clags_subcmd_t subcmd = subcmds->items[i];
        if (subcmd.name == NULL) continue;
        if (strcmp(subcmd.name, arg) == 0){
            if (variable) *(clags_subcmd_t**) variable = &subcmds->items[i];
            return true;
        }
    }
    clags_log(config, Clags_Error, "Unknown subcommand '%s' for argument '%s'!", arg, arg_name);
    return false;
}

CLAGS_API bool clags_verify_custom(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    clags_custom_t *custom = (clags_custom_t*) data;
    if (custom && custom->verify){
        return custom->verify(config, arg_name, arg, variable, custom->data);
    }
    return false;
}

/* Flag Setters */

CLAGS_PRIV clags_error_type_t clags__set_bool_flag(clags_config_t *config, void *variable)
{
    (void) config;
    if (variable != NULL) *(bool*) variable = true;
    return Clags_Error_Ok;
}

CLAGS_PRIV clags_error_type_t clags__set_config_flag(clags_config_t *config, void *variable)
{
    if (variable != NULL) *(clags_config_t**) variable = config;
    return Clags_Error_Ok;
}

CLAGS_PRIV clags_error_type_t clags__set_count_flag(clags_config_t *config, void *variable)
{
    (void) config;
    if (variable != NULL) *(size_t*) variable += 1;
    return Clags_Error_Ok;
}

/* Verifier Dispatching */

CLAGS_PRIV clags_error_type_t clags__set_arg(clags_config_t *config, clags_value_type_t value_type, const char *arg_name, const char *arg, void *variable, void *data, bool is_list);

CLAGS_PRIV clags_error_type_t clags__append_to_list(clags_config_t *config, clags_value_type_t value_type, const char *arg_name, const char *arg, void *variable, void *data)
{
    clags_list_t *list = (clags_list_t*) variable;
    char *ptr = NULL;
    if (list != NULL){
        if (!clags__handle_list_resizing(config, list)) return Clags_Error_ListOverflow;
        // calculate the pointer to write to based on the stored items' size
        ptr = ((char*) list->items) + list->item_size * list->count;
    }
    // dispatch to the correct verifier based on the provided value type
    clags_error_type_t result = clags__set_arg(config, value_type, arg_name, arg, ptr, data, false);
    if (result == Clags_Error_Ok && list != NULL){
        // update the list's size
        list->count++;
    }
    return result;
}

CLAGS_PRIV clags_error_type_t clags__set_arg(clags_config_t *config, clags_value_type_t value_type, const char *arg_name, const char *arg, void *variable, void *data, bool is_list)
{
    // dispatch to the correct verifier based on the provided value type
    if (is_list){
        // for lists we first need to calculate the exact address to write to and handle possible reallocations
        return clags__append_to_list(config, value_type, arg_name, arg, variable, data);
    } else{
        bool result = clags__types[value_type].verify(config, arg_name, arg, variable, data);
        return result? Clags_Error_Ok : Clags_Error_InvalidValue;
    }
}

CLAGS_PRIV clags_error_type_t clags__set_flag(clags_config_t *config, clags_flag_t *flag)
{
    // dispatch to the correct setter based on the flag's type
    switch (flag->type){
#define X(type, setter) case type: return setter(config, flag->variable);
        CLAGS__FLAG_TYPES
#undef X
        default:{
            clags_unreachable("Invalid clags_flag_type_t");
        }
    }
}

/* Validation */

CLAGS_PRIV bool clags__validate_default(clags_config_t *config, clags_value_type_t value_type, const char *arg_name, const char *value, void *data, bool is_list)
{
    if (is_list){
        clags_log(config, Clags_ConfigError, "default values are not supported for lists. Argument '%s' must be initialized via user input only.", arg_name);
        return false;
    }
    // verify but do not write to the variable
    if (clags__set_arg(config, value_type, arg_name, value, NULL, data, false) != Clags_Error_Ok){
        clags_log(config, Clags_ConfigError, "invalid default value for argument '%s': '%s'", arg_name, value);
        return false;
    }
    return true;
}

CLAGS_PRIV bool clags__validate_list(clags_config_t *config, void *variable, clags_value_type_t value_type, const char *arg_name)
{
    clags_list_t *list = (clags_list_t*) variable;
    if (list != NULL){
        // ensure "type safety"
        if (list->value_type != value_type){
            clags_log(config, Clags_ConfigError, "list argument '%s' expects type '%s', but list was created as '%s'!", arg_name, clags__types[value_type].name, clags__types[list->value_type].name);
            return false;
        }
        if (list->value_type == Clags_Custom){
            // custom lists must be initialized with a item size to store
            if (!list->custom_size_set){
                clags_log(config, Clags_ConfigError, "`clags_list` must not be used with type `Clags_Custom` for argument '%s'! Instead, use `clags_custom_list` and provide the size of the stored value!", arg_name);
                return false;
            }
            if (list->item_size == 0){
                clags_log(config, Clags_ConfigError, "`item_size` of custom list for argument '%s' must not be zero!", arg_name);
                return false;
            }
        }
        if (list->item_size == 0){
            // set item size if not initialized yet
            list->item_size = clags__types[list->value_type].value_size;
        }
    }
    return true;
}

CLAGS_PRIV clags_config_t* clags__validate_none(clags_config_t *config, const char *arg_name, void *data)
{
    (void) config;
    (void) arg_name;
    (void) data;
    return NULL;
}

CLAGS_PRIV clags_config_t* clags__validate_subcmd(clags_config_t *config, const char *arg_name, void *data)
{
    // a subcommand argument should be provided with a valid `clags_subcmds_t` 
    if (data == NULL){
        clags_log(config, Clags_ConfigError, "incomplete subcommand definition for argument '%s'! Define `.subcmds` for subcommand verification!", arg_name);
        return config;
    }

    clags_subcmds_t *subcmds = (clags_subcmds_t*) data;
    if (subcmds->count == 0){
        clags_log(config, Clags_ConfigWarning, "`clags_subcmds_t` definition should contain at least one subcommand for argument '%s'!", arg_name);
    } else if (subcmds->items == NULL){
        clags_log(config, Clags_ConfigError, "invalid `clags_subcmds_t` definition for argument '%s'! `items` must not be NULL when `count` is non-zero!", arg_name);
        return config;
    }
    for (size_t i=0; i<subcmds->count; ++i){
        clags_subcmd_t subcmd = subcmds->items[i];
        clags_config_t *child_config = subcmd.config;
        if (subcmd.name == NULL){
            clags_log(config, Clags_ConfigError, "incomplete subcommand definition for argument '%s'! Subcommand %zu has no name assigned!", arg_name, i);
            return config;
        }
        if (child_config == NULL){
            clags_log(config, Clags_ConfigError, "incomplete subcommand definition for argument '%s'! Subcommand '%s' has no config assigned!", arg_name, subcmd.name);
            return config;
        }

        child_config->parent = config;
        clags_config_t *failed_config = clags_validate(subcmd.name, child_config);
        if (failed_config != NULL) return failed_config;
    }
    return NULL;
}

CLAGS_PRIV clags_config_t* clags__validate_choice(clags_config_t *config, const char *arg_name, void *data)
{
    // a choice argument should be provided with a valid `clags_choices_t`
    clags_choices_t* choices = (clags_choices_t*) data;
    if (choices == NULL){
        clags_log(config, Clags_ConfigError, "incomplete choice definition for argument '%s'! Define `.choices` for choice verification!", arg_name);
        return config;
    }
    if (choices->count == 0){
        clags_log(config, Clags_ConfigWarning, "`clags_choices_t` definition should contain at least one choice for argument '%s'!", arg_name);
    } else if (choices->items == NULL){
        clags_log(config, Clags_ConfigError, "invalid `clags_choices_t` definition for argument '%s'! `items` must not be NULL when `count` is non-zero!", arg_name);
        return config;
    }
    for (size_t i=0; i<choices->count; ++i){
        if (choices->items[i].value == NULL){
            clags_log(config, Clags_ConfigError, "incomplete choice definition for argument '%s'! Choice %zu has no value assigned!", arg_name, i);
            return config;
        }
    }
    return NULL;
}

CLAGS_PRIV clags_config_t* clags__validate_custom(clags_config_t *config, const char *arg_name, void *data)
{
    // a custom argument should be provided with a valid `clags_custom_t`
    clags_custom_t *custom = (clags_custom_t*) data;
    if (custom == NULL){
        clags_log(config, Clags_ConfigError, "incomplete custom verifier definition for argument '%s'! Define `.custom` for custom verification!", arg_name);
        return config;
    }
    // call the custom validator if specified
    if (custom->validate != NULL){
        return custom->validate(config, arg_name, custom->data);
    }
    return NULL;
}

#ifndef CLAGS_NO_INT_TYPES
CLAGS_PRIV clags_config_t* clags__validate_range(clags_config_t *config, clags_value_type_t type, const char *arg_name, void *data)
{
    // check whether the provided range contraint is of the correct type
    clags_range_t *range = (clags_range_t*)data;
    if (range != NULL){
        if (range->type != type){
            clags_log(config, Clags_ConfigError, "incorrect range type for argument '%s': expected range type '%s', but got '%s'!", arg_name, clags__types[type].name, clags__types[range->type].name);
            return config;
        }
        switch (range->type){
#ifndef CLAGS_NO_INT_TYPES            
            case Clags_Int:{
                if (range->min.as_int > range->max.as_int){
                    clags_log(config, Clags_ConfigError, "invalid range definition for argument '%s': expected a maximum bound greater than or equal to the minimum bound, but got [%"PRId64"-%"PRId64"]!", arg_name, range->min.as_int, range->max.as_int);
                    return config;
                }
            } break;
            case Clags_UInt:{
                if (range->min.as_uint > range->max.as_uint){
                    clags_log(config, Clags_ConfigError, "invalid range definition for argument '%s': expected a maximum bound greater than or equal to the minimum bound, but got [%"PRIu64"-%"PRIu64"]!", arg_name, range->min.as_uint, range->max.as_uint);
                    return config;
                }
            } break;
#endif // !CLAGS_NO_INT_TYPES
#ifndef CLAGS_NO_REAL_TYPES
            case Clags_Real:{
                if (range->min.as_double > range->max.as_double){
                    clags_log(config, Clags_ConfigError, "invalid range definition for argument '%s': expected a maximum bound greater than or equal to the minimum bound, but got [%g-%g]!", arg_name, range->min.as_double, range->max.as_double);
                    return config;
                }
            } break;
#endif // !CLAGS_NO_REAL_TYPES
            default:{
                clags_log(config, Clags_ConfigError, "invalid range type for argument '%s': '%s'!", arg_name, clags__types[range->type].name);
                return config;
            }
        }
    }
    return NULL;
}


CLAGS_PRIV clags_config_t* clags__validate_int(clags_config_t *config, const char *arg_name, void *data)
{
    return clags__validate_range(config, Clags_Int, arg_name, data);
}

CLAGS_PRIV clags_config_t* clags__validate_uint(clags_config_t *config, const char *arg_name, void *data)
{
    return clags__validate_range(config, Clags_UInt, arg_name, data);
}

#endif // !CLAGS_NO_INT_TYPES

#ifndef CLAGS_NO_REAL_TYPES

CLAGS_PRIV clags_config_t* clags__validate_real(clags_config_t *config, const char *arg_name, void *data)
{
    return clags__validate_range(config, Clags_Real, arg_name, data);
}

#endif // !CLAGS_NO_REAL_TYPES

CLAGS_PRIV bool clags__validate_value_type(clags_config_t *config, const char *arg_name, clags_value_type_t type)
{
    switch (type){
#define X(type, verify_func, name, vtype, validate_func, usage_func) case type: return true;
        CLAGS__VALUE_TYPES
#undef X
    }
    clags_log(config, Clags_ConfigError, "invalid `value_type` of argument '%s': %d! Only use types from `clags_value_type_t`!", arg_name, type);
    return false;
}

CLAGS_PRIV clags_config_t* clags__validate_positional(clags_config_t *config, clags_positional_t pos)
{
    // validate the argument's value type and its requirements
    if (!clags__validate_value_type(config, pos.arg_name, pos.value_type)) return config;
    clags_config_t *failed_config = clags__types[pos.value_type].validate(config, pos.arg_name, pos.using._data);
    if (failed_config != NULL) return failed_config;
    if (pos.is_list && !clags__validate_list(config, pos.variable, pos.value_type, pos.arg_name)) return config;

    // validate the default input, if defined
    if (pos.default_input){
        if (!pos.optional){
            clags_log(config, Clags_ConfigWarning, "default value set for required argument '%s'! This value will always be overwritten and never used.", pos.arg_name);
        }
        if (!clags__validate_default(config, pos.value_type, pos.arg_name, pos.default_input, pos.using._data, pos.is_list)) return config;
    }

    return NULL;
}

CLAGS_PRIV clags_config_t* clags__validate_option(clags_config_t *config, clags_option_t opt)
{
    // build a temporary argument name for logging
    char buf[3] = {'-', '\0', '\0'};
    const char *name = opt.long_flag ? opt.long_flag
                      : (opt.short_flag ? (buf[1] = opt.short_flag, buf) : "(unnamed)");

    // validate reachability and format
    if (opt.short_flag == '\0' && opt.long_flag == NULL){
        clags_log(config, Clags_ConfigWarning, "unreachable option! Define at least one of `short_flag` and `long_flag`.");
    }
    if (opt.long_flag && strncmp(opt.long_flag, "--", 2) == 0){
        clags_log(config, Clags_ConfigWarning,
                  "option long flag '%s' in config '%s' should not start with '--'. "
                  "The parser automatically handles leading '--' for long flags, "
                  "so including it in the config may cause incorrect parsing.",
                  opt.long_flag, config->name);
    }

    // validate the option's value type and its requirements
    if (!clags__validate_value_type(config, name, opt.value_type)) return config;
    if (opt.value_type == Clags_Subcmd){
        clags_log(config, Clags_ConfigError, "option argument '%s' in config '%s' may not be a subcommand!", name, config->name);
        return config;
    }
    clags_config_t *failed_config = clags__types[opt.value_type].validate(config, name, opt.using._data);
    if (failed_config != NULL) return failed_config;
    if (opt.is_list && !clags__validate_list(config, opt.variable, opt.value_type, name)) return config;

    // validate the default input, if defined
    if (opt.default_input){
        if (!clags__validate_default(config, opt.value_type, name, opt.default_input, opt.using._data, opt.is_list)) return config;
    }

    return NULL;
}

CLAGS_PRIV clags_config_t* clags__validate_flag(clags_config_t *config, clags_flag_t flag)
{
    // build a temporary argument name for logging
    char buf[3] = {'-', '\0', '\0'};
    const char *name = flag.long_flag ? flag.long_flag
                      : (flag.short_flag ? (buf[1] = flag.short_flag, buf) : "(unnamed)");

    // validate reachability and format
    if (flag.short_flag == '\0' && flag.long_flag == NULL){
        clags_log(config, Clags_ConfigWarning, "unreachable flag in config '%s'. Define at least one of `short_flag` and `long_flag`.", config->name);
    }
    if (flag.long_flag && strncmp(flag.long_flag, "--", 2) == 0){
        clags_log(config, Clags_ConfigWarning,
                  "long flag '%s' should not start with '--'. "
                  "The parser automatically handles leading '--' for long flags, "
                  "so including it in the config may cause incorrect parsing.",
                  flag.long_flag);
    }
    switch (flag.type){
#define X(type, setter) case type: break;
        CLAGS__FLAG_TYPES
#undef X
        default:{
            clags_log(config, Clags_ConfigError, "invalid flag type for flag '%s': %d!", name, flag.type);
            return config;
        }
    }

    return NULL;
}

CLAGS_PRIV bool clags__validate_duplicates(clags_config_t *config, char short_flag, const char *long_flag, size_t index)
{
    clags_config_t *ancestor = config;
    bool reachable = true;
    while (ancestor != NULL) {
        // are we in the initial config?
        bool is_self = ancestor == config;
        // if so, start with the next argument, otherwise a duplication with itself would be detected
        size_t j = is_self ? (index+1) : 0;

        for (; j < ancestor->args_count; ++j) {
            clags_arg_t *p_arg = &ancestor->args[j];
            if (p_arg->type != Clags_Option && p_arg->type != Clags_Flag) continue;

            // extract the argument's short and long flag
            char p_short = (p_arg->type == Clags_Option) ? p_arg->as.opt.short_flag : p_arg->as.flag.short_flag;
            const char *p_long = (p_arg->type == Clags_Option) ? p_arg->as.opt.long_flag : p_arg->as.flag.long_flag;

            // check whether the current argument is inheritable
            bool is_inheritable = reachable && ((p_arg->type == Clags_Option) ? p_arg->as.opt.inherit : p_arg->as.flag.inherit);

            // check for short flag match
            if (short_flag != '\0' && short_flag == p_short) {
                if (is_self || is_inheritable){
                    // argument comes from the same config or can be inherited
                    // flags are conflicting, report an error
                    clags_log(config, Clags_ConfigError, "Duplicate short flag '-%c' is already defined in config '%s'.", short_flag, ancestor->name);
                    return false;
                } else{
                    // argument is not inheritable but still has the same flags
                    // report a warning
                    clags_log(config, Clags_ConfigWarning, "Local short flag '-%c' shadows a flag from ancestor config '%s'.", short_flag, ancestor->name);
                }
            }
            // check for long flag match
            if (long_flag && p_long && strcmp(long_flag, p_long) == 0) {
                if (is_self || is_inheritable){
                    // argument comes from the same config or can be inherited
                    // flags are conflicting, report an error
                    clags_log(config, Clags_ConfigError, "Duplicate long flag '--%s' is already defined in config '%s'.", long_flag, ancestor->name);
                    return false;
                } else{
                    // argument is not inheritable but still has the same flags
                    // report a warning
                    clags_log(config, Clags_ConfigWarning, "Local long flag '--%s' shadows a flag from ancestor config '%s'.", long_flag, ancestor->name);
                }
            }
        }
        if (ancestor->options.no_inheritance) reachable = false; // mark all arguments from ancestors as not inheritable
        // continue searching in parent config
        ancestor = ancestor->parent;
    }
    return true;
}

// update the usage buffer size and clamp to the config's bounds
typedef struct{
    size_t total;
    size_t prefix;
    size_t suffix;
} clags__usage_lengths_t;

CLAGS_PRIV clags__usage_lengths_t clags__print_lhs(clags_sb_t *sb, char short_flag, const char *long_flag, const char *arg_name, bool optional, int remaining);

CLAGS_PRIV void clags__validate_alignment(clags_config_t *config, size_t *buff_size, size_t needed, const char *arg_name)
{
    size_t bytes_needed = needed + 1; // for null-terminator
    size_t max_bytes = CLAGS__USAGE_TEMP_BUFFER_SIZE(config->options.max_usage_alignment);
    if (bytes_needed > *buff_size){
        if (bytes_needed > max_bytes){
            // alignment is too long
            clags_log(config, Clags_ConfigWarning, "usage for argument '%s' exceeds the alignment width (%zu > %zu) and will be truncated. Consider increasing `.max_usage_alignment`.", arg_name, config->options.max_usage_alignment + (bytes_needed - max_bytes), config->options.max_usage_alignment);
            *buff_size = max_bytes;
        } else{
            // current buffer is too small, update size
            *buff_size = bytes_needed;
        }
    }
}

CLAGS_API clags_config_t* clags_validate(const char *name, clags_config_t *config)
{
    if (config == NULL) return NULL;
    if (config->state != Clags_Config_Unvalidated) return NULL;
    clags_config_t *result = NULL;
    // set the config name
    if (config->name == NULL) config->name = clags_config_duplicate_string(config, name);

    if (config->args == NULL){
        clags_log(config, Clags_ConfigWarning, "config '%s' does not have any arguments defined! Use the `clags_config(args)` macro to create a config linked to an arguments definition.", config->name);
    }

    // validate config options
    if (config->options.list_terminator && strcmp(config->options.list_terminator, "--") == 0){
        clags_log(config, Clags_ConfigError,"'.list_terminator' of config '%s' may not be '--' because it is reserved for toggling option and flag parsing!", config->name);
        clags_return_defer(config);
    }
    if (config->options.ignore_prefix && strcmp(config->options.ignore_prefix, "--") == 0){
        clags_log(config, Clags_ConfigError, "'.ignore_prefix' of config '%s' may not be '--' because it is reserved for toggling option and flag parsing!", config->name);
        clags_return_defer(config);
    }
    if (config->options.ignored_args){
        clags_list_t *ignored_args = config->options.ignored_args;
        if (ignored_args->value_type != Clags_String || !(ignored_args->item_size == sizeof(char*) || ignored_args->item_size == 0)){
            clags_log(config, Clags_ConfigError, "'.ignored_args' of config '%s' list is not correctly initialized as a string list!", config->name);
            clags_return_defer(config);
        }
    }
    // validate min usage alignment
    if (config->options.min_usage_alignment == 0){
        config->options.min_usage_alignment = CLAGS_DEFAULT_MIN_USAGE_ALIGNMENT;
    }
    if (config->options.min_usage_alignment < CLAGS__MIN_USAGE_ALIGNMENT){
        clags_log(config, Clags_ConfigError, "'.min_usage_alignment' must be at least %d (got %zu)!", CLAGS__MIN_USAGE_ALIGNMENT, config->options.min_usage_alignment);
        clags_return_defer(config);
    }

    // validate max usage alignment
    if (config->options.max_usage_alignment == 0){
        config->options.max_usage_alignment = CLAGS_DEFAULT_MAX_USAGE_ALIGNMENT;
    }

    if (config->options.min_usage_alignment > config->options.max_usage_alignment){
        clags_log(config, Clags_ConfigError, "'.min_usage_alignment' (%zu) is greater than '.max_usage_alignment' (%zu)!", config->options.min_usage_alignment, config->options.max_usage_alignment);
        clags_return_defer(config);
    }

    // validate args
    bool last_was_list = false;
    bool subcmd_found = false;
    bool optional_found = false;
    const char *last_pos_name = NULL;
    size_t alignment_buffer_size = CLAGS__USAGE_TEMP_BUFFER_SIZE(config->options.min_usage_alignment);

    for (size_t i=0; i<config->args_count; ++i){
        clags_arg_type_t arg_type = config->args[i].type;
        switch (arg_type){
            case Clags_Positional:{
                // validate positional argument
                clags_positional_t pos = config->args[i].as.pos;
                clags_config_t *failed_config = clags__validate_positional(config, pos);
                if (failed_config != NULL) clags_return_defer(failed_config);

                // validate positional argument ordering
                // 1. positional arguments may not follow optional ones
                if (optional_found && !pos.optional){
                    clags_log(config, Clags_ConfigError, "invalid positional argument order: required argument '%s' appears after optional argument '%s'", pos.arg_name, last_pos_name);
                    clags_return_defer(config);
                }
                optional_found = pos.optional;

                // 2. subcommands must be the only positional argument in a config
                if (pos.value_type == Clags_Subcmd){
                    subcmd_found = true;
                    if (last_pos_name != NULL){
                        clags_log(config, Clags_ConfigError, "subcommand '%s' must be the only positional argument!", pos.arg_name);
                        clags_return_defer(config);
                    }
                } else if (subcmd_found){
                    clags_log(config, Clags_ConfigError, "trailing positional argument after subcommand: '%s'!", pos.arg_name);
                    clags_return_defer(config);
                }

                // 3. if a positional argument follows a list, a list terminator must be defined
                if (last_was_list && config->options.list_terminator == NULL){
                    clags_sb_t sb = {0};
                    clags_sb_appendf(&sb, "positional argument '%s' is unreachable after list '%s'! Define '.list_terminator' in 'clags_config' to separate them", pos.arg_name, last_pos_name);
                    if (!pos.is_list){
                        clags_sb_appendf(&sb, " or make '%s' option", pos.arg_name);
                    }
                    clags_sb_appendf(&sb, ".");
                    clags_log_sb(config, Clags_ConfigError, &sb);
                    clags_sb_free(&sb);
                    clags_return_defer(config);
                }
                last_was_list = pos.is_list;
                last_pos_name = pos.arg_name;

                // update alignment
                clags__usage_lengths_t lengths = clags__print_lhs(NULL, 0, NULL, pos.arg_name, pos.optional, -1);
                clags__validate_alignment(config, &alignment_buffer_size, lengths.total, pos.arg_name);
            } break;
            case Clags_Option:{
                last_was_list = false;
                clags_option_t opt = config->args[i].as.opt;
                if (opt.inherit) config->inherit_options_count += 1;
                // validate option argument
                clags_config_t *failed_config = clags__validate_option(config, opt);
                if (failed_config != NULL) clags_return_defer(failed_config);
                // check if the option's flags already exist in the config tree
                if (!clags__validate_duplicates(config, opt.short_flag, opt.long_flag, i)) clags_return_defer(config);

                // update alignment
                char buf[3] = {'-', '\0', '\0'};
                const char *opt_name = opt.long_flag ? opt.long_flag
                    : (opt.short_flag ? (buf[1] = opt.short_flag, buf) : "(unnamed)");

                clags__usage_lengths_t lengths = clags__print_lhs(NULL, opt.short_flag, opt.long_flag, opt.arg_name, false, -1);
                clags__validate_alignment(config, &alignment_buffer_size, lengths.total, opt_name);
            } break;
            case Clags_Flag:{
                last_was_list = false;
                clags_flag_t flag = config->args[i].as.flag;
                if (flag.inherit) config->inherit_flags_count += 1;
                // validate flag argument
                clags_config_t *failed_config = clags__validate_flag(config, flag);
                if (failed_config != NULL) clags_return_defer(failed_config);
                // check if the flag's flags already exist in the config tree
                if (!clags__validate_duplicates(config, flag.short_flag, flag.long_flag, i)) clags_return_defer(config);

                // update alignment
                char buf[3] = {'-', '\0', '\0'};
                const char *flag_name = flag.long_flag ? flag.long_flag
                    : (flag.short_flag ? (buf[1] = flag.short_flag, buf) : "(unnamed)");
                clags__usage_lengths_t lengths = clags__print_lhs(NULL, flag.short_flag, flag.long_flag, NULL, false, -1);
                clags__validate_alignment(config, &alignment_buffer_size, lengths.total, flag_name);
            } break;
            default:{
                clags_log(config, Clags_ConfigError, "invalid `arg_type` of argument %zu: %d! Only use types from `clags_arg_type_t`!", i, arg_type);
                clags_return_defer(config);
            }
        }
    }
defer:
    // set the config state
    if (result != NULL){
        result->error = clags_error(Clags_Error_InvalidConfig, NULL, NULL);
        result->state = Clags_Config_Invalid;
    } else{
        config->usage_alignment = CLAGS__USAGE_ALIGNMENT_FROM_BUFFER_SIZE(alignment_buffer_size);
        config->state = Clags_Config_Valid;
    }
    return result;
}

/* Parsing */

// containers and counters for all argument types
typedef struct{
    clags_arg_t **positionals;
    size_t positional_count;
    size_t required_count;
    clags_arg_t **options;
    size_t option_count;
    clags_arg_t **flags;
    size_t flag_count;
} clags__args_t;

// the internal parser state
typedef struct {
    clags__args_t args;     // the sorted config's argument definitions
    size_t argc;            // the number of remaining arguments to parse
    char **argv;            // the remaining arguments to parse
    size_t index;           // the index of current argument
    bool args_ignored;      // arguments have been ignored using the `ignore_prefix`
    bool in_list;           // currently parsing a list
    bool parsing_optionals; // finished with parsing required positional arguments
    bool accept_options;    // parse options and flags if not disabled via '--'
    size_t pos_count;       // the number of positional arguments found
    size_t req_count;       // the number of required positional arguments found
    size_t pos_pos;         // the position of the last positional argument
} clags__parser_t;

// sort the argument types into their respective pre-allocated containers and track their count
CLAGS_PRIV void clags__sort_args(clags__args_t *args, clags_config_t *config)
{
    for (size_t i=0; i<config->args_count; ++i){
        clags_arg_t *arg = &config->args[i];
        switch(arg->type){
            case Clags_Positional:{
                if (args->positionals != NULL){
                    args->positionals[args->positional_count] = arg;
                }
                args->positional_count += 1;
                if (!arg->as.pos.optional) args->required_count += 1;
            } break;
            case Clags_Option:{
                if (args->options != NULL){
                    args->options[args->option_count] = arg;
                }
                args->option_count += 1;
            } break;
            case Clags_Flag:{
                if (args->flags != NULL){
                    args->flags[args->flag_count] = arg;
                }
                args->flag_count += 1;
            } break;
            default: {
                clags_unreachable("Invalid clags_arg_type_t");
            }
        }
    }
}

// apply the default inputs for all arguments of a config
CLAGS_PRIV clags_error_t clags__set_defaults(clags_config_t *config)
{
    for (size_t i=0; i<config->args_count; ++i){
        clags_arg_t *arg = &config->args[i];
        switch (arg->type){
            case Clags_Positional:{
                clags_positional_t pos = arg->as.pos;
                if (pos.optional && pos.default_input != NULL){
                    clags_assert(!pos.is_list, "list arguments may not have default values!");
                    if (clags__set_arg(config, pos.value_type, pos.arg_name, pos.default_input, pos.variable, pos.using._data, false) != Clags_Error_Ok) return clags_error(Clags_Error_InvalidDefault, arg, config);
                }
            } break;
            case Clags_Option:{
                clags_option_t opt = arg->as.opt;
                if (opt.default_input != NULL){
                    clags_assert(!opt.is_list, "list arguments may not have default values!");
                    char buf[3] = {'-', '\0', '\0'};
                    const char *name = opt.long_flag ? opt.long_flag : (opt.short_flag ? (buf[1] = opt.short_flag, buf) : "(unnamed)");
                    if (clags__set_arg(config, opt.value_type, name, opt.default_input, opt.variable, opt.using._data, false) != Clags_Error_Ok) return clags_error(Clags_Error_InvalidDefault, arg, config);
                }
            } break;
            default: continue;
        }
    }
    return clags_error(Clags_Error_Ok, NULL, NULL);
}

CLAGS_PRIV clags_arg_t* clags__search_long_options_and_flags(clags_config_t *config, const char *long_flag, clags_config_t **found_in)
{
    clags_config_t *ancestor = config;
    bool findable = true;
    while (ancestor != NULL){
        for (size_t i=0; i<ancestor->args_count; ++i){
            clags_arg_t *arg = &ancestor->args[i];
            bool match = false;
            bool inherit = false;
            // check for match
            if (arg->type == Clags_Option){
                clags_option_t opt = arg->as.opt;
                if (opt.long_flag != NULL){
                    size_t long_flag_len = strlen(opt.long_flag);
                    // check if `long_flag` starts with the current long_flag
                    if (strncmp(opt.long_flag, long_flag, long_flag_len) == 0){
                        // only mark as matching if remaining argument is empty or part of a designated option assignment
                        const char *end = long_flag + long_flag_len;
                        if (*end == '\0' || *end == '='){
                            match = true;
                            inherit = opt.inherit;
                        }
                    }
                }
            } else if (arg->type == Clags_Flag){
                clags_flag_t flag = arg->as.flag;
                if (flag.long_flag != NULL && strcmp(flag.long_flag, long_flag) == 0){
                    match = true;
                    inherit = flag.inherit;
                }
            }
            if (match){
                // always report the config in which a match occured even if the argument was not reachable
                *found_in = ancestor;
                // only return if argument is reachable
                if (findable && (config == ancestor || inherit)) return arg;
            }
        }
        if (ancestor->options.no_inheritance) findable = false; // reached non-inheriting config -> all further ancestor arguments are no longer reachable
        // haven't found perfect match yet, search in parent
        ancestor = ancestor->parent;
    }
    return NULL;
}

CLAGS_PRIV clags_arg_t* clags__search_short_options_and_flags(clags_config_t *config, char short_flag, clags_config_t **found_in)
{
    clags_config_t *ancestor = config;
    bool findable = true;
    while (ancestor != NULL){
        for (size_t i=0; i<ancestor->args_count; ++i){
            clags_arg_t *arg = &ancestor->args[i];
            bool inherit = false;
            bool match = false;
            // check for flag matching
            if (arg->type == Clags_Flag && arg->as.flag.short_flag == short_flag){
                inherit = arg->as.flag.inherit;
                match = true;
            } else if (arg->type == Clags_Option && arg->as.opt.short_flag == short_flag){
                inherit = arg->as.opt.inherit;
                match = true;
            }
            if (match){
                // always report the config in which a match occured even if the argument was not reachable
                *found_in = ancestor;
                // only return if argument is reachable
                if (findable && (config == ancestor || inherit)) return arg;
            }
        }
        if (ancestor->options.no_inheritance) findable = false; // reached non-inheriting config -> all further ancestor arguments are no longer reachable
        // haven't found perfect match yet, search in parent
        ancestor = ancestor->parent;
    }
    return NULL;
}

// try to return the next not-ignored argument
CLAGS_PRIV const char* clags__get_next_arg(clags_config_t *config, clags__parser_t *parser)
{
    const char *ignore_prefix = config->options.ignore_prefix;
    size_t ignore_prefix_len = ignore_prefix ? strlen(ignore_prefix) : 0;
    while (true){
        if (parser->index >= parser->argc - 1){
            // not enough arguments
            return NULL;
        }
        const char *arg = parser->argv[++parser->index];
        if (ignore_prefix == NULL || strncmp(arg, ignore_prefix, ignore_prefix_len) != 0) return arg;
        parser->args_ignored = true;
    }
}

CLAGS_PRIV clags_error_type_t clags__parse_option(clags_config_t *config, clags__parser_t *parser, const char *value, clags_option_t *opt, bool is_long_flag)
{
    const char *arg = parser->argv[parser->index];
    if (*value == '\0'){
        // value is next argument
        value = clags__get_next_arg(config, parser);
        if (value == NULL){
            clags_log(config, Clags_Error, "Option '%s' requires argument!", arg);
            return Clags_Error_InvalidOption;
        }
    } else if (is_long_flag){
        clags_assert(*value == '=', "arg has changed and no longer matches");
        // argument uses designated option assignment
        if (*++value == '\0'){
            clags_log(config, Clags_Error, "Designated option assignment may not have an empty value: '%s'!", arg);
            return Clags_Error_InvalidOption;
        }
    }
    // verify argument and write variable
    return clags__set_arg(config, opt->value_type, arg, value, opt->variable, opt->using._data, opt->is_list);
}

CLAGS_PRIV clags_error_type_t clags__parse_flag(clags_config_t *config, clags_flag_t *flag, bool *exit)
{
    *exit = flag->exit;
    return clags__set_flag(config, flag);
}

CLAGS_PRIV clags_error_t clags__parse_long_options_and_flags(clags_config_t *config, clags__parser_t *parser, clags_config_t **error_config, bool *do_exit)
{
    *error_config = config;
    *do_exit = false;
    const char *arg = parser->argv[parser->index];
    const char *flag_name = arg + 2; // skip `--`
    if (*flag_name == '\0'){
        clags_log(config, Clags_Error, "Missing flag name: '%s'!", arg);
        return clags_error(Clags_Error_InvalidOption, NULL, NULL);
    }
    // search for option or flag in config and all ancestors
    clags_config_t *arg_config = NULL; // the config a matching flag was found in even it not reachable
    clags_arg_t *parg = clags__search_long_options_and_flags(config, flag_name, &arg_config);
    if (parg == NULL){
        // could not find flag in tree
        if (arg_config != NULL){
            clags_log(config, Clags_Error, "Unknown long flag '%s'! Did you mean to use it with the '%s' subcommand?", arg, arg_config->name);
        } else{
            clags_log(config, Clags_Error, "Unknown long flag '%s'!", arg);
        }
        return clags_error(Clags_Error_InvalidOption, NULL, NULL);
    }
    if (parg->type == Clags_Option){
        const char *value = flag_name + strlen(parg->as.opt.long_flag);
        clags_error_type_t error = clags__parse_option(config, parser, value, &parg->as.opt, true);
        if (error != Clags_Error_Ok) return clags_error(error, parg, arg_config);
    } else if (parg->type == Clags_Flag){
        clags_error_type_t error = clags__parse_flag(config, &parg->as.flag, do_exit);
        if (error != Clags_Error_Ok) return clags_error(error, parg, arg_config);
    } else {
        clags_unreachable("arg type has changed");
    }
    return clags_error(Clags_Error_Ok, NULL, NULL);
}

CLAGS_PRIV clags_error_t clags__parse_short_options_and_flags(clags_config_t *config, clags__parser_t *parser, clags_config_t **error_config, bool *do_exit)
{
    *do_exit = false;
    *error_config = config;
    const char *arg = parser->argv[parser->index];
    const char *rest = arg + 1; // skip '-'
    size_t flag_len = strlen(rest);
    if (flag_len == 0){
        clags_log(config, Clags_Error, "Missing flag or option name: '-'!");
        return clags_error(Clags_Error_InvalidOption, NULL, NULL);
    }
    // iterate over each character of the flag group
    for (const char *c=rest; c<rest+flag_len; ++c){
        // search for the current flag in the argument tree
        clags_config_t *arg_config = NULL;
        clags_arg_t *parg = clags__search_short_options_and_flags(config, *c, &arg_config);
        if (parg != NULL){
            if (parg->type == Clags_Option){
                // parse option
                clags_error_type_t error = clags__parse_option(config, parser, c+1, &parg->as.opt, false);
                if (error != Clags_Error_Ok) return clags_error(error, parg, arg_config);
                break;
            } else if (parg->type == Clags_Flag){
                clags_error_type_t error = clags__parse_flag(config, &parg->as.flag, do_exit);
                if (error != Clags_Error_Ok) return clags_error(error, parg, arg_config);
            } else {
                clags_unreachable("arg type has changed");
            }
        } else if (arg_config != NULL){
            // flag matches unreachable argument definition
            if (flag_len > 1){
                clags_log(config, Clags_Error, "Unknown short flag '-%c' in combination '%s'! Did you mean to use it with the '%s' subcommand?", *c, arg, arg_config->name);
            } else{
                clags_log(config, Clags_Error, "Unknown short flag '-%c'! Did you mean to use it with the '%s' subcommand?", *c, arg_config->name);
            }
            return clags_error(Clags_Error_InvalidOption, NULL, NULL);
        } else {
            // could not find a matching argument definition
            if (flag_len > 1){
                clags_log(config, Clags_Error, "Unknown short flag '-%c' in combination '%s'!", *c, arg);
            } else{
                clags_log(config, Clags_Error, "Unknown short flag '-%c'!", *c);
            }
            return clags_error(Clags_Error_InvalidOption, NULL, NULL);
        }
    }
    return clags_error(Clags_Error_Ok, NULL, NULL);
}

CLAGS_PRIV clags_error_t clags__parse_positional(clags_config_t *config, clags__parser_t *parser, clags_config_t **error_config, bool *do_exit)
{
    *error_config = config;
    *do_exit = false;
    const char *arg = parser->argv[parser->index];
    if (parser->pos_count >= parser->args.positional_count){
        clags_log(config, Clags_Error, "Unknown additional argument (%zu/%zu): '%s'!", parser->pos_count+1, parser->args.positional_count, arg);
        return clags_error(Clags_Error_TooManyArguments, NULL, NULL);
    }
    clags_arg_t *parg = parser->args.positionals[parser->pos_count];
    clags_positional_t pos = parg->as.pos;
    if (pos.value_type == Clags_Subcmd){
        *do_exit = true; // no more parsing after subcommand

        clags_subcmd_t *selected_subcmd = NULL; // catch the selected subcommand
        clags_error_type_t error = clags__set_arg(config, pos.value_type, pos.arg_name, arg, &selected_subcmd, pos.using.subcmds, false);
        if (error != Clags_Error_Ok) return clags_error(error, parg, config);

        if (pos.variable){
            *(clags_subcmd_t**) pos.variable = selected_subcmd;
        }
        // recursively parse the subcommand's config
        clags_config_t *failed_config = clags_parse((int) (parser->argc - parser->index), parser->argv + parser->index, selected_subcmd->config);
        if (failed_config != NULL){
            *error_config = failed_config;
            return clags_error(Clags_Error_ChildConfigError, NULL, NULL);
        }
    }
    if (pos.is_list){
        parser->in_list = true;
    } else {
        parser->pos_count += 1;
        if (!pos.optional) parser->req_count += 1;
    }
    parser->parsing_optionals = pos.optional;

    clags_error_type_t error = clags__set_arg(config, pos.value_type, pos.arg_name, arg, pos.variable, pos.using._data, pos.is_list);
    if (error != Clags_Error_Ok) return clags_error(error, parg, config);

    return clags_error(Clags_Error_Ok, NULL, NULL);
}

CLAGS_PRIV bool clags__parse_option_toggle(clags_config_t *config, clags__parser_t *parser, const char *arg)
{
    if (strcmp(arg, "--") == 0){
        if (parser->accept_options || config->options.allow_option_parsing_toggle){
            parser->accept_options = !parser->accept_options;
            return true;
        }
    }
    return false;
}

CLAGS_PRIV bool clags__parse_ignore_prefix(clags_config_t *config, clags__parser_t *parser, const char *arg, const char *ignore_prefix)
{
    if (ignore_prefix != NULL){
        size_t ignore_prefix_len = strlen(ignore_prefix);
        if (strncmp(arg, ignore_prefix, ignore_prefix_len) == 0){
            parser->args_ignored = true;
            // track ignored arguments if thus configured
            if (config->options.ignored_args){
                if (config->options.ignored_args->item_size == 0) config->options.ignored_args->item_size = sizeof(char*);
                (void) clags__append_to_list(config, Clags_String, NULL, arg+ignore_prefix_len, config->options.ignored_args, NULL);
            }
            return true;
        }
    }
    return false;
}

CLAGS_PRIV bool clags__parse_list_terminator(clags__parser_t *parser, const char *arg, const char *list_term)
{
    if (list_term != NULL && strcmp(arg, list_term) == 0){
        if (parser->in_list){
            parser->in_list = false;
            parser->pos_count += 1;
            if (!parser->parsing_optionals) parser->req_count += 1;
        }
        return true;
    }
    return false;
}

CLAGS_API clags_config_t* clags_parse(int argc, char *argv[], clags_config_t *config)
{
    if (config == NULL || argc < 1 || argv == NULL) return NULL;
    // validate the config
    if (config->state == Clags_Config_Unvalidated){
        clags_config_t *failed_config = clags_validate(argv[0], config);
        if (failed_config != NULL){
            clags_log(config, Clags_ConfigError, "validation of config '%s' failed!", failed_config->name);
            return failed_config;
        }
    }
    if (config->state != Clags_Config_Valid){
        clags_log(config, Clags_ConfigError, "passed config is invalid!");
        return config;
    }

    clags_error_t default_error = clags__set_defaults(config);
    if (default_error.type != Clags_Error_Ok){
        config->error = default_error;
        clags_log(config, Clags_ConfigError, "failed to set default values for config");
        return config;
    }

    config->error = clags_error(Clags_Error_Ok, NULL, NULL);
    clags_config_t *result = NULL;

    // sort arguments by type
    clags_arg_t **positionals = CLAGS_CALLOC(config->args_count, sizeof(*positionals));
    clags_assert(!config->args_count || positionals, "Out of memory!");

    clags__parser_t parser = {
        .args={.positionals=positionals, .options=NULL, .flags=NULL},
        .argc = (size_t) argc,
        .argv = argv,
        .args_ignored = false,
        .in_list = false,
        .parsing_optionals = false,
        .accept_options = true,
        .pos_count = 0,
        .req_count = 0,
    };
    clags__sort_args(&parser.args, config);

    const char *ignore_prefix = config->options.ignore_prefix;
    const char *list_term = config->options.list_terminator;

    // iterate over the provided arguments, skipping the first one
    for (parser.index=1; parser.index<parser.argc; ++parser.index){
        const char *arg = parser.argv[parser.index];

        // toggle option and flag parsing based on '--'
        if (clags__parse_option_toggle(config, &parser, arg)) continue;

        // ignore arguments prefixed with `ignore_prefix`
        if (clags__parse_ignore_prefix(config, &parser, arg, ignore_prefix)) continue;

        // detect list terminator
        if (clags__parse_list_terminator(&parser, arg, list_term)) continue;

        bool exit = false;
        clags_config_t *error_config;
        clags_error_t error;

        // detect argument type and parse
        if (parser.accept_options && strncmp(arg, "--", 2) == 0){
            error = clags__parse_long_options_and_flags(config, &parser, &error_config, &exit);
        } else if (parser.accept_options && arg[0] == '-' && !isdigit((unsigned char)arg[1]) && !(arg[1] == '.' && isdigit((unsigned char)arg[2]))){
            // argument is a short flag and not a negative number
            error = clags__parse_short_options_and_flags(config, &parser, &error_config, &exit);
        } else{
            error = clags__parse_positional(config, &parser, &error_config, &exit);
        }

        if (error.type != Clags_Error_Ok){
            if (error.type != Clags_Error_ChildConfigError) error_config->error = error;
            clags_return_defer(error_config);
        }
        if (exit) clags_return_defer(NULL);
    }
    if (parser.in_list){
        // terminate last positional argument
        parser.pos_count += 1;
        if (!parser.parsing_optionals) parser.req_count += 1;
    }

    if (parser.args_ignored) clags_log(config, Clags_Warning, "Arguments were ignored because they were prefixed with '%s'", ignore_prefix);

    // report missing positional arguments
    if (parser.req_count < parser.args.required_count){
        clags_sb_t sb = {0};
        clags_sb_appendf(&sb, "Missing required arguments (%zu/%zu):", parser.req_count, parser.args.required_count);
        for (size_t i=parser.pos_count; i<parser.args.required_count; ++i){
            clags_sb_appendf(&sb, " <%s>", parser.args.positionals[i]->as.pos.arg_name);
        }
        clags_sb_appendf(&sb, "!");
        clags_log_sb(config, Clags_Error, &sb);
        clags_sb_free(&sb);

        config->error = clags_error(Clags_Error_TooFewArguments, parser.args.positionals[parser.pos_count], config);
        clags_return_defer(config);
    }

    clags_return_defer(NULL);

defer:
    // cleanup memory of sorted args
    CLAGS_FREE(positionals, config->args_count*sizeof(*positionals));
    return result;
}

/* Usage Printing */

CLAGS_PRIV clags__usage_lengths_t clags__print_lhs(clags_sb_t *sb, char short_flag, const char *long_flag, const char *arg_name, bool optional, int remaining)
{
    clags__usage_lengths_t result = {0};
    
    bool is_option = arg_name != NULL;
    bool truncate = remaining >= 0;

    int n = 0;
    size_t chars_written = 0;

    // Write Prefix
    if (short_flag && long_flag){
        n = clags_sb_appendf(sb, "-%c, --", short_flag);
    } else if (long_flag){
        n = clags_sb_appendf(sb, "--");
    } else if (short_flag){
        n = clags_sb_appendf(sb, "-%c%s", short_flag, is_option?" ":"");
    }
    chars_written += (n < 0)? 0 : (size_t) n;
    result.prefix = chars_written;

    // Write Name (truncatable part)
    n = 0;
    if (long_flag){
        n = clags_sb_appendf(sb, "%.*s%s", remaining, long_flag, truncate ? CLAGS__USAGE_TRUNCATION_ELLIPSIS:"");
    } else if (!short_flag || is_option){
        n = clags_sb_appendf(sb, "%.*s%s", remaining, arg_name, truncate ? CLAGS__USAGE_TRUNCATION_ELLIPSIS:"");
    }
    chars_written += (n < 0)? 0 : (size_t) n;

    // Write Suffix
    n = 0;
    size_t before_suffix = chars_written;
    if (long_flag && is_option){
        n = clags_sb_appendf(sb, "(=)%s", arg_name);
    } else if (optional){
        n = clags_sb_appendf(sb, " (optional)");
    }
    chars_written += (n < 0)? 0 : (size_t) n;

    result.total = chars_written;
    result.suffix = result.total - before_suffix;

    clags_sb_append_null(sb);
    return result;
}

// write the lhs of a usage line into a buffer
// truncate the format where possible if the format would exceed the config's usage alignment
CLAGS_PRIV void clags__format_lhs(clags_sb_t *sb, size_t buffer_size, char short_flag, const char *long_flag, const char *arg_name, bool optional)
{
    // get the length of the format to print
    clags__usage_lengths_t lengths = clags__print_lhs(NULL, short_flag, long_flag, arg_name, optional, -1);

    // check whether the needed format fits into the buffer
    if (lengths.total < buffer_size){
        // write into the buffer
        (void) clags__print_lhs(sb, short_flag, long_flag, arg_name, optional, -1);
        return;
    }
    
    // format is too long, try to truncate `long_flag` for options and flags or `arg_name` for positionals

    // chars to print for the name to truncate
    int remaining = (int) buffer_size - (int) (lengths.prefix + lengths.suffix) - strlen(CLAGS__USAGE_TRUNCATION_ELLIPSIS) - 1;
    if (remaining < 0){
        // even with truncation the lhs does not fit into the buffer,
        // let clags_sb_appendf truncate the format blindly
        remaining = 0;
    }
    // write the truncated format into the buffer
    (void) clags__print_lhs(sb, short_flag, long_flag, arg_name, optional, remaining);
}

CLAGS_PRIV void clags__choice_usage(FILE* file, clags_config_t *config, clags_value_type_t type, bool is_list, const char *default_value, void *data)
{
    clags_choices_t *choices = (clags_choices_t*) data;
    if (!choices->print_no_details || choices->count > CLAGS_MAX_INLINE_CHOICES){
        // print detailed one-choice-per-line usage if thus configured or too many choices are given
        fprintf(file, " (%s%s)", clags__types[type].name, is_list?"[]":"");
        if (choices->case_insensitive || default_value) {
            fprintf(file, " (%s%s%s",
                   choices->case_insensitive ? "case-insensitive" : "",
                   (choices->case_insensitive && default_value) ? ", " : "",
                   default_value ? "default: " : "");

            if (default_value) fprintf(file, "%s", default_value);
            fprintf(file, ")");
        }
        fprintf(file, "\n        Choices:");
        for (size_t i=0; i<choices->count; ++i){
            clags_choice_t choice = choices->items[i];
            fprintf(file, "\n          - %*s", CLAGS__USAGE_PRINTF_ALIGNMENT(config->usage_alignment)+8, choice.value);
            if (choice.description) fprintf(file, " : %s", choice.description);
        }
    } else{
        // print inline usage
        fprintf(file, " (%s%s:", clags__types[type].name, is_list?"[]":"");
        for (size_t i=0; i<choices->count; ++i){
            fprintf(file, "%s%s", i>0?" | ":" ", choices->items[i].value);
        }
        fprintf(file, ")");
        if (choices->case_insensitive || default_value) {
            fprintf(file, " (%s%s%s",
                   choices->case_insensitive ? "case-insensitive" : "",
                   (choices->case_insensitive && default_value) ? ", " : "",
                   default_value ? "default: " : "");

            if (default_value) fprintf(file, "%s", default_value);
            fprintf(file, ")");
        }
    }
}

CLAGS_PRIV void clags__subcmd_usage(FILE *file, clags_config_t *config, clags_value_type_t type, bool is_list, const char *default_value, void *data)
{
    (void) is_list;
    clags_subcmds_t *subcmds = (clags_subcmds_t*) data;
    fprintf(file, " (%s)", clags__types[type].name);
    if (default_value) fprintf(file, " (default: %s)", default_value);
    fprintf(file, "\n      Subcommands:");
    for (size_t i=0; i<subcmds->count; ++i){
        clags_subcmd_t subcmd = subcmds->items[i];
        fprintf(file, "\n        - %*s : %s", CLAGS__USAGE_PRINTF_ALIGNMENT(config->usage_alignment)+6, subcmd.name, subcmd.description);
    }
}

CLAGS_PRIV void clags__custom_usage(FILE *file, clags_config_t *config, clags_value_type_t type, bool is_list, const char *default_value, void *data)
{
    (void) config;
    (void) type;
    clags_custom_t *custom = (clags_custom_t*) data;
    clags_usage_func_t func = custom->usage;
    if (func != NULL){
        func(file, config, type, is_list, default_value, custom->data);
    } else{
        fprintf(file, " (%s%s)", custom->name, is_list?"[]":"");
        if (default_value) fprintf(file, " (default: %s)", default_value);
    }
}

#ifndef CLAGS_NO_INT_TYPES

CLAGS_PRIV void clags__int_usage(FILE *file, clags_config_t *config, clags_value_type_t type, bool is_list, const char *default_value, void *data)
{
    (void) config;
    clags_range_t *range = (clags_range_t*) data;
    fprintf(file, " (%s%s", clags__types[type].name, is_list?"[]":"");
    if (range) fprintf(file, ", %"PRId64"-%"PRId64, range->min.as_int, range->max.as_int);
    fprintf(file, ")");
    if (default_value) fprintf(file, " (default: %s)", default_value);
}

CLAGS_PRIV void clags__uint_usage(FILE *file, clags_config_t *config, clags_value_type_t type, bool is_list, const char *default_value, void *data)
{
    (void) config;
    clags_range_t *range = (clags_range_t*) data;
    fprintf(file, " (%s%s", clags__types[type].name, is_list?"[]":"");
    if (range) fprintf(file, ", %"PRIu64"-%"PRIu64, range->min.as_uint, range->max.as_uint);
    fprintf(file, ")");
    if (default_value) fprintf(file, " (default: %s)", default_value);
}

#endif // !CLAGS_NO_INT_TYPES

#ifndef CLAGS_NO_REAL_TYPES

CLAGS_PRIV void clags__real_usage(FILE* file, clags_config_t *config, clags_value_type_t type, bool is_list, const char *default_value, void *data)
{
    (void) config;
    clags_range_t *range = (clags_range_t*) data;
    fprintf(file, " (%s%s", clags__types[type].name, is_list?"[]":"");
    if (range) fprintf(file, ", %g-%g", range->min.as_double, range->max.as_double);
    fprintf(file, ")");
    if (default_value) fprintf(file, " (default: %s)", default_value);
}

#endif // !CLAGS_NO_REAL_TYPES

CLAGS_PRIV void clags__default_usage(FILE* file, clags_config_t *config, clags_value_type_t type, bool is_list, const char *default_value, void *data)
{
    (void) config;
    (void) data;
    fprintf(file, " (%s%s)", clags__types[type].name, is_list?"[]":"");
    if (default_value) fprintf(file, " (default: %s)", default_value);
}

CLAGS_PRIV void clags__subcommand_path_usage(FILE *file, const char *program_name, clags_config_t *config)
{
    if (config->parent){
        clags__subcommand_path_usage(file, program_name, config->parent);
        fprintf(file, " %s", config->name);
    } else{
        fprintf(file, "Usage: %s", program_name?program_name:config->name);
    }
}

CLAGS_PRIV void clags__summary_usage(FILE *file, clags_config_t *config, const char *program_name, clags__args_t args)
{
    clags__subcommand_path_usage(file, program_name, config); // the subcommand path (e.g. './main foo bar baz')

    if (args.option_count) fprintf(file, " [OPTIONS]");
    if (args.flag_count) fprintf(file, " [FLAGS]");

    // print positional arguments
    bool last_was_list = false;
    for (size_t i = 0; i < args.positional_count; ++i) {
        if (last_was_list) {
            // print `list_terminator` after each list argument
            fprintf(file, " %s", config->options.list_terminator);
            last_was_list = false;
        }
        // print positional argument with correct usage line syntax
        clags_positional_t pos = args.positionals[i]->as.pos;
        fprintf(file, " %c", pos.optional? '[':'<');
        if (pos.is_list) {
            fprintf(file, "%s..", pos.arg_name);
            last_was_list = true;
        } else {
            fprintf(file, "%s", pos.arg_name);
        }
        fprintf(file, "%c", pos.optional? ']':'>');
    }
    fprintf(file, "\n");
}

CLAGS_PRIV void clags__positional_usage(clags_config_t *config, FILE *file, clags_positional_t pos, clags_sb_t *sb)
{
    // positional usage is : <name> [(optional)]
    sb->count = 0;
    size_t buffer_size = CLAGS__USAGE_TEMP_BUFFER_SIZE(config->usage_alignment);
    clags__format_lhs(sb, buffer_size, 0, NULL, pos.arg_name, pos.optional);
    fprintf(file, "    %*.*s : %s", CLAGS__USAGE_PRINTF_ALIGNMENT(config->usage_alignment), (int) buffer_size - 1, sb->items, pos.description);
    clags__types[pos.value_type].usage(file, config, pos.value_type, pos.is_list, pos.default_input, pos.using._data);
    fprintf(file, "\n");
}

CLAGS_PRIV void clags__option_usage(clags_config_t *config, FILE *file, clags_option_t opt, clags_sb_t *sb)
{
    // option usage is : <flags> : <description> <type-specific-usage>
    sb->count = 0;
    size_t buffer_size = CLAGS__USAGE_TEMP_BUFFER_SIZE(config->usage_alignment);
    clags__format_lhs(sb, buffer_size, opt.short_flag, opt.long_flag, opt.arg_name, false);
    fprintf(file, "    %*.*s : %s", CLAGS__USAGE_PRINTF_ALIGNMENT(config->usage_alignment), (int) buffer_size - 1, sb->items, opt.description);
    clags__types[opt.value_type].usage(file, config, opt.value_type, opt.is_list, opt.default_input, opt.using._data);
    fprintf(file, "\n");
}

CLAGS_PRIV void clags__flag_usage(clags_config_t *config, FILE *file, clags_flag_t flag, clags_sb_t *sb)
{
    // flag usage is : <flags> : <description>
    sb->count = 0;
    size_t buffer_size = CLAGS__USAGE_TEMP_BUFFER_SIZE(config->usage_alignment);
    clags__format_lhs(sb, buffer_size, flag.short_flag, flag.long_flag, NULL, false);
    fprintf(file, "    %*.*s : %s%s\n", CLAGS__USAGE_PRINTF_ALIGNMENT(config->usage_alignment), (int) buffer_size - 1, sb->items, flag.description, flag.exit?" and exit":"");
}

CLAGS_PRIV void clags__inherited_options_usage(FILE *file, clags_config_t *config, clags_sb_t *sb)
{
    if (config->options.no_inheritance) return;
    clags_config_t *ancestor = config->parent;
    // print the options inherited from each reachable ancestor config
    while (ancestor != NULL){
        if (ancestor->inherit_options_count > 0){
            fprintf(file, "  Inherited Options (from '%s'):\n", ancestor->name);
            for (size_t i=0; i<ancestor->args_count; ++i){
                clags_arg_t arg = ancestor->args[i];
                if (arg.type == Clags_Option && arg.as.opt.inherit){
                    clags__option_usage(config, file, arg.as.opt, sb);
                }
            }
        }
        if (ancestor->options.no_inheritance) break;
        ancestor = ancestor->parent;
    }
}

CLAGS_PRIV void clags__inherited_flags_usage(FILE *file, clags_config_t *config, clags_sb_t *sb)
{
    if (config->options.no_inheritance) return;
    clags_config_t *ancestor = config->parent;
    // print the flags inherited from each reachable ancestor config
    while (ancestor != NULL){
        if (ancestor->inherit_flags_count > 0){
            fprintf(file, "  Inherited Flags (from '%s'):\n", ancestor->name);
            for (size_t i=0; i<ancestor->args_count; ++i){
                clags_arg_t arg = ancestor->args[i];
                if (arg.type == Clags_Flag && arg.as.flag.inherit){
                    clags__flag_usage(config, file, arg.as.flag, sb);
                }
            }
        }
        if (ancestor->options.no_inheritance) break;
        ancestor = ancestor->parent;
    }
}

CLAGS_API void clags_usage_to_file(FILE *file, const char *program_name, clags_config_t *config)
{
    if (file == NULL || config == NULL || config->args == NULL || config->state != Clags_Config_Valid) return;

    // sort arguments by type
    clags_arg_t **positionals = CLAGS_CALLOC(config->args_count, sizeof(*positionals));
    clags_arg_t     **options = CLAGS_CALLOC(config->args_count, sizeof(*options));
    clags_arg_t       **flags = CLAGS_CALLOC(config->args_count, sizeof(*flags));
    clags_assert((!config->args_count) || (positionals && options && flags), "Out of memory!");

    clags__args_t args = {.positionals=positionals, .options=options, .flags=flags};
    clags__sort_args(&args, config);

    // a temporary buffer for prefix printing
    size_t temp_buffer_size = CLAGS__USAGE_TEMP_BUFFER_SIZE(config->usage_alignment);
    clags_sb_t sb = {0};
    clags__sb_reserve(&sb, temp_buffer_size);

    // usage line printing
    clags__summary_usage(file, config, program_name, args);

    // print arguments
    // positional arguments
    if (args.positional_count) {
        fprintf(file, "  Arguments:\n");
        for (size_t i = 0; i < args.positional_count; ++i) {
            clags_positional_t pos = args.positionals[i]->as.pos;
            clags__positional_usage(config, file, pos, &sb);
        }
    }

    // option arguments
    if (args.option_count) {
        fprintf(file, "  Options:\n");
        for (size_t i = 0; i < args.option_count; ++i) {
            clags_option_t opt = args.options[i]->as.opt;
            clags__option_usage(config, file, opt, &sb);
        }
    }

    // inherited option arguments
    clags__inherited_options_usage(file, config, &sb);

    // flag arguments
    if (args.flag_count) {
        fprintf(file, "  Flags:\n");
        for (size_t i = 0; i < args.flag_count; ++i) {
            clags_flag_t flag = args.flags[i]->as.flag;
            clags__flag_usage(config, file, flag, &sb);
        }
    }

    // inherited flag arguments
    clags__inherited_flags_usage(file, config, &sb);

    // print 'Notes' section if not disabled
    if (!config->options.print_no_notes &&
        (config->options.list_terminator || config->options.ignore_prefix || config->options.allow_option_parsing_toggle)) {
        fprintf(file, "\n  Notes:\n");
        if (config->options.allow_option_parsing_toggle){
            fprintf(file, "    '--' toggles option and flag parsing and can re-enable parsing when provided again.\n");
        }
        if (config->options.list_terminator){
            fprintf(file, "    '%s' terminates a list argument.\n", config->options.list_terminator);
        }
        if (config->options.ignore_prefix){
            fprintf(file, "    Arguments prefixed with '%s' are ignored.\n", config->options.ignore_prefix);
        }
    }

    // free argument containers and temporary buffer
    CLAGS_FREE(positionals, config->args_count*sizeof(*positionals));
    CLAGS_FREE(options, config->args_count*sizeof(*options));
    CLAGS_FREE(flags, config->args_count*sizeof(*flags));
    clags_sb_free(&sb);
}

CLAGS_API void clags_usage(const char *program_name, clags_config_t *config)
{
    clags_usage_to_file(stdout, program_name, config);
}

/* Deallocation Functions */

CLAGS_API void clags_list_free(clags_list_t *list)
{
    if (list == NULL) return;
    // free the allocated memory
    CLAGS_FREE(list->items, list->capacity*list->item_size);
    // reset the list's state
    list->items = NULL;
    list->count = list->capacity = 0;
}

CLAGS_API void clags_config_free_allocs(clags_config_t *config)
{
    if (config == NULL) return;
    clags_list_t *allocs = &config->allocs;
    for (size_t i=0; i<allocs->count; ++i){
        // free each allocated blob with this respective size
        clags__alloc_t alloc = ((clags__alloc_t*) allocs->items)[i];
        CLAGS_FREE(alloc.data, alloc.size);
        (void) alloc; // to prevent `Wunused-variable` if freeing is disabled
    }
    // free the allocs container itself
    clags_list_free(allocs);
}

CLAGS_API void clags_config_free(clags_config_t *config)
{
    if (config == NULL) return;
    for (size_t i=0; i<config->args_count; ++i){
        clags_arg_t arg = config->args[i];
        if (arg.type == Clags_Positional && arg.as.pos.is_list){
            // free positional lists
            clags_list_free(arg.as.pos.variable);
        } else if (arg.type == Clags_Option && arg.as.opt.is_list){
            // free option lists
            clags_list_free(arg.as.opt.variable);
        }
    }
    // free ignored args list
    clags_list_free(config->options.ignored_args);
    // free config's allocs
    clags_config_free_allocs(config);
}

CLAGS_API void clags_free(clags_config_t *config)
{
    if (config == NULL) return;
    for (size_t i=0; i<config->args_count; ++i){
        clags_arg_t arg = config->args[i];
        if (arg.type == Clags_Positional){
            clags_positional_t pos = arg.as.pos;
            // look for subcommand configs into which to descend
            if (pos.value_type == Clags_Subcmd && pos.using.subcmds != NULL){
                clags_subcmds_t *subcmds = pos.using.subcmds;
                for (size_t j=0; j<subcmds->count; ++j){
                    clags_free(subcmds->items[j].config);
                }
            }
            // free positional lists
            if (pos.is_list) clags_list_free(pos.variable);
        } else if (arg.type == Clags_Option && arg.as.opt.is_list){
            // free option lists
            clags_list_free(arg.as.opt.variable);
        }
    }
    // free ignored args list
    clags_list_free(config->options.ignored_args);
    // free config's allocs
    clags_config_free_allocs(config);
}

#endif // CLAGS_IMPLEMENTATION
