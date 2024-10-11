#include "jpeg_read.h"


// * Read the JPEG header to fetch, width, height and channel number. 
// * param: jpeg_file: the pointer to the file that we need information about. 
// * param: height: pointer to where the height of the image will be stored. 
// * param: width: pointer to where the width of the image will be stored. 
// * param: channel_count: pointer to where the color channel count of the  
// *                       image will be stored. 
void read_header_jpeg(FILE* jpeg_file, size_t* height, size_t* width, size_t* channel_count)
{
    struct jpeg_decompress_struct jpeg; 
    struct jpeg_error_mgr         jpeg_err_handler; 

    // Link error manager strut to the JPEG decompress struct. 
    jpeg.err = jpeg_std_error(&jpeg_err_handler); 
    jpeg_create_decompress(&jpeg); 

    // Add the JPEG file to the JPEG decompress struct. 
    jpeg_stdio_src(&jpeg, jpeg_file); 
    jpeg_read_header(&jpeg, 1);

    // Get every informations about the images needed. 
    *width = jpeg.image_width;
    *height = jpeg.image_height;
    *channel_count = jpeg.num_components;

    // Free the memory. 
    jpeg_destroy_decompress(&jpeg);
}
