#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define CLAGS_IMPLEMENTATION
#include <clags.h>

union ipv4_t{
    uint32_t addr;
    uint8_t segs[4];
};

bool verify_ipv4(clags_config_t *config, const char *arg_name, const char *arg, void *variable, void *data)
{
    (void) data;
    const char *pseg = arg;
    union ipv4_t *ip = (union ipv4_t*) variable;
    clags_strtoint_res_t res;
    for (size_t i=0; i<4; ++i){
        if (i > 0 && *pseg++ != '.'){
            clags_log(config, Clags_Error, "missing separator after segment %zu for ipv4 argument '%s': '%s'!", i, arg_name, arg);
            return false;
        }
        uint8_t seg = clags_strtouint8_s(pseg, &res, 10);
        if (res.no_digits || res.leading_spaces){
            clags_log(config, Clags_Error, "invalid number for segment %zu of ipv4 argument '%s': '%s'!", i+1, arg_name, pseg);
            return false;
        }
        if (res.out_of_range || res.negative){
            clags_log(config, Clags_Error, "segment value out of range [0, %"PRIu8"] for segment %zu of ipv4 argument '%s': '%s'!", UINT8_MAX, i+1, arg_name, pseg);
            return false;
        }
        if (ip){
            ip->segs[i] = seg;
        }
        pseg = res.endptr;
    }
    if (*pseg != '\0'){
        clags_log(config, Clags_Error, "trailing characters after valid ipv4 address for argument '%s': '%s': '%s'!", arg_name, arg, res.endptr);
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    union ipv4_t ip;
    bool help = false;

    clags_custom_t ip_type = {"ipv4", verify_ipv4, NULL};
    
    clags_arg_t args[] = {
        clags_positional(&ip, "address", "the ip address to connect to", .value_type=Clags_Custom, .custom=&ip_type),
        clags_flag_help(&help),
    };
    clags_config_t config = clags_config(args);

    const char *name = argv[0];
    clags_config_t *failed = clags_parse(argc, argv, &config);
    if (failed){
        clags_usage(name, failed);
        return 1;
    }
    if (help){
        clags_usage(name, &config);
        return 0;
    }
    
    printf("addr: %"PRIu32"\n", ip.addr);
    printf("segs:");
    for (size_t i=0; i<4; ++i){
        printf(" %"PRIu8, ip.segs[i]);
    }
    printf("\n");
    return 0;
}
