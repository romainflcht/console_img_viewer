#ifndef _JPEG_READ_H_
#define _JPEG_READ_H_

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
JPEG_HANDLER_t* create_jpeg_handler(char* filename);

void free_jpeg_handler(JPEG_HANDLER_t* handler);
#endif