#include <stdio.h>

#include "tests.h"
#define CLAGS_IMPLEMENTATION
#include "../clags.h"

bool string_list_matches(clags_list_t list, const char *strings[], size_t count)
{
    if (list.count != count) return false;
    for (size_t i=0; i<count; ++i){
        const char *value = clags_list_element(list, char*, i);
        if (strcmp(value, strings[i]) != 0) return false;
    }
    return true;
}

bool int_list_matches(clags_list_t list, int64_t values[], size_t count)
{
    if (list.count != count) return false;
    for (size_t i=0; i<count; ++i){
        int64_t value = clags_list_element(list, int64_t, i);
        if (value != values[i]) return false;
    }
    return true;
}

void print_usage(const char *name, clags_config_t *config)
{
    printf("%s - the test suite of clags.h\n", name);
    clags_usage(name, config);
}

int main(int argc, char *argv[])
{
    bool no_building = false;
    bool help = false;

    clags_arg_t args[] = {
        clags_flag('b', "no-build", &no_building, "do not perform build tests",),
        clags_flag_help(&help),
    };
    clags_config_t config = clags_config(args,);

    clags_error_t error = clags_parse(argc, argv, &config);
    if (error.type != Clags_Error_Ok){
        print_usage(argv[0], error.config);
        return 1;
    }
    if (help){
        print_usage(argv[0], &config);
        return 0;
    }

    TestSection sections[] = {
#define X(name) name,
        TEST_SECTIONS
#undef X
    };

    size_t total = 0;
    size_t passed = 0;

    clags_sb_t sb = {0};

    for (size_t i=0; i<arr_len(sections); ++i){
        if (i == 0 && no_building) continue;
        TestSection section = sections[i];
        printf("\n%s:\n", section.name);
        for (size_t j=0; j<section.test_count; ++j){
            sb.count = 0;
            Test test = section.tests[j];
            if (!test.func) continue;
            if (test.func(&sb)){
                printf("  [PASSED]: %s\n", test.name);
                passed++;
            } else{
                printf("  [FAILED]: %s: %.*s\n", test.name, (int)sb.count, sb.items);
            }
            total++;
        }
    }
    printf("\nSummary:\n");
    printf("  Total  : %zu\n", total);
    printf("  Passed : %zu (%zu%%)\n", passed, (passed*100)/total);

    if (total == passed){
        printf("\nAll tests passed successfully!\n");
        clags_sb_free(&sb);
        return 0;
    } else{
        fprintf(stderr, "Not all tests passed successfully!\n");
        clags_sb_free(&sb);
        return 1;
    }
}
