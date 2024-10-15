#include <stdio.h>
#include <ctype.h>

#include "image.h"
#include "jpeg_read.h"

int main(void)
{
    IMG_t *img;

    img = create_img("img/img(80x40).jpeg");

    if (!img)
        return 1;

    load_jpeg(img); 
    draw_image(img); 
    return 0;
}
