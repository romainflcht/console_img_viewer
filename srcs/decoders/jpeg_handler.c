#include "decoders/jpeg_handler.h"


// * Check if the header bytes match the JPEG signature.
// * param: header: the first bytes of the file.
// * param: len: the number of bytes available in header.
// * return: 1 if the header is a JPEG signature and 0 otherwise.
int is_jpeg(const unsigned char* header, size_t len)
{
    if (len < JPEG_SIGNATURE_LEN)
        return 0;

    return header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF;
}


// * Replace libjpeg default error_exit: save the error message and jump back 
// * to the last setjmp() instead of exiting the program. 
// * param: cinfo: the libjpeg structure that raised the error. 
static void jpeg_error_exit(j_common_ptr cinfo)
{
    JPEG_ERROR_t* err; 

    err = (JPEG_ERROR_t*)cinfo->err; 
    (*cinfo->err->format_message)(cinfo, err->message); 
    longjmp(err->jump_buffer, 1); 
}


// * Create a JPEG_HANDLER_t structure. 
// * param: filename: path to the the JPEG image. 
// * return: the newly created JPEG handler or NULL in case of errors.  
JPEG_HANDLER_t* create_jpeg_handler(char* filename)
{
    FILE*           jpeg_file; 
    JPEG_HANDLER_t* new_handler; 

    jpeg_file = fopen(filename, "rb"); 
    if (!jpeg_file)
    {
        printf("File \"%s\" not found, please check your path.\n", filename);
        return NULL; 
    }

    new_handler = malloc(sizeof(JPEG_HANDLER_t));
    if (!new_handler)
    {
        fclose(jpeg_file);
        return NULL; 
    }

    new_handler->cinfo.err = jpeg_std_error(&new_handler->jpeg_err_handler.pub); 
    new_handler->jpeg_err_handler.pub.error_exit = jpeg_error_exit; 
    new_handler->file_jpeg = jpeg_file;

    // Initialize cinfo with a file, decompress and header init.
    jpeg_create_decompress(&(new_handler->cinfo)); 
    jpeg_stdio_src(&(new_handler->cinfo), new_handler->file_jpeg);

    // libjpeg jumps back here if the header is corrupted. 
    if (setjmp(new_handler->jpeg_err_handler.jump_buffer))
    {
        printf("Unable to read JPEG \"%s\": %s\n", filename, new_handler->jpeg_err_handler.message); 
        free_jpeg_handler(new_handler); 
        return NULL; 
    }

    // Check if the JPEG file is in the correct format. 
    if (jpeg_read_header(&(new_handler->cinfo), 1) != JPEG_HEADER_OK) 
    {
        free_jpeg_handler(new_handler); 
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

    if (handler->file_jpeg)
    {
        fclose(handler->file_jpeg);
        handler->file_jpeg = NULL; 
    }

    free(handler); 
}


// * Decode a JPEG image into a RGB buffer (3 bytes per pixel).
// * param: filename: path to the JPEG image.
// * param: width: where the image width will be stored.
// * param: height: where the image height will be stored.
// * param: channel_count: where the original channel count will be stored.
// * return: the RGB buffer (that need to be freed) or NULL in case of errors.
unsigned char* decode_jpeg(char* filename, size_t* width, size_t* height, size_t* channel_count)
{
    JPEG_HANDLER_t* handler;
    unsigned char*  buffer;
    unsigned char*  row;
    size_t          row_len;

    handler = create_jpeg_handler(filename);
    if (!handler)
        return NULL; 

    *width = handler->cinfo.image_width;
    *height = handler->cinfo.image_height;
    *channel_count = handler->cinfo.num_components;

    // Ask libjpeg to convert grayscale images to RGB.
    handler->cinfo.out_color_space = JCS_RGB;

    row_len = *width * 3;
    buffer = malloc(sizeof(unsigned char) * row_len * *height);
    if (!buffer)
    {
        free_jpeg_handler(handler);
        return NULL; 
    }

    // libjpeg jumps back here if the image data is corrupted or if the color 
    // space can't be converted to RGB. 
    if (setjmp(handler->jpeg_err_handler.jump_buffer))
    {
        printf("Unable to decode JPEG \"%s\": %s\n", filename, handler->jpeg_err_handler.message); 
        free(buffer);
        free_jpeg_handler(handler);
        return NULL; 
    }

    jpeg_start_decompress(&(handler->cinfo));

    while (handler->cinfo.output_scanline < handler->cinfo.output_height)
    {
        // Read one row of pixels directly at its place inside the buffer.
        row = buffer + handler->cinfo.output_scanline * row_len;
        jpeg_read_scanlines(&(handler->cinfo), &row, 1);
    }

    // Free the decompression.
    jpeg_finish_decompress(&(handler->cinfo));
    free_jpeg_handler(handler);
    return buffer;
}
