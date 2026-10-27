#ifndef _BMP_HANDLER_H_
#define _BMP_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decoders/decoder.h"
#include "utils/utils.h"


// __ DEFINIFIONS ______________________________________________________________
#define BMP_SIGNATURE_LEN   2
#define BMP_FILE_HEADER_LEN 14

// DIB header sizes. 
#define BMP_CORE_HEADER_LEN 12      // OS/2 BITMAPCOREHEADER. 
#define BMP_INFO_HEADER_LEN 40      // BITMAPINFOHEADER, V2 to V5 are bigger. 

// Compression methods. 
#define BMP_RGB             0
#define BMP_RLE8            1
#define BMP_RLE4            2
#define BMP_BITFIELDS       3
#define BMP_ALPHABITFIELDS  6

// Biggest accepted width or height, protects against absurd headers. 
#define BMP_MAX_SIZE        65535


// __ TYPES DEFINITIONS ________________________________________________________
// Header fields needed to decode the pixels. 
typedef struct
{
    size_t                  width; 
    size_t                  height; 
    int                     top_down;   // rows stored from the top. 
    unsigned int            bpp; 
    unsigned int            compression; 
    unsigned long           masks[4];   // red, green, blue, alpha. 
    const unsigned char*    palette; 
    size_t                  palette_count; 
    size_t                  palette_entry_len; 
    const unsigned char*    pixels; 
    size_t                  pixels_len; 
} BMP_INFO_t;


// __ FUNCTIONS ________________________________________________________________
// * Check if the header bytes match the BMP signature. 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a BMP signature and 0 otherwise. 
int is_bmp(const unsigned char* header, size_t len); 

// * Decode a BMP image (1, 4, 8, 16, 24 or 32 bits, palette, bit fields and 
// * RLE4/RLE8 compression) and give it to the decoder callback. Transparent 
// * pixels are blended with a black background. 
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if no error occured and 1 otherwise. 
int decode_bmp(DECODER_t* decoder); 

#endif
