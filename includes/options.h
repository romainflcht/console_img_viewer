#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <stdlib.h>


// __ TYPES DEFINITIONS ________________________________________________________
// Display options given on the command line. 
typedef struct
{
    int     loop;           // play animations until Ctrl-C. 
    size_t  term_cols;      // terminal size forced with -s, 0 to detect it. 
    size_t  term_rows; 
} OPTIONS_t;

#endif
