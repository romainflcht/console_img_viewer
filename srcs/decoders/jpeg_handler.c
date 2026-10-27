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


// * Replace libjpeg default output_message: print the warnings (corrupted 
// * data...) like the other errors of the program. 
// * param: cinfo: the libjpeg structure that raised the warning. 
static void jpeg_output_message(j_common_ptr cinfo)
{
    JPEG_ERROR_t*   err; 
    char            message[JMSG_LENGTH_MAX]; 

    err = (JPEG_ERROR_t*)cinfo->err; 
    (*cinfo->err->format_message)(cinfo, message); 
    print_error(err->name, "warning: %s", message); 
}


// * Create a JPEG_HANDLER_t structure and read the JPEG header.
// * param: decoder: the encoded image.
// * return: the newly created JPEG handler or NULL in case of errors.
JPEG_HANDLER_t* create_jpeg_handler(DECODER_t* decoder)
{
    JPEG_HANDLER_t* new_handler;

    new_handler = malloc(sizeof(JPEG_HANDLER_t));
    if (!new_handler)
        return NULL;

    new_handler->cinfo.err = jpeg_std_error(&new_handler->jpeg_err_handler.pub);
    new_handler->jpeg_err_handler.pub.error_exit = jpeg_error_exit; 
    new_handler->jpeg_err_handler.pub.output_message = jpeg_output_message; 
    new_handler->jpeg_err_handler.name = decoder->name; 

    // Initialize cinfo with the data in memory, decompress and header init.
    jpeg_create_decompress(&(new_handler->cinfo));
    jpeg_mem_src(&(new_handler->cinfo), decoder->data, decoder->len);

    // Keep the APP1 markers in memory to read the EXIF orientation.
    jpeg_save_markers(&(new_handler->cinfo), JPEG_APP0 + 1, 0xFFFF);

    // libjpeg jumps back here if the header is corrupted.
    if (setjmp(new_handler->jpeg_err_handler.jump_buffer))
    {
        print_error(decoder->name, "unable to read JPEG: %s", new_handler->jpeg_err_handler.message);
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
    free(handler);
}


// * Read the EXIF orientation of a JPEG image (APP1 marker).
// * param: handler: the JPEG handler, its header must already be read.
// * return: the orientation [1-8], ORIENTATION_NORMAL if there is none.
int get_jpeg_orientation(JPEG_HANDLER_t* handler)
{
    jpeg_saved_marker_ptr marker;

    for (marker = handler->cinfo.marker_list; marker; marker = marker->next)
    {
        if (marker->marker != JPEG_APP0 + 1)
            continue;

        if (marker->data_length < EXIF_HEADER_LEN || memcmp(marker->data, EXIF_HEADER, EXIF_HEADER_LEN))
            continue;

        return exif_get_orientation(marker->data, marker->data_length);
    }

    return ORIENTATION_NORMAL;
}


// * Choose the smallest DCT scaling (M/8) that still gives at least the 
// * displayed size, so big photos are decoded faster and with less memory. 
// * param: handler: the JPEG handler, its header must already be read. 
// * param: decoder: the decoder, its size callback gives the displayed size. 
// * param: orientation: the EXIF orientation of the image. 
static void set_jpeg_scale(JPEG_HANDLER_t* handler, DECODER_t* decoder, int orientation)
{
    size_t  width; 
    size_t  height; 
    size_t  target_width; 
    size_t  target_height; 
    size_t  scale_w; 
    size_t  scale_h; 

    width = handler->cinfo.image_width; 
    height = handler->cinfo.image_height; 

    // The displayed size is computed on the rotated image. 
    if (orientation >= ORIENTATION_TRANSPOSE)
        decoder->on_size(height, width, handler->cinfo.num_components, &target_height, &target_width, decoder->user); 

    else
        decoder->on_size(width, height, handler->cinfo.num_components, &target_width, &target_height, decoder->user); 

    // M = ceil(8 * target / size), between 1 and 8. 
    scale_w = (target_width * JPEG_SCALE_DENOM + width - 1) / width; 
    scale_h = (target_height * JPEG_SCALE_DENOM + height - 1) / height; 
    handler->cinfo.scale_num = scale_w > scale_h ? scale_w : scale_h; 
    if (handler->cinfo.scale_num < 1)
        handler->cinfo.scale_num = 1; 

    if (handler->cinfo.scale_num > JPEG_SCALE_DENOM)
        handler->cinfo.scale_num = JPEG_SCALE_DENOM; 

    handler->cinfo.scale_denom = JPEG_SCALE_DENOM; 
}


// * Decode a JPEG image rotated following its EXIF orientation and give it 
// * to the decoder callback. The image is decoded at the smallest size that 
// * is still bigger than its displayed size.
// * param: decoder: the encoded image and the frame callback.
// * return: 0 if no error occured and 1 otherwise.
int decode_jpeg(DECODER_t* decoder)
{
    JPEG_HANDLER_t* handler;
    unsigned char*  buffer;
    unsigned char*  row;
    size_t          width;
    size_t          height;
    size_t          channel_count;
    size_t          row_len;
    int             orientation;
    int             retval;

    handler = create_jpeg_handler(decoder);
    if (!handler)
        return 1;

    channel_count = handler->cinfo.num_components;
    orientation = get_jpeg_orientation(handler); 

    // Ask libjpeg to convert grayscale images to RGB.
    handler->cinfo.out_color_space = JCS_RGB;

    // libjpeg jumps back here if the output size can't be computed. 
    if (setjmp(handler->jpeg_err_handler.jump_buffer))
    {
        print_error(decoder->name, "unable to decode JPEG: %s", handler->jpeg_err_handler.message); 
        free_jpeg_handler(handler);
        return 1; 
    }

    if (decoder->on_size)
        set_jpeg_scale(handler, decoder, orientation); 

    jpeg_calc_output_dimensions(&(handler->cinfo)); 
    width = handler->cinfo.output_width; 
    height = handler->cinfo.output_height; 

    row_len = width * 3;
    buffer = malloc(sizeof(unsigned char) * row_len * height);
    if (!buffer)
    {
        free_jpeg_handler(handler);
        return 1;
    }

    // libjpeg jumps back here if the image data is corrupted or if the color
    // space can't be converted to RGB.
    if (setjmp(handler->jpeg_err_handler.jump_buffer))
    {
        print_error(decoder->name, "unable to decode JPEG: %s", handler->jpeg_err_handler.message);
        free(buffer);
        free_jpeg_handler(handler);
        return 1;
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

    // Photos taken with a rotated camera need to be turned to be displayed
    // the right way up.
    buffer = apply_orientation(buffer, &width, &height, orientation);
    if (!buffer)
        return 1;

    retval = decoder->on_frame(buffer, width, height, channel_count, 0, decoder->user);
    free(buffer);
    return retval;
}
