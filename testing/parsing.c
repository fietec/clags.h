#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../clags.h"
#include "tests.h"

bool custom_verifier(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    (void) arg_name;
    if (arg == NULL) return false;
    if (strncmp(arg, "C:", 2) != 0){
        clags_log(config, Clags_Error, "custom value must start with 'C:'");
        return false;
    }
    const char *payload = arg + 2;
    if (variable) *(char**)variable = clags_config_duplicate_string(config, payload);
    return true;
}

bool test_value_string(clags_sb_t *sb)
{
    char *val = NULL;
    char *argv[] = {"", "hello"};
    clags_arg_t args[] = {
        clags_positional(&val, "str", "a string"),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(val && strcmp(val, "hello") == 0);
    return true;
}

bool test_value_bool(clags_sb_t *sb)
{
    bool b = false;
    char *argv[] = {"", "--bool", "true"};
    clags_arg_t args[] = {
        clags_option('\0', "bool", &b, "BOOL", "boolean option", .value_type=Clags_Bool),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(b == true);
    return true;
}

bool test_value_int(clags_sb_t *sb)
{
    int64_t v = 0;
    char *argv[] = {"", "--val", "-42"};
    clags_arg_t args[] = {
        clags_option('\0', "val", &v, "N", "signed int", .value_type=Clags_Int),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(v == -42);
    return true;
}

bool test_int_range(clags_sb_t *sb)
{
    char *argv[] = {"", "11"};
    clags_range_t range = clags_int_range(0, 10);
    clags_arg_t args[] = {
        clags_positional(NULL, "", "", .value_type=Clags_Int, .using.range=&range),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    clags_config_t *failed = clags_parse(arr_len(argv), argv, &config);
    test(failed != NULL);
    test(failed->error.type == Clags_Error_InvalidValue);

    return true;
}

bool test_value_int8_int16_int32_int64(clags_sb_t *sb)
{
    int8_t i8 = 0;
    int16_t i16 = 0;
    int32_t i32 = 0;
    int64_t i64 = 0;

    char *argv[] = {"", "--i8", "127", "--i16", "-32000", "--i32", "2147483", "--i64", "9000000000"};
    clags_arg_t args[] = {
        clags_option('\0', "i8",  &i8,  "I8",  "int8",  .value_type=Clags_Int8),
        clags_option('\0', "i16", &i16, "I16", "int16", .value_type=Clags_Int16),
        clags_option('\0', "i32", &i32, "I32", "int32", .value_type=Clags_Int32),
        clags_option('\0', "i64", &i64, "I64", "int64", .value_type=Clags_Int64),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(i8  == 127);
    test(i16 == -32000);
    test(i32 == 2147483);
    test(i64 == 9000000000LL);
    return true;
}

bool test_value_uint_uint8_uint16_uint32_uint64(clags_sb_t *sb)
{
    uint64_t u = 0;
    uint8_t u8 = 0;
    uint16_t u16 = 0;
    uint32_t u32 = 0;
    uint64_t u64 = 0;

    char *argv[] = {"", "--u", "42", "--u8", "255", "--u16", "65000", "--u32", "4000000000", "--u64", "9000000000000"};
    clags_arg_t args[] = {
        clags_option('\0', "u",   &u,  "N", "uint",  .value_type=Clags_UInt),
        clags_option('\0', "u8",  &u8, "U8", "uint8", .value_type=Clags_UInt8),
        clags_option('\0', "u16", &u16,"U16","uint16",.value_type=Clags_UInt16),
        clags_option('\0', "u32", &u32,"U32","uint32",.value_type=Clags_UInt32),
        clags_option('\0', "u64", &u64,"U64","uint64",.value_type=Clags_UInt64),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(u  == 42);
    test(u8 == 255);
    test(u16 == 65000);
    test(u32 == 4000000000U);
    test(u64 == 9000000000000ULL);
    return true;
}

bool test_value_real_float_double(clags_sb_t *sb)
{
    double r = 0.0;
    float f = 0.0f;
    double d = 0.0;

    char *argv[] = {"", "--r", "3.14", "--f", "-1.5", "--d", "2.71828"};
    clags_arg_t args[] = {
        clags_option('\0', "r", &r, "R", "real",   .value_type=Clags_Real),
        clags_option('\0', "f", &f, "F", "float",  .value_type=Clags_Float),
        clags_option('\0', "d", &d, "D", "double", .value_type=Clags_Double),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(r > 3.139 && r < 3.141);
    test(f > -1.501f && f < -1.499f);
    test(d > 2.71827 && d < 2.71829);
    return true;
}

bool test_value_choice(clags_sb_t *sb)
{
    clags_choice_t choices_arr[] = {
        { "red", "color red" },
        { "green", "color green" },
        { "blue", "color blue" },
    };
    clags_choices_t choices = clags_choices(choices_arr);
    clags_choice_t *sel = NULL;

    char *argv[] = {"", "green"};
    clags_arg_t args[] = {
        clags_positional(&sel, "color", "choose color", .value_type=Clags_Choice, .using.choices=&choices),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(sel != NULL);
    test(strcmp(sel->value, "green") == 0);
    return true;
}

bool test_value_subcmd(clags_sb_t *sb)
{
    /* child config that expects a single positional string */
    char *child_val = NULL;
    clags_arg_t child_args[] = {
        clags_positional(&child_val, "inner", "inner positional"),
    };
    clags_config_t child_config = clags_config(child_args, .description="child");

    /* top-level subcmd array */
    clags_subcmd_t subs[] = {
        { "do", "do something", &child_config },
    };

    clags_subcmds_t subwrap = clags_subcmds(subs);
    clags_subcmd_t *selected = NULL;

    char *argv[] = {"", "do", "work"};
    clags_arg_t args[] = {
        clags_positional(&selected, "cmd", "subcommand", .value_type=Clags_Subcmd, .using.subcmds=&subwrap),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    /* after parsing, child_config should have been used and child_val set */
    test(child_val != NULL && strcmp(child_val, "work") == 0);
    return true;
}

bool test_value_path_file_dir(clags_sb_t *sb)
{
    #include <time.h>
#ifdef _WIN32
    #include <direct.h>
    #include <process.h>
    #define GETPID _getpid
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #define GETPID getpid
    #define MKDIR(path) mkdir((path), 0700)
#endif

    /* determine a temp base directory */
    const char *tmpbase = getenv("TMPDIR");
    if (!tmpbase) tmpbase = getenv("TMP");
    if (!tmpbase) tmpbase = getenv("TEMP");
    if (!tmpbase) tmpbase = ".";

    /* build unique names */
    char tmpfile[512];
    char tmpdir[512];
    unsigned rnd = (unsigned)time(NULL) ^ (unsigned)GETPID();
    snprintf(tmpfile, sizeof(tmpfile), "%s/clags_test_file_%u_%u", tmpbase, (unsigned)GETPID(), rnd);
    snprintf(tmpdir,  sizeof(tmpdir),  "%s/clags_test_dir_%u_%u",  tmpbase, (unsigned)GETPID(), rnd ^ 0xabcdef);

    /* create file */
    FILE *f = fopen(tmpfile, "wb");
    test(f != NULL);
    if (f) fclose(f);

    /* create directory */
    int mkres = MKDIR(tmpdir);
    /* MKDIR may return -1 and set errno==EEXIST if it already existed; accept that too */
    test(mkres == 0 || errno == EEXIST);

    char *path_var = NULL;
    char *file_var = NULL;
    char *dir_var = NULL;

    /* Use three separate args for the three checks */
    char *argv[] = {"", tmpfile, tmpfile, tmpdir};
    clags_arg_t args[] = {
        clags_positional(&path_var, "path", "any path", .value_type=Clags_Path),
        clags_positional(&file_var, "file", "regular file", .value_type=Clags_File),
        clags_positional(&dir_var,  "dir",  "directory",    .value_type=Clags_Dir),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(path_var != NULL && strcmp(path_var, tmpfile) == 0);
    test(file_var != NULL && strcmp(file_var, tmpfile) == 0);
    test(dir_var  != NULL && strcmp(dir_var,  tmpdir) == 0);

    /* cleanup */
    remove(tmpfile);
    remove(tmpdir);
    return true;
}

bool test_value_size(clags_sb_t *sb)
{
    clags_fsize_t sval = 0;
    char *argv[] = {"", "--size", "10KiB"};
    clags_arg_t args[] = {
        clags_option('\0', "size", &sval, "SIZE", "size value", .value_type=Clags_Size),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(sval == (clags_fsize_t)(10ULL << 10)); /* 10KiB == 10240 */
    return true;
}

bool test_value_time_s_time_ns(clags_sb_t *sb)
{
    clags_time_t ts = 0;
    clags_time_t tns = 0;

    char *argv[] = {"", "--ts", "1m2s", "--tns", "1s"};
    clags_arg_t args[] = {
        clags_option('\0', "ts",  &ts,  "TIME", "time seconds", .value_type=Clags_TimeS),
        clags_option('\0', "tns", &tns, "TIME", "time ns",      .value_type=Clags_TimeNS),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(ts == 62); /* 1 minute + 2 seconds */
    test(tns == 1000000000ULL); /* 1s == 1_000_000_000 ns */
    return true;
}

bool test_value_custom(clags_sb_t *sb)
{
    char *out = NULL;
    clags_custom_t custom = clags_custom("mycust", custom_verifier);
    char *argv[] = {"", "C:payload"};
    clags_arg_t args[] = {
        clags_positional(&out, "cust", "custom", .value_type=Clags_Custom, .using.custom=&custom),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(out != NULL && strcmp(out, "payload") == 0);
    return true;
}

bool test_int_list(clags_sb_t *sb)
{
    char *argv[] = {"", "-l123", "--list", "-120"};
    clags_list_t list = clags_list(Clags_Int);
    clags_arg_t args[] = {
        clags_option('l', "list", &list, "", "", .is_list=true, .value_type=Clags_Int),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    clags_config_t *failed = clags_parse(arr_len(argv), argv, &config);
    test(failed == NULL);

    int64_t values[] = {123, -120};
    test(int_list_matches(list, values, arr_len(values)));
    
    return true;
}

bool test_custom_list(clags_sb_t *sb)
{
    char *argv[] = {"", "-lC:/Users", "--list=C:/clags.h","--list", "potato"};
    clags_list_t list = clags_custom_list(sizeof(char*));
    clags_custom_t custom = clags_custom("mycust", custom_verifier);
    clags_arg_t args[] = {
        clags_option('l', "list", &list, "", "", .is_list=true, .value_type=Clags_Custom, .using.custom=&custom),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    clags_config_t *failed = clags_parse(arr_len(argv), argv, &config);
    test(failed == &config);

    const char * values[] = {"/Users", "/clags.h"};
    test(string_list_matches(list, values, arr_len(values)));
    
    return true;
}

static Test parsing_tests[] = {
    {"string", test_value_string},
    {"bool", test_value_bool},
    {"int", test_value_int},
    {"int range", test_int_range},
    {"int8/int16/int32/int64", test_value_int8_int16_int32_int64},
    {"uint types", test_value_uint_uint8_uint16_uint32_uint64},
    {"real/float/double", test_value_real_float_double},
    {"choice", test_value_choice},
    {"subcommand", test_value_subcmd},
    {"path/file/dir", test_value_path_file_dir},
    {"size", test_value_size},
    {"time_s/time_ns", test_value_time_s_time_ns},
    {"custom", test_value_custom},
    {"integer list", test_int_list},
    {"custom list", test_custom_list},
};

TestSection parsing_section = {
    "Parsing",
    parsing_tests,
    arr_len(parsing_tests),
};
