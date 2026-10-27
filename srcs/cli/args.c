#include "cli/args.h"


// * Print how to use the program. 
// * param: stream: where the usage is printed. 
static void print_usage(FILE* stream)
{
    fprintf(
        stream, 
        "Usage: tv [-l] [-h] file1 [file2 ...]\n"
        "Display JPEG, PNG, GIF and WebP images in the terminal.\n\n"
        "  -l  loop animations until Ctrl-C (played once by default)\n"
        "  -h  show this help\n"
    ); 
}


// * Parse command-line options and filenames. 
// * param: argc: arguments count.
// * param: argv: arguments vector (array). 
// * param: args: where the options and the filenames will be stored. 
// * return: ARGS_OK, ARGS_ERROR if the arguments are invalid or ARGS_HELP if 
// *         the help was asked. 
int parse_arg(int argc, char** argv, ARGS_t* args)
{
    int opt; 

    args->loop = 0; 

    while ((opt = getopt(argc, argv, "lh")) != -1)
    {
        switch (opt)
        {
            case 'l':
                args->loop = 1; 
                break; 

            case 'h':
                print_usage(stdout); 
                return ARGS_HELP; 

            default:
                print_usage(stderr); 
                return ARGS_ERROR; 
        }
    }

    // Everything after the options are filenames. 
    args->files = argv + optind; 
    args->file_count = argc - optind; 

    if (args->file_count < 1)
    {
        print_usage(stderr); 
        return ARGS_ERROR; 
    }

    return ARGS_OK; 
}
