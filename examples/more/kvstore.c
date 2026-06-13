#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#define CLAGS_IMPLEMENTATION
#include <clags.h>

typedef struct{
    char *key;
    size_t key_len;
    char *value;
    size_t value_len;
} Dict;

bool verify_dict(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    char *psep = strchr(arg, '=');
    if (!psep){
        clags_log(config, Clags_Error, "missing separator '=' in key-value pair for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    size_t key_len = psep - arg;
    if (key_len == 0){
        clags_log(config, Clags_Error, "missing key in key-value pair for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    char *pvalue = psep + 1;
    if (*pvalue == '\0'){
        clags_log(config, Clags_Error, "missing value in key-value pair for argument '%s': '%s'!", arg_name, arg);
        return false;
    }
    size_t value_len = strlen(pvalue);
    if (variable){
        *(Dict*)variable = (Dict){
            .key = (char*) clags_config_duplicate(config, arg, key_len),
            .key_len = key_len,
            .value = (char*) clags_config_duplicate(config, pvalue, value_len),
            .value_len = value_len,
        };
    }
    return true;
}

bool verify_addr(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    // TODO: implement
    return clags_verify_string(config, arg_name, arg, variable, data);
}

clags_custom_t dict_type = {"key-value", verify_dict, NULL};
clags_custom_t addr_type = {"ip:port", verify_addr, NULL};

// --- Global Definitions ---
struct{
    size_t verbosity;
    uint64_t timeout;
    clags_config_t *help_config;
} global = {0};
const clags_options_t global_options = {.ignore_prefix="!", .duplicate_strings=true};

// --- Put Config ---
struct{
    char *key;
    char *value;
    uint64_t ttl;
    clags_list_t tags;
} put = {
    NULL, NULL, 0,
    clags_custom_list(sizeof(Dict)),
};

clags_arg_t put_args[] = {
    clags_positional(&put.key, "key", "the key of the entry to update"),
    clags_positional(&put.value, "value", "the value the entry should be set to"),
    clags_option(0, "ttl", &put.ttl, "SECS", "time to live in seconds", .value_type=Clags_UInt),
    clags_option(0, "tag", &put.tags, "TAG", "tags to add", .value_type=Clags_Custom, .custom=&dict_type, .is_list=true),
};
clags_config_t put_config = clags_config_with_options(put_args, global_options);

// --- Delete Config ---
// Interval type ommited for simplicity now but would be implemented similarily to the other custom types
struct{
    clags_list_t keys;
    clags_list_t exclude;
} delete = {
    clags_list(Clags_String),
    clags_list(Clags_String),
};

clags_arg_t delete_args[] = {
    clags_positional(&delete.keys, "keys", "the keys of the entries to delete", .is_list=true),
    clags_option('e', "exclude", &delete.exclude, "KEY", "keys to shield", .is_list=true),
};
clags_config_t delete_config = clags_config_with_options(delete_args, global_options);

// --- Query Config ---
struct{
    char *query;
    uint64_t mem_size;
    uint64_t window;
} query = {0};

clags_arg_t query_args[] = {
    clags_positional(&query.query, "query", "the query to search for"),
    clags_option(0, "max-mem", &query.mem_size, "SIZE", "the mamimum amount of memory to use", .value_type=Clags_Size),
    clags_option(0, "window", &query.window, "TIME", "the maximum amount of time to spend", .value_type=Clags_TimeS),
};
clags_config_t query_config = clags_config_with_options(query_args, global_options);

// --- Data Config ---
struct{
    clags_subcmd_t *command;
} data = {0};

typedef enum{
    Put,
    Delete,
    Query,
} Data_Cmd;

clags_subcmd_t data_commands[] = {
    [Put] = {"put", "insert or update values", &put_config},
    [Delete] = {"delete", "delete an entry", &delete_config},
    [Query] = {"query", "search for an entry", &query_config},
};
clags_subcmds_t data_subcmds = clags_subcmds(data_commands);

clags_arg_t data_args[] = {
    clags_positional(&data.command, "command", "the command to run", .value_type=Clags_Subcmd, .subcmds=&data_subcmds)
};
clags_config_t data_config = clags_config_with_options(data_args, global_options);

// --- Set Config ---
struct{
    char *environment;
    char *addr;
    float backoff;
} set = {0};

clags_arg_t set_args[] = {
    clags_positional(&set.environment, "environment", "the name of the target environment"),
    clags_option('l', "listen-addr", &set.addr, "ADDR", "the address to listen for", .value_type=Clags_Custom, .custom=&addr_type),
    clags_option('b', "backoff", &set.backoff, "INTERVAL", "the retry multiplier interval", .value_type=Clags_Float),
};
clags_config_t set_config = clags_config_with_options(set_args, global_options);

// --- Export Config ---
struct{
    bool json;
    bool secrets;
} export = {0};

clags_arg_t export_args[] = {
    clags_flag(0, "json", &export.json, "output in the json format"),
    clags_flag(0, "include-secrets", &export.secrets, "include api keys in the export"),
};
clags_config_t export_config = clags_config_with_options(export_args, global_options);

// --- "Config" Config ---
struct{
    clags_subcmd_t *command;
} config = {0};

typedef enum{
    Set,
    Export,
} Config_Cmd;

clags_subcmd_t config_commands[] = {
    [Set] = {"set", "configure settings", &set_config},
    [Export] = {"export", "export the current config profile", &export_config},
};
clags_subcmds_t config_subcmds = clags_subcmds(config_commands);

clags_arg_t config_args[] = {
    clags_positional(&config.command, "command", "the command to run", .value_type=Clags_Subcmd, .subcmds=&config_subcmds)
};
clags_config_t config_config = clags_config_with_options(config_args, global_options);

// --- Root Config ---
struct{
    clags_subcmd_t *command;
} root = {0};

typedef enum{
    Config,
    Data,
} Root_Cmd;

clags_subcmd_t root_commands[] = {
    [Config] = {"config", "manage your configuration", &config_config},
    [Data] = {"data", "manage your database", &data_config},
};
clags_subcmds_t root_subcmds = clags_subcmds(root_commands);

clags_arg_t root_args[] = {
    clags_positional(&root.command, "command", "the command to run", .value_type=Clags_Subcmd, .subcmds=&root_subcmds),
    clags_option('t', "timeout", &global.timeout, "TIME", "api timeout in milliseconds", .value_type=Clags_UInt, .default_input="5000", .inherit=true),
    clags_flag('v', "verbose", &global.verbosity, "the logging verbosity", .type=Clags_CountFlag, .inherit=true),
    clags_flag_help_config(&global.help_config),
};
clags_config_t root_config = clags_config_with_options(root_args, global_options);

int config_set(void)
{
    printf("config set:\n");
    printf("  - environment: %s\n", set.environment);
    printf("  - addr: %s\n", set.addr);
    printf("  - backoff: %g\n", set.backoff);
    return 0;
}

int config_export(void)
{
    printf("config export:\n");
    printf("  - json: %s\n", export.json?"true":"false");
    printf("  - secrets: %s\n", export.secrets?"true":"false");
    return 0;
}

int data_put(void)
{
    printf("data put:\n");
    printf("  - key: %s\n", put.key);
    printf("  - value: %s\n", put.value);
    printf("  - ttl: %"PRIu64"\n", put.ttl);
    printf("  - tags (%zu):\n", put.tags.count);
    for (size_t i=0; i<put.tags.count; ++i){
        Dict dict = clags_list_element(put.tags, Dict, i);
        printf("    %.*s : %.*s\n", (int)dict.key_len, dict.key, (int)dict.value_len, dict.value);
    }
    return 0;
}

int data_delete(void)
{
    printf("data delete:\n");
    printf("  - keys:\n");
    for (size_t i=0; i<delete.keys.count; ++i){
        printf("    - %s\n", clags_list_element(delete.keys, char*, i));
    }
    printf("  - exclude:\n");
    for (size_t i=0; i<delete.exclude.count; ++i){
        printf("    - %s\n", clags_list_element(delete.exclude, char*, i));
    }
    return 0;
}

int data_query(void)
{
    printf("data query:\n");
    printf("  - query: %s\n", query.query);
    printf("  - mem_size: %"PRIu64"\n", query.mem_size);
    printf("  - window: %"PRIu64"\n", query.window);
    return 0;
}

int main(int argc, char *argv[])
{
    int result = 0;
    const char *name = argv[0];

    clags_config_t *failed_config = clags_parse(argc, argv, &root_config);
    if (failed_config){
        clags_usage(name, failed_config);
        clags_return_defer(1);
    }
    if (global.help_config){
        clags_usage(name, global.help_config);
        clags_return_defer(0);
    }

    printf("Verbosity: %zu\n", global.verbosity);
    printf("Timeout: %"PRIu64"s\n", global.timeout);

    Root_Cmd root_command = clags_subcmd_index(&root_subcmds, root.command);
    switch (root_command){
        case Config:{
            Config_Cmd config_command = clags_subcmd_index(&config_subcmds, config.command);
            switch (config_command){
                case Set:{
                    clags_return_defer(config_set());
                } break;
                case Export:{
                    clags_return_defer(config_export());
                } break;
            }
        } break;
        case Data:{
            Data_Cmd data_command = clags_subcmd_index(&data_subcmds, data.command);
            switch (data_command){
                case Put:{
                    clags_return_defer(data_put());
                } break;
                case Delete:{
                    clags_return_defer(data_delete());
                } break;
                case Query:{
                    clags_return_defer(data_query());
                } break;
            }
        } break;
    }
defer:
    clags_free(&root_config);
    return result;
}
