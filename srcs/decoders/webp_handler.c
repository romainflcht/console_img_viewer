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
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if at least one frame was decoded and 1 otherwise. 
int decode_webp(DECODER_t* decoder)
{
    WebPData                webp_data;
    WebPAnimDecoderOptions  options;
    WebPAnimDecoder*        anim_decoder;
    WebPAnimInfo            info; 
    WebPBitstreamFeatures   features; 
    unsigned char*          rgba;
    unsigned char*          rgb; 
    size_t                  channel_count;
    int                     timestamp; 
    int                     previous_timestamp; 
    int                     delay_ms; 
    int                     retval; 

    if (WebPGetFeatures(decoder->data, decoder->len, &features) != VP8_STATUS_OK)
    {
        print_error(decoder->name, "unable to decode WebP: invalid header");
        return 1;
    }

    channel_count = features.has_alpha ? 4 : 3; 

    // The animation decoder also decodes still images (as a single frame) 
    // and composes the frames of animations for us. 
    webp_data.bytes = decoder->data;
    webp_data.size = decoder->len;
    anim_decoder = NULL;
    if (WebPAnimDecoderOptionsInit(&options))
    {
        options.color_mode = MODE_RGBA; 
        options.use_threads = 0; 
        anim_decoder = WebPAnimDecoderNew(&webp_data, &options); 
    }

    if (!anim_decoder || !WebPAnimDecoderGetInfo(anim_decoder, &info))
    {
        print_error(decoder->name, "unable to decode WebP");
        WebPAnimDecoderDelete(anim_decoder); 
        return 1; 
    }

    rgb = malloc((size_t)info.canvas_width * info.canvas_height * 3); 
    if (!rgb)
    {
        WebPAnimDecoderDelete(anim_decoder); 
        return 1; 
    }

    retval = 1; 
    previous_timestamp = 0; 
    while (WebPAnimDecoderHasMoreFrames(anim_decoder))
    {
        // A corrupted frame stops the animation, the frames before are kept. 
        if (!WebPAnimDecoderGetNext(anim_decoder, &rgba, &timestamp))
        {
            print_error(decoder->name, "unable to decode a frame of WebP");
            break; 
        }

        // The timestamp is the end time of the frame. 
        delay_ms = timestamp - previous_timestamp; 
        previous_timestamp = timestamp; 
        if (delay_ms < WEBP_MIN_DELAY_MS)
            delay_ms = WEBP_DEFAULT_DELAY_MS; 

        rgba_to_rgb(rgba, rgb, (size_t)info.canvas_width * info.canvas_height); 
        if (decoder->on_frame(rgb, info.canvas_width, info.canvas_height, channel_count, delay_ms, decoder->user))
        {
            retval = 1; 
            break; 
        }

        retval = 0; 
    }

    free(rgb); 
    WebPAnimDecoderDelete(anim_decoder); 
    return retval; 
}
