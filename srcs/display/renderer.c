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


// * Append a "r;g;b" color to the output buffer.
// * param: out: the output buffer.
// * param: hex: the color to append.
static void append_color(OUTPUT_BUFFER_t* out, int hex)
{
    int r, g, b;

    hex_to_rgb(hex, &r, &g, &b);
    append_u8(out, r);
    out->data[out->len++] = ';';
    append_u8(out, g);
    out->data[out->len++] = ';';
    append_u8(out, b);
}


// * Append one cell to the output buffer, with the escape sequence needed to
// * switch from the current colors to the cell colors.
// * param: out: the output buffer.
// * param: pixel: the cell to append.
// * param: f_color: the current foreground color, -1 if unknown.
// * param: b_color: the current background color, -1 if unknown.
static void append_cell(OUTPUT_BUFFER_t* out, PIXEL_t* pixel, int* f_color, int* b_color)
{
    int f_changed;
    int b_changed;

    f_changed = pixel->f_color != *f_color;
    b_changed = pixel->b_color != *b_color;

    // Both colors are set in a single escape sequence when possible.
    if (f_changed || b_changed)
    {
        append_str(out, "\x1b[");

        if (f_changed)
        {
            append_str(out, "38;2;");
            append_color(out, pixel->f_color);
        }

        if (f_changed && b_changed)
            out->data[out->len++] = ';';

        if (b_changed)
        {
            append_str(out, "48;2;");
            append_color(out, pixel->b_color);
        }

        out->data[out->len++] = 'm';
        *f_color = pixel->f_color;
        *b_color = pixel->b_color;
    }

    append_str(out, PIXEL_CHAR);
}


// * Draw a pixel matrix to the stdout with a single write. Color escape
// * sequences are only sent when the color changes from the previous cell.
// * param: pixels: the pixel matrix (width * rows cells).
// * param: width: number of cells per row.
// * param: rows: number of rows of cells.
// * return: 0 if no error occured and 1 otherwise.
int draw_pixels(PIXEL_t* pixels, size_t width, size_t rows)
{
    OUTPUT_BUFFER_t out;
    size_t          x, y;
    int             f_color;
    int             b_color;

    out.len = 0;
    out.data = malloc(rows * (width * CELL_MAX_LEN + ROW_END_MAX_LEN));
    if (!out.data)
        return 1;

    for (y = 0; y < rows; y++)
    {
        // Colors are reset at the end of each row so the background color
        // doesn't leak on the rest of the line, start again from unknown.
        f_color = -1;
        b_color = -1;

        for (x = 0; x < width; x++)
            append_cell(&out, pixels + y * width + x, &f_color, &b_color);

        append_str(&out, ESC_RESET "\n");
    }

    fwrite(out.data, 1, out.len, stdout);
    fflush(stdout);
    free(out.data);
    return 0;
}
