#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display/pixel.h"
#include "options.h"


// __ DEFINIFIONS ______________________________________________________________
// Worst case size of one cell: cursor move "\x1b[65535C" and colors 
// "\x1b[38;2;255;255;255;48;2;255;255;255m" + ▄. 
#define CELL_MAX_LEN 48 

// Worst case size of the escape sequences added at the end of each row. 
#define ROW_END_MAX_LEN 8 

// Worst case size of the escape sequences added around a frame. 
#define FRAME_EXTRA_LEN 32 

#define ESC_RESET       "\x1b[0m"
#define ESC_BOLD        "\x1b[1m"
#define ESC_SYNC_BEGIN  "\x1b[?2026h"
#define ESC_SYNC_END    "\x1b[?2026l"


// __ TYPES DEFINITIONS ________________________________________________________
typedef struct
{
    char*   data; 
    size_t  len; 
} OUTPUT_BUFFER_t;

// How a pixel matrix is drawn. 
typedef struct
{
    size_t          width;      // number of cells per row. 
    size_t          rows;       // number of rows of cells. 
    size_t          offset;     // empty columns before the image (centering). 
    COLOR_MODE_t    colors;     // how the pixel colors are written. 
} RENDER_t;


// __ FUNCTIONS ________________________________________________________________
// * Draw a pixel matrix to the stdout with a single write. Color escape 
// * sequences are only sent when the color changes from the previous cell. 
// * param: pixels: the pixel matrix (width * rows cells). 
// * param: previous: the pixel matrix drawn just before, right above the 
// *        cursor, or NULL. When given, the cursor goes back to its top and 
// *        only the cells that changed are drawn again. 
// * param: render: size, position and color mode of the matrix. 
// * return: 0 if no error occured and 1 otherwise. 
int draw_pixels(PIXEL_t* pixels, PIXEL_t* previous, const RENDER_t* render); 

#endif
