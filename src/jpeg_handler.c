#include "jpeg_handler.h"


// * Create a JPEG_HANDLER_t structure. 
// * param: filename: path to the the JPEG image. 
// * return: the newly created JPEG handler or NULL in case of errors.  
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
    new_handler->file_jpeg = jpeg_file;
    new_handler->pixel_scan_row = NULL;  

    // Initialize cinfo with a file, decompress and header init.
    jpeg_create_decompress(&(new_handler->cinfo)); 
    jpeg_stdio_src(&(new_handler->cinfo), new_handler->file_jpeg);

    // Check if the JPEG file is in the correct format. 
    if (jpeg_read_header(&(new_handler->cinfo), 1) != JPEG_HEADER_OK) 
    {
        jpeg_destroy_decompress(&(new_handler->cinfo)); 
        fclose(new_handler->file_jpeg); 
        free(new_handler);
        return NULL; 
    }
    
    return new_handler; 
}


// * Free the JPEG_HANDLER_t structure correctly.
// * param: handler: the JPEG handler that need to be freed. 
void free_jpeg_handler(JPEG_HANDLER_t* handler)
{
    if (!handler)
        return; 

    jpeg_destroy_decompress(&(handler->cinfo));
   
    if (handler->pixel_scan_row)
    {
        free(handler->pixel_scan_row);
        handler->pixel_scan_row = NULL; 
    }

    if (handler->file_jpeg)
    { 
        fclose(handler->file_jpeg);
        handler->file_jpeg = NULL; 
    }

    free(handler); 
}
