#ifndef _UTILS_H
#define _UTILS_H


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#define _DEBUG(str)    printf(str)

#define PROGRAM_NAME    "tv"


// __ FUNCTIONS ________________________________________________________________
// * Convert hex color to rgb color. 
// * param: hex: the color in hex that will be converted to rgb. 
// * param: r: the red value [0-255] from the hex color. 
// * param: g: the green value [0-255] from the hex color. 
// * param: b: the blue value [0-255] from the hex color. 
void hex_to_rgb(int hex, int* r, int* g, int* b); 

// * Convert rgb color to hex color. 
// * param: r: the red value [0-255]. 
// * param: g: the green value [0-255]. 
// * param: b: the blue value [0-255]. 
// * param: hex: where the converted hex color will be stored. 
void rgb_to_hex(int r, int g, int b, int* hex); 

// * Print an error message on stderr, prefixed with the program name and the 
// * file concerned: "tv: file: message". 
// * param: filename: the file concerned or NULL. 
// * param: format: printf like format of the message. 
void print_error(const char* filename, const char* format, ...); 

#endif