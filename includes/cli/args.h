#ifndef _ARGS_H_
#define _ARGS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include "utils/utils.h"
#include "version.h"
#include "options.h"
#include "display/colors.h"


// __ DEFINIFIONS ______________________________________________________________
#define ARGS_OK     0
#define ARGS_ERROR  1
#define ARGS_EXIT   2

// Values of the long options without a short option. 
#define OPT_FIT     256
#define OPT_DITHER  257
#define OPT_UTF8    258
#define OPT_FRAME   259


// __ TYPES DEFINITIONS ________________________________________________________
typedef struct
{
    OPTIONS_t   options; 
    int         file_count; 
    char**      files; 
} ARGS_t;


//__ FUNCTIONS _________________________________________________________________
// * Parse command-line options and filenames. 
// * param: argc: arguments count.
// * param: argv: arguments vector (array). 
// * param: args: where the options and the filenames will be stored. 
// * return: ARGS_OK, ARGS_ERROR if the arguments are invalid or ARGS_EXIT if 
// *         the program must exit successfully (help or version). 
int parse_arg(int argc, char** argv, ARGS_t* args); 

#endif
