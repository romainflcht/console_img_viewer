#include "image/format.h"


// * Check if the header bytes match the JPEG signature (SOI marker). 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a JPEG signature and 0 otherwise. 
static int is_jpeg(const unsigned char* header, size_t len)
{
    return len >= JPEG_SIGNATURE_LEN && header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF; 
}


// * Check if the header bytes match the PNG signature. 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a PNG signature and 0 otherwise. 
static int is_png(const unsigned char* header, size_t len)
{
    return len >= PNG_SIGNATURE_LEN && !memcmp(header, "\x89PNG\r\n\x1a\n", PNG_SIGNATURE_LEN); 
}


// * Check if the header bytes match the GIF signature. 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a GIF signature and 0 otherwise. 
static int is_gif(const unsigned char* header, size_t len)
{
    return len >= GIF_SIGNATURE_LEN 
        && (!memcmp(header, "GIF87a", GIF_SIGNATURE_LEN) || !memcmp(header, "GIF89a", GIF_SIGNATURE_LEN)); 
}


// * Check if the header bytes match the WebP signature: "RIFF", the file 
// * size (4 bytes), "WEBP". 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a WebP signature and 0 otherwise. 
static int is_webp(const unsigned char* header, size_t len)
{
    return len >= WEBP_SIGNATURE_LEN && !memcmp(header, "RIFF", 4) && !memcmp(header + 8, "WEBP", 4); 
}


// * Check if the header bytes match a netpbm signature ("P1" to "P6" and a 
// * whitespace). 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a netpbm signature and 0 otherwise. 
static int is_pnm(const unsigned char* header, size_t len)
{
    return len >= PNM_SIGNATURE_LEN && header[0] == 'P' && header[1] >= '1' && header[1] <= '6' && isspace(header[2]); 
}


// * Check if the header bytes match the BMP signature ("BM", the file 
// * header and the size of the DIB header). 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a BMP signature and 0 otherwise. 
static int is_bmp(const unsigned char* header, size_t len)
{
    return len >= BMP_SIGNATURE_LEN && header[0] == 'B' && header[1] == 'M'; 
}


// * Detect the format of an image from its signature, even when its decoder 
// * is not built. 
// * param: data: the content of the file. 
// * param: len: the size of the data. 
// * return: the format of the image or FORMAT_UNKNOWN. 
IMG_FORMAT_t get_img_format(const unsigned char* data, size_t len)
{
    if (is_jpeg(data, len))
        return FORMAT_JPEG;

    if (is_png(data, len))
        return FORMAT_PNG;

    if (is_gif(data, len))
        return FORMAT_GIF;

    if (is_webp(data, len))
        return FORMAT_WEBP;

    if (is_pnm(data, len))
        return FORMAT_PNM;

    if (is_bmp(data, len))
        return FORMAT_BMP; 

    return FORMAT_UNKNOWN;
}


// * Get the name of a format. 
// * param: format: the format. 
// * return: the name of the format ("JPEG", "PNG"...). 
const char* get_format_name(IMG_FORMAT_t format)
{
    static const char* names[] = {"unknown", "JPEG", "PNG", "GIF", "WebP", "PNM", "BMP"}; 

    return names[format]; 
}
