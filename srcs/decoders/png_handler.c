#include "decoders/png_handler.h"


// * Check if the header bytes match the PNG signature. 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a PNG signature and 0 otherwise. 
int is_png(const unsigned char* header, size_t len)
{
    if (len < PNG_SIGNATURE_LEN)
        return 0; 

    return !png_sig_cmp(header, 0, PNG_SIGNATURE_LEN); 
}


// * Decode a PNG image into RGB and give it to the decoder callback. 
// * Transparent pixels are blended with a black background. 
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if no error occured and 1 otherwise. 
int decode_png(DECODER_t* decoder)
{
    png_image       image; 
    png_color       background; 
    unsigned char*  buffer; 
    size_t          channel_count; 
    int             retval; 

    memset(&image, 0, sizeof(png_image)); 
    image.version = PNG_IMAGE_VERSION; 

    if (!png_image_begin_read_from_memory(&image, decoder->data, decoder->len))
    {
        print_error(decoder->name, "unable to read PNG: %s", image.message); 
        return 1; 
    }

    channel_count = PNG_IMAGE_SAMPLE_CHANNELS(image.format); 

    // Ask libpng to convert every PNG flavour (palette, grayscale, 16 bits, 
    // alpha...) to plain 8 bits RGB. 
    image.format = PNG_FORMAT_RGB; 

    buffer = malloc(PNG_IMAGE_SIZE(image)); 
    if (!buffer)
    {
        png_image_free(&image); 
        return 1; 
    }

    // Transparent pixels are blended with this background color. 
    background.red = 0; 
    background.green = 0; 
    background.blue = 0; 

    if (!png_image_finish_read(&image, &background, buffer, 0, NULL))
    {
        print_error(decoder->name, "unable to decode PNG: %s", image.message); 
        png_image_free(&image); 
        free(buffer); 
        return 1; 
    }

    retval = decoder->on_frame(buffer, image.width, image.height, channel_count, 0, decoder->user); 
    free(buffer); 
    return retval; 
}
