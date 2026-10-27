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
        "  -w, --width=COLS      maximum width of the image in columns\n"
        "  -H, --height=ROWS     maximum height of the image in rows\n"
        "  -u, --upscale         enlarge small images up to the maximum size\n"
        "      --fit=MODE        terminal dimensions limiting the image: both\n"
        "                        (default), width, height or none\n"
        "  -C, --center          center the image horizontally\n"
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


// * Parse a strictly positive number. 
// * param: str: the string to parse. 
// * param: value: where the number will be stored. 
// * return: 0 if the number is valid and 1 otherwise. 
static int parse_count(const char* str, size_t* value)
{
    char*           end; 
    unsigned long   number; 

    number = strtoul(str, &end, 10); 
    if (end == str || *end || !number || *str == '-')
        return 1; 

    *value = number; 
    return 0; 
}


// * Parse a --fit mode. 
// * param: str: the string to parse. 
// * param: fit: where the mode will be stored. 
// * return: 0 if the mode is valid and 1 otherwise. 
static int parse_fit(const char* str, FIT_MODE_t* fit)
{
    static const char* names[] = {"both", "width", "height", "none"}; 
    size_t i; 

    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++)
    {
        if (!strcmp(str, names[i]))
        {
            *fit = i; 
            return 0; 
        }
    }

    return 1; 
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
        {"width",   required_argument, NULL, 'w'}, 
        {"height",  required_argument, NULL, 'H'}, 
        {"upscale", no_argument,       NULL, 'u'}, 
        {"fit",     required_argument, NULL, OPT_FIT}, 
        {"center",  no_argument,       NULL, 'C'}, 
        {"loop",    no_argument,       NULL, 'l'}, 
        {"help",    no_argument,       NULL, 'h'}, 
        {"version", no_argument,       NULL, 'V'}, 
        {NULL,      0,                 NULL, 0}
    }; 
    int opt; 

    memset(&args->options, 0, sizeof(OPTIONS_t)); 

    while ((opt = getopt_long(argc, argv, "s:w:H:uClhV", long_options, NULL)) != -1)
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

            case 'w':
                if (parse_count(optarg, &args->options.max_cols))
                {
                    print_error(NULL, "invalid width '%s'", optarg); 
                    return ARGS_ERROR; 
                }
                break; 

            case 'H':
                if (parse_count(optarg, &args->options.max_rows))
                {
                    print_error(NULL, "invalid height '%s'", optarg); 
                    return ARGS_ERROR; 
                }
                break; 

            case 'u':
                args->options.upscale = 1; 
                break; 

            case OPT_FIT:
                if (parse_fit(optarg, &args->options.fit))
                {
                    print_error(NULL, "invalid fit mode '%s', expected both, width, height or none", optarg); 
                    return ARGS_ERROR; 
                }
                break; 

            case 'C':
                args->options.center = 1; 
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
