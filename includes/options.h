#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <stdlib.h>


// __ DEFINIFIONS ______________________________________________________________
// plays value to follow the loop count stored in the file (-L). 
#define PLAYS_FROM_FILE -1


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

// Characters used to draw the image (-a, --utf8). 
typedef enum
{
    CHARSET_AUTO,       // ASCII when the locale is not UTF-8. 
    CHARSET_UTF8,       // half blocks, 2 pixels per cell. 
    CHARSET_ASCII       // one character per pixel, chosen by brightness. 
} CHARSET_t;

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
    int     plays;          // times animations are played: 0 until Ctrl-C, 
                            // PLAYS_FROM_FILE to follow the file. 
    int     max_fps;        // maximum frames drawn per second, 0 no limit. 
    size_t  frame;          // only display this frame (from 1), 0 for all. 
    size_t  term_cols;      // terminal size forced with -s, 0 to detect it. 
    size_t  term_rows; 
    size_t  max_cols;       // maximum image size given with -w/-H, 0 if none. 
    size_t  max_rows; 
    int     upscale;        // enlarge small images up to the maximum size. 
    FIT_MODE_t fit; 
    int     center;         // center the image horizontally. 
    COLOR_MODE_t colors; 
    DITHER_MODE_t dither; 
    CHARSET_t charset; 
    int     info;           // only print information about the images. 
} OPTIONS_t;

#endif
