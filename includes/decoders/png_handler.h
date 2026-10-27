#ifndef _PNG_HANDLER_H_
#define _PNG_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "decoders/decoder.h"
#include "utils/utils.h"


// __ DEFINIFIONS ______________________________________________________________
#define PNG_SIGNATURE_LEN 8


// __ FUNCTIONS ________________________________________________________________
// * Check if the header bytes match the PNG signature. 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a PNG signature and 0 otherwise. 
int is_png(const unsigned char* header, size_t len); 

// * Decode a PNG image into RGB and give it to the decoder callback. 
// * Transparent pixels are blended with a black background. 
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if no error occured and 1 otherwise. 
int decode_png(DECODER_t* decoder); 

#endif
