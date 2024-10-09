#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "pixel.h"

// __ DEFINIFIONS ______________________________________________________________

#define BLACK 0x000000; 
#define WHITE 0xFFFFFF; 

typedef struct
{
    size_t height; 
    size_t width; 
    PIXEL_t **img; 
} IMG_t;

IMG_t *create_image(int height, int width); 
void draw_image(IMG_t *img); 
#endif