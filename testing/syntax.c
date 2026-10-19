#include <string.h>
#include <stdint.h>

#include "../clags.h"
#include "tests.h"

bool test_positional(clags_sb_t *sb)
{
    char *argv[] = {"", "foo", "bar"};
    char *arg1 = NULL;
    char *arg2 = NULL;
    clags_arg_t args[] = {
        clags_positional(&arg1, "arg1", ""),
        clags_positional(&arg2, "arg2", ""),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(strcmp(arg1, "foo") == 0);
    test(strcmp(arg2, "bar") == 0);
    return true;
}

bool test_options(clags_sb_t *sb)
{
    char *single = NULL;
    clags_list_t files = clags_list(Clags_String);

    char *argv[] = {"", "-s", "a", "--file=one", "--file", "two","-fthree"};
    clags_arg_t args[] = {
        clags_option('s', "single", &single, "VAL", "single option"),
        clags_option('f', "file", &files, "FILE", "file option", .is_list=true),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(strcmp(single, "a") == 0);
    test(files.count == 3);
    test(strcmp(clags_list_element(files, char*, 0), "one") == 0);
    test(strcmp(clags_list_element(files, char*, 1), "two") == 0);
    test(strcmp(clags_list_element(files, char*, 2), "three") == 0);
    return true;
}

bool test_flags(clags_sb_t *sb)
{
    bool warnings = false;
    clags_config_t *help = NULL;
    size_t verbose_count = 0;

    char *argv[] = {"", "-whvv"};
    clags_arg_t args[] = {
        clags_flag('w', "warnings", &warnings, "print warnings"),
        clags_flag('h', "help", &help, "print help", .type=Clags_ConfigFlag),
        clags_flag('v', "verbose", &verbose_count, "increase verbosity", .type=Clags_CountFlag),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(warnings == true);
    test(help == &config);
    test(verbose_count == 2);
    return true;
}

bool test_ignore_prefix(clags_sb_t *sb)
{
    char *val = NULL;
    clags_list_t ignored = clags_list(Clags_String);

    char *argv[] = {"", "!skip", "value"};
    clags_arg_t args[] = {
        clags_positional(&val, "arg", "positional argument"),
    };
    clags_config_t config = clags_config(args, .ignore_prefix = "!", .ignored_args = &ignored, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(val && strcmp(val, "value") == 0);
    test(ignored.count == 1);
    test(strcmp(clags_list_element(ignored, char*, 0), "skip") == 0);
    return true;
}

bool test_double_dash_disable(clags_sb_t *sb)
{
    bool a = false;
    char *pos = NULL;

    char *argv[] = {"", "--", "-a"};
    clags_arg_t args[] = {
        clags_flag('a', "a", &a, "flag a"),
        clags_positional(&pos, "arg", "positional"),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(a == false);
    test(strcmp(pos, "-a") == 0);
    return true;
}

bool test_double_dash_reenable(clags_sb_t *sb)
{
    bool a = false;

    char *argv[] = {"", "--", "--", "-a"};
    clags_arg_t args[] = {
        clags_flag('a', "a", &a, "flag a"),
    };
    clags_config_t config = clags_config(args, .allow_option_parsing_toggle = true, GLOBAL_OPTIONS);

    test(clags_parse(arr_len(argv), argv, &config) == NULL);
    test(a == true);
    return true;
}

bool test_positional_list(clags_sb_t *sb)
{
    char *argv[] = {"", "foo", "bar", "baz"};
    clags_list_t list = clags_list(Clags_String);
    clags_arg_t args[] = {
        clags_positional(&list, "", "", .is_list=true),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    clags_config_t *failed = clags_parse(arr_len(argv), argv, &config);
    test(failed == NULL);

    test(string_list_matches(list, (const char **)argv + 1, arr_len(argv) - 1));

    return true;
}

bool test_option_list(clags_sb_t *sb)
{
    char *argv[] = {"", "-lfoo", "-l", "bar", "--list=baz", "--list", "aboba"};
    clags_list_t list = clags_list(Clags_String);
    clags_arg_t args[] = {
        clags_option('l', "list", &list, "", "", .is_list=true),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    clags_config_t *failed = clags_parse(arr_len(argv), argv, &config);
    test(failed == NULL);

    const char *strings[] = {"foo", "bar", "baz", "aboba"};
    test(string_list_matches(list, strings, arr_len(strings)));
    
    return true;
}

bool test_missing_required(clags_sb_t *sb)
{
    char *input = NULL;
    clags_arg_t args[] = {
        clags_positional(&input, "input", "required input file", .optional=false)
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    char *argv[] = {"prog"};
    clags_parse(1, argv, &config);

    test(config.error.type == Clags_Error_TooFewArguments);

    clags_config_free(&config);
    return true;
}

bool test_inheritance(clags_sb_t *sb)
{
    char *pos;
    clags_arg_t child_args[] = {
        clags_positional(&pos, "pos", ""),
    };
    clags_config_t child_config = clags_config(child_args, GLOBAL_OPTIONS);

    clags_subcmd_t *subcmd;
    char *value = NULL;

    clags_subcmd_t commands[] = {
        {"cmd", "", &child_config},
    };
    clags_subcmds_t subcmds = clags_subcmds(commands);

    clags_arg_t parent_args[] = {
        clags_positional(&subcmd, "command", "", .value_type=Clags_Subcmd, .using.subcmds=&subcmds),
        clags_option('v', "value", &value, "VAL", "", .inherit=true),
    };
    clags_config_t parent_config = clags_config(parent_args, GLOBAL_OPTIONS);

    char *argv[] = {"prog", "cmd", "--value=input", "pos"};
    test(clags_parse(clags_arr_len(argv), argv, &parent_config) == NULL);

    test(subcmd == &commands[0]);
    test(strcmp(value, "input") == 0);
    test(strcmp(pos, "pos") == 0);

    return true;
}

static Test syntax_tests[] = {
    {"positional arguments", test_positional},
    {"missing required argument", test_missing_required},
    {"options", test_options},
    {"flags", test_flags},
    {"ignore prefix", test_ignore_prefix},
    {"'--' disables parsing", test_double_dash_disable},
    {"'--' can re-enable parsing when allowed", test_double_dash_reenable},
    {"positional lists", test_positional_list},
    {"option lists", test_option_list},
    {"inheritance", test_inheritance},
};

TestSection syntax_section = {
    "Syntax",
    syntax_tests,
    arr_len(syntax_tests),
};
