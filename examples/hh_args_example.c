#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "h.h"

#include <stdbool.h>

struct options {
    args_t args;
    const bool* verbose;
    const char* const* input;
    struct {
        args_t* args;
        const char* const* content;
    } create;
    struct {
        args_t* args;
        const char* const* output;
        const bool* backup;
    } rename;
    struct {
        args_t* args;
        const bool* recursive;
        const bool* yes;
    } delete;
};

void
options_init(struct options* opt) {
    opt->verbose = args_add_flag(&opt->args, HH_FLAG_BOOL, 
        .flag = 'v', 
        .flag_long = "verbose", 
        .desc = "Enable verbose output");
    opt->input = args_add_flag(&opt->args, HH_FLAG_PATH,
        .flag = 'i',
        .flag_long = "input",
        .name = "file",
        .desc = "the file to perform operations on",
        .required = 1);

    opt->create.args = args_add_command(&opt->args, "create", "touch a new file");
    opt->create.content = args_add_flag(opt->create.args, HH_FLAG_CSTR,
        .flag = 'c',
        .flag_long = "content",
        .desc = "data to write into the file");

    opt->rename.args = args_add_command(&opt->args, "rename", "rename an existing file");
    opt->rename.output = args_add_flag(opt->rename.args, HH_FLAG_PATH,
        .flag = 'o',
        .flag_long = "output",
        .name = "file",
        .desc = "file's new path",
        .required = 1);
    opt->rename.backup = args_add_flag(opt->rename.args, HH_FLAG_BOOL,
        .flag = 'b',
        .flag_long = "backup",
        .desc = "create a backup of the input file");

    opt->delete.args = args_add_command(&opt->args, "delete", "delete a file");
    opt->delete.recursive = args_add_flag(opt->delete.args, HH_FLAG_BOOL,
        .flag = 'r',
        .flag_long = "recursive",
        .desc = "delete directories and their contents");
    opt->delete.yes = args_add_flag(opt->delete.args, HH_FLAG_BOOL,
        .flag = 'y',
        .flag_long = "yes",
        .desc = "skip confirmation prompt");
}

void
opt_free(struct options* opt) {
    args_free(&opt->args);
}

int
main(int argc, char* argv[]) {
    struct options opt = {0};
    options_init(&opt);

    // parse the command line arguments
    if(!args_parse(&opt.args, HH_MSG_STREAM, argc, argv)) {
        args_print_error(&opt.args, HH_ERR_STREAM);
        printf("\n");
        args_print_usage(&opt.args, HH_MSG_STREAM, argc, argv);
        opt_free(&opt);
        return 1;
    }

    // print what operation we 'performed'
    if(args_parsed_cmd(&opt.args, opt.create.args)) {
        printf("%s created with%s%s%s\n", 
            *(opt.input), 
            *(opt.create.content) ? " \"" : " no data",
            *(opt.create.content) ? *(opt.create.content) : "",
            *(opt.create.content) ? "\" as content [-c]" : "");
    } else if(args_parsed_cmd(&opt.args, opt.rename.args)) {
        printf("%s renamed to %s%s\n", 
            *(opt.input), 
            *(opt.rename.output), 
            *(opt.rename.backup) ? " with backup [-b]" : "");
    } else if(args_parsed_cmd(&opt.args, opt.delete.args)) {
        printf("%s deleted%s%s\n", 
            *(opt.input), 
            *(opt.delete.recursive) ? " recursively [-r]" : "", 
            *(opt.delete.yes) ? " without confirmation [-y]" : "");
    }
    opt_free(&opt);
    return 0;
}
