#include "display/renderer.h"


// * Append a string to the output buffer. The buffer is allocated with its
// * worst case size so there is no bound check.
// * param: out: the output buffer.
// * param: str: the string to append.
static void append_str(OUTPUT_BUFFER_t* out, const char* str)
{
    size_t len;

    len = strlen(str);
    memcpy(out->data + out->len, str, len);
    out->len += len;
}


// * Append a color component [0-255] in decimal to the output buffer.
// * param: out: the output buffer.
// * param: value: the value to append.
static void append_u8(OUTPUT_BUFFER_t* out, int value)
{
    if (value >= 100)
        out->data[out->len++] = '0' + value / 100;

    if (value >= 10)
        out->data[out->len++] = '0' + value / 10 % 10;

    out->data[out->len++] = '0' + value % 10;
}


// * Append a number in decimal to the output buffer. 
// * param: out: the output buffer. 
// * param: value: the value to append. 
static void append_number(OUTPUT_BUFFER_t* out, size_t value)
{
    char    digits[20]; 
    int     len; 

    len = 0; 
    do
    {
        digits[len++] = '0' + value % 10; 
        value /= 10; 
    } while (value); 

    while (len)
        out->data[out->len++] = digits[--len]; 
}


// * Append the SGR parameters of a color: "38;2;r;g;b" (truecolor), 
// * "38;5;n" (256 colors) or "3n"/"9n" (16 and 8 colors), 48/4n/10n for the 
// * background. 
// * param: out: the output buffer. 
// * param: color: the color code (0xRRGGBB or a palette index). 
// * param: background: 1 for the background color, 0 for the foreground. 
// * param: mode: the color mode. 
static void append_color(OUTPUT_BUFFER_t* out, int color, int background, COLOR_MODE_t mode)
{
    int r, g, b; 

    switch (mode)
    {
        case COLORS_TRUECOLOR:
            hex_to_rgb(color, &r, &g, &b); 
            append_str(out, background ? "48;2;" : "38;2;"); 
            append_u8(out, r); 
            out->data[out->len++] = ';'; 
            append_u8(out, g); 
            out->data[out->len++] = ';'; 
            append_u8(out, b); 
            break; 

        case COLORS_256:
            append_str(out, background ? "48;5;" : "38;5;"); 
            append_u8(out, color); 
            break; 

        default:
            // Colors 8 to 15 are the bright versions (90-97 and 100-107). 
            if (color < 8)
                append_u8(out, (background ? 40 : 30) + color); 

            else
                append_u8(out, (background ? 100 : 90) + color - 8); 
            break; 
    }
}


// * Append one cell to the output buffer, with the escape sequence needed to
// * switch from the current colors to the cell colors. In mono, the cell is 
// * drawn with a block character and no color. 
// * param: out: the output buffer.
// * param: pixel: the cell to append.
// * param: f_color: the current foreground color, -1 if unknown.
// * param: b_color: the current background color, -1 if unknown.
// * param: mode: the color mode. 
static void append_cell(OUTPUT_BUFFER_t* out, PIXEL_t* pixel, int* f_color, int* b_color, COLOR_MODE_t mode)
{
    // Mono: b_color is the top pixel, f_color the bottom one (0 or 1). 
    static const char* mono_blocks[4] = {" ", MONO_BOTTOM_CHAR, MONO_TOP_CHAR, MONO_FULL_CHAR}; 
    int f_changed;
    int b_changed;

    if (mode == COLORS_MONO)
    {
        append_str(out, mono_blocks[(pixel->b_color << 1) | pixel->f_color]);
        return;
    }

    // Only the top pixel: ▀ with the top color, default background. 
    if (pixel->f_color == COLOR_NONE)
    {
        if (*b_color != -1 || *f_color != pixel->b_color)
        {
            append_str(out, "\x1b[0;"); 
            append_color(out, pixel->b_color, 0, mode); 
            out->data[out->len++] = 'm'; 
            *f_color = pixel->b_color; 
            *b_color = -1; 
        }

        append_str(out, MONO_TOP_CHAR); 
        return; 
    }

    f_changed = pixel->f_color != *f_color;
    b_changed = pixel->b_color != *b_color;

    // Both colors are set in a single escape sequence when possible.
    if (f_changed || b_changed)
    {
        append_str(out, "\x1b[");

        if (f_changed)
            append_color(out, pixel->f_color, 0, mode); 

        if (f_changed && b_changed)
            out->data[out->len++] = ';';

        if (b_changed)
            append_color(out, pixel->b_color, 1, mode); 

        out->data[out->len++] = 'm';
        *f_color = pixel->f_color;
        *b_color = pixel->b_color;
    }

    append_str(out, PIXEL_CHAR);
}


// * Append one ASCII cell: a character chosen by the brightness of the pixel 
// * (b_color), with the pixel color (f_color) as foreground color, except in 
// * mono. 
// * param: out: the output buffer. 
// * param: pixel: the cell to append. 
// * param: f_color: the current foreground color, -1 if unknown. 
// * param: mode: the color mode. 
static void append_ascii_cell(OUTPUT_BUFFER_t* out, PIXEL_t* pixel, int* f_color, COLOR_MODE_t mode)
{
    static const char   ramp[] = ASCII_RAMP; 
    int                 level; 

    if (mode != COLORS_MONO && pixel->f_color != *f_color)
    {
        append_str(out, "\x1b["); 
        append_color(out, pixel->f_color, 0, mode); 
        out->data[out->len++] = 'm'; 
        *f_color = pixel->f_color; 
    }

    level = pixel->b_color * (int)(sizeof(ramp) - 2) / 255; 
    out->data[out->len++] = ramp[level]; 
}


// * Draw a pixel matrix to the stdout with a single write.Color escape 
// * sequences are only sent when the color changes from the previous cell. 
// * param: pixels: the pixel matrix (width * rows cells). 
// * param: previous: the pixel matrix drawn just before, right above the 
// *        cursor, or NULL. When given, the cursor goes back to its top and 
// *        only the cells that changed are drawn again. 
// * param: render: size, position and color mode of the matrix. 
// * return: 0 if no error occured and 1 otherwise. 
int draw_pixels(PIXEL_t* pixels, PIXEL_t* previous, const RENDER_t* render)
{
    OUTPUT_BUFFER_t out; 
    PIXEL_t*        pixel; 
    size_t          width; 
    size_t          rows; 
    size_t          offset; 
    size_t          x, y; 
    size_t          skipped; 
    int             f_color; 
    int             b_color; 

    width = render->width; 
    rows = render->rows; 
    offset = render->offset; 

    out.len = 0; 
    out.data = malloc(rows * (offset + width * CELL_MAX_LEN + ROW_END_MAX_LEN) + FRAME_EXTRA_LEN); 
    if (!out.data)
        return 1; 

    // Ask the terminal to display the whole frame at once (no flickering) 
    // and go back to the top left of the previous frame. 
    if (previous)
    {
        append_str(&out, ESC_SYNC_BEGIN "\x1b["); 
        append_number(&out, rows); 
        append_str(&out, "A\r"); 
    }

    for (y = 0; y < rows; y++)
    {
        // Colors are reset at the end of each row so the background color 
        // doesn't leak on the rest of the line, start again from unknown. 
        f_color = -1; 
        b_color = -1; 
        skipped = 0; 

        // The offset is written with spaces the first time, then skipped 
        // with a cursor move like the cells that didn't change. 
        if (previous)
            skipped = offset; 

        else
        {
            memset(out.data + out.len, ' ', offset); 
            out.len += offset; 
        }

        for (x = 0; x < width; x++)
        {
            pixel = pixels + y * width + x; 

            // Cells that didn't change are skipped by moving the cursor. 
            if (previous 
                && pixel->f_color == previous[y * width + x].f_color 
                && pixel->b_color == previous[y * width + x].b_color)
            {
                skipped++; 
                continue; 
            }

            if (skipped)
            {
                append_str(&out, "\x1b["); 
                append_number(&out, skipped); 
                out.data[out.len++] = 'C'; 
                skipped = 0; 
            }

            if (render->ascii)
                append_ascii_cell(&out, pixel, &f_color, render->colors); 

            else
                append_cell(&out, pixel, &f_color, &b_color, render->colors); 
        }

        // Reset the colors only if some were written (never in mono). 
        if (f_color != -1 || b_color != -1)
            append_str(&out, ESC_RESET); 

        append_str(&out, previous ? "\r\n" : "\n"); 
    }

    if (previous)
        append_str(&out, ESC_SYNC_END); 

    fwrite(out.data, 1, out.len, stdout); 
    fflush(stdout); 
    free(out.data); 
    return 0; 
}
