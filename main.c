#include <stdio.h>
#include <ctype.h>
#include <termios.h>

#include "args.h"
#include "image.h"


int main(int argc, char** argv)
{
    IMG_t   *img;
    char**  files; 

    files = parse_arg(argc, argv);
    if (!files)
        return 1;

    for (int i = 0; i < argc - 1; i++)
    {
        img = create_img(files[i]);
        if (!img)
            continue; 

        draw_image(img); 
        free_img(img);
        img = NULL; 
    } 

    
    free(files);
    files = NULL; 
    return 0;
}
