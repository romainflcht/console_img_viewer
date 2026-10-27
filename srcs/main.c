#include <stdio.h>

#include "cli/args.h"
#include "image/image.h"


// __ DEFINIFIONS ______________________________________________________________
// Exit codes: every file displayed, at least one file failed, Ctrl-C (shell 
// convention: 128 + SIGINT). 
#define EXIT_OK          0
#define EXIT_FILE_ERROR  1
#define EXIT_INTERRUPTED 130


int main(int argc, char** argv)
{
    IMG_t*  img;
    ARGS_t  args; 
    int     retval; 
    int     exit_code; 

    retval = parse_arg(argc, argv, &args);
    if (retval != ARGS_OK)
        return retval == ARGS_EXIT ? EXIT_OK : EXIT_FILE_ERROR;

    exit_code = EXIT_OK; 
    for (int i = 0; i < args.file_count; i++)
    {
        img = create_img(args.files[i]);
        if (!img)
        {
            exit_code = EXIT_FILE_ERROR; 
            continue; 
        }

        // Ctrl-C during an animation stops the program. 
        retval = draw_image(img, args.loop); 
        free_img(img);
        img = NULL; 

        if (retval)
            return EXIT_INTERRUPTED; 
    } 

    return exit_code;
}
