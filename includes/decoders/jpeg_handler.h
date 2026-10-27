#ifndef _JPEG_HANDLER_H_
#define _JPEG_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>
#include "decoders/decoder.h"
#include "decoders/exif.h"
#include "image/transform.h"
#include "utils/utils.h"


// __ DEFINIFIONS ______________________________________________________________
// libjpeg can decode at M/8 of the image size (DCT scaling). 
#define JPEG_SCALE_DENOM 8


// __ TYPES DEFINITIONS ________________________________________________________
// libjpeg calls exit() on fatal errors by default, this error manager jumps
// back to the decoder instead so the next files can still be displayed.
typedef struct {
    struct jpeg_error_mgr         pub;
    jmp_buf                       jump_buffer;
    char                          message[JMSG_LENGTH_MAX]; 
    const char*                   name;     // file name for the warnings. 
} JPEG_ERROR_t;

typedef struct {
    struct jpeg_decompress_struct cinfo;
    JPEG_ERROR_t                  jpeg_err_handler;
} JPEG_HANDLER_t;


// __ FUNCTIONS ________________________________________________________________
// * Create a JPEG_HANDLER_t structure and read the JPEG header.
// * param: decoder: the encoded image.
// * return: the newly created JPEG handler or NULL in case of errors.
JPEG_HANDLER_t* create_jpeg_handler(DECODER_t* decoder);

// * Free the JPEG_HANDLER_t structure correctly.
// * param: handler: the JPEG handler that need to be freed.
void free_jpeg_handler(JPEG_HANDLER_t* handler);

// * Read the EXIF orientation of a JPEG image (APP1 marker).
// * param: handler: the JPEG handler, its header must already be read.
// * return: the orientation [1-8], ORIENTATION_NORMAL if there is none.
int get_jpeg_orientation(JPEG_HANDLER_t* handler);

// * Decode a JPEG image rotated following its EXIF orientation and give it
// * to the decoder callback.
// * param: decoder: the encoded image and the frame callback.
// * return: 0 if no error occured and 1 otherwise.
int decode_jpeg(DECODER_t* decoder);

#endif
