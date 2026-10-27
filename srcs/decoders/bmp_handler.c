#include "decoders/bmp_handler.h"


// * Check if the header bytes match the BMP signature. 
// * param: header: the first bytes of the file. 
// * param: len: the number of bytes available in header. 
// * return: 1 if the header is a BMP signature and 0 otherwise. 
int is_bmp(const unsigned char* header, size_t len)
{
    if (len < BMP_FILE_HEADER_LEN + 4)
        return 0; 

    return header[0] == 'B' && header[1] == 'M'; 
}


// * Read a little endian 16 bits value. 
// * param: p: pointer to the value. 
// * return: the value. 
static unsigned int read_le16(const unsigned char* p)
{
    return p[0] | (p[1] << 8); 
}


// * Read a little endian 32 bits value. 
// * param: p: pointer to the value. 
// * return: the value. 
static unsigned long read_le32(const unsigned char* p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned long)p[3] << 24); 
}


// * Extract a channel from a pixel with a bit mask, scaled to [0-255]. 
// * param: pixel: the pixel value. 
// * param: mask: the mask of the channel. 
// * return: the channel value, 255 if the mask is empty. 
static int extract_channel(unsigned long pixel, unsigned long mask)
{
    unsigned long   max; 
    int             shift; 

    if (!mask)
        return 255; 

    shift = 0; 
    while (!((mask >> shift) & 1))
        shift++; 

    max = mask >> shift; 
    return (((pixel & mask) >> shift) * 255 + max / 2) / max; 
}


// * Read the headers: size, bits per pixel, compression, masks, palette and 
// * position of the pixels. 
// * param: decoder: the encoded image. 
// * param: info: where the header fields will be stored. 
// * return: 0 if the headers are valid and 1 otherwise (error printed). 
static int read_bmp_info(DECODER_t* decoder, BMP_INFO_t* info)
{
    const unsigned char*    data; 
    unsigned long           header_len; 
    unsigned long           pixels_offset; 
    unsigned long           colors_used; 
    long                    width; 
    long                    height; 
    size_t                  masks_offset; 
    size_t                  palette_offset; 

    data = decoder->data; 
    pixels_offset = read_le32(data + 10); 
    header_len = read_le32(data + BMP_FILE_HEADER_LEN); 
    memset(info, 0, sizeof(BMP_INFO_t)); 

    if (header_len != BMP_CORE_HEADER_LEN && header_len < BMP_INFO_HEADER_LEN)
    {
        print_error(decoder->name, "unable to read BMP: unsupported header"); 
        return 1; 
    }

    if (decoder->len < BMP_FILE_HEADER_LEN + header_len || pixels_offset > decoder->len)
    {
        print_error(decoder->name, "unable to read BMP: truncated header"); 
        return 1; 
    }

    data += BMP_FILE_HEADER_LEN; 
    colors_used = 0; 

    if (header_len == BMP_CORE_HEADER_LEN)
    {
        // OS/2 header: 16 bits size, palette entries of 3 bytes. 
        width = read_le16(data + 4); 
        height = (short)read_le16(data + 6); 
        info->bpp = read_le16(data + 10); 
        info->compression = BMP_RGB; 
        info->palette_entry_len = 3; 
    }

    else
    {
        width = (int)read_le32(data + 4); 
        height = (int)read_le32(data + 8); 
        info->bpp = read_le16(data + 14); 
        info->compression = read_le32(data + 16); 
        colors_used = read_le32(data + 32); 
        info->palette_entry_len = 4; 
    }

    // A negative height means the rows are stored from the top. 
    info->top_down = height < 0; 
    info->width = width; 
    info->height = height < 0 ? -height : height; 

    if (width <= 0 || !info->height || info->width > BMP_MAX_SIZE || info->height > BMP_MAX_SIZE)
    {
        print_error(decoder->name, "unable to read BMP: unsupported size"); 
        return 1; 
    }

    // Default masks: 5 bits per channel for 16 bits, 8 bits for 32 bits. 
    if (info->bpp == 16)
    {
        info->masks[0] = 0x7C00; 
        info->masks[1] = 0x03E0; 
        info->masks[2] = 0x001F; 
    }

    else if (info->bpp == 32)
    {
        info->masks[0] = 0xFF0000; 
        info->masks[1] = 0x00FF00; 
        info->masks[2] = 0x0000FF; 
    }

    // Bit fields masks follow the 40 bytes header, or are inside the bigger 
    // headers (V2 and later). 
    masks_offset = BMP_FILE_HEADER_LEN + BMP_INFO_HEADER_LEN; 
    palette_offset = BMP_FILE_HEADER_LEN + header_len; 
    if (info->compression == BMP_BITFIELDS || info->compression == BMP_ALPHABITFIELDS)
    {
        if (header_len == BMP_INFO_HEADER_LEN)
            palette_offset += info->compression == BMP_ALPHABITFIELDS ? 16 : 12; 

        if (masks_offset + 12 > decoder->len)
        {
            print_error(decoder->name, "unable to read BMP: truncated header"); 
            return 1; 
        }

        info->masks[0] = read_le32(decoder->data + masks_offset); 
        info->masks[1] = read_le32(decoder->data + masks_offset + 4); 
        info->masks[2] = read_le32(decoder->data + masks_offset + 8); 
    }

    // The alpha mask exists in V3 headers and later, or with ALPHABITFIELDS. 
    if ((header_len >= 56 || info->compression == BMP_ALPHABITFIELDS) && masks_offset + 16 <= decoder->len)
        info->masks[3] = read_le32(decoder->data + masks_offset + 12); 

    if (info->bpp <= 8)
    {
        info->palette_count = colors_used ? colors_used : 1u << info->bpp; 
        if (info->palette_count > 256 || palette_offset + info->palette_count * info->palette_entry_len > decoder->len)
        {
            print_error(decoder->name, "unable to read BMP: invalid palette"); 
            return 1; 
        }

        info->palette = decoder->data + palette_offset; 
    }

    if (info->bpp != 1 && info->bpp != 4 && info->bpp != 8 && info->bpp != 16 && info->bpp != 24 && info->bpp != 32)
    {
        print_error(decoder->name, "unable to read BMP: %u bits per pixel not supported", info->bpp); 
        return 1; 
    }

    if ((info->compression == BMP_RLE8 && info->bpp != 8) || (info->compression == BMP_RLE4 && info->bpp != 4) 
        || info->compression > BMP_ALPHABITFIELDS || info->compression == 4 || info->compression == 5)
    {
        print_error(decoder->name, "unable to read BMP: unsupported compression"); 
        return 1; 
    }

    info->pixels = decoder->data + pixels_offset; 
    info->pixels_len = decoder->len - pixels_offset; 
    return 0; 
}


// * Write a palette color in the RGB buffer. 
// * param: info: the BMP header fields. 
// * param: rgb: the RGB buffer. 
// * param: x: column of the pixel. 
// * param: y: row of the pixel, from the bottom of the file. 
// * param: index: the palette index. 
static void put_palette_pixel(BMP_INFO_t* info, unsigned char* rgb, size_t x, size_t y, unsigned int index)
{
    const unsigned char*    entry; 
    unsigned char*          pixel; 

    if (x >= info->width || y >= info->height)
        return; 

    // Rows are stored from the bottom, unless top_down. 
    if (!info->top_down)
        y = info->height - 1 - y; 

    pixel = rgb + (y * info->width + x) * 3; 
    if (index >= info->palette_count)
    {
        memset(pixel, 0, 3); 
        return; 
    }

    // Palette entries are stored as BGR(x). 
    entry = info->palette + index * info->palette_entry_len; 
    pixel[0] = entry[2]; 
    pixel[1] = entry[1]; 
    pixel[2] = entry[0]; 
}


// * Decode RLE8 or RLE4 compressed pixels: runs of a color, absolute runs, 
// * end of line, end of image and deltas. 
// * param: info: the BMP header fields. 
// * param: rgb: the RGB buffer (black where no pixel is written). 
// * return: 0 if no error occured and 1 otherwise. 
static int decode_rle(BMP_INFO_t* info, unsigned char* rgb)
{
    const unsigned char*    p; 
    const unsigned char*    end; 
    size_t                  x, y; 
    unsigned int            count; 
    unsigned int            i; 
    int                     rle4; 

    p = info->pixels; 
    end = info->pixels + info->pixels_len; 
    rle4 = info->compression == BMP_RLE4; 
    x = 0; 
    y = 0; 

    while (p + 2 <= end)
    {
        count = p[0]; 

        // Encoded run: count pixels of the same color (2 alternating colors 
        // for RLE4). 
        if (count)
        {
            for (i = 0; i < count; i++)
                put_palette_pixel(info, rgb, x++, y, rle4 ? (i % 2 ? p[1] & 0x0F : p[1] >> 4) : p[1]); 

            p += 2; 
            continue; 
        }

        switch (p[1])
        {
            case 0:     // end of line 
                x = 0; 
                y++; 
                p += 2; 
                break; 

            case 1:     // end of image 
                return 0; 

            case 2:     // delta: move right and up 
                if (p + 4 > end)
                    return 1; 

                x += p[2]; 
                y += p[3]; 
                p += 4; 
                break; 

            default:    // absolute run of p[1] pixels, padded to 16 bits 
                count = p[1]; 
                p += 2; 
                if (p + (rle4 ? (count + 1) / 2 : count) > end)
                    return 1; 

                for (i = 0; i < count; i++)
                    put_palette_pixel(info, rgb, x++, y, rle4 ? (i % 2 ? p[i / 2] & 0x0F : p[i / 2] >> 4) : p[i]); 

                p += rle4 ? ((count + 3) / 4) * 2 : ((count + 1) / 2) * 2; 
                break; 
        }
    }

    return 0; 
}


// * Check if the alpha channel is really used: many 32 bits images declare 
// * an alpha mask but leave it to 0 everywhere, they are opaque. 
// * param: info: the BMP header fields, the rows must not be truncated. 
// * param: stride: the length of a row in bytes. 
// * return: 1 if at least one pixel has a non zero alpha and 0 otherwise. 
static int is_alpha_used(BMP_INFO_t* info, size_t stride)
{
    const unsigned char*    row; 
    unsigned long           value; 
    size_t                  x, y; 

    for (y = 0; y < info->height; y++)
    {
        row = info->pixels + y * stride; 
        for (x = 0; x < info->width; x++)
        {
            value = info->bpp == 16 ? read_le16(row + x * 2) : read_le32(row + x * 4); 
            if (value & info->masks[3])
                return 1; 
        }
    }

    return 0; 
}


// * Decode uncompressed or bit fields pixels.
// * param: info: the BMP header fields. 
// * param: rgb: the RGB buffer. 
// * param: has_alpha: where 1 is stored if a pixel is not fully opaque. 
// * return: 0 if no error occured and 1 otherwise (truncated image). 
static int decode_rows(BMP_INFO_t* info, unsigned char* rgb, int* has_alpha)
{
    const unsigned char*    row; 
    unsigned char*          pixel; 
    unsigned long           value; 
    size_t                  stride; 
    size_t                  x, y; 
    size_t                  file_y; 
    unsigned int            index; 
    int                     alpha; 
    int                     c; 

    // Rows are padded to 4 bytes. 
    stride = ((info->width * info->bpp + 31) / 32) * 4; 
    if (stride * info->height > info->pixels_len)
        return 1;

    if (info->bpp >= 16 && info->masks[3] && !is_alpha_used(info, stride))
        info->masks[3] = 0; 

    *has_alpha = 0; 
    for (y = 0; y < info->height; y++)
    {
        file_y = info->top_down ? y : info->height - 1 - y; 
        row = info->pixels + file_y * stride; 

        for (x = 0; x < info->width; x++)
        {
            if (info->bpp <= 8)
            {
                // Palette indexes, most significant bits first. 
                index = (row[x * info->bpp / 8] >> (8 - info->bpp - (x * info->bpp) % 8)) & ((1u << info->bpp) - 1); 
                put_palette_pixel(info, rgb, x, info->top_down ? y : info->height - 1 - y, index); 
                continue; 
            }

            pixel = rgb + (y * info->width + x) * 3; 
            if (info->bpp == 24)
            {
                pixel[0] = row[x * 3 + 2]; 
                pixel[1] = row[x * 3 + 1]; 
                pixel[2] = row[x * 3]; 
                continue; 
            }

            value = info->bpp == 16 ? read_le16(row + x * 2) : read_le32(row + x * 4); 
            alpha = info->masks[3] ? extract_channel(value, info->masks[3]) : 255; 
            if (alpha < 255)
                *has_alpha = 1; 

            // Transparent pixels are blended with black. 
            for (c = 0; c < 3; c++)
                pixel[c] = (extract_channel(value, info->masks[c]) * alpha + 127) / 255; 
        }
    }

    return 0; 
}


// * Decode a BMP image (1, 4, 8, 16, 24 or 32 bits, palette, bit fields and 
// * RLE4/RLE8 compression) and give it to the decoder callback. Transparent 
// * pixels are blended with a black background. 
// * param: decoder: the encoded image and the frame callback. 
// * return: 0 if no error occured and 1 otherwise. 
int decode_bmp(DECODER_t* decoder)
{
    BMP_INFO_t      info; 
    unsigned char*  rgb; 
    int             has_alpha; 
    int             retval; 

    if (read_bmp_info(decoder, &info))
        return 1; 

    rgb = calloc(info.width * info.height, 3); 
    if (!rgb)
        return 1; 

    has_alpha = 0; 
    if (info.compression == BMP_RLE8 || info.compression == BMP_RLE4)
        retval = decode_rle(&info, rgb); 

    else
        retval = decode_rows(&info, rgb, &has_alpha); 

    if (retval)
    {
        print_error(decoder->name, "unable to decode BMP: truncated image"); 
        free(rgb); 
        return 1; 
    }

    retval = decoder->on_frame(rgb, info.width, info.height, has_alpha ? 4 : 3, 0, decoder->user); 
    free(rgb); 
    return retval; 
}
