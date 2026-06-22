#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

clags_options_t global_options = {
    .min_log_level = Clags_NoLogs,
};

// 1. Integer option
void test_int_option() {
    int32_t num = 0;
    clags_arg_t args[] = {
        clags_option('n', "num", &num, "NUM", "an int option", .value_type=Clags_Int32)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "--num", "123"};
    clags_parse(3, argv, &config);

    assert(num == 123);
    assert(config.error == Clags_Error_Ok);
    clags_config_free(&config);
}

// 2. Float option
void test_float_option() {
    double fval = 0.0;
    clags_arg_t args[] = {
        clags_option('v', "value", &fval, "VAL", "a float option", .value_type=Clags_Real)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "--value", "3.14"};
    clags_parse(3, argv, &config);

    assert(fabs(fval - 3.14) < 1e-6);
    assert(config.error == Clags_Error_Ok);
    clags_config_free(&config);
}

// 3. Boolean option
void test_bool_option() {
    bool flag = false;
    clags_arg_t args[] = {
        clags_option('e', "enable", &flag, "BOOL", "a bool option", .value_type=Clags_Bool)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "--enable", "yes"};
    clags_parse(3, argv, &config);

    assert(flag == true);
    assert(config.error == Clags_Error_Ok);
    clags_config_free(&config);
}

// 4. Short flag
void test_short_flag() {
    bool verbose = false;
    clags_arg_t args[] = {
        clags_flag('v', "verbose", &verbose, "enable verbosity", .type=Clags_BoolFlag)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "-v"};
    clags_parse(2, argv, &config);

    assert(verbose == true);
    assert(config.error == Clags_Error_Ok);
    clags_config_free(&config);
}

// 5. Positional argument
void test_positional() {
    char *file = NULL;
    clags_arg_t args[] = {
        clags_positional(&file, "file", "input file", .value_type=Clags_String)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "input.txt"};
    clags_parse(2, argv, &config);

    assert(file && strcmp(file, "input.txt") == 0);
    assert(config.error == Clags_Error_Ok);
    clags_config_free(&config);
}

// 6. Positional list
void test_positional_list() {
    clags_list_t files = clags_list(Clags_String);
    clags_arg_t args[] = {
        clags_positional(&files, "files", "list of files", .is_list=true)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "a.txt", "b.txt", "c.txt"};
    clags_parse(4, argv, &config);

    assert(files.count == 3);
    assert(strcmp(clags_list_element(files, char*, 0), "a.txt") == 0);
    assert(strcmp(clags_list_element(files, char*, 2), "c.txt") == 0);
    assert(config.error == Clags_Error_Ok);

    clags_list_free(&files);
    clags_config_free(&config);
}

// 7. Invalid value
void test_invalid_value() {
    int32_t num = 0;
    clags_arg_t args[] = {
        clags_option('n', "num", &num, "NUM", "int option", .value_type=Clags_Int32)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "--num", "abc"};
    clags_parse(3, argv, &config);

    assert(config.error == Clags_Error_InvalidValue);
    clags_config_free(&config);
}

// 8. Subcommand
void test_subcommand() {
    clags_arg_t init_args[] = {};
    clags_config_t init_config = clags_config_with_options(init_args, global_options);
    clags_subcmd_t subcmd_items[] = {
        {"init", "initialize project", &init_config}
    };
    clags_subcmds_t subcmds = clags_subcmds(subcmd_items);
    clags_subcmd_t *selected = NULL;

    clags_arg_t args[] = {
        clags_positional(&selected, "command", "subcmd", .value_type=Clags_Subcmd, .subcmds=&subcmds)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "init"};
    clags_parse(2, argv, &config);

    assert(config.error == Clags_Error_Ok);
    assert(selected != NULL && strcmp(selected->name, "init") == 0);
    clags_config_free(&config);
}

// 9. Ranges Validation
void test_int_range() {
    int64_t quality = 0;
    clags_range_t q_range = clags_int_range(0, 100);
    clags_arg_t args[] = {
        clags_option('q', "quality", &quality, "Q", "quality", .value_type=Clags_Int, .range=&q_range)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    // Test out of bounds
    char *argv_bad[] = {"prog", "-q", "150"};
    clags_parse(3, argv_bad, &config);
    assert(config.error == Clags_Error_InvalidValue);

    // Reset and test valid
    config.state = Clags_Config_Unvalidated;
    config.error = Clags_Error_Ok;
    char *argv_good[] = {"prog", "-q", "90"};
    clags_parse(3, argv_good, &config);
    assert(config.error == Clags_Error_Ok);
    assert(quality == 90);

    clags_config_free(&config);
}

// 10. Choices Validation
void test_choices() {
    clags_choice_t format_items[] = {{"PNG", ""}, {"JPEG", ""}};
    clags_choices_t formats = clags_choices(format_items);
    clags_choice_t *selected_format = NULL;

    clags_arg_t args[] = {
        clags_option('f', "format", &selected_format, "FMT", "format", .value_type=Clags_Choice, .choices=&formats)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "-f", "PNG"};
    clags_parse(3, argv, &config);
    assert(config.error == Clags_Error_Ok);
    assert(strcmp(selected_format->value, "PNG") == 0);

    clags_config_free(&config);
}

// 11. Combined Short Flags
void test_combined_flags() {
    bool a = false, b = false, c = false;
    clags_arg_t args[] = {
        clags_flag('a', NULL, &a, "flag a"),
        clags_flag('b', NULL, &b, "flag b"),
        clags_flag('c', NULL, &c, "flag c")
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog", "-abc"};
    clags_parse(2, argv, &config);

    assert(config.error == Clags_Error_Ok);
    assert(a && b && c);

    clags_config_free(&config);
}

// 12. List Terminator
void test_list_terminator() {
    clags_list_t files = clags_list(Clags_String);
    clags_list_t tags = clags_list(Clags_String);

    clags_arg_t args[] = {
        clags_positional(&files, "files", "files", .is_list=true),
        clags_positional(&tags, "tags", "tags", .is_list=true)
    };

    clags_options_t opts = global_options;
    opts.list_terminator = "::";
    clags_config_t config = clags_config_with_options(args, opts);

    char *argv[] = {"prog", "f1.txt", "f2.txt", "::", "tag1", "tag2"};
    clags_parse(6, argv, &config);

    assert(config.error == Clags_Error_Ok);
    assert(files.count == 2);
    assert(tags.count == 2);
    assert(strcmp(clags_list_element(tags, char*, 0), "tag1") == 0);

    clags_list_free(&files);
    clags_list_free(&tags);
    clags_config_free(&config);
}

// 13. Missing Required Positional
void test_missing_required() {
    char *input = NULL;
    clags_arg_t args[] = {
        clags_positional(&input, "input", "required input file", .optional=false)
    };
    clags_config_t config = clags_config_with_options(args, global_options);

    char *argv[] = {"prog"};
    clags_parse(1, argv, &config);

    assert(config.error == Clags_Error_TooFewArguments);
    clags_config_free(&config);
}

void test_inheritance()
{
    char *pos;
    clags_arg_t child_args[] = {
        clags_positional(&pos, "pos", ""),
    };
    clags_config_t child_config = clags_config_with_options(child_args, global_options);

    clags_subcmd_t *subcmd;
    char *value = NULL;

    clags_subcmd_t commands[] = {
        {"cmd", "", &child_config},
    };
    clags_subcmds_t subcmds = clags_subcmds(commands);

    clags_arg_t parent_args[] = {
        clags_positional(&subcmd, "command", "", .value_type=Clags_Subcmd, .subcmds=&subcmds),
        clags_option('v', "value", &value, "VAL", "", .inherit=true),
    };
    clags_config_t parent_config = clags_config_with_options(parent_args, global_options);

    char *argv[] = {"prog", "cmd", "--value=inputs", "pos"};
    clags_parse(clags_arr_len(argv), argv, &parent_config);

    assert(subcmd == &commands[0]);
    assert(strcmp(value, "input") == 0);
    assert(strcmp(pos, "pos") == 0);
}

typedef struct{
    const char *name;
    void (*func)(void);
} Test;

static Test tests[] = {
    {"int-option", test_int_option},
    {"float-option", test_float_option},
    {"bool-option", test_bool_option},
    {"short-flag", test_short_flag},
    {"positional", test_positional},
    {"positional-list", test_positional_list},
    {"invalid-value", test_invalid_value},
    {"int-range", test_int_range},
    {"choices", test_choices},
    {"combined-flags", test_combined_flags},
    {"list-terminator", test_list_terminator},
    {"missing-required", test_missing_required},
    {"subcommand", test_subcommand},
    {"inheritance", test_inheritance}
};

static size_t test_count = sizeof(tests)/sizeof(Test);

int main(void)
{
    for (size_t i=0; i<test_count; ++i){
        Test test = tests[i];
        test.func();
        printf("- Test '%s' passed!\n", test.name);
    }
    printf("\nAll tests passed successfully!\n");
    return 0;
}
