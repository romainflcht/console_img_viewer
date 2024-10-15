#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdio.h>
#include <jpeglib.h>
#include "jpeg_read.h"
#include "pixel.h"
#include "utils.h"

// __ DEFINIFIONS ______________________________________________________________
#define BLACK 0x000000; 


// __ TYPES DEFINITIONS ________________________________________________________
typedef unsigned char pixel_row_t;

typedef struct
{
    size_t  height; 
    size_t  width; 
    size_t  channel_count; 
    PIXEL_t *img; 
    JPEG_HANDLER_t* handler; 
} IMG_t;


// __ FUNCTIONS ________________________________________________________________
// * Create an IMG_t struct and initialise the image with BLACK pixels. 
// * param: height: height of the image. 
// * param: width: width of the image. 
// * return: the pointer to the IMG_t struct of NULL in case of errors. 
IMG_t* create_img(char* filename); 

// * Free the IMG_t structure correctly.
// * param: img: the image that need to be freed. 
// * return: 0 in case of success and 1 in case of an error. 
void free_img(IMG_t* img);

// * Draw the IMG_t pixel matrix to the stdout. 
// * param: img: image that will be drawn to the screen. 
void draw_image(IMG_t *img); 

// * Load the jpeg img into the IMG_t data structure. 
// * param: img: the IMG_t structure where JPEG data need to be inserted in. 
// * param: file_path: the file path to the JPEG image. 
// * return: 0 if no error occured and 1 otherwise. 
int load_jpeg(IMG_t* img); 

// * Load a pixel row scan into the correct pixel row of the IMG_t struct. 
// * param: img: the img where the pixel row need to be inserted. 
// * param: pixel_row_t: the pixel row that need to be inserted 
// *                     (unsigned char ptr). 
// * param: line: the row where the pixel row need to be inserted. 
void load_pixelscan(IMG_t* img, pixel_row_t* pixel_scanline, unsigned int line);

#endif