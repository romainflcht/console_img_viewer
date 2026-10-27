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
// * keep the right speed. Ctrl-C stops the animation.
// * param: frames: frame_count pixel matrices of width * rows cells.
// * param: delays: display time of each frame in milliseconds.
// * param: frame_count: number of frames.
// * param: width: number of cells per row.
// * param: rows: number of rows of cells.
// * param: offset: number of empty columns before the image (centering). 
// * param: loop: 1 to play the animation until Ctrl-C, 0 to play it once. 
// * return: 1 if the animation was stopped with Ctrl-C and 0 otherwise. 
int play_animation(PIXEL_t* frames, int* delays, size_t frame_count, size_t width, size_t rows, size_t offset, int loop)
{
    struct sigaction    action;
    struct sigaction    old_action;
    sigset_t            sigint_set;
    struct timespec     deadline;
    struct timespec     frame_end;
    struct timespec     now;
    PIXEL_t*            frame;
    PIXEL_t*            previous;
    size_t              cell_count;
    size_t              i;
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

    cell_count = width * rows;
    previous = NULL;
    error = 0;
    clock_gettime(CLOCK_MONOTONIC, &deadline);

    do
    {
        for (i = 0; i < frame_count && !interrupted && !error; i++)
        {
            frame = frames + i * cell_count;

            // deadline is when this frame should appear. If the time to show
            // it is already over, skip it (except the last one, so the
            // animation always ends on the right image).
            frame_end = deadline;
            timespec_add_ms(&frame_end, delays[i]);
            clock_gettime(CLOCK_MONOTONIC, &now);

            if (previous && i + 1 < frame_count && timespec_after(&now, &frame_end))
            {
                deadline = frame_end;
                continue;
            }

            // Ctrl-C is delayed while a frame is written so the frame is
            // never cut and the cursor always ends under the image.
            sigprocmask(SIG_BLOCK, &sigint_set, NULL);
            error = draw_pixels(frame, previous, width, rows, offset); 
            sigprocmask(SIG_UNBLOCK, &sigint_set, NULL);
            if (error)
                break;

            previous = frame;
            deadline = frame_end;

            // Wait until the next frame, Ctrl-C interrupts the wait.
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL);
        }
    } while (loop && !interrupted && !error);

    printf(ESC_SHOW_CURSOR);
    fflush(stdout);
    sigaction(SIGINT, &old_action, NULL);
    return interrupted;
}
