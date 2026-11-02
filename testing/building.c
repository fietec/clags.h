#include "tests.h"
#include "../clags.h"
#include "nob.h"

static const char *stripping_macros[] = {
    "CLAGS_STRIP_INT_TYPES",
    "CLAGS_STRIP_REAL_TYPES",
    "CLAGS_STRIP_PATH_TYPES",
    "CLAGS_STRIP_SIZE_TYPES",
    "CLAGS_STRIP_TIME_TYPES",
    "CLAGS_STRIP_STRTOINT",
};
const size_t stripping_count = arr_len(stripping_macros);

static clags_sb_t current_cmd;

void custom_log_handler(Nob_Log_Level level, const char *fmt, va_list args)
{
    if (level == INFO){
        current_cmd.count = 0;
        clags_sb_vappendf(&current_cmd, fmt, args);
    }
}

bool test_feature_stripping(clags_sb_t *sb)
{
    Cmd cmd = {0};
    set_log_handler(custom_log_handler);
    mkdir_if_not_exists("build");
    size_t nsubsets = 1 << stripping_count;
    for (size_t i=0; i<nsubsets; ++i){
        cmd_append(&cmd, "cc");
        cmd_append(&cmd, "-Wall", "-Wextra", "-Werror", "-pedantic", "-std=c99");
        cmd_append(&cmd, "-x", "c");
        cmd_append(&cmd, "-o", "build/test_feature_stripped.o");
        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "../clags.h");
        cmd_append(&cmd, "-D", "CLAGS_IMPLEMENTATION");

        for (size_t j=0; j<stripping_count; ++j){
            if ((i >> j) & 1){
                cmd_append(&cmd, "-D", stripping_macros[j]);
            }
        }
        test_msg(cmd_run(&cmd,), "failed running %.*s", (int)current_cmd.count, current_cmd.items);
    }
    return true;
}

static Test build_tests[] = {
    {"feature stripping", test_feature_stripping},
};

TestSection building_section = {
    "Building",
    build_tests,
    arr_len(build_tests),
};
