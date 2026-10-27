#ifndef _PNG_HANDLER_H_
#define _PNG_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>


// __ DEFINIFIONS ______________________________________________________________
#define PNG_SIGNATURE_LEN 8


// __ FUNCTIONS ________________________________________________________________
// * Check if the header bytes match the PNG signature.
// * param: header: the first bytes of the file.
// * param: len: the number of bytes available in header.
// * return: 1 if the header is a PNG signature and 0 otherwise.
int is_png(const unsigned char* header, size_t len);

// * Decode a PNG image into a RGB buffer (3 bytes per pixel). Transparent
// * pixels are blended with a black background.
// * param: filename: path to the PNG image.
// * param: width: where the image width will be stored.
// * param: height: where the image height will be stored.
// * param: channel_count: where the original channel count will be stored.
// * return: the RGB buffer (that need to be freed) or NULL in case of errors.
unsigned char* decode_png(char* filename, size_t* width, size_t* height, size_t* channel_count);

#endif
