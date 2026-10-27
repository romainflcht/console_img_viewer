#ifndef _GIF_HANDLER_H_
#define _GIF_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gif_lib.h>
#include "decoders/frame.h"
#include "utils/utils.h"


// __ DEFINIFIONS ______________________________________________________________
#define GIF_SIGNATURE_LEN 6

// Browsers display frames with a delay under 20ms during 100ms, do the same.
#define GIF_MIN_DELAY_MS     20
#define GIF_DEFAULT_DELAY_MS 100


// __ FUNCTIONS ________________________________________________________________
// * Check if the header bytes match the GIF signature.
// * param: header: the first bytes of the file.
// * param: len: the number of bytes available in header.
// * return: 1 if the header is a GIF signature and 0 otherwise.
int is_gif(const unsigned char* header, size_t len);

// * Decode every frame of a GIF image. Each frame is composed over the
// * previous ones (following its disposal mode) and given to the callback.
// * Transparent pixels are black.
// * param: filename: path to the GIF image.
// * param: callback: function called with each decoded frame.
// * param: user: pointer given to the callback.
// * return: 0 if at least one frame was decoded and 1 otherwise.
int decode_gif(char* filename, FRAME_CALLBACK_t callback, void* user);

#endif
