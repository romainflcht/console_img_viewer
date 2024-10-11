#include "pixel.h"


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


// * Draw the pixel to the stdout. 
// * param: pixel: the pixel that will be drawn to the stdout. 
void draw_pixel(PIXEL_t *pixel)
{
    int f_r, f_g, f_b; 
    int b_r, b_g, b_b; 

    hex_to_rgb(pixel->f_color, &f_r, &f_g, &f_b); 
    hex_to_rgb(pixel->b_color, &b_r, &b_g, &b_b); 
    
    printf("\x1b[38;2;%d;%d;%dm", f_r, f_g, f_b); 
    printf("\x1b[48;2;%d;%d;%dm", b_r, b_g, b_b);
    printf(PIXEL_CHAR);
    printf("\x1b[0m"); 
    return; 
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