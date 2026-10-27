#include "image/transform.h"


// * Rotate and/or mirror a RGB buffer to display it with its EXIF orientation.
// * param: rgb: the RGB buffer (3 bytes per pixel) to transform.
// * param: width: the buffer width, swapped with height for 90° rotations.
// * param: height: the buffer height, swapped with width for 90° rotations.
// * param: orientation: the EXIF orientation [1-8].
// * return: the transformed buffer (rgb is freed if a new buffer is
// *         allocated) or NULL in case of errors (rgb is freed).
unsigned char* apply_orientation(unsigned char* rgb, size_t* width, size_t* height, int orientation)
{
    unsigned char*  dst;
    unsigned char*  src_pixel;
    unsigned char*  dst_pixel;
    size_t          w;
    size_t          h;
    size_t          dst_width;
    size_t          dst_height;
    size_t          x, y;
    size_t          sx, sy;

    if (orientation <= ORIENTATION_NORMAL || orientation > ORIENTATION_ROTATE_270)
        return rgb;

    w = *width;
    h = *height;

    // Orientations 5 to 8 swap the width and the height.
    if (orientation >= ORIENTATION_TRANSPOSE)
    {
        dst_width = h;
        dst_height = w;
    }

    else
    {
        dst_width = w;
        dst_height = h;
    }

    dst = malloc(sizeof(unsigned char) * w * h * 3);
    if (!dst)
    {
        free(rgb);
        return NULL;
    }

    for (y = 0; y < dst_height; y++)
    {
        for (x = 0; x < dst_width; x++)
        {
            // Find the source pixel displayed at (x, y).
            switch (orientation)
            {
                case ORIENTATION_MIRROR_H:   sx = w - 1 - x; sy = y;         break;
                case ORIENTATION_ROTATE_180: sx = w - 1 - x; sy = h - 1 - y; break;
                case ORIENTATION_MIRROR_V:   sx = x;         sy = h - 1 - y; break;
                case ORIENTATION_TRANSPOSE:  sx = y;         sy = x;         break;
                case ORIENTATION_ROTATE_90:  sx = y;         sy = h - 1 - x; break;
                case ORIENTATION_TRANSVERSE: sx = w - 1 - y; sy = h - 1 - x; break;
                default:                     sx = w - 1 - y; sy = x;         break;
            }

            src_pixel = rgb + (sy * w + sx) * 3;
            dst_pixel = dst + (y * dst_width + x) * 3;
            dst_pixel[0] = src_pixel[0];
            dst_pixel[1] = src_pixel[1];
            dst_pixel[2] = src_pixel[2];
        }
    }

    free(rgb);
    *width = dst_width;
    *height = dst_height;
    return dst;
}
