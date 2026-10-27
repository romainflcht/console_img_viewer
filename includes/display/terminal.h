#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
#include <sys/ioctl.h>


// __ DEFINIFIONS ______________________________________________________________
// Ask the cursor position after moving it to the bottom right corner, the 
// answer is "\x1b[ROWS;COLSR". The cursor is saved and restored around. 
#define ESC_QUERY_SIZE "\x1b" "7\x1b[999;999H\x1b[6n\x1b" "8"

// How long to wait for the answer, serial links can be slow. 
#define QUERY_TIMEOUT_MS 500

#define QUERY_ANSWER_MAX_LEN 32


// __ TYPES DEFINITIONS ________________________________________________________
typedef struct
{
    size_t  cols; 
    size_t  rows; 
    size_t  cell_width; 
    size_t  cell_height; 
} TERMINAL_t;


// __ FUNCTIONS ________________________________________________________________
// * Get the size of the terminal the program is running in. The size is 
// * asked to the kernel (ioctl), then read from $COLUMNS and $LINES, then 
// * asked to the terminal itself with an escape sequence (serial consoles 
// * often report a 0x0 size to the kernel). The result is cached. 
// * param: term: where the number of columns and rows will be stored, with 
// *        the size of a character cell in pixels (0 if unknown). 
// * return: 0 if the size was found and 1 otherwise. 
int get_terminal_size(TERMINAL_t* term); 

#endif
