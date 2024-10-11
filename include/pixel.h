#ifndef _PIXEL_H_
#define _PIXEL_H_


#include <stdio.h>
#include <stdlib.h>
#include "utils.h"


// __ DEFINIFIONS ______________________________________________________________
#define PIXEL_CHAR "▄"


// __ TYPES DEFINITIONS ________________________________________________________
typedef struct 
{
    int f_color; 
    int b_color; 
} PIXEL_t;


// __ FUNCTIONS ________________________________________________________________
// * Create a PIXEL_t struct and initialise the colors.
// * param: f_color: foreground color of the pixel (bottom when using ▄). 
// * param: b_color: background color of the pixel (top when using ▄). 
// * return: a pointer to the newly created pixel and NULL in case of errors. 
PIXEL_t* create_pixel(int f_color, int b_color);

// * Draw the pixel to the stdout. 
// * param: pixel: the pixel that will be drawn to the stdout. 
void draw_pixel(PIXEL_t *pixel); 

// * Change the color of a pixel. 
// * param: pixel: the pixel that need to be modified. 
// * param: hex_color: the new color of the pixel. 
// * param: type: 0 if we need to modify the background color 
// *        or 1 for the foreground.
void change_pixel_color(PIXEL_t* pixel, int hex_color, short int type);

#endif