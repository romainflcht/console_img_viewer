#include "jpeg_read.h"


JPEG_HANDLER_t* create_jpeg_handler(char* filename)
{
    FILE*           jpeg_file; 
    JPEG_HANDLER_t* new_handler; 

    jpeg_file = fopen(filename, "rb"); 
    if (!jpeg_file)
        return NULL; 

    new_handler = malloc(sizeof(JPEG_HANDLER_t));
    if (!new_handler)
        return NULL; 

    new_handler->cinfo.err = jpeg_std_error(&new_handler->jpeg_err_handler); 

    // Initialize cinfo with a file, decompress and header init.
    jpeg_create_decompress(&(new_handler->cinfo)); 
    jpeg_stdio_src(&(new_handler->cinfo), jpeg_file);
    // fclose(jpeg_file); 

    if (jpeg_read_header(&(new_handler->cinfo), 1) != JPEG_HEADER_OK) 
    {
        jpeg_destroy_decompress(&(new_handler->cinfo)); 
        free(new_handler);
        return NULL; 
    }
    
    return new_handler; 
}


void free_jpeg_handler(JPEG_HANDLER_t* handler)
{
    if (!handler)
        return; 

    jpeg_destroy_decompress(&(handler->cinfo));
}
