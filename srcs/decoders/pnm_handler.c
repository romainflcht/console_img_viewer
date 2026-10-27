#include "decoders/pnm_handler.h"


// * Skip whitespaces and comments (from # to the end of the line). 
// * param: reader: the read position. 
static void skip_blanks(PNM_READER_t* reader)
{
    while (reader->pos < reader->len)
    {
        if (reader->data[reader->pos] == '#')
        {
            while (reader->pos < reader->len && reader->data[reader->pos] != '\n')
                reader->pos++; 
        }

        else if (isspace(reader->data[reader->pos]))
            reader->pos++; 

        else
            break; 
    }
}


// * Read an ASCII number of the header or of an ASCII image. 
// * param: reader: the read position. 
// * param: value: where the number will be stored. 
// * return: 0 if a number was read and 1 otherwise. 
static int read_number(PNM_READER_t* reader, unsigned long* value)
{
    skip_blanks(reader); 
    if (reader->pos >= reader->len || !isdigit(reader->data[reader->pos]))
        return 1; 

    *value = 0; 
    while (reader->pos < reader->len && isdigit(reader->data[reader->pos]))
    {
        *value = *value * 10 + reader->data[reader->pos] - '0'; 
        if (*value > 0xFFFFFFFF)
            return 1; 

        reader->pos++; 
    }

    return 0; 
}


// * Read the next sample of the image, ASCII or binary, scaled to [0-255]. 
// * param: reader: the read position. 
// * param: ascii: 1 for ASCII images (P1, P2, P3). 
// * param: maxval: the biggest sample value. 
// * param: value: where the sample will be stored. 
// * return: 0 if a sample was read and 1 otherwise (truncated image). 
static int read_sample(PNM_READER_t* reader, int ascii, unsigned long maxval, int* value)
{
    unsigned long sample; 

    if (ascii)
    {
        if (read_number(reader, &sample))
            return 1; 
    }

    // Binary samples are 1 byte, or 2 bytes big endian when maxval > 255. 
    else if (maxval > 255)
    {
        if (reader->pos + 2 > reader->len)
            return 1; 

        sample = (reader->data[reader->pos] << 8) | reader->data[reader->pos + 1]; 
        reader->pos += 2; 
    }

    else
    {
        if (reader->pos >= reader->len)
            return 1; 

        sample = reader->data[reader->pos++]; 
    }

    if (sample > maxval)
        sample = maxval; 

    *value = (sample * 255 + maxval / 2) / maxval; 
    return 0; 
}


// * Read the pixels of a bitmap (P1 or P4), 1 is black. 
// * param: reader: the read position. 
// * param: rgb: where the pixels are written. 
// * param: width: width of the image. 
// * param: height: height of the image. 
// * param: ascii: 1 for P1, 0 for P4 (8 pixels per byte, rows padded). 
// * return: 0 if no error occured and 1 otherwise. 
static int read_bitmap(PNM_READER_t* reader, unsigned char* rgb, size_t width, size_t height, int ascii)
{
    size_t  x, y; 
    int     bit; 

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            if (ascii)
            {
                // P1 digits may be written without spaces between them. 
                skip_blanks(reader); 
                if (reader->pos >= reader->len)
                    return 1; 

                bit = reader->data[reader->pos++] == '1'; 
            }

            else
            {
                if (reader->pos + x / 8 >= reader->len)
                    return 1; 

                bit = (reader->data[reader->pos + x / 8] >> (7 - x % 8)) & 1; 
            }

            memset(rgb + (y * width + x) * 3, bit ? 0 : 255, 3); 
        }

        if (!ascii)
            reader->pos += (width + 7) / 8; 
    }

    return 0; 
}


// * Decode a netpbm image: PBM (P1, P4), PGM (P2, P5) or PPM (P3, P6), in 
// * ASCII or binary, 8 or 16 bits, and give it to the decoder callback. 
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if no error occured and 1 otherwise. 
int decode_pnm(DECODER_t* decoder)
{
    PNM_READER_t    reader; 
    unsigned char*  rgb; 
    unsigned long   width; 
    unsigned long   height; 
    unsigned long   maxval; 
    size_t          channel_count; 
    size_t          i; 
    int             type; 
    int             ascii; 
    int             value; 
    int             c; 
    int             retval; 

    reader.data = decoder->data; 
    reader.len = decoder->len; 
    reader.pos = 2; 
    type = decoder->data[1] - '0'; 
    ascii = type <= 3; 
    channel_count = type == 3 || type == 6 ? 3 : 1; 

    // Header: width, height and maxval (except for bitmaps). 
    maxval = 1; 
    if (read_number(&reader, &width) || read_number(&reader, &height) 
        || (type != 1 && type != 4 && read_number(&reader, &maxval)))
    {
        print_error(decoder->name, "unable to read netpbm: invalid header"); 
        return 1; 
    }

    if (!width || !height || width > PNM_MAX_SIZE || height > PNM_MAX_SIZE || !maxval || maxval > 65535)
    {
        print_error(decoder->name, "unable to read netpbm: unsupported size or maxval"); 
        return 1; 
    }

    // A single whitespace separates the header from binary data. 
    if (reader.pos < reader.len)
        reader.pos++; 

    rgb = malloc(width * height * 3); 
    if (!rgb)
        return 1; 

    if (type == 1 || type == 4)
        retval = read_bitmap(&reader, rgb, width, height, ascii); 

    else
    {
        retval = 0; 
        value = 0; 
        for (i = 0;i < width * height && !retval; i++)
        {
            for (c = 0; c < (int)channel_count && !retval; c++)
            {
                retval = read_sample(&reader, ascii, maxval, &value); 
                rgb[i * 3 + c] = value; 
            }

            // Gray images use the same value for the 3 channels. 
            if (channel_count == 1)
                rgb[i * 3 + 1] = rgb[i * 3 + 2] = rgb[i * 3]; 
        }
    }

    if (retval)
    {
        print_error(decoder->name, "unable to decode netpbm: truncated image"); 
        free(rgb); 
        return 1; 
    }

retval = decoder->on_frame(rgb, width, height, channel_count, 0, decoder->user); 
    free(rgb); 
    return retval; 
}
