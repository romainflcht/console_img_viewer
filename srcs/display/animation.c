#include "display/animation.h"


// Set by the SIGINT handler to stop the animation.
static volatile sig_atomic_t interrupted = 0;


// * SIGINT handler: ask the animation to stop.
// * param: sig: the signal number (unused).
static void on_sigint(int sig)
{
    (void)sig;
    interrupted = 1;
}


// * Add milliseconds to a timespec.
// * param: ts: the timespec to modify.
// * param: ms: the milliseconds to add.
static void timespec_add_ms(struct timespec* ts, int ms)
{
    ts->tv_sec += ms / 1000;
    ts->tv_nsec += (long)(ms % 1000) * 1000000;

    if (ts->tv_nsec >= 1000000000)
    {
        ts->tv_sec++;
        ts->tv_nsec -= 1000000000;
    }
}


// * Compare two timespec.
// * param: a: the first timespec.
// * param: b: the second timespec.
// * return: 1 if a is after b and 0 otherwise.
static int timespec_after(struct timespec* a, struct timespec* b)
{
    if (a->tv_sec != b->tv_sec)
        return a->tv_sec > b->tv_sec;

    return a->tv_nsec > b->tv_nsec;
}


// * Play an animation in place in the terminal. Late frames are skipped to 
// * keep the right speed, frames are also skipped to respect the maximum 
// * frame rate. Ctrl-C stops the animation. 
// * param: frames: frame_count pixel matrices of render->width * rows cells. 
// * param: delays: display time of each frame in milliseconds. 
// * param: frame_count: number of frames. 
// * param: render: size, position and color mode of the frames. 
// * param: plays: how many times the animation is played, 0 until Ctrl-C. 
// * param: max_fps: maximum number of frames drawn per second, 0 for no 
// *        limit. 
// * return: 1 if the animation was stopped with Ctrl-C and 0 otherwise. 
int play_animation(PIXEL_t* frames, int* delays, size_t frame_count, const RENDER_t* render, int plays, int max_fps)
{
    struct sigaction    action; 
    struct sigaction    old_action; 
    sigset_t            sigint_set; 
    struct timespec     deadline; 
    struct timespec     frame_end; 
    struct timespec     next_allowed; 
    struct timespec     now; 
    PIXEL_t*            frame; 
    PIXEL_t*            previous; 
    size_t              cell_count; 
    size_t              i; 
    int                 played; 
    int                 last; 
    int                 error; 

    // No SA_RESTART so Ctrl-C also wakes up clock_nanosleep(). 
    interrupted = 0; 
    action.sa_handler = on_sigint; 
    action.sa_flags = 0; 
    sigemptyset(&action.sa_mask); 
    sigaction(SIGINT, &action, &old_action); 

    sigemptyset(&sigint_set); 
    sigaddset(&sigint_set, SIGINT); 

    printf(ESC_HIDE_CURSOR); 

    cell_count = render->width * render->rows; 
    previous = NULL; 
    error = 0; 
    played = 0; 
    clock_gettime(CLOCK_MONOTONIC, &deadline); 
    next_allowed = deadline; 

    do
    {
        for (i = 0; i < frame_count && !interrupted && !error; i++)
        {
            frame = frames + i * cell_count; 
            last = i + 1 == frame_count && plays && played + 1 == plays; 

            // deadline is when this frame should appear and frame_end when 
            // the next one replaces it. A frame is skipped if its time is 
            // already over, or if it would be replaced before the frame rate 
            // allows to draw it. The very last frame is always drawn so the 
            // animation ends on the right image. 
            frame_end = deadline; 
            timespec_add_ms(&frame_end, delays[i]); 
            clock_gettime(CLOCK_MONOTONIC, &now); 

            if (previous && !last 
                && (timespec_after(&now, &frame_end) || !timespec_after(&frame_end, &next_allowed)))
            {
                deadline = frame_end; 
                continue; 
            }

            // Wait for the frame rate to allow a new frame. 
            if (timespec_after(&next_allowed, &now))
                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_allowed, NULL); 

            if (interrupted)
                break; 

            // Ctrl-C is delayed while a frame is written so the frame is 
            // never cut and the cursor always ends under the image. 
            sigprocmask(SIG_BLOCK, &sigint_set, NULL); 
            error = draw_pixels(frame, previous, render); 
            sigprocmask(SIG_UNBLOCK, &sigint_set, NULL); 
            if (error)
                break; 

            previous = frame; 
            if (max_fps)
            {
                clock_gettime(CLOCK_MONOTONIC, &next_allowed); 
                timespec_add_ms(&next_allowed, 1000 / max_fps); 
            }

            deadline = frame_end; 

            // Wait until the next frame, Ctrl-C interrupts the wait. 
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL); 
        }

        played++; 
    } while ((!plays || played < plays) && !interrupted && !error); 

    printf(ESC_SHOW_CURSOR); 
    fflush(stdout); 
    sigaction(SIGINT, &old_action, NULL); 
    return interrupted; 
}
