#include <stdio.h>
#include <ctype.h>

#include "args.h"
#include "image.h"
#include "jpeg_handler.h"

int main(int argc, char** argv)
{
    IMG_t   *img;
    char*   filename; 
    int     retval;

    filename = parse_arg(argc, argv); 
    img = create_img(filename);

    if (!img)
        return 1;

    retval = load_jpeg(img);
    if (retval)
    {
        free_img(img);
        return 1; 
    } 

    draw_image(img); 
    free_img(img);

    return 0;
}
