#ifndef _JPEG_HANDLER_H_
#define _JPEG_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>


typedef struct {
    FILE*                         file_jpeg; 
    struct jpeg_decompress_struct cinfo; 
    struct jpeg_error_mgr         jpeg_err_handler; 
    unsigned char*                pixel_scan_row;
} JPEG_HANDLER_t;

// __ FUNCTIONS ________________________________________________________________
// * Create a JPEG_HANDLER_t structure. 
// * param: filename: path to the the JPEG image. 
// * return: the newly created JPEG handler or NULL in case of errors.  
JPEG_HANDLER_t* create_jpeg_handler(char* filename);

// * Free the JPEG_HANDLER_t structure correctly.
// * param: handler: the JPEG handler that need to be freed. 
void free_jpeg_handler(JPEG_HANDLER_t* handler);

#endif