#ifndef _COLORS_H_
#define _COLORS_H_

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <locale.h>
#include <langinfo.h>
#include "options.h"
#include "utils/utils.h"


// __ DEFINIFIONS ______________________________________________________________
// The 256 colors palette: 16 system colors (they depend on the terminal, 
// never used), a 6x6x6 color cube and 24 grays. 
#define CUBE_FIRST_INDEX    16
#define GRAY_FIRST_INDEX    232
#define GRAY_COUNT          24

// Spread of the ordered dithering for each palette (about the distance 
// between two colors of the palette). 
#define ORDERED_SPREAD_256  40
#define ORDERED_SPREAD_16   96
#define ORDERED_SPREAD_MONO 255


// __ FUNCTIONS ________________________________________________________________
// * Choose the color mode from the environment: NO_COLOR and TERM=dumb give 
// * mono, COLORTERM=truecolor or 24bit gives truecolor, a TERM with 256color 
// * gives 256 colors and any other terminal gives 16 colors. 
// * return: the detected color mode. 
COLOR_MODE_t detect_color_mode(void); 

// * Choose the characters from the locale: half blocks when it uses UTF-8, 
// * ASCII otherwise (consoles without UTF-8 can't draw ▄). 
// * return: CHARSET_UTF8 or CHARSET_ASCII. 
CHARSET_t detect_charset(void); 

// * Convert a RGB frame to the color codes of the color mode:0xRRGGBB for 
// * truecolor, a palette index for 256/16/8 colors and 0 (black) or 1 
// * (white) for mono. 
// * param: rgb: the RGB frame (3 bytes per pixel). 
// * param: width: width of the frame. 
// * param: height: height of the frame. 
// * param: mode: the color mode (not COLORS_AUTO). 
// * param: dither: the dithering (not DITHER_AUTO). 
// * return: width * height color codes (that need to be freed) or NULL in 
// *         case of errors. 
int* map_colors(const unsigned char* rgb, size_t width, size_t height, COLOR_MODE_t mode, DITHER_MODE_t dither); 

#endif
