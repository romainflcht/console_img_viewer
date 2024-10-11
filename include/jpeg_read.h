#ifndef _JPEG_READ_H_
#define _JPEG_READ_H_

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

// __ FUNCTIONS ________________________________________________________________
// * Read the JPEG header to fetch, width, height and channel number. 
// * param: jpeg_file: the pointer to the file that we need information about. 
// * param: height: pointer to where the height of the image will be stored. 
// * param: width: pointer to where the width of the image will be stored. 
// * param: channel_count: pointer to where the color channel count of the  
// *                       image will be stored. 
void read_header_jpeg(FILE* jpeg_file, size_t* height, size_t* width, size_t* channel_count); 

#endif