#ifndef _FRAME_H_
#define _FRAME_H_

#include <stdlib.h>


// __ TYPES DEFINITIONS ________________________________________________________
// * Called by animated image decoders for each decoded frame.
// * param: rgb: the full frame in RGB (3 bytes per pixel), only valid during
// *        the call.
// * param: width: width of the frame.
// * param: height: height of the frame.
// * param: channel_count: channel count of the original image.
// * param: delay_ms: how long the frame is displayed, in milliseconds.
// * param: user: the pointer given to the decoder.
// * return: 0 to continue decoding and 1 to stop (error).
typedef int (*FRAME_CALLBACK_t)(unsigned char* rgb, size_t width, size_t height, size_t channel_count, int delay_ms, void* user);

#endif
