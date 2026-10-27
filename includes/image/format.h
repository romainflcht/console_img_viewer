#ifndef _FORMAT_H_
#define _FORMAT_H_

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// __ DEFINIFIONS ______________________________________________________________
// Decoders that need a library can be disabled at build time (make 
// WITH_JPEG=0...), BMP and netpbm have no dependency and are always built. 
#ifdef WITH_JPEG
    #define FORMAT_NAME_JPEG "JPEG, "
#else
    #define FORMAT_NAME_JPEG ""
#endif

#ifdef WITH_PNG
    #define FORMAT_NAME_PNG "PNG, "
#else
    #define FORMAT_NAME_PNG ""
#endif

#ifdef WITH_GIF
    #define FORMAT_NAME_GIF "GIF, "
#else
    #define FORMAT_NAME_GIF ""
#endif

#ifdef WITH_WEBP
    #define FORMAT_NAME_WEBP "WebP, "
#else
    #define FORMAT_NAME_WEBP ""
#endif

// Formats listed in the error, help and version messages. 
#define SUPPORTED_FORMATS FORMAT_NAME_JPEG FORMAT_NAME_PNG FORMAT_NAME_GIF FORMAT_NAME_WEBP "BMP, PBM/PGM/PPM"

// Number of bytes needed to recognize every format. 
#define JPEG_SIGNATURE_LEN  3
#define PNG_SIGNATURE_LEN   8
#define GIF_SIGNATURE_LEN   6
#define WEBP_SIGNATURE_LEN  12
#define PNM_SIGNATURE_LEN   3
#define BMP_SIGNATURE_LEN   18


// __ TYPES DEFINITIONS ________________________________________________________
typedef enum
{
    FORMAT_UNKNOWN,
    FORMAT_JPEG,
    FORMAT_PNG,
    FORMAT_GIF,
    FORMAT_WEBP,
    FORMAT_PNM,
    FORMAT_BMP
} IMG_FORMAT_t;


// __ FUNCTIONS ________________________________________________________________
// * Detect the format of an image from its signature, even when its decoder 
// * is not built. 
// * param: data: the content of the file. 
// * param: len: the size of the data. 
// * return: the format of the image or FORMAT_UNKNOWN. 
IMG_FORMAT_t get_img_format(const unsigned char* data, size_t len); 

// * Get the name of a format. 
// * param: format: the format. 
// * return: the name of the format ("JPEG", "PNG"...). 
const char* get_format_name(IMG_FORMAT_t format); 

#endif
