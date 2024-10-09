#include "utils.h"




void hex_to_rgb(int hex, int* r, int* g, int* b)
{
    *r = (hex >> 16) & 0xFF; 
    *g = (hex >> 8)  & 0xFF; 
    *b = (hex >> 0)  & 0xFF; 
    return; 
}