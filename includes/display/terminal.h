#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>


// __ FUNCTIONS ________________________________________________________________
// * Get the size of the terminal the program is running in.
// * param: cols: where the number of columns will be stored.
// * param: rows: where the number of rows will be stored.
// * return: 0 if the size was found and 1 otherwise (no terminal attached).
int get_terminal_size(size_t* cols, size_t* rows);

#endif
