#ifndef _WEBP_HANDLER_H_
#define _WEBP_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webp/decode.h>
#include <webp/demux.h>
#include "decoders/decoder.h"
#include "utils/utils.h"


// __ DEFINIFIONS ______________________________________________________________
// "RIFF" + file size (4 bytes) + "WEBP". 
#define WEBP_SIGNATURE_LEN 12

// Browsers display frames with a delay under 20ms during 100ms, do the same. 
#define WEBP_MIN_DELAY_MS     20
#define WEBP_DEFAULT_DELAY_MS 100


// __ FUNCTIONS ________________________________________________________________
// * Check if the header bytes match the WebP signature. 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a WebP signature and 0 otherwise. 
int is_webp(const unsigned char* header, size_t len); 

// * Decode every frame of a WebP image (still or animated) and give them to 
// * the callback. Transparent pixels are blended with a black background. 
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if at least one frame was decoded and 1 otherwise. 
int decode_webp(DECODER_t* decoder); 

#endif
