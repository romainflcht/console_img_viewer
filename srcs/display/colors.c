#include "display/colors.h"


// xterm default values of the 16 system colors, used for 16 and 8 colors. 
static const unsigned char system_colors[16][3] = {
    {  0,   0,   0}, {205,   0,   0}, {  0, 205,   0}, {205, 205,   0}, 
    {  0,   0, 238}, {205,   0, 205}, {  0, 205, 205}, {229, 229, 229}, 
    {127, 127, 127}, {255,   0,   0}, {  0, 255,   0}, {255, 255,   0}, 
    { 92,  92, 255}, {255,   0, 255}, {  0, 255, 255}, {255, 255, 255}
}; 

// Levels of each channel in the 6x6x6 cube of the 256 colors palette. 
static const int cube_levels[6] = {0, 95, 135, 175, 215, 255}; 

// 4x4 Bayer matrix for the ordered dithering, values from 0 to 15. 
static const int bayer[4][4] = {
    { 0,  8,  2, 10}, 
    {12,  4, 14,  6}, 
    { 3, 11,  1,  9}, 
    {15,  7, 13,  5}
}; 


// * Choose the color mode from the environment: NO_COLOR and TERM=dumb give 
// * mono, COLORTERM=truecolor or 24bit gives truecolor, a TERM with 256color 
// * gives 256 colors and any other terminal gives 16 colors. 
// * return: the detected color mode. 
COLOR_MODE_t detect_color_mode(void)
{
    const char* no_color; 
    const char* colorterm; 
    const char* term; 

    // https://no-color.org: any non empty value disables the colors. 
    no_color = getenv("NO_COLOR"); 
    if (no_color && *no_color)
        return COLORS_MONO; 

    term = getenv("TERM"); 
    if (term && !strcmp(term, "dumb"))
        return COLORS_MONO; 

    colorterm = getenv("COLORTERM"); 
    if (colorterm && (!strcmp(colorterm, "truecolor") || !strcmp(colorterm, "24bit")))
        return COLORS_TRUECOLOR; 

    if (term && strstr(term, "256color"))
        return COLORS_256; 

    return COLORS_16; 
}


// * Squared distance between two colors, weighted for the human eye. 
// * param: r1, g1, b1: the first color. 
// * param: r2, g2, b2: the second color. 
// * return: the distance. 
static int color_distance(int r1, int g1, int b1, int r2, int g2, int b2)
{
    return 2 * (r1 - r2) * (r1 - r2) + 4 * (g1 - g2) * (g1 - g2) + 3 * (b1 - b2) * (b1 - b2); 
}


// * Index of the nearest level of the 256 colors cube. 
// * param: value: a channel value [0-255]. 
// * return: the level index [0-5]. 
static int nearest_cube_level(int value)
{
    int i; 

    for (i = 0; i < 5; i++)
    {
        if (value < (cube_levels[i] + cube_levels[i + 1]) / 2)
            return i; 
    }

    return 5; 
}


// * Find the palette color nearest to a RGB color. 
// * param: r, g, b: the color [0-255]. 
// * param: mode: the color mode (palette). 
// * param: out: where the RGB value of the palette color is stored. 
// * return: the color code (index, or 0/1 for mono). 
static int nearest_color(int r, int g, int b, COLOR_MODE_t mode, int out[3])
{
    int ri, gi, bi; 
    int gray_index; 
    int gray; 
    int best; 
    int best_distance; 
    int distance; 
    int count; 
    int i; 

    if (mode == COLORS_MONO)
    {
        // Luminance (Rec. 601) against the middle gray. 
        best = (299 * r + 587 * g + 114 * b) / 1000 >= 128; 
        out[0] = out[1] = out[2] = best ? 255 : 0; 
        return best; 
    }

    if (mode == COLORS_256)
    {
        // Nearest color of the cube and nearest gray, keep the closest. 
        ri = nearest_cube_level(r); 
        gi = nearest_cube_level(g); 
        bi = nearest_cube_level(b); 

        gray_index = ((r + g + b) / 3 - 3) / 10; 
        gray_index = gray_index < 0 ? 0 : (gray_index >= GRAY_COUNT ? GRAY_COUNT - 1 : gray_index); 
        gray = 8 + gray_index * 10; 

        if (color_distance(r, g, b, gray, gray, gray) 
            < color_distance(r, g, b, cube_levels[ri], cube_levels[gi], cube_levels[bi]))
        {
            out[0] = out[1] = out[2] = gray; 
            return GRAY_FIRST_INDEX + gray_index; 
        }

        out[0] = cube_levels[ri]; 
        out[1] = cube_levels[gi]; 
        out[2] = cube_levels[bi]; 
        return CUBE_FIRST_INDEX + 36 * ri + 6 * gi + bi; 
    }

    // 16 or 8 system colors. 
    count = mode == COLORS_8 ? 8 : 16; 
    best = 0; 
    best_distance = -1; 
    for (i = 0; i < count; i++)
    {
        distance = color_distance(r, g, b, system_colors[i][0], system_colors[i][1], system_colors[i][2]); 
        if (best_distance < 0 || distance < best_distance)
        {
            best = i; 
            best_distance = distance; 
        }
    }

    out[0] = system_colors[best][0]; 
    out[1] = system_colors[best][1]; 
    out[2] = system_colors[best][2]; 
    return best; 
}


// * Clamp a value to [0-255]. 
// * param: value: the value to clamp. 
// * return: the clamped value. 
static int clamp_u8(int value)
{
    return value < 0 ? 0 : (value > 255 ? 255 : value); 
}


// * Map a frame to a palette with Floyd-Steinberg dithering: the error of 
// * each pixel is spread to its neighbours not yet processed. 
// * param: rgb: the RGB frame. 
// * param: codes: where the color codes are written. 
// * param: width: width of the frame. 
// * param: height: height of the frame. 
// * param: mode: the color mode (palette). 
// * return: 0 if no error occured and 1 otherwise. 
static int map_floyd_steinberg(const unsigned char* rgb, int* codes, size_t width, size_t height, COLOR_MODE_t mode)
{
    int*    errors; 
    int*    current; 
    int*    next; 
    int*    swap; 
    int     value[3]; 
    int     palette[3]; 
    int     error; 
    size_t  x, y; 
    int     c; 

    // Error of the current and next rows, with one more pixel on each side 
    // so the borders don't need special cases. 
    errors = calloc((width + 2) * 3 * 2, sizeof(int)); 
    if (!errors)
        return 1; 

    current = errors; 
    next = errors + (width + 2) * 3; 

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            for (c = 0; c < 3; c++)
                value[c] = clamp_u8(rgb[(y * width + x) * 3 + c] + current[(x + 1) * 3 + c] / 16); 

            codes[y * width + x] = nearest_color(value[0], value[1], value[2], mode, palette); 

            for (c = 0; c < 3; c++)
            {
                error = value[c] - palette[c]; 
                current[(x + 2) * 3 + c] += error * 7; 
                next[x * 3 + c] += error * 3; 
                next[(x + 1) * 3 + c] += error * 5; 
                next[(x + 2) * 3 + c] += error; 
            }
        }

        swap = current; 
        current = next; 
        next = swap; 
        memset(next, 0, (width + 2) * 3 * sizeof(int)); 
    }

    free(errors); 
    return 0; 
}


// * Convert a RGB frame to the color codes of the color mode: 0xRRGGBB for 
// * truecolor, a palette index for 256/16/8 colors and 0 (black) or 1 
// * (white) for mono. 
// * param: rgb: the RGB frame (3 bytes per pixel). 
// * param: width: width of the frame. 
// * param: height: height of the frame. 
// * param: mode: the color mode (not COLORS_AUTO). 
// * param: dither: the dithering (not DITHER_AUTO). 
// * return: width * height color codes (that need to be freed) or NULL in 
// *         case of errors. 
int* map_colors(const unsigned char* rgb, size_t width, size_t height, COLOR_MODE_t mode, DITHER_MODE_t dither)
{
    const unsigned char*    pixel; 
    int*                    codes; 
    int                     palette[3]; 
    int                     spread; 
    int                     bias; 
    size_t                  x, y; 

    codes = malloc(sizeof(int) * width * height); 
    if (!codes)
        return NULL; 

    if (mode == COLORS_TRUECOLOR)
    {
        for (x = 0; x < width * height; x++)
            rgb_to_hex(rgb[x * 3], rgb[x * 3 + 1], rgb[x * 3 + 2], codes + x); 

        return codes; 
    }

    if (dither == DITHER_FS)
    {
        if (map_floyd_steinberg(rgb, codes, width, height, mode))
        {
            free(codes); 
            return NULL; 
        }

        return codes; 
    }

    // Ordered dithering adds a fixed pattern before looking for the nearest 
    // color, the same pixel always gives the same color (stable animations). 
    spread = 0; 
    if (dither == DITHER_ORDERED)
        spread = mode == COLORS_MONO ? ORDERED_SPREAD_MONO : (mode == COLORS_256 ? ORDERED_SPREAD_256 : ORDERED_SPREAD_16); 

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel = rgb + (y * width + x) * 3; 
            bias = (bayer[y % 4][x % 4] * 2 - 15) * spread / 32; 
            codes[y * width + x] = nearest_color(
                clamp_u8(pixel[0] + bias), 
                clamp_u8(pixel[1] + bias), 
                clamp_u8(pixel[2] + bias), 
                mode, 
                palette
            ); 
        }
    }

    return codes; 
}
