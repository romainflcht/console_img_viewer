#ifndef _ARGS_H_
#define _ARGS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "utils/utils.h"
#include "version.h"


// __ DEFINIFIONS ______________________________________________________________
#define ARGS_OK     0
#define ARGS_ERROR  1
#define ARGS_EXIT   2


// __ TYPES DEFINITIONS ________________________________________________________
typedef struct
{
    int     loop; 
    int     file_count; 
    char**  files; 
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
