#include <stdio.h>

#include "image.h"

int main(void)
{
    IMG_t* img;

    img = create_image(50, 30); 
    if (!img)
        return 1; 

    draw_image(img); 
    return 0; 
}