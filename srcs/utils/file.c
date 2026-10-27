#include "utils/file.h"


// * Read a whole stream in memory. 
// * param: stream: the stream to read. 
// * param: len: where the size of the data will be stored. 
// * return: the data (that need to be freed) or NULL in case of errors. 
static unsigned char* read_stream(FILE* stream, size_t* len)
{
    unsigned char*  data; 
    unsigned char*  bigger; 
    size_t          capacity; 
    size_t          read_len; 

    capacity = READ_CHUNK_LEN; 
    data = malloc(capacity); 
    if (!data)
        return NULL; 

    *len = 0; 
    while ((read_len = fread(data + *len, 1, capacity - *len, stream)) > 0)
    {
        *len += read_len; 
        if (*len < capacity)
            continue; 

        // The buffer is full, double its size and keep reading. 
        capacity *= 2; 
        bigger = realloc(data, capacity); 
        if (!bigger)
        {
            free(data); 
            return NULL; 
        }

        data = bigger; 
    }

    if (ferror(stream))
    {
        free(data); 
        return NULL; 
    }

    return data; 
}


// * Read a whole file in memory. Works with regular files, pipes and devices 
// * since the size is not asked to the file system. 
// * param: filename: path to the file, or "-" for the standard input. 
// * param: len: where the size of the data will be stored. 
// * return: the content of the file (that need to be freed) or NULL in case 
// *         of errors (the error is printed). 
unsigned char* read_file(const char* filename, size_t* len)
{
    FILE*           file; 
    unsigned char*  data; 

    if (!strcmp(filename, STDIN_FILENAME))
    {
        data = read_stream(stdin, len); 
        if (!data)
            print_error(STDIN_NAME, "%s", strerror(errno)); 

        return data; 
    }

    file = fopen(filename, "rb"); 
    if (!file)
    {
        print_error(filename, "%s", strerror(errno)); 
        return NULL; 
    }

    data = read_stream(file, len); 
    if (!data)
        print_error(filename, "%s", strerror(errno)); 

    fclose(file); 
    return data; 
}
