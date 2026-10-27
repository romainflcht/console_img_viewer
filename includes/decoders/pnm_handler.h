#ifndef _PNM_HANDLER_H_
#define _PNM_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "decoders/decoder.h"
#include "utils/utils.h"


// __ DEFINIFIONS ______________________________________________________________
// "P1" to "P6" followed by a whitespace. 
#define PNM_SIGNATURE_LEN 3

// Biggest accepted width or height, protects against absurd headers. 
#define PNM_MAX_SIZE 65535


// __ TYPES DEFINITIONS ________________________________________________________
// Read position inside the encoded image. 
typedef struct
{
    const unsigned char*    data; 
    size_t                  len; 
    size_t                  pos; 
} PNM_READER_t;


// __ FUNCTIONS ________________________________________________________________
// * Check if the header bytes match a netpbm signature (P1 to P6). 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a netpbm signature and 0 otherwise. 
int is_pnm(const unsigned char* header, size_t len); 

// * Decode a netpbm image: PBM (P1, P4), PGM (P2, P5) or PPM (P3, P6), in 
// * ASCII or binary, 8 or 16 bits, and give it to the decoder callback. 
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if no error occured and 1 otherwise. 
int decode_pnm(DECODER_t* decoder); 

#endif
