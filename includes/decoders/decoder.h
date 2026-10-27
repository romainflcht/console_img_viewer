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

// What a decoder needs: the encoded image in memory and where to send the 
// decoded frames. 
typedef struct
{
    const char*             name;       // shown in error messages. 
    const unsigned char*    data; 
    size_t                  len; 
    FRAME_CALLBACK_t        on_frame; 
    void*                   user; 
} DECODER_t;

#endif
