#ifndef _PIXEL_H_
#define _PIXEL_H_


#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define PIXEL_CHAR "▄"

// __ TYPES DEFINITIONS ________________________________________________________
typedef struct 
{
    int f_color; 
    int b_color; 
} PIXEL_t;


PIXEL_t* create_pixel(int f_color, int b_color);
void draw_pixel(PIXEL_t *pixel); 
#endif