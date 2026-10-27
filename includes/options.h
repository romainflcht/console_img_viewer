#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <stdlib.h>


// __ TYPES DEFINITIONS ________________________________________________________
// Which terminal dimensions limit the image size (--fit). 
typedef enum
{
    FIT_BOTH, 
    FIT_WIDTH, 
    FIT_HEIGHT, 
    FIT_NONE
} FIT_MODE_t;

// Display options given on the command line. 
typedef struct
{
    int     loop;           // play animations until Ctrl-C. 
    size_t  term_cols;      // terminal size forced with -s, 0 to detect it. 
    size_t  term_rows; 
    size_t  max_cols;       // maximum image size given with -w/-H, 0 if none. 
    size_t  max_rows; 
    int     upscale;        // enlarge small images up to the maximum size. 
    FIT_MODE_t fit; 
    int     center;         // center the image horizontally. 
} OPTIONS_t;

#endif
