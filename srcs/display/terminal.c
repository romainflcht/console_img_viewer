#include "display/terminal.h"


// * Get the size of the terminal the program is running in.
// * param: cols: where the number of columns will be stored.
// * param: rows: where the number of rows will be stored.
// * return: 0 if the size was found and 1 otherwise (no terminal attached).
int get_terminal_size(size_t* cols, size_t* rows)
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
            *cols = ws.ws_col;
            *rows = ws.ws_row;
            return 0;
        }
    }

    return 1;
}
