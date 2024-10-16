#include "args.h"


// * Parse command-line arguments (filenames).
// * param: argc: arguments count.
// * param: argv: arguments vector (array). 
// * return: an array of filenames. 
char** parse_arg(int argc, char** argv)
{
    int i; 
    char** files; 

    if (argc < 2)
    {
        printf("- [USAGE] <exec_name> file1 file2 ...\n"); 
        return NULL; 
    }

    files = malloc(sizeof(char*) * (argc - 1)); 

    for (i = 0; i < argc - 1; i++)
        files[i] = argv[i + 1]; 

    return files; 
}
