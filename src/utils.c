#include "utils.h"


// * Convert hex color to rgb color. 
// * param: hex: the color in hex that will be converted to rgb. 
// * param: r: the red value [0-255] from the hex color. 
// * param: g: the green value [0-255] from the hex color. 
// * param: b: the blue value [0-255] from the hex color. 
void hex_to_rgb(int hex, int* r, int* g, int* b)
{
    *r = (hex >> 16) & 0xFF; 
    *g = (hex >> 8)  & 0xFF; 
    *b = (hex >> 0)  & 0xFF; 
    return; 
}

// * Convert rgb color to hex color. 
// * param: r: the red value [0-255]. 
// * param: g: the green value [0-255]. 
// * param: b: the blue value [0-255]. 
// * param: hex: where the converted hex color will be stored. 
void rgb_to_hex(int r, int g, int b, int* hex)
{
    *hex = (r << 16) | (g << 8) | (b << 0);
    return; 
}
