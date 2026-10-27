#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <stdlib.h>
#include "decoders/exif.h"


// __ FUNCTIONS ________________________________________________________________
// * Rotate and/or mirror a RGB buffer to display it with its EXIF orientation.
// * param: rgb: the RGB buffer (3 bytes per pixel) to transform.
// * param: width: the buffer width, swapped with height for 90° rotations.
// * param: height: the buffer height, swapped with width for 90° rotations.
// * param: orientation: the EXIF orientation [1-8].
// * return: the transformed buffer (rgb is freed if a new buffer is
// *         allocated) or NULL in case of errors (rgb is freed).
unsigned char* apply_orientation(unsigned char* rgb, size_t* width, size_t* height, int orientation);

#endif
