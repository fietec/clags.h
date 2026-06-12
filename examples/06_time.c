/*
  Example 6: Time Strings
      This example demonstrates how to verify and parse time strings.
*/

#include <stdio.h>
#include <inttypes.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

int main(int argc, char *argv[])
{
    char *host;
    // `clags_time_t` holds a converted time string and is a typedef of uint64_t.
    // It is used for both `Clags_TimeS` and `Clags_TimeNS`.
    clags_time_t timeout;
    clags_time_t interval;
    bool help = false;

    clags_arg_t args[] = {
        clags_positional(&host, "host", "the host to connect to"),
        // Verify a time string (like '1h', '2.5ms', '1h25m18s')
        // `Clags_TimeS` supports these units (case-insensitively): 'd', 'h', 'm', 's' and yields the input converted to seconds
        // `Clags_TimeNS` supports all the above and additionally: 'ns', 'us', 'ms' and yields the input converted to nanoseconds
        clags_positional(&timeout, "timeout", "total duration to hold the connection", .value_type=Clags_TimeS),
        clags_option('i', "interval", &interval, "TIME", "delay between connection attempts", .value_type=Clags_TimeNS, .default_input="10ms"),
        clags_flag_help(&help),
    };
    clags_config_t config = clags_config(args);

    clags_config_t *failed = clags_parse(argc, argv, &config);
    if (failed){
        clags_usage(argv[0], failed);
        return 1;
    }
    if (help){
        clags_usage(argv[0], &config);
        return 0;
    }
    printf("Connecting to '%s'..\n", host);
    printf("Timeout:  %"PRIu64" seconds\n", timeout);
    printf("Interval: %"PRIu64" nanoseconds\n", interval);
    return 0;
}
