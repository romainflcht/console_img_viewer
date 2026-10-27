#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "decoders/jpeg_handler.h"
#include "decoders/png_handler.h"
#include "decoders/gif_handler.h"
#include "decoders/webp_handler.h"
#include "display/terminal.h"
#include "display/pixel.h"
#include "display/renderer.h"
#include "display/animation.h"
#include "utils/utils.h"
#include "utils/file.h"
#include "options.h"

// __ DEFINIFIONS ______________________________________________________________
#define BLACK 0x000000; 

// Terminal rows kept free under the image (info line, blank line, prompt).
#define RESERVED_ROWS 3


// __ TYPES DEFINITIONS ________________________________________________________
typedef unsigned char pixel_row_t;

typedef enum
{
    FORMAT_UNKNOWN,
    FORMAT_JPEG,
    FORMAT_PNG,
    FORMAT_GIF,
    FORMAT_WEBP
} IMG_FORMAT_t;

typedef struct
{
    size_t  height; 
    size_t  width; 
    size_t  src_height;
    size_t  src_width;
    size_t  channel_count; 
    int     size_known; 
    size_t  frame_count; 
    size_t  frame_capacity; 
    const OPTIONS_t*    options; 
    PIXEL_t *img;// frame_count pixel matrices, one after the other. 
    int*    delays;     // display time of each frame in milliseconds. 
} IMG_t;


// __ FUNCTIONS ________________________________________________________________
// * Create an IMG_t struct from a JPEG, PNG, GIF or WebP file, resized to 
// * fit inside the current terminal. Animated images keep all their frames.
// * param: filename: path to the image, or "-" for the standard input. 
// * param: options: the display options. 
// * return: the pointer to the IMG_t struct of NULL in case of errors. 
IMG_t* create_img(char* filename, const OPTIONS_t* options); 

// * Free the IMG_t structure correctly.
// * param: img: the image that need to be freed.
void free_img(IMG_t* img);

// * Draw the IMG_t pixel matrix to the stdout, animated images are played 
// * when the output is a terminal. 
// * param: img: image that will be drawn to the screen. 
// * return: 1 if the user pressed Ctrl-C during an animation, 0 otherwise. 
int draw_image(IMG_t *img); 

// * Detect the format of an image from its signature. 
// * param: data: the content of the file. 
// * param: len: the size of the data. 
// * return: the format of the image or FORMAT_UNKNOWN. 
IMG_FORMAT_t get_img_format(const unsigned char* data, size_t len); 

// * Resize a RGB buffer using area averaging.
// * param: src: the RGB buffer to resize.
// * param: src_width: width of the source buffer.
// * param: src_height: height of the source buffer.
// * param: dst_width: width of the resized buffer.
// * param: dst_height: height of the resized buffer.
// * return: the resized RGB buffer (that need to be freed) or NULL in case of
// *         errors.
unsigned char* resize_rgb(unsigned char* src, size_t src_width, size_t src_height, size_t dst_width, size_t dst_height);

// * Load a pixel row scan into the correct pixel row of a frame. 
// * param: img: the img where the pixel row need to be inserted. 
// * param: frame: the pixel matrix of the frame. 
// * param: pixel_row_t: the RGB pixel row that need to be inserted
// *                     (unsigned char ptr). 
// * param: line: the row where the pixel row need to be inserted. 
void load_pixelscan(IMG_t* img, PIXEL_t* frame, pixel_row_t* pixel_scanline, unsigned int line);

#endif
