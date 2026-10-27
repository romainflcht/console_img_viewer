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


// * Decode a PNG image into a RGB buffer (3 bytes per pixel). Transparent
// * pixels are blended with a black background.
// * param: filename: path to the PNG image.
// * param: width: where the image width will be stored.
// * param: height: where the image height will be stored.
// * param: channel_count: where the original channel count will be stored.
// * return: the RGB buffer (that need to be freed) or NULL in case of errors.
unsigned char* decode_png(char* filename, size_t* width, size_t* height, size_t* channel_count)
{
    png_image       image;
    png_color       background;
    unsigned char*  buffer;

    memset(&image, 0, sizeof(png_image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, filename))
    {
        print_error(filename, "unable to read PNG: %s", image.message);
        return NULL;
    }

    *width = image.width;
    *height = image.height;
    *channel_count = PNG_IMAGE_SAMPLE_CHANNELS(image.format);

    // Ask libpng to convert every PNG flavour (palette, grayscale, 16 bits,
    // alpha...) to plain 8 bits RGB.
    image.format = PNG_FORMAT_RGB;

    buffer = malloc(PNG_IMAGE_SIZE(image));
    if (!buffer)
    {
        png_image_free(&image);
        return NULL;
    }

    // Transparent pixels are blended with this background color.
    background.red = 0;
    background.green = 0;
    background.blue = 0;

    if (!png_image_finish_read(&image, &background, buffer, 0, NULL))
    {
        print_error(filename, "unable to decode PNG: %s", image.message);
        png_image_free(&image);
        free(buffer);
        return NULL;
    }

    return buffer;
}
