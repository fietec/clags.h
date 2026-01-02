/*
    Example 8: Image Processor
      This is a full example of most of clags's features and indents to show 
      It implements the cli for an image processor that is capable of converting, resizing and tagging images.
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define CLAGS_IMPLEMENTATION
#include "../clags.h"

#define return_defer(res) do{result=(res); goto defer;}while(0)

/* Global arguments*/

bool help = false;
bool version = false;

/* Convert subcommand config */

char *convert_input_file = NULL;
char *convert_output_file = "out.png";
uint8_t convert_quality = 90;
bool convert_warnings = false;

clags_choice_t convert_choices[] = {
    {"PNG", ""},
    {"JPEG", ""},
    {"TIFF", ""},
};

clags_choices_t choices = clags_choice(convert_choices, .print_no_details=true);
clags_choice_t *convert_format = clags_choice_default(convert_choices, 0);

clags_arg_t convert_args[] = {
    clags_required(&convert_input_file, "input_file", "the file to convert", .value_type=Clags_File),
    clags_optional('o', "output", &convert_output_file, "FILE", "the file to output"),
    clags_optional('f', "format", &convert_format, "FORMAT", "the format which to convert to", .value_type=Clags_Choice, .choices=&choices),
    clags_optional('q', "quality", &convert_quality, "QUALITY", "the quality of the convertion", .value_type=Clags_UInt8),
    clags_flag('w', "warnings", &convert_warnings, "print warnings"),
    clags_flag('v', "version", &version, "print the version", .exit=true),
    clags_flag_help(&help),
};

clags_config_t convert_config = clags_config(convert_args, .ignore_prefix="!");

/* Resize subcommand config */

char *resize_input_file = NULL;
char *resize_output_file = "resized.png";
uint32_t resize_width = 0;
uint32_t resize_height = 0;
bool resize_keep_aspect = false;

clags_arg_t resize_args[] = {
    clags_required(&resize_input_file, "input_file", "the file to resize", .value_type=Clags_File),
    clags_optional('o', "output", &resize_output_file, "FILE", "the file to output"),
    clags_optional('w', "width", &resize_width, "PIXELS", "the width of the output image", .value_type=Clags_UInt32),
    clags_optional('h', "height", &resize_height, "PIXELS", "the height of the output image", .value_type=Clags_UInt32),
    clags_flag('k', "keep-aspect", &resize_keep_aspect, "keep the aspect ratio of the input image"),
    clags_flag('v', "version", &version, "print the version", .exit=true),
    clags_flag_help(&help),
};

clags_config_t resize_config = clags_config(resize_args, .ignore_prefix="!");

/* Tag subcommand config */

clags_list_t tag_images = clags_list();
clags_list_t tag_values = clags_list();

bool tag_overwrite = false;

clags_choice_t tag_formats[] = {
    {"JSON", "Save tags in JSON format"},
    {"XML", "Save tags in XML format"},
    {"YAML", "Save tags in YAML format"},
};
clags_choices_t tag_choices = clags_choice(tag_formats);
clags_choice_t *tag_format = clags_choice_default(tag_formats, 0);

clags_arg_t tag_args[] = {
    clags_required(&tag_images, "images", "list of images to tag", .is_list=true, .value_type=Clags_File),
    clags_required(&tag_values, "tags", "tags to apply to all images", .is_list=true),
    clags_optional('f', "format", &tag_format, "FORMAT", "format to save tags", .value_type=Clags_Choice, .choices=&tag_choices),
    clags_flag('o', "overwrite", &tag_overwrite, "replace existing tags"),
    clags_flag('v', "version", &version, "print the version", .exit=true),
    clags_flag_help(&help),
};

clags_config_t tag_config = clags_config(tag_args, .ignore_prefix="!", .list_terminator="::");

/* Parent config */

clags_subcmd_t subcommands[] = {
    {"convert", "convert an image to a different format", &convert_config},
    {"resize", "resize an image ", &resize_config},
    {"tag", "tag multiple images", &tag_config}
};

clags_subcmds_t subcmds = clags_subcommand(subcommands);
clags_subcmd_t *selected_subcmd = NULL;

clags_arg_t parent_args[] = {
    clags_required(&selected_subcmd, "action", "the subcommand to run", .value_type=Clags_Subcmd, .subcmds=&subcmds),
    clags_flag('v', "version", &version, "print the version", .exit=true),
    clags_flag_help(&help),
};

clags_config_t parent_config = clags_config(parent_args, .ignore_prefix="!");

int main(int argc, char **argv)
{
    int result = 0;
    const char *program_name = argv[0];

    // parsing
    clags_config_t *failed_config = clags_parse(argc, argv, &parent_config);
    if (failed_config){
        // print usage for failed subcommand
        clags_usage(program_name, failed_config);
        return_defer(1);
    }
    if (help){
        // print help for the subcommand for which `--help` was provided
        if (selected_subcmd){
            clags_usage(program_name, selected_subcmd->config);
        } else{
            clags_usage(program_name, &parent_config);
        }
        return_defer(0);
    }
    if (version){
        // this could have been set from any subcommand
        printf("%s - v.1.0.0\n", program_name);
        return_defer(0);
    }

    // execute chosen subcommand
    if (selected_subcmd == &subcommands[0]){
        printf("Convert subcommand selected:\n");
        printf("  Input file : %s\n", convert_input_file);
        printf("  Output file: %s\n", convert_output_file);
        printf("  Format     : %s\n", convert_format->value);
        printf("  Quality    : %" PRIu8 "\n", convert_quality);
        printf("  Warnings   : %s\n", convert_warnings ? "true" : "false");

        // convertion implementation goes here
        
    } else if (selected_subcmd == &subcommands[1]){
        printf("Resize subcommand selected:\n");
        printf("  Input file : %s\n", resize_input_file);
        printf("  Output file: %s\n", resize_output_file);
        printf("  Width      : %" PRIu32 "\n", resize_width);
        printf("  Height     : %" PRIu32 "\n", resize_height);
        printf("  Keep Aspect: %s\n", resize_keep_aspect ? "true" : "false");
        
        // resizing implementation goes here
    } else if (selected_subcmd == &subcommands[2]){
        printf("Tagging %zu images:\n", tag_images.count);
        for (size_t i=0; i<tag_images.count; ++i){
            printf("  Image: %s\n", clags_list_element(tag_images, char*, i));
        }
        printf("Tags to apply (%zu):\n", tag_values.count);
        for (size_t i=0; i<tag_values.count; ++i){
            printf("  %s\n", clags_list_element(tag_values, char*, i));
        }
        printf("Overwrite: %s\n", tag_overwrite ? "yes" : "no");
        printf("Format: %s\n", tag_format->value);

        // tagging implementation goes here
        
    } else{
        fprintf(stderr, "[ERROR] Invalid subcommand selected: %p!\n", selected_subcmd);
        return_defer(1);
    }
    
defer:
    // free all the memory that clags might have allocated during parsing
    clags_list_free(&tag_images);
    clags_list_free(&tag_values);
    return result;
}
