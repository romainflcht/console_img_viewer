#ifndef _ARGS_H_
#define _ARGS_H_

#include <stdio.h>

//__ FUNCTIONS _________________________________________________________________
// * Parse command-line arguments (filenames).
// * param: argc: arguments count.
// * param: argv: arguments vector (array). 
// * return: the first filename (for now). 
char* parse_arg(int argc, char** argv);
#endif