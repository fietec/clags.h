#include <stdint.h>
#include "tests.h"
#include "../clags.h"

static clags_config_t* mock_failed_custom_validate(clags_config_t *config, const char *arg_name, void *data)
{
    (void)arg_name;
    (void)data;
    return config;
}

bool test_null_and_state_guards(clags_sb_t *sb)
{
    test(clags_validate("null_config", NULL) == NULL);

    clags_arg_t args[] = { clags_positional(NULL, "arg", "") };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);
    config.state = Clags_Config_Valid;
    test(clags_validate("already_valid", &config) == NULL);

    return true;
}

bool test_valid_config(clags_sb_t *sb)
{
    char *input_file;
    int64_t *quality;
    clags_range_t quality_range = clags_int_range(0, 100);
    clags_arg_t args[] = {
        clags_positional(&input_file, "", ""),
        clags_option('q', "quality", &quality, "", "", .value_type = Clags_Int, .using.range = &quality_range),
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);
    clags_config_t *result = clags_validate("", &config);
    test(result == NULL);

    return true;
}

bool test_invalid_options(clags_sb_t *sb)
{
    clags_arg_t args[] = {};
    clags_config_t config;
    clags_config_t *result;

    config = (clags_config_t)clags_config(args, GLOBAL_OPTIONS, .list_terminator = "--");
    result = clags_validate("", &config);
    test(result == &config);

    config = (clags_config_t)clags_config(args, GLOBAL_OPTIONS, .ignore_prefix = "--");
    result = clags_validate("", &config);
    test(result == &config);

    clags_list_t ignored_args = clags_list(Clags_Bool);
    config = (clags_config_t)clags_config(args, GLOBAL_OPTIONS, .ignored_args = &ignored_args);
    result = clags_validate("", &config);
    test(result == &config);

    config = (clags_config_t)clags_config(args, GLOBAL_OPTIONS, .min_usage_alignment = 10);
    result = clags_validate("", &config);
    test(result == &config);

    config = (clags_config_t)clags_config(args, GLOBAL_OPTIONS, .min_usage_alignment = 50, .max_usage_alignment = 40);
    result = clags_validate("", &config);
    test(result == &config);

    return true;
}

bool test_positional_validation(clags_sb_t *sb)
{
    clags_arg_t optional_args[] = {
        clags_positional(NULL, "", "", .optional = true),
        clags_positional(NULL, "", ""),
    };
    clags_config_t optional_config = clags_config(optional_args, GLOBAL_OPTIONS);
    test(clags_validate("", &optional_config) == &optional_config);

    clags_arg_t subcmd_args[] = {
        clags_positional(NULL, "", "", .value_type = Clags_Subcmd),
        clags_positional(NULL, "", ""),
    };
    clags_config_t subcmd_config = clags_config(subcmd_args, GLOBAL_OPTIONS);
    test(clags_validate("", &subcmd_config) == &subcmd_config);

    clags_arg_t list_args[] = {
        clags_positional(NULL, "", "", .is_list = true),
        clags_positional(NULL, "", ""),
    };
    clags_config_t list_config = clags_config(list_args, GLOBAL_OPTIONS);
    test(clags_validate("", &list_config) == &list_config);

    // invalid value_type enum
    clags_arg_t invalid_type_args[] = {
        clags_positional(NULL, "invalid_type", "", .value_type = (clags_value_type_t)999)
    };
    clags_config_t invalid_type_config = clags_config(invalid_type_args, GLOBAL_OPTIONS);
    test(clags_validate("", &invalid_type_config) == &invalid_type_config);

    return true;
}

bool test_option_validation(clags_sb_t *sb)
{
    clags_arg_t args[] = { clags_option(0, "", NULL, "", "", .value_type = Clags_Subcmd) };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);
    test(clags_validate("", &config) == &config);

    // list option with default value (unsupported)
    clags_list_t list = clags_list(Clags_String);
    clags_arg_t list_default_args[] = {
        clags_option('l', "list", &list, "", "", .is_list = true, .value_type = Clags_String, .default_input = "default_val")
    };
    clags_config_t list_default_config = clags_config(list_default_args, GLOBAL_OPTIONS);
    test(clags_validate("", &list_default_config) == &list_default_config);

    return true;
}

bool test_flag_validation(clags_sb_t *sb)
{
    clags_arg_t invalid_flag_args[] = {
        clags_flag('f', "flag", NULL, "", .type = (clags_flag_type_t)999)
    };
    clags_config_t invalid_flag_config = clags_config(invalid_flag_args, GLOBAL_OPTIONS);
    test(clags_validate("", &invalid_flag_config) == &invalid_flag_config);

    return true;
}

bool test_list_validation(clags_sb_t *sb)
{
    // type mismatch between list container and argument value type
    clags_list_t string_list = clags_list(Clags_String);
    clags_arg_t mismatch_args[] = {
        clags_option('i', "int-list", &string_list, "", "", .is_list = true, .value_type = Clags_Int)
    };
    clags_config_t mismatch_config = clags_config(mismatch_args, GLOBAL_OPTIONS);
    test(clags_validate("", &mismatch_config) == &mismatch_config);

    // custom list missing custom_size_set flag
    clags_list_t uninit_custom_list = clags_list(Clags_Custom);
    clags_arg_t uninit_custom_args[] = {
        clags_option('c', "custom-list", &uninit_custom_list, "", "", .is_list = true, .value_type = Clags_Custom)
    };
    clags_config_t uninit_custom_config = clags_config(uninit_custom_args, GLOBAL_OPTIONS);
    test(clags_validate("", &uninit_custom_config) == &uninit_custom_config);

    // custom list with zero item_size
    clags_list_t zero_size_custom_list = clags_list(Clags_Custom);
    zero_size_custom_list.custom_size_set = true;
    zero_size_custom_list.item_size = 0;
    clags_arg_t zero_size_custom_args[] = {
        clags_option('c', "custom-list", &zero_size_custom_list, "", "", .is_list = true, .value_type = Clags_Custom)
    };
    clags_config_t zero_size_custom_config = clags_config(zero_size_custom_args, GLOBAL_OPTIONS);
    test(clags_validate("", &zero_size_custom_config) == &zero_size_custom_config);

    return true;
}

bool test_subcmd_validation(clags_sb_t *sb)
{
    // missing subcmds definition data
    clags_arg_t null_data_args[] = {
        clags_positional(NULL, "subcmd", "", .value_type = Clags_Subcmd)
    };
    clags_config_t null_data_config = clags_config(null_data_args, GLOBAL_OPTIONS);
    test(clags_validate("", &null_data_config) == &null_data_config);

    // NULL items array when count > 0
    clags_subcmds_t null_items_subcmds = { .count = 1, .items = NULL };
    clags_arg_t null_items_args[] = {
        clags_positional(NULL, "subcmd", "", .value_type = Clags_Subcmd, .using.subcmds = &null_items_subcmds)
    };
    clags_config_t null_items_config = clags_config(null_items_args, GLOBAL_OPTIONS);
    test(clags_validate("", &null_items_config) == &null_items_config);

    // subcommand missing name
    clags_subcmd_t unnamed_subcmd = { .name = NULL, .config = NULL };
    clags_subcmds_t unnamed_subcmds = { .count = 1, .items = &unnamed_subcmd };
    clags_arg_t unnamed_args[] = {
        clags_positional(NULL, "subcmd", "", .value_type = Clags_Subcmd, .using.subcmds = &unnamed_subcmds)
    };
    clags_config_t unnamed_config = clags_config(unnamed_args, GLOBAL_OPTIONS);
    test(clags_validate("", &unnamed_config) == &unnamed_config);

    // subcommand missing config
    clags_subcmd_t null_cfg_subcmd = { .name = "build", .config = NULL };
    clags_subcmds_t null_cfg_subcmds = { .count = 1, .items = &null_cfg_subcmd };
    clags_arg_t null_cfg_args[] = {
        clags_positional(NULL, "subcmd", "", .value_type = Clags_Subcmd, .using.subcmds = &null_cfg_subcmds)
    };
    clags_config_t null_cfg_config = clags_config(null_cfg_args, GLOBAL_OPTIONS);
    test(clags_validate("", &null_cfg_config) == &null_cfg_config);

    // child subcommand config validation failure
    clags_arg_t invalid_child_args[] = {
        clags_positional(NULL, "pos1", "", .optional = true),
        clags_positional(NULL, "pos2", ""),
    };
    clags_config_t invalid_child_config = clags_config(invalid_child_args, GLOBAL_OPTIONS);
    clags_subcmd_t failed_child_subcmd = { .name = "failing_child", .config = &invalid_child_config };
    clags_subcmds_t failed_child_subcmds = { .count = 1, .items = &failed_child_subcmd };
    clags_arg_t parent_args[] = {
        clags_positional(NULL, "subcmd", "", .value_type = Clags_Subcmd, .using.subcmds = &failed_child_subcmds)
    };
    clags_config_t parent_config = clags_config(parent_args, GLOBAL_OPTIONS);
    test(clags_validate("", &parent_config) == &invalid_child_config);

    return true;
}

bool test_choice_validation(clags_sb_t *sb)
{
    // missing choice definition data
    clags_arg_t null_choice_args[] = {
        clags_option('c', "choice", NULL, "", "", .value_type = Clags_Choice)
    };
    clags_config_t null_choice_config = clags_config(null_choice_args, GLOBAL_OPTIONS);
    test(clags_validate("", &null_choice_config) == &null_choice_config);

    // NULL items array when count > 0
    clags_choices_t null_items_choices = { .count = 1, .items = NULL };
    clags_arg_t null_items_args[] = {
        clags_option('c', "choice", NULL, "", "", .value_type = Clags_Choice, .using.choices = &null_items_choices)
    };
    clags_config_t null_items_config = clags_config(null_items_args, GLOBAL_OPTIONS);
    test(clags_validate("", &null_items_config) == &null_items_config);

    // choice item with NULL value
    clags_choice_t null_val_choice = { .value = NULL };
    clags_choices_t null_val_choices = { .count = 1, .items = &null_val_choice };
    clags_arg_t null_val_args[] = {
        clags_option('c', "choice", NULL, "", "", .value_type = Clags_Choice, .using.choices = &null_val_choices)
    };
    clags_config_t null_val_config = clags_config(null_val_args, GLOBAL_OPTIONS);
    test(clags_validate("", &null_val_config) == &null_val_config);

    return true;
}

bool test_range_validation(clags_sb_t *sb)
{
    // range type mismatch
    clags_range_t mismatch_range = clags_uint_range(0, 10);
    clags_arg_t mismatch_args[] = {
        clags_option('i', "int", NULL, "", "", .value_type = Clags_Int, .using.range = &mismatch_range)
    };
    clags_config_t mismatch_config = clags_config(mismatch_args, GLOBAL_OPTIONS);
    test(clags_validate("", &mismatch_config) == &mismatch_config);

    // int range min > max
    clags_range_t inv_int_range = clags_int_range(100, 10);
    clags_arg_t inv_int_args[] = {
        clags_option('i', "int", NULL, "", "", .value_type = Clags_Int, .using.range = &inv_int_range)
    };
    clags_config_t inv_int_config = clags_config(inv_int_args, GLOBAL_OPTIONS);
    test(clags_validate("", &inv_int_config) == &inv_int_config);

    // uint range min > max
    clags_range_t inv_uint_range = clags_uint_range(100, 10);
    clags_arg_t inv_uint_args[] = {
        clags_option('u', "uint", NULL, "", "", .value_type = Clags_UInt, .using.range = &inv_uint_range)
    };
    clags_config_t inv_uint_config = clags_config(inv_uint_args, GLOBAL_OPTIONS);
    test(clags_validate("", &inv_uint_config) == &inv_uint_config);

    // real range min > max
    clags_range_t inv_real_range = clags_real_range(10.5, 1.5);
    clags_arg_t inv_real_args[] = {
        clags_option('r', "real", NULL, "", "", .value_type = Clags_Real, .using.range = &inv_real_range)
    };
    clags_config_t inv_real_config = clags_config(inv_real_args, GLOBAL_OPTIONS);
    test(clags_validate("", &inv_real_config) == &inv_real_config);

    return true;
}

bool test_custom_validation(clags_sb_t *sb)
{
    // missing custom definition data
    clags_arg_t null_custom_args[] = {
        clags_option('c', "custom", NULL, "", "", .value_type = Clags_Custom)
    };
    clags_config_t null_custom_config = clags_config(null_custom_args, GLOBAL_OPTIONS);
    test(clags_validate("", &null_custom_config) == &null_custom_config);

    // custom validate callback returns error config
    clags_custom_t custom_struct = { .validate = mock_failed_custom_validate };
    clags_arg_t failed_custom_args[] = {
        clags_option('c', "custom", NULL, "", "", .value_type = Clags_Custom, .using.custom = &custom_struct)
    };
    clags_config_t failed_custom_config = clags_config(failed_custom_args, GLOBAL_OPTIONS);
    test(clags_validate("", &failed_custom_config) == &failed_custom_config);

    return true;
}

bool test_duplicates(clags_sb_t *sb)
{
    // local duplicate short flag in same config
    clags_arg_t local_short_args[] = {
        clags_flag('o', "output", NULL, ""),
        clags_flag('o', "other", NULL, ""),
    };
    clags_config_t local_short_config = clags_config(local_short_args, GLOBAL_OPTIONS);
    test(clags_validate("", &local_short_config) == &local_short_config);

    // local duplicate long flag in same config
    clags_arg_t local_long_args[] = {
        clags_flag('a', "output", NULL, ""),
        clags_flag('b', "output", NULL, ""),
    };
    clags_config_t local_long_config = clags_config(local_long_args, GLOBAL_OPTIONS);
    test(clags_validate("", &local_long_config) == &local_long_config);

    // inherited duplicate short/long flag conflict across parent and child
    clags_arg_t parent_args[] = {
        clags_option('o', "output", NULL, "", "", .inherit = true),
        clags_flag(0, "output", NULL, ""),
    };
    clags_config_t parent_config = clags_config(parent_args, GLOBAL_OPTIONS);

    clags_arg_t child_args[] = {
        clags_flag('o', "overwrite", NULL, "", ""),
    };
    clags_config_t child_config = clags_config(child_args, GLOBAL_OPTIONS);
    child_config.parent = &parent_config;

    test(clags_validate("", &child_config) == &child_config);
    test(clags_validate("", &parent_config) == &parent_config);

    return true;
}

bool test_default_input_type_mismatch(clags_sb_t *sb)
{
    clags_arg_t args[] = {
        clags_option('n', "num", NULL, "N", "number", .value_type = Clags_Int, .default_input = "notanumber")
    };
    clags_config_t config = clags_config(args, GLOBAL_OPTIONS);

    test(clags_validate("", &config) == &config);
    return true;
}

Test validation_tests[] = {
    {"null and state guards", test_null_and_state_guards},
    {"valid config", test_valid_config},
    {"invalid options", test_invalid_options},
    {"positionals", test_positional_validation},
    {"options", test_option_validation},
    {"flags", test_flag_validation},
    {"lists", test_list_validation},
    {"subcommands", test_subcmd_validation},
    {"choices", test_choice_validation},
    {"ranges", test_range_validation},
    {"custom verifier", test_custom_validation},
    {"inherited duplicates", test_duplicates},
    {"default input type mismatch", test_default_input_type_mismatch},
};

TestSection validation_section = {
    "Validation",
    validation_tests,
    arr_len(validation_tests),
};
