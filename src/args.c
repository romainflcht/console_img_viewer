#include "args.h"

// * Parse command-line arguments (filenames).
// * param: argc: arguments count.
// * param: argv: arguments vector (array). 
// * return: the first filename (for now).  
char* parse_arg(int argc, char** argv)
{
    if (argc < 2)
        return NULL; 
    
    // TODO: Handle multiple JPEG files. 
    return argv[1]; 
}
