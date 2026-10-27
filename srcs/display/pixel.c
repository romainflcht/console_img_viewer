#include "display/pixel.h"


// * Create a PIXEL_t struct and initialise the colors.
// * param: f_color: foreground color of the pixel (bottom when using ▄). 
// * param: b_color: background color of the pixel (top when using ▄). 
// * return: a pointer to the newly created pixel and NULL in case of errors. 
PIXEL_t* create_pixel(int f_color, int b_color)
{
    PIXEL_t* new_pixel; 

    new_pixel = malloc(sizeof(PIXEL_t)); 
    if (!new_pixel)
        return NULL; 

    new_pixel->f_color = f_color; 
    new_pixel->b_color = b_color; 
    return new_pixel; 
}


// * Change the color of a pixel. 
// * param: pixel: the pixel that need to be modified. 
// * param: hex_color: the new color of the pixel. 
// * param: type: 1 if we need to modify the background color 
// *        or 0 for the foreground.
void change_pixel_color(PIXEL_t* pixel, int hex_color, short int type)
{
    if (type)
        pixel->f_color = hex_color; 
    
    else
        pixel->b_color = hex_color; 
}