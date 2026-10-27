#include "display/terminal.h"


// * Ask the size to the kernel (ioctl). 
// * param: term: where the size will be stored. 
// * return: 0 if the size was found and 1 otherwise. 
static int ioctl_terminal_size(TERMINAL_t* term)
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


// * Read the size from the $COLUMNS and $LINES environment variables. 
// * param: term: where the size will be stored. 
// * return: 0 if both variables are valid and 1 otherwise. 
static int env_terminal_size(TERMINAL_t* term)
{
    const char*     cols; 
    const char*     rows; 
    char*           end; 
    unsigned long   value; 

    cols = getenv("COLUMNS"); 
    rows = getenv("LINES"); 
    if (!cols || !rows)
        return 1; 

    value = strtoul(cols, &end, 10); 
    if (end == cols || *end || !value)
        return 1; 

    term->cols = value; 
    value = strtoul(rows, &end, 10); 
    if (end == rows || *end || !value)
        return 1; 

    term->rows = value; 
    return 0; 
}


// * Ask the size to the terminal itself: move the cursor to the bottom right 
// * corner and ask its position. Works through serial links when the 
// * terminal emulator answers. 
// * param: term: where the size will be stored. 
// * return: 0 if the terminal answered and 1 otherwise. 
static int query_terminal_size(TERMINAL_t* term)
{
    struct termios  old_attr; 
    struct termios  raw_attr; 
    struct pollfd   pfd; 
    char            answer[QUERY_ANSWER_MAX_LEN + 1]; 
    char*           start; 
    size_t          len; 
    ssize_t         read_len; 
    unsigned int    rows; 
    unsigned int    cols; 

    // The answer comes on the input, both sides must be the terminal. 
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
        return 1; 

    if (tcgetattr(STDIN_FILENO, &old_attr))
        return 1; 

    // Read the answer byte by byte without echoing it. 
    raw_attr = old_attr; 
    raw_attr.c_lflag &= ~(ICANON | ECHO); 
    raw_attr.c_cc[VMIN] = 0; 
    raw_attr.c_cc[VTIME] = 0; 
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw_attr))
        return 1; 

    fflush(stdout); 
    if (write(STDOUT_FILENO, ESC_QUERY_SIZE, strlen(ESC_QUERY_SIZE)) < 0)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_attr); 
        return 1; 
    }

    len = 0; 
    pfd.fd = STDIN_FILENO; 
    pfd.events = POLLIN; 
    while (len < QUERY_ANSWER_MAX_LEN && poll(&pfd, 1, QUERY_TIMEOUT_MS) > 0)
    {
        read_len = read(STDIN_FILENO, answer + len, QUERY_ANSWER_MAX_LEN - len); 
        if (read_len <= 0)
            break; 

        len += read_len; 
        if (answer[len - 1] == 'R')
            break; 
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_attr); 
    answer[len] = '\0'; 

    // Keys typed before the answer may come first, look for the last ESC [. 
    start = NULL; 
    for (char* p = strstr(answer, "\x1b["); p; p = strstr(p + 1, "\x1b["))
        start = p; 

    if (!start || sscanf(start, "\x1b[%u;%uR", &rows, &cols) != 2 || !rows || !cols)
        return 1; 

    term->cols = cols; 
    term->rows = rows; 
    return 0; 
}


// * Get the size of the terminal the program is running in. The size is 
// * asked to the kernel (ioctl), then read from $COLUMNS and $LINES, then 
// * asked to the terminal itself with an escape sequence (serial consoles 
// * often report a 0x0 size to the kernel). The result is cached. 
// * param: term: where the number of columns and rows will be stored, with 
// *        the size of a character cell in pixels (0 if unknown). 
// * return: 0 if the size was found and 1 otherwise. 
int get_terminal_size(TERMINAL_t* term)
{
    static TERMINAL_t   cached; 
    static int          state = -1;     // -1: not searched, 0: found, 1: none

    if (state == -1)
    {
        memset(&cached, 0, sizeof(TERMINAL_t)); 
        state = ioctl_terminal_size(&cached) 
            && env_terminal_size(&cached) 
            && query_terminal_size(&cached); 
    }

    *term = cached; 
    return state; 
}
