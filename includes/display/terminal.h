#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>


// __ TYPES DEFINITIONS ________________________________________________________
typedef struct
{
    size_t  cols; 
    size_t  rows; 
    size_t  cell_width; 
    size_t  cell_height; 
} TERMINAL_t;


// __ FUNCTIONS ________________________________________________________________
// * Get the size of the terminal the program is running in. 
// * param: term: where the number of columns and rows will be stored, with 
// *        the size of a character cell in pixels (0 if the terminal doesn't 
// *        report it). 
// * return: 0 if the size was found and 1 otherwise (no terminal attached). 
int get_terminal_size(TERMINAL_t* term); 

#endif
