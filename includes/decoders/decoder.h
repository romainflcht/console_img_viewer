#ifndef _DECODER_H_
#define _DECODER_H_

#include <stdlib.h>


// __ TYPES DEFINITIONS ________________________________________________________
// * Called by the decoders for each decoded frame (once for still images). 
// * param: rgb: the full frame in RGB (3 bytes per pixel), only valid during 
// *        the call. 
// * param: width: width of the frame. 
// * param: height: height of the frame. 
// * param: channel_count: channel count of the original image. 
// * param: delay_ms: how long the frame is displayed, in milliseconds (0 for 
// *        still images). 
// * param: user: the user pointer of the decoder. 
// * return: 0 to continue decoding and 1 to stop (error). 
typedef int (*FRAME_CALLBACK_t)(unsigned char* rgb, size_t width, size_t height, size_t channel_count, int delay_ms, void* user); 

// * Called by the decoders once the size of the image is known, before the 
// * pixels are decoded. Decoders that can decode at a smaller size (JPEG) use 
// * the answer to decode only what is needed. 
// * param: width: width of the image. 
// * param: height: height of the image. 
// * param: channel_count: channel count of the image. 
// * param: target_width: where the displayed width will be stored. 
// * param: target_height: where the displayed height will be stored. 
// * param: user: the user pointer of the decoder. 
typedef void (*SIZE_CALLBACK_t)(size_t width, size_t height, size_t channel_count, size_t* target_width, size_t* target_height, void* user); 

// What a decoder needs: the encoded image in memory and where to send the 
// decoded frames.
typedef struct
{
    const char*             name;       // shown in error messages. 
    const unsigned char*    data; 
    size_t                  len; 
    SIZE_CALLBACK_t         on_size; 
    FRAME_CALLBACK_t        on_frame; 
    void*                   user; 
    int                     loop_count; // out: times the animation is played 
                                        // (0 forever), -1 if not stored. 
    int                     orientation;// out: EXIF orientation [1-8]. 
} DECODER_t;

#endif
