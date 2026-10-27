#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdio.h>
#include "jpeg_handler.h"
#include "png_handler.h"
#include "pixel.h"
#include "utils.h"

// __ DEFINIFIONS ______________________________________________________________
#define BLACK 0x000000; 


// __ TYPES DEFINITIONS ________________________________________________________
typedef unsigned char pixel_row_t;

typedef enum
{
    FORMAT_UNKNOWN,
    FORMAT_JPEG,
    FORMAT_PNG
} IMG_FORMAT_t;

typedef struct
{
    size_t  height; 
    size_t  width; 
    size_t  channel_count; 
    PIXEL_t *img; 
} IMG_t;


// __ FUNCTIONS ________________________________________________________________
// * Create an IMG_t struct from a JPEG or PNG file. 
// * param: filename: path to the image.
// * return: the pointer to the IMG_t struct of NULL in case of errors. 
IMG_t* create_img(char* filename); 

// * Free the IMG_t structure correctly.
// * param: img: the image that need to be freed.
void free_img(IMG_t* img);

// * Draw the IMG_t pixel matrix to the stdout. 
// * param: img: image that will be drawn to the screen. 
void draw_image(IMG_t *img); 

// * Detect the format of an image file by reading its signature.
// * param: filename: path to the image.
// * return: the format of the image or FORMAT_UNKNOWN.
IMG_FORMAT_t get_img_format(char* filename);

// * Load a pixel row scan into the correct pixel row of the IMG_t struct. 
// * param: img: the img where the pixel row need to be inserted. 
// * param: pixel_row_t: the RGB pixel row that need to be inserted
// *                     (unsigned char ptr). 
// * param: line: the row where the pixel row need to be inserted. 
void load_pixelscan(IMG_t* img, pixel_row_t* pixel_scanline, unsigned int line);

#endif
