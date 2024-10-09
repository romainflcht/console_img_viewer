#include "pixel.h"



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