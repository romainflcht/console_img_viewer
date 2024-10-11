#include <stdio.h>

#include "image.h"
#include "jpeg_read.h"


int main(void)
{
    IMG_t* img;

    img = create_img("img/img1.jpeg"); 
    if (!img)
        return 1; 

    draw_image(img);
    free_img(img); 
    return 0; 
}