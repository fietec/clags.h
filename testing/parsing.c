#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../clags.h"
#include "tests.h"

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define TEST_GETPID _getpid
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_RMDIR(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_GETPID getpid
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_RMDIR(path) rmdir(path)
#endif

static bool parse_test_custom_verify(
    clags_config_t *config,
    const char *arg_name,
    const char *arg,
    void *variable,
    void *data)
{
    (void)data;

    if (arg == NULL || strncmp(arg, "num:", 4) != 0){
        clags_log(config, Clags_Error, "expected a value beginning with 'num:'");
        return false;
    }

    char *end = NULL;
    errno = 0;
    long value = strtol(arg + 4, &end, 10);

    if (errno != 0 || end == arg + 4 || *end != '\0' || value < 1 || value > 9){
        clags_log(config, Clags_Error, "custom value for '%s' must be num:1 through num:9", arg_name);
        return false;
    }

    if (variable != NULL) *(int*)variable = (int)value;
    return true;
}

bool test_value_string(clags_sb_t *sb)
{
    char *value = NULL;
    clags_arg_t args[] = {
        clags_option('s', "string", &value, "TEXT", "a string value",),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv1[] = {"test", "--string", "hello world"};
    test(clags_parse(arr_len(argv1), argv1, &config).type == Clags_Error_Ok);
    test(value != NULL && strcmp(value, "hello world") == 0);

    char *argv2[] = {"test", "--string", ""};
    test(clags_parse(arr_len(argv2), argv2, &config).type == Clags_Error_Ok);
    test(value != NULL && strcmp(value, "") == 0);

    return true;
}

bool test_value_bool(clags_sb_t *sb)
{
    bool a = false;
    bool b = false;
    clags_arg_t args[] = {
        clags_option('a', NULL, &a, "BOOL", "boolean option", .value_type=Clags_Bool,),
        clags_option('b', NULL, &b, "BOOL", "boolean option", .value_type=Clags_Bool,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv1[] = {"", "-a", "true", "-b", "FALSE"};
    test(clags_parse(arr_len(argv1), argv1, &config).type == Clags_Error_Ok);
    test(a == true);
    test(b == false);

    char *argv2[] = {"", "-a", "no", "-b", "y"};
    test(clags_parse(arr_len(argv2), argv2, &config).type == Clags_Error_Ok);
    test(a == false);
    test(b == true);

    char *argv3[] = {"", "-a", "trues"};
    test(clags_parse(arr_len(argv3), argv3, &config).type == Clags_Error_InvalidValue);

    return true;
}


bool test_value_choice(clags_sb_t *sb)
{
    clags_choice_t values[] = {
        {"red", "red"},
        {"green", "green"},
        {"blue", "blue"},
    };
    clags_choices_t choices = clags_choices(values, .case_insensitive=true);
    clags_choice_t *selected = NULL;

    clags_arg_t args[] = {
        clags_option('c', "color", &selected, "COLOR", "a color",
            .value_type=Clags_Choice, .using.choices=&choices,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv1[] = {"test", "--color", "GREEN"};
    test(clags_parse(arr_len(argv1), argv1, &config).type == Clags_Error_Ok);
    test(selected == &values[1]);

    char *argv2[] = {"test", "--color", "purple"};
    test(clags_parse(arr_len(argv2), argv2, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_subcmd(clags_sb_t *sb)
{
    char *output = NULL;
    clags_arg_t child_args[] = {
        clags_positional(&output, "output", "the output value",),
    };
    clags_config_t child_config = clags_config(child_args, GLOBAL_OPTIONS,);

    clags_subcmd_t commands[] = {
        {"run", "run a command", &child_config},
        {"inspect", "inspect a value", NULL},
    };
    clags_subcmds_t subcommands = clags_subcmds(commands);
    clags_subcmd_t *selected = NULL;

    clags_arg_t args[] = {
        clags_positional(&selected, "command", "the command to run",
            .value_type=Clags_Subcmd, .using.subcmds=&subcommands,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv1[] = {"test", "run", "result.txt"};
    test(clags_parse(arr_len(argv1), argv1, &config).type == Clags_Error_Ok);
    test(selected == &commands[0]);
    test(output != NULL && strcmp(output, "result.txt") == 0);

    char *argv2[] = {"test", "missing"};
    test(clags_parse(arr_len(argv2), argv2, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_custom(clags_sb_t *sb)
{
    int value = 0;
    clags_custom_t custom = clags_custom("number 1-9", parse_test_custom_verify,);

    clags_arg_t args[] = {
        clags_option('n', "number", &value, "NUMBER", "a custom number",
            .value_type=Clags_Custom, .using.custom=&custom,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv1[] = {"test", "--number", "num:7"};
    test(clags_parse(arr_len(argv1), argv1, &config).type == Clags_Error_Ok);
    test(value == 7);

    char *argv2[] = {"test", "--number", "num:10"};
    test(clags_parse(arr_len(argv2), argv2, &config).type == Clags_Error_InvalidValue);

    char *argv3[] = {"test", "--number", "7"};
    test(clags_parse(arr_len(argv3), argv3, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_int(clags_sb_t *sb)
{
    int64_t value = 0;
    clags_arg_t args[] = {
        clags_option('i', "int", &value, "INTEGER", "a signed integer",
            .value_type=Clags_Int,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv1[] = {"test", "--int", "-0x2a"};
    test(clags_parse(arr_len(argv1), argv1, &config).type == Clags_Error_Ok);
    test(value == -42);

    char *argv2[] = {"test", "--int", "0b10101"};
    test(clags_parse(arr_len(argv2), argv2, &config).type == Clags_Error_Ok);
    test(value == 21);

    char *argv3[] = {"test", "--int", "21x"};
    test(clags_parse(arr_len(argv3), argv3, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_int_widths(clags_sb_t *sb)
{
    int8_t i8 = 0;
    int16_t i16 = 0;
    int32_t i32 = 0;
    int64_t i64 = 0;

    clags_arg_t args[] = {
        clags_option('\0', "i8", &i8, "N", "8-bit signed integer", .value_type=Clags_Int8),
        clags_option('\0', "i16", &i16, "N", "16-bit signed integer", .value_type=Clags_Int16),
        clags_option('\0', "i32", &i32, "N", "32-bit signed integer", .value_type=Clags_Int32),
        clags_option('\0', "i64", &i64, "N", "64-bit signed integer", .value_type=Clags_Int64),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv[] = {
        "test",
        "--i8", "-128",
        "--i16", "32767",
        "--i32", "-2147483648",
        "--i64", "9223372036854775807",
    };
    test(clags_parse(arr_len(argv), argv, &config).type == Clags_Error_Ok);
    test(i8 == INT8_MIN);
    test(i16 == INT16_MAX);
    test(i32 == INT32_MIN);
    test(i64 == INT64_MAX);

    char *bad_i8[] = {"test", "--i8", "128"};
    test(clags_parse(arr_len(bad_i8), bad_i8, &config).type == Clags_Error_InvalidValue);

    char *bad_i16[] = {"test", "--i16", "-32769"};
    test(clags_parse(arr_len(bad_i16), bad_i16, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_uint(clags_sb_t *sb)
{
    uint64_t value = 0;
    clags_arg_t args[] = {
        clags_option('u', "uint", &value, "INTEGER", "an unsigned integer",
            .value_type=Clags_UInt,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv1[] = {"test", "--uint", "0xFE"};
    test(clags_parse(arr_len(argv1), argv1, &config).type == Clags_Error_Ok);
    test(value == 254);

    char *argv2[] = {"test", "--uint", "0"};
    test(clags_parse(arr_len(argv2), argv2, &config).type == Clags_Error_Ok);
    test(value == 0);

    char *argv3[] = {"test", "--uint", "-1"};
    test(clags_parse(arr_len(argv3), argv3, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_uint_widths(clags_sb_t *sb)
{
    uint8_t u8 = 0;
    uint16_t u16 = 0;
    uint32_t u32 = 0;
    uint64_t u64 = 0;

    clags_arg_t args[] = {
        clags_option('\0', "u8", &u8, "N", "8-bit unsigned integer", .value_type=Clags_UInt8),
        clags_option('\0', "u16", &u16, "N", "16-bit unsigned integer", .value_type=Clags_UInt16),
        clags_option('\0', "u32", &u32, "N", "32-bit unsigned integer", .value_type=Clags_UInt32),
        clags_option('\0', "u64", &u64, "N", "64-bit unsigned integer", .value_type=Clags_UInt64),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv[] = {
        "test",
        "--u8", "255",
        "--u16", "65535",
        "--u32", "4294967295",
        "--u64", "18446744073709551615",
    };
    test(clags_parse(arr_len(argv), argv, &config).type == Clags_Error_Ok);
    test(u8 == UINT8_MAX);
    test(u16 == UINT16_MAX);
    test(u32 == UINT32_MAX);
    test(u64 == UINT64_MAX);

    char *bad_u8[] = {"test", "--u8", "256"};
    test(clags_parse(arr_len(bad_u8), bad_u8, &config).type == Clags_Error_InvalidValue);

    char *bad_u32[] = {"test", "--u32", "4294967296"};
    test(clags_parse(arr_len(bad_u32), bad_u32, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_int_custom_range(clags_sb_t *sb)
{
    int64_t value = 0;
    clags_range_t range = clags_int_range(-12, 34);

    clags_arg_t args[] = {
        clags_option('i', "int", &value, "N", "integer constrained to -12..34",
            .value_type=Clags_Int, .using.range=&range,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *lower[] = {"test", "--int", "-12"};
    test(clags_parse(arr_len(lower), lower, &config).type == Clags_Error_Ok);
    test(value == -12);

    char *upper[] = {"test", "--int", "34"};
    test(clags_parse(arr_len(upper), upper, &config).type == Clags_Error_Ok);
    test(value == 34);

    char *below[] = {"test", "--int", "-13"};
    test(clags_parse(arr_len(below), below, &config).type == Clags_Error_InvalidValue);

    char *above[] = {"test", "--int", "35"};
    test(clags_parse(arr_len(above), above, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_uint_custom_range(clags_sb_t *sb)
{
    uint64_t value = 0;
    clags_range_t range = clags_uint_range(5, 500);

    clags_arg_t args[] = {
        clags_option('u', "uint", &value, "N", "integer constrained to 5..500",
            .value_type=Clags_UInt, .using.range=&range,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *lower[] = {"test", "--uint", "5"};
    test(clags_parse(arr_len(lower), lower, &config).type == Clags_Error_Ok);
    test(value == 5);

    char *upper[] = {"test", "--uint", "500"};
    test(clags_parse(arr_len(upper), upper, &config).type == Clags_Error_Ok);
    test(value == 500);

    char *below[] = {"test", "--uint", "4"};
    test(clags_parse(arr_len(below), below, &config).type == Clags_Error_InvalidValue);

    char *above[] = {"test", "--uint", "501"};
    test(clags_parse(arr_len(above), above, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_real_float_double(clags_sb_t *sb)
{
    double real = 0.0;
    float single = 0.0f;
    double precise = 0.0;

    clags_arg_t args[] = {
        clags_option('\0', "real", &real, "N", "ranged real", .value_type=Clags_Real),
        clags_option('\0', "float", &single, "N", "single precision", .value_type=Clags_Float),
        clags_option('\0', "double", &precise, "N", "double precision", .value_type=Clags_Double),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv[] = {
        "test",
        "--real", "-0.125",
        "--float", "1.5e2",
        "--double", "2.718281828",
    };
    test(clags_parse(arr_len(argv), argv, &config).type == Clags_Error_Ok);
    test(real > -0.126 && real < -0.124);
    test(single > 149.9f && single < 150.1f);
    test(precise > 2.71828 && precise < 2.71829);

    char *invalid[] = {"test", "--double", "3.14tail"};
    test(clags_parse(arr_len(invalid), invalid, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_real_custom_range(clags_sb_t *sb)
{
    double value = 0.0;
    clags_range_t range = clags_real_range(-0.5, 1.25);

    clags_arg_t args[] = {
        clags_option('r', "ratio", &value, "N", "real constrained to -0.5..1.25",
            .value_type=Clags_Real, .using.range=&range,),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *lower[] = {"test", "--ratio", "-0.5"};
    test(clags_parse(arr_len(lower), lower, &config).type == Clags_Error_Ok);
    test(value == -0.5);

    char *upper[] = {"test", "--ratio", "1.25"};
    test(clags_parse(arr_len(upper), upper, &config).type == Clags_Error_Ok);
    test(value == 1.25);

    char *below[] = {"test", "--ratio", "-0.51"};
    test(clags_parse(arr_len(below), below, &config).type == Clags_Error_InvalidValue);

    char *above[] = {"test", "--ratio", "1.251"};
    test(clags_parse(arr_len(above), above, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_path_file_dir(clags_sb_t *sb)
{
    char file_path[128];
    char dir_path[128];
    int pid = (int)TEST_GETPID();

    snprintf(file_path, sizeof(file_path), "clags_test_file_%d.tmp", pid);
    snprintf(dir_path, sizeof(dir_path), "clags_test_dir_%d", pid);

    FILE *file = fopen(file_path, "wb");
    test(file != NULL);
    if (file != NULL) fclose(file);

    int mkdir_result = TEST_MKDIR(dir_path);
    test(mkdir_result == 0 || errno == EEXIST);

    char *path_value = NULL;
    char *file_value = NULL;
    char *dir_value = NULL;

    clags_arg_t args[] = {
        clags_option('\0', "path", &path_value, "PATH", "any existing path", .value_type=Clags_Path),
        clags_option('\0', "file", &file_value, "FILE", "an existing file", .value_type=Clags_File),
        clags_option('\0', "dir", &dir_value, "DIR", "an existing directory", .value_type=Clags_Dir),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv[] = {
        "test",
        "--path", file_path,
        "--file", file_path,
        "--dir", dir_path,
    };
    test(clags_parse(arr_len(argv), argv, &config).type == Clags_Error_Ok);
    test(path_value != NULL && strcmp(path_value, file_path) == 0);
    test(file_value != NULL && strcmp(file_value, file_path) == 0);
    test(dir_value != NULL && strcmp(dir_value, dir_path) == 0);

    char *file_given_as_dir[] = {"test", "--dir", file_path};
    test(clags_parse(arr_len(file_given_as_dir), file_given_as_dir, &config).type == Clags_Error_InvalidValue);

    char *dir_given_as_file[] = {"test", "--file", dir_path};
    test(clags_parse(arr_len(dir_given_as_file), dir_given_as_file, &config).type == Clags_Error_InvalidValue);

    char *missing_path[] = {"test", "--path", "clags_path_that_does_not_exist_987654321"};
    test(clags_parse(arr_len(missing_path), missing_path, &config).type == Clags_Error_InvalidValue);

    remove(file_path);
    TEST_RMDIR(dir_path);
    return true;
}

bool test_value_size(clags_sb_t *sb)
{
    clags_fsize_t binary = 0;
    clags_fsize_t decimal = 0;
    clags_fsize_t bytes = 0;

    clags_arg_t args[] = {
        clags_option('\0', "binary", &binary, "SIZE", "binary size", .value_type=Clags_Size),
        clags_option('\0', "decimal", &decimal, "SIZE", "decimal size", .value_type=Clags_Size),
        clags_option('\0', "bytes", &bytes, "SIZE", "byte count", .value_type=Clags_Size),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv[] = {
        "test",
        "--binary", "2KiB",
        "--decimal", "3MB",
        "--bytes", "42B",
    };
    test(clags_parse(arr_len(argv), argv, &config).type == Clags_Error_Ok);
    test(binary == 2048);
    test(decimal == 3000000);
    test(bytes == 42);

    char *bad_unit[] = {"test", "--binary", "1XB"};
    test(clags_parse(arr_len(bad_unit), bad_unit, &config).type == Clags_Error_InvalidValue);

    char *negative[] = {"test", "--bytes", "-1"};
    test(clags_parse(arr_len(negative), negative, &config).type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_time_s_time_ns(clags_sb_t *sb)
{
    clags_time_t seconds = 0;
    clags_time_t nanoseconds = 0;

    clags_arg_t args[] = {
        clags_option('\0', "seconds", &seconds, "TIME", "duration in seconds", .value_type=Clags_TimeS),
        clags_option('\0', "nanoseconds", &nanoseconds, "TIME", "duration in nanoseconds", .value_type=Clags_TimeNS),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS,);

    char *argv[] = {
        "test",
        "--seconds", "1d2h3m4s",
        "--nanoseconds", "2ms500us",
    };
    test(clags_parse(arr_len(argv), argv, &config).type == Clags_Error_Ok);
    test(seconds == 93784);
    test(nanoseconds == 2500000);

    char *subsecond_for_seconds[] = {"test", "--seconds", "1ms"};
    test(clags_parse(arr_len(subsecond_for_seconds), subsecond_for_seconds, &config).type == Clags_Error_InvalidValue);

    char *unknown_unit[] = {"test", "--nanoseconds", "1fortnight"};
    test(clags_parse(arr_len(unknown_unit), unknown_unit, &config).type == Clags_Error_InvalidValue);

    return true;
}

static Test parsing_tests[] = {
    {"string values", test_value_string},
    {"boolean values", test_value_bool},
    {"choice matching", test_value_choice},
    {"subcommand selection", test_value_subcmd},
    {"custom value validation", test_value_custom},
    {"signed integer syntax", test_value_int},
    {"signed integer widths and bounds", test_value_int_widths},
    {"unsigned integer syntax", test_value_uint},
    {"unsigned integer widths and bounds", test_value_uint_widths},
    {"custom signed integer range", test_value_int_custom_range},
    {"custom unsigned integer range", test_value_uint_custom_range},
    {"real, float, and double values", test_value_real_float_double},
    {"custom real range", test_value_real_custom_range},
    {"path, file, and directory values", test_value_path_file_dir},
    {"size units", test_value_size},
    {"time units", test_value_time_s_time_ns},
};

TestSection parsing_section = {
    "Parsing",
    parsing_tests,
    arr_len(parsing_tests),
};
