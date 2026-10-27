#ifndef _PNG_HANDLER_H_
#define _PNG_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "decoders/decoder.h"
#include "utils/utils.h"


// __ FUNCTIONS ________________________________________________________________
// * Decode a PNG image into RGB and give it to the decoder callback. 
// * Transparent pixels are blended with a black background. 
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if no error occured and 1 otherwise. 
int decode_png(DECODER_t* decoder); 

#endif
