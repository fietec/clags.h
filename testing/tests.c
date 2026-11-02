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

int main(void)
{
    TestSection sections[] = {
#define X(name) name,
        TEST_SECTIONS
#undef X
    };

    size_t total = 0;
    size_t passed = 0;

    clags_sb_t sb = {0};

    for (size_t i=0; i<arr_len(sections); ++i){
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
