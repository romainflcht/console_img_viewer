#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display/pixel.h"


// __ DEFINIFIONS ______________________________________________________________
// Worst case size of one cell: "\x1b[38;2;255;255;255;48;2;255;255;255m" + ▄.
#define CELL_MAX_LEN 48

// Worst case size of the escape sequences added at the end of each row.
#define ROW_END_MAX_LEN 8

#define ESC_RESET "\x1b[0m"


// __ TYPES DEFINITIONS ________________________________________________________
typedef struct
{
    char*   data;
    size_t  len;
} OUTPUT_BUFFER_t;


// __ FUNCTIONS ________________________________________________________________
// * Draw a pixel matrix to the stdout with a single write. Color escape
// * sequences are only sent when the color changes from the previous cell.
// * param: pixels: the pixel matrix (width * rows cells).
// * param: width: number of cells per row.
// * param: rows: number of rows of cells.
// * return: 0 if no error occured and 1 otherwise.
int draw_pixels(PIXEL_t* pixels, size_t width, size_t rows);

#endif
