#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <stdlib.h>


// __ TYPES DEFINITIONS ________________________________________________________
// Colors used to draw the image (-c). 
typedef enum
{
    COLORS_AUTO, 
    COLORS_TRUECOLOR, 
    COLORS_256, 
    COLORS_16, 
    COLORS_8, 
    COLORS_MONO
} COLOR_MODE_t;

// How colors are approximated with a palette (--dither). 
typedef enum
{
    DITHER_AUTO,        // Floyd-Steinberg for still images, ordered for animations. 
    DITHER_FS, 
    DITHER_ORDERED, 
    DITHER_NONE
} DITHER_MODE_t;

// Which terminal dimensions limit the image size (--fit).
typedef enum
{
    FIT_BOTH, 
    FIT_WIDTH, 
    FIT_HEIGHT, 
    FIT_NONE
} FIT_MODE_t;

// Display options given on the command line. 
typedef struct
{
    int     loop;           // play animations until Ctrl-C. 
    size_t  term_cols;      // terminal size forced with -s, 0 to detect it. 
    size_t  term_rows; 
    size_t  max_cols;       // maximum image size given with -w/-H, 0 if none. 
    size_t  max_rows; 
    int     upscale;        // enlarge small images up to the maximum size. 
    FIT_MODE_t fit; 
    int     center;         // center the image horizontally. 
    COLOR_MODE_t colors; 
    DITHER_MODE_t dither; 
} OPTIONS_t;

#endif
