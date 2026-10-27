#include "cli/args.h"


// * Print how to use the program. 
// * param: stream: where the usage is printed. 
static void print_usage(FILE* stream)
{
    fprintf(
        stream, 
        "Usage: " PROGRAM_NAME " [OPTION]... FILE...\n"
        "Display JPEG, PNG, GIF and WebP images in the terminal.\n"
        "With FILE -, read the standard input.\n"
        "\n"
        "Size:\n"
        "  -s, --size=COLSxROWS  terminal size, when it can't be detected\n"
        "\n"
        "Animation:\n"
        "  -l, --loop            loop animations until Ctrl-C (played once by default)\n"
        "\n"
        "Other:\n"
        "  -h, --help            show this help and exit\n"
        "  -V, --version         show the version and exit\n"
        "\n"
        "Exit status: 0 if every file was displayed, 1 if a file could not be\n"
        "displayed, 130 if stopped with Ctrl-C.\n"
    ); 
}


// * Parse a "COLSxROWS" size. 
// * param: str: the string to parse. 
// * param: cols: where the number of columns will be stored. 
// * param: rows: where the number of rows will be stored. 
// * return: 0 if the size is valid and 1 otherwise. 
static int parse_size(const char* str, size_t* cols, size_t* rows)
{
    char*           end; 
    unsigned long   value; 

    value = strtoul(str, &end, 10); 
    if (end == str || (*end != 'x' && *end != 'X') || !value)
        return 1; 

    *cols = value; 
    str = end + 1; 
    value = strtoul(str, &end, 10); 
    if (end == str || *end || !value)
        return 1; 

    *rows = value; 
    return 0; 
}


// * Parse command-line options and filenames.
// * param: argc: arguments count.
// * param: argv: arguments vector (array). 
// * param: args: where the options and the filenames will be stored. 
// * return: ARGS_OK, ARGS_ERROR if the arguments are invalid or ARGS_EXIT if 
// *         the program must exit successfully (help or version). 
int parse_arg(int argc, char** argv, ARGS_t* args)
{
    static const struct option long_options[] = {
        {"size",    required_argument, NULL, 's'}, 
        {"loop",    no_argument,       NULL, 'l'}, 
        {"help",    no_argument,       NULL, 'h'}, 
        {"version", no_argument,       NULL, 'V'}, 
        {NULL,      0,                 NULL, 0}
    }; 
    int opt; 

    memset(&args->options, 0, sizeof(OPTIONS_t)); 

    while ((opt = getopt_long(argc, argv, "s:lhV", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 's':
                if (parse_size(optarg, &args->options.term_cols, &args->options.term_rows))
                {
                    print_error(NULL, "invalid size '%s', expected COLSxROWS (e.g. 80x24)", optarg); 
                    return ARGS_ERROR; 
                }
                break; 

            case 'l':
                args->options.loop = 1; 
                break; 

            case 'h':
                print_usage(stdout); 
                return ARGS_EXIT; 

            case 'V':
                printf(PROGRAM_NAME " " TV_VERSION "\n"); 
                return ARGS_EXIT; 

            default:
                fprintf(stderr, "Try '" PROGRAM_NAME " --help' for more information.\n"); 
                return ARGS_ERROR; 
        }
    }

    // Everything after the options are filenames. 
    args->files = argv + optind; 
    args->file_count = argc - optind; 

    if (args->file_count < 1)
    {
        print_error(NULL, "missing file operand"); 
        fprintf(stderr, "Try '" PROGRAM_NAME " --help' for more information.\n"); 
        return ARGS_ERROR; 
    }

    return ARGS_OK; 
}
