#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "display/pixel.h"
#include "display/renderer.h"


// __ DEFINIFIONS ______________________________________________________________
#define ESC_HIDE_CURSOR "\x1b[?25l"
#define ESC_SHOW_CURSOR "\x1b[?25h"


// __ FUNCTIONS ________________________________________________________________
// * Play an animation in place in the terminal. Late frames are skipped to
// * keep the right speed. Ctrl-C stops the animation.
// * param: frames: frame_count pixel matrices of render->width * rows cells. 
// * param: delays: display time of each frame in milliseconds.
// * param: frame_count: number of frames.
// * param: render: size, position and color mode of the frames. 
// * param: loop: 1 to play the animation until Ctrl-C, 0 to play it once. 
// * return: 1 if the animation was stopped with Ctrl-C and 0 otherwise. 
int play_animation(PIXEL_t* frames, int* delays, size_t frame_count, const RENDER_t* render, int loop); 

#endif
