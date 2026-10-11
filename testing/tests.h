#ifndef CLAGS_TESTS_H
#define CLAGS_TESTS_H

#include <stddef.h>
#include <stdbool.h>
#include "../clags.h"

#define GLOBAL_OPTIONS .min_log_level = Clags_NoLogs

#define test(expr) do{if (!(expr)){clags_sb_appendf(sb, "%s:%d: `"#expr"`", __FILE__, __LINE__); return false;}}while(0)

#define test_msg(expr, fmt, ...) do{if (!(expr)){clags_sb_appendf((sb), "`"fmt"`", __VA_ARGS__); return false;}}while(0)

#define arr_len(arr) ((arr) == NULL? 0 : sizeof(arr)/sizeof(*arr))

typedef struct{
    const char *name;
    bool (*func)(clags_sb_t *sb);
} Test;

typedef struct{
    const char *name;
    Test *tests;
    size_t test_count;
} TestSection;

#define TEST_SECTIONS     \
    X(syntax_section)     \
    X(parsing_section)    \
    X(validation_section) \

#define X(name) extern TestSection name;
TEST_SECTIONS
#undef X

bool string_list_matches(clags_list_t list, const char *strings[], size_t count);
bool int_list_matches(clags_list_t list, int64_t values[], size_t count);

#endif // CLAGS_TESTS_H
