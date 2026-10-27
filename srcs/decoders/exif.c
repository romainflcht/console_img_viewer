#include "decoders/exif.h"


// * Read a 16 bits value with the byte order of the EXIF data.
// * param: p: pointer to the value.
// * param: big_endian: 1 if the EXIF data is big endian ("MM"), 0 otherwise.
// * return: the value.
static unsigned int read_u16(const unsigned char* p, int big_endian)
{
    if (big_endian)
        return (p[0] << 8) | p[1];

    return (p[1] << 8) | p[0];
}


// * Read a 32 bits value with the byte order of the EXIF data.
// * param: p: pointer to the value.
// * param: big_endian: 1 if the EXIF data is big endian ("MM"), 0 otherwise.
// * return: the value.
static unsigned long read_u32(const unsigned char* p, int big_endian)
{
    if (big_endian)
        return ((unsigned long)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];

    return ((unsigned long)p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}


// * Read the orientation tag from EXIF data.
// * param: data: the EXIF data, with or without the "Exif\0\0" header.
// * param: len: the length of data in bytes.
// * return: the orientation [1-8] or ORIENTATION_NORMAL if it is missing or
// *         invalid.
int exif_get_orientation(const unsigned char* data, size_t len)
{
    int             big_endian;
    unsigned long   ifd_offset;
    unsigned int    entry_count;
    unsigned int    i;
    unsigned int    orientation;
    const unsigned char* entry;

    // JPEG APP1 segments start with "Exif\0\0", the TIFF data comes after.
    if (len >= EXIF_HEADER_LEN && !memcmp(data, EXIF_HEADER, EXIF_HEADER_LEN))
    {
        data += EXIF_HEADER_LEN;
        len -= EXIF_HEADER_LEN;
    }

    // TIFF header: byte order ("II" or "MM"), 42, offset of the first IFD.
    if (len < 8)
        return ORIENTATION_NORMAL;

    if (data[0] == 'M' && data[1] == 'M')
        big_endian = 1;

    else if (data[0] == 'I' && data[1] == 'I')
        big_endian = 0;

    else
        return ORIENTATION_NORMAL;

    if (read_u16(data + 2, big_endian) != 42)
        return ORIENTATION_NORMAL;

    ifd_offset = read_u32(data + 4, big_endian);
    if (ifd_offset > len - 2)
        return ORIENTATION_NORMAL;

    // Each IFD entry is 12 bytes: tag, type, count and value.
    entry_count = read_u16(data + ifd_offset, big_endian);
    for (i = 0; i < entry_count; i++)
    {
        if (ifd_offset + 2 + (i + 1) * 12 > len)
            break;

        entry = data + ifd_offset + 2 + i * 12;
        if (read_u16(entry, big_endian) != EXIF_TAG_ORIENTATION)
            continue;

        // The orientation is a SHORT stored at the start of the value field.
        orientation = read_u16(entry + 8, big_endian);
        if (orientation < ORIENTATION_NORMAL || orientation > ORIENTATION_ROTATE_270)
            return ORIENTATION_NORMAL;

        return orientation;
    }

    return ORIENTATION_NORMAL;
}
