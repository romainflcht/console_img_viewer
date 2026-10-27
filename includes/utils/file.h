#ifndef _FILE_H_
#define _FILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "utils/utils.h"


// __ DEFINIFIONS ______________________________________________________________
// Filename meaning the standard input. 
#define STDIN_FILENAME  "-"
#define STDIN_NAME      "standard input"

#define READ_CHUNK_LEN  65536


// __ FUNCTIONS ________________________________________________________________
// * Read a whole file in memory. Works with regular files, pipes and devices 
// * since the size is not asked to the file system. 
// * param: filename: path to the file, or "-" for the standard input. 
// * param: len: where the size of the data will be stored. 
// * return: the content of the file (that need to be freed) or NULL in case 
// *         of errors (the error is printed). 
unsigned char* read_file(const char* filename, size_t* len); 

#endif
