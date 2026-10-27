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
        "Colors:\n"
        "  -c, --colors=MODE     truecolor, 256, 16, 8 or mono (no color, only\n"
        "                        blocks); auto (default) uses NO_COLOR, TERM and\n"
        "                        COLORTERM\n"
        "      --dither=MODE     how colors are approximated: auto (default: fs\n"
        "                        for images, ordered for animations), fs, ordered\n"
        "                        or none\n"
        "\n"
        "Characters:\n"
        "  -a, --ascii           draw with ASCII characters (one per pixel, chosen\n"
        "                        by brightness), used when the locale is not UTF-8\n"
        "      --utf8            draw with half blocks even if the locale is not\n"
        "                        UTF-8\n"
        "\n"
        "Animation:\n"
        "  -l, --loop            play animations until Ctrl-C (once by default)\n"
        "  -n, --plays=N         play animations N times\n"
        "  -L, --file-loop       play animations as many times as the file says\n"
        "  -f, --fps=N           draw at most N frames per second (slow links)\n"
        "      --frame=N         only display the frame N (from 1) of an animation\n"
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


// * Find a name in a list of names. 
// * param: str: the name to find. 
// * param: names: the list of names. 
// * param: count: the number of names. 
// * param: index: where the index of the name will be stored. 
// * return: 0 if the name was found and 1 otherwise. 
static int parse_name(const char* str, const char** names, size_t count, int* index)
{
    size_t i; 

    for (i = 0; i < count; i++)
    {
        if (!strcmp(str, names[i]))
        {
            *index = i; 
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
        {"colors",  required_argument, NULL, 'c'}, 
        {"dither",  required_argument, NULL, OPT_DITHER}, 
        {"ascii",   no_argument,       NULL, 'a'}, 
        {"utf8",    no_argument,       NULL, OPT_UTF8}, 
        {"loop",      no_argument,       NULL, 'l'}, 
        {"plays",     required_argument, NULL, 'n'}, 
        {"file-loop", no_argument,       NULL, 'L'}, 
        {"fps",       required_argument, NULL, 'f'}, 
        {"frame",     required_argument, NULL, OPT_FRAME}, 
        {"help",    no_argument,       NULL, 'h'}, 
        {"version", no_argument,       NULL, 'V'}, 
        {NULL,      0,                 NULL, 0}
    }; 
    static const char* color_names[] = {"auto", "truecolor", "256", "16", "8", "mono"}; 
    static const char* dither_names[] = {"auto", "fs", "ordered", "none"}; 
    static const char* fit_names[] = {"both", "width", "height", "none"}; 
    size_t  count; 
    int     opt; 
    int     index; 

    memset(&args->options, 0, sizeof(OPTIONS_t)); 
    args->options.plays = 1; 

    while ((opt = getopt_long(argc, argv, "s:w:H:uCc:aln:Lf:hV", long_options, NULL)) != -1)
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
                if (parse_name(optarg, fit_names, sizeof(fit_names) / sizeof(fit_names[0]), &index))
                {
                    print_error(NULL, "invalid fit mode '%s', expected both, width, height or none", optarg);
                    return ARGS_ERROR;
                }
                args->options.fit = index; 
                break;

            case 'C':
                args->options.center = 1; 
                break; 

            case 'c':
                if (parse_name(optarg, color_names, sizeof(color_names) / sizeof(color_names[0]), &index))
                {
                    print_error(NULL, "invalid color mode '%s', expected auto, truecolor, 256, 16, 8 or mono", optarg); 
                    return ARGS_ERROR; 
                }
                args->options.colors = index; 
                break; 

            case OPT_DITHER:
                if (parse_name(optarg, dither_names, sizeof(dither_names) / sizeof(dither_names[0]), &index))
                {
                    print_error(NULL, "invalid dither mode '%s', expected auto, fs, ordered or none", optarg); 
                    return ARGS_ERROR; 
                }
                args->options.dither = index; 
                break; 

            case 'a':
                args->options.charset = CHARSET_ASCII; 
                break; 

            case OPT_UTF8:
                args->options.charset = CHARSET_UTF8; 
                break; 

            case 'l':
                args->options.plays = 0; 
                break; 

            case 'n':
                if (parse_count(optarg, &count) || count > INT_MAX)
                {
                    print_error(NULL, "invalid number of plays '%s'", optarg); 
                    return ARGS_ERROR; 
                }
                args->options.plays = count; 
                break; 

            case 'L':
                args->options.plays = PLAYS_FROM_FILE; 
                break; 

            case 'f':
                if (parse_count(optarg, &count) || count > 1000)
                {
                    print_error(NULL, "invalid frame rate '%s', expected 1 to 1000", optarg); 
                    return ARGS_ERROR; 
                }
                args->options.max_fps = count; 
                break; 

            case OPT_FRAME:
                if (parse_count(optarg, &args->options.frame))
                {
                    print_error(NULL, "invalid frame '%s'", optarg); 
                    return ARGS_ERROR; 
                }
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

    // The color mode is detected from the environment when not given. 
    if (args->options.colors == COLORS_AUTO)
        args->options.colors = detect_color_mode();

    if (args->options.charset == CHARSET_AUTO)
        args->options.charset = detect_charset(); 

    if (args->file_count < 1)
    {
        print_error(NULL, "missing file operand"); 
        fprintf(stderr, "Try '" PROGRAM_NAME " --help' for more information.\n"); 
        return ARGS_ERROR; 
    }

    return ARGS_OK; 
}
