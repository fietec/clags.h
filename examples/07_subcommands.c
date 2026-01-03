/*
  Example 7: Subcommands
      This example demonstrates how to define subcommands (like 'git commit' or 'git push').
      Each subcommand has its own argument parser configuration.
*/

#include <stdio.h>
#include <stdbool.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

// Global help flag
bool help = false;

// --- COPY subcommand ---
char *copy_source = NULL;
char *copy_dest   = NULL;

clags_arg_t copy_args[] = {
    clags_required(&copy_source, "source", "Source file path", .value_type=Clags_File),
    clags_required(&copy_dest, "dest", "Destination file path"),
    clags_flag_help(&help)
};

clags_config_t copy_config = clags_config(copy_args);

// --- DELETE subcommand ---
char *delete_target = NULL;
bool delete_force   = false;

clags_arg_t delete_args[] = {
    clags_required(&delete_target, "target", "Target file to delete", .value_type=Clags_File),
    clags_flag('f', "force", &delete_force, "Force deletion"),
    clags_flag_help(&help)
};

clags_config_t delete_config = clags_config(delete_args);

// --- Subcommands ---
// this links the previously created subcommand configs to their respective arguments
clags_subcmd_t subcmds[] = {
    {"copy",   "Copy a file from source to destination", &copy_config},
    {"delete", "Delete a file", &delete_config}
};

clags_subcmds_t my_subcmds = clags_subcmds(subcmds);

// define a variable that will hold a pointer to the selected `clags_subcmd_t`
clags_subcmd_t *selected_subcmd = NULL;

// --- Main CLI config ---
clags_arg_t main_args[] = {
    // with `.subcmds` you set the subcommand parsing verifier
    clags_required(&selected_subcmd, "command", "Subcommand to run", .value_type=Clags_Subcmd, .subcmds=&my_subcmds),
    clags_flag_help(&help)
};

clags_config_t main_config = clags_config(main_args);

int main(int argc, char **argv) {

    // `clags_parse` returns the config that failed.
    // In basic examples this will always be the config provided to `clags_parse` or `NULL`.
    // But since we have nested configs here, this can be used to determine which subcommand failed
    clags_config_t* failed_config = clags_parse(argc, argv, &main_config);
    if (failed_config) {
        // print the usage for the failed subcommand
        clags_usage(argv[0], failed_config);
        return 1;
    }

    if (help){
        // in this example, the `help` flag is allowed to be set by all subcommands
        // Therefore, we call `clags_usage` based on the selected subcommand, which stores its config
        if (selected_subcmd){
            clags_usage(argv[0], selected_subcmd->config);
        } else{
            clags_usage(argv[0], &main_config);
        }
        return 0;
    }
    
    // Act based on which subcommand was selected
    if (selected_subcmd == &subcmds[0]) {  // copy
        printf("Copying file '%s' to '%s'\n", copy_source, copy_dest);
        // actual copy logic goes here
    } else if (selected_subcmd == &subcmds[1]) {  // delete
        printf("Deleting file '%s'%s\n", delete_target, delete_force ? " (force)" : "");
        // actual delete logic goes here
    }

    return 0;
}
