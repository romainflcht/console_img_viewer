#include "decoders/webp_handler.h"


// * Check if the header bytes match the WebP signature. 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a WebP signature and 0 otherwise. 
int is_webp(const unsigned char* header, size_t len)
{
    if (len < WEBP_SIGNATURE_LEN)
        return 0; 

    return !memcmp(header, "RIFF", 4) && !memcmp(header + 8, "WEBP", 4); 
}


// * Read a whole file in memory, libwebp only decodes from memory. 
// * param: filename: path to the file. 
// * param: len: where the size of the file will be stored. 
// * return: the content of the file (that need to be freed) or NULL in case 
// *         of errors. 
static unsigned char* read_file(char* filename, size_t* len)
{
    FILE*           file; 
    unsigned char*  data; 
    long            size; 

    file = fopen(filename, "rb"); 
    if (!file)
        return NULL; 

    if (fseek(file, 0, SEEK_END) || (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET))
    {
        fclose(file); 
        return NULL; 
    }

    data = malloc(size); 
    if (data && fread(data, 1, size, file) != (size_t)size)
    {
        free(data); 
        data = NULL; 
    }

    fclose(file); 
    *len = size; 
    return data; 
}


// * Convert a RGBA buffer to RGB, blending transparent pixels with black. 
// * param: rgba: the RGBA buffer. 
// * param: rgb: the RGB buffer where the result is written. 
// * param: pixel_count: the number of pixels to convert. 
static void rgba_to_rgb(const unsigned char* rgba, unsigned char* rgb, size_t pixel_count)
{
    size_t  i; 
    int     alpha; 

    for (i = 0; i < pixel_count; i++)
    {
        alpha = rgba[i * 4 + 3]; 
        rgb[i * 3 + 0] = (rgba[i * 4 + 0] * alpha + 127) / 255; 
        rgb[i * 3 + 1] = (rgba[i * 4 + 1] * alpha + 127) / 255; 
        rgb[i * 3 + 2] = (rgba[i * 4 + 2] * alpha + 127) / 255; 
    }
}


// * Decode every frame of a WebP image (still or animated) and give them to 
// * the callback. Transparent pixels are blended with a black background. 
// * param: filename: path to the WebP image. 
// * param: callback: function called with each decoded frame. 
// * param: user: pointer given to the callback. 
// * return: 0 if at least one frame was decoded and 1 otherwise. 
int decode_webp(char* filename, FRAME_CALLBACK_t callback, void* user)
{
    WebPData                webp_data; 
    WebPAnimDecoderOptions  options; 
    WebPAnimDecoder*        decoder; 
    WebPAnimInfo            info; 
    WebPBitstreamFeatures   features; 
    unsigned char*          data; 
    unsigned char*          rgba; 
    unsigned char*          rgb; 
    size_t                  len; 
    size_t                  channel_count; 
    int                     timestamp; 
    int                     previous_timestamp; 
    int                     delay_ms; 
    int                     retval; 

    data = read_file(filename, &len); 
    if (!data)
    {
        print_error(filename, "unable to read WebP");
        return 1; 
    }

    if (WebPGetFeatures(data, len, &features) != VP8_STATUS_OK)
    {
        print_error(filename, "unable to decode WebP: invalid header");
        free(data); 
        return 1; 
    }

    channel_count = features.has_alpha ? 4 : 3; 

    // The animation decoder also decodes still images (as a single frame) 
    // and composes the frames of animations for us. 
    webp_data.bytes = data; 
    webp_data.size = len; 
    decoder = NULL; 
    if (WebPAnimDecoderOptionsInit(&options))
    {
        options.color_mode = MODE_RGBA; 
        options.use_threads = 0; 
        decoder = WebPAnimDecoderNew(&webp_data, &options); 
    }

    if (!decoder || !WebPAnimDecoderGetInfo(decoder, &info))
    {
        print_error(filename, "unable to decode WebP");
        WebPAnimDecoderDelete(decoder); 
        free(data); 
        return 1; 
    }

    rgb = malloc((size_t)info.canvas_width * info.canvas_height * 3); 
    if (!rgb)
    {
        WebPAnimDecoderDelete(decoder); 
        free(data); 
        return 1; 
    }

    retval = 1; 
    previous_timestamp = 0; 
    while (WebPAnimDecoderHasMoreFrames(decoder))
    {
        // A corrupted frame stops the animation, the frames before are kept. 
        if (!WebPAnimDecoderGetNext(decoder, &rgba, &timestamp))
        {
            print_error(filename, "unable to decode a frame of WebP");
            break; 
        }

        // The timestamp is the end time of the frame. 
        delay_ms = timestamp - previous_timestamp; 
        previous_timestamp = timestamp; 
        if (delay_ms < WEBP_MIN_DELAY_MS)
            delay_ms = WEBP_DEFAULT_DELAY_MS; 

        rgba_to_rgb(rgba, rgb, (size_t)info.canvas_width * info.canvas_height); 
        if (callback(rgb, info.canvas_width, info.canvas_height, channel_count, delay_ms, user))
        {
            retval = 1; 
            break; 
        }

        retval = 0; 
    }

    free(rgb); 
    WebPAnimDecoderDelete(decoder); 
    free(data); 
    return retval; 
}
