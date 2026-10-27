#include <stdio.h>

#include "cli/args.h"
#include "image/image.h"


int main(int argc, char** argv)
{
    IMG_t*  img;
    ARGS_t  args; 
    int     retval; 

    retval = parse_arg(argc, argv, &args);
    if (retval != ARGS_OK)
        return retval == ARGS_HELP ? 0 : 1;

    for (int i = 0; i < args.file_count; i++)
    {
        img = create_img(args.files[i]);
        if (!img)
            continue; 

        // Ctrl-C during an animation stops the program. 
        retval = draw_image(img, args.loop); 
        free_img(img);
        img = NULL; 

        if (retval)
            break; 
    } 

    return 0;
}
