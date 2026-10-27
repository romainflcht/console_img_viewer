#include "display/terminal.h"


// * Get the size of the terminal the program is running in. 
// * param: term: where the number of columns and rows will be stored, with 
// *        the size of a character cell in pixels (0 if the terminal doesn't 
// *        report it). 
// * return: 0 if the size was found and 1 otherwise (no terminal attached). 
int get_terminal_size(TERMINAL_t* term)
{
    struct winsize  ws;
    int             fds[] = {STDOUT_FILENO, STDERR_FILENO, STDIN_FILENO};
    size_t          i;

    // stdout may be redirected, try every standard stream to find the
    // terminal.
    for (i = 0; i < sizeof(fds) / sizeof(fds[0]); i++)
    {
        if (ioctl(fds[i], TIOCGWINSZ, &ws) == 0 && ws.ws_col && ws.ws_row)
        {
            term->cols = ws.ws_col;
            term->rows = ws.ws_row;

            // The window size in pixels gives the size of a cell, which 
            // depends on the font. Some terminals report 0. 
            term->cell_width = ws.ws_xpixel / ws.ws_col; 
            term->cell_height = ws.ws_ypixel / ws.ws_row; 
            return 0;
        }
    }

    return 1;
}
