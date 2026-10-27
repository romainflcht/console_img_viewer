#ifndef _JPEG_HANDLER_H_
#define _JPEG_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <jpeglib.h>
#include "decoders/exif.h"
#include "image/transform.h"


// __ DEFINIFIONS ______________________________________________________________
#define JPEG_SIGNATURE_LEN 3


// __ TYPES DEFINITIONS ________________________________________________________
// libjpeg calls exit() on fatal errors by default, this error manager jumps 
// back to the decoder instead so the next files can still be displayed. 
typedef struct {
    struct jpeg_error_mgr         pub; 
    jmp_buf                       jump_buffer; 
    char                          message[JMSG_LENGTH_MAX]; 
} JPEG_ERROR_t;

typedef struct {
    FILE*                         file_jpeg; 
    struct jpeg_decompress_struct cinfo; 
    JPEG_ERROR_t                  jpeg_err_handler; 
} JPEG_HANDLER_t;


// __ FUNCTIONS ________________________________________________________________
// * Check if the header bytes match the JPEG signature.
// * param: header: the first bytes of the file.
// * param: len: the number of bytes available in header.
// * return: 1 if the header is a JPEG signature and 0 otherwise.
int is_jpeg(const unsigned char* header, size_t len);

// * Create a JPEG_HANDLER_t structure. 
// * param: filename: path to the the JPEG image. 
// * return: the newly created JPEG handler or NULL in case of errors.  
JPEG_HANDLER_t* create_jpeg_handler(char* filename);

// * Free the JPEG_HANDLER_t structure correctly.
// * param: handler: the JPEG handler that need to be freed. 
void free_jpeg_handler(JPEG_HANDLER_t* handler);

// * Read the EXIF orientation of a JPEG image (APP1 marker). 
// * param: handler: the JPEG handler, its header must already be read. 
// * return: the orientation [1-8], ORIENTATION_NORMAL if there is none. 
int get_jpeg_orientation(JPEG_HANDLER_t* handler); 

// * Decode a JPEG image into a RGB buffer (3 bytes per pixel) rotated 
// * following its EXIF orientation.
// * param: filename: path to the JPEG image.
// * param: width: where the image width will be stored.
// * param: height: where the image height will be stored.
// * param: channel_count: where the original channel count will be stored.
// * return: the RGB buffer (that need to be freed) or NULL in case of errors.
unsigned char* decode_jpeg(char* filename, size_t* width, size_t* height, size_t* channel_count);

#endif
