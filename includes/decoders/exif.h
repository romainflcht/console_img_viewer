#ifndef _EXIF_H_
#define _EXIF_H_

#include <stdlib.h>
#include <string.h>


// __ DEFINIFIONS ______________________________________________________________
#define EXIF_HEADER         "Exif\0\0"
#define EXIF_HEADER_LEN     6
#define EXIF_TAG_ORIENTATION 0x0112

// Values of the EXIF orientation tag.
#define ORIENTATION_NORMAL      1
#define ORIENTATION_MIRROR_H    2
#define ORIENTATION_ROTATE_180  3
#define ORIENTATION_MIRROR_V    4
#define ORIENTATION_TRANSPOSE   5
#define ORIENTATION_ROTATE_90   6
#define ORIENTATION_TRANSVERSE  7
#define ORIENTATION_ROTATE_270  8


// __ FUNCTIONS ________________________________________________________________
// * Read the orientation tag from EXIF data.
// * param: data: the EXIF data, with or without the "Exif\0\0" header.
// * param: len: the length of data in bytes.
// * return: the orientation [1-8] or ORIENTATION_NORMAL if it is missing or
// *         invalid.
int exif_get_orientation(const unsigned char* data, size_t len);

#endif
