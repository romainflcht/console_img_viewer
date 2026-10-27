#include "image/image.h"


// * Compute the size of the image once fitted inside the terminal while
// * keeping its aspect ratio. Images are only shrunk, never enlarged, but 
// * their height is corrected when the font cells are not twice as high as 
// * wide so the image is not stretched. 
// * param: width: the image width, replaced by the fitted width.
// * param: height: the image height, replaced by the fitted height.
static void fit_to_terminal(size_t* width, size_t* height)
{
    TERMINAL_t term; 
    size_t max_width;
    size_t max_height;
    size_t new_width;
    size_t new_height;

    if (get_terminal_size(&term))
        return; 

    // Each character cell holds 2 pixels vertically (▄), so a pixel is 
    // cell_width wide and cell_height / 2 high. When the font size is known, 
    // scale the height so the pixels displayed have the right proportions. 
    if (term.cell_width && term.cell_height)
    {
        new_height = (*height * 2 * term.cell_width + term.cell_height / 2) / term.cell_height; 
        *height = new_height ? new_height : 1; 
    }

    max_width = term.cols;
    max_height = term.rows > RESERVED_ROWS ? (term.rows - RESERVED_ROWS) * 2 : 2;

    if (*width <= max_width && *height <= max_height)
        return; 

    // Keep the most limiting dimension and scale the other one.
    if (*width * max_height > *height * max_width)
    {
        new_width = max_width;
        new_height = (*height * max_width + *width / 2) / *width;
    }

    else
    {
        new_height = max_height;
        new_width = (*width * max_height + *height / 2) / *height;
    }

    *width = new_width ? new_width : 1;
    *height = new_height ? new_height : 1;
}


// * Create an IMG_t struct from a JPEG or PNG file, resized to fit inside the
// * current terminal.
// * param: filename: path to the image.
// * return: the pointer to the IMG_t struct of NULL in case of errors. 
IMG_t* create_img(char* filename)
{
    IMG_t*          new_img; 
    unsigned char*  rgb;
    unsigned char*  resized;
    size_t          src_width;
    size_t          src_height;
    size_t          width; 
    size_t          height; 
    size_t          channel_count; 
    size_t          line;

    switch (get_img_format(filename))
    {
        case FORMAT_JPEG:
            rgb = decode_jpeg(filename, &src_width, &src_height, &channel_count);
            break;

        case FORMAT_PNG:
            rgb = decode_png(filename, &src_width, &src_height, &channel_count);
            break;

        default:
            return NULL;
    }

    if (!rgb)
        return NULL; 

    // Shrink the image if it doesn't fit inside the terminal.
    width = src_width;
    height = src_height;
    fit_to_terminal(&width, &height);

    if (width != src_width || height != src_height)
    {
        resized = resize_rgb(rgb, src_width, src_height, width, height);
        free(rgb);
        if (!resized)
            return NULL;

        rgb = resized;
    }

    new_img = malloc(sizeof(IMG_t));
    if (!new_img)
    {
        free(rgb);
        return NULL; 
    }

    // Initialize new_img width, height and channel count.
    new_img->height = height; 
    new_img->width = width; 
    new_img->src_height = src_height;
    new_img->src_width = src_width;
    new_img->channel_count = channel_count; 

    // Allocate memory for the pixel matrix and add one pixel if the
    // image height is odd. calloc leaves the missing bottom row black.
    new_img->img = calloc(new_img->width * ((new_img->height + 1) / 2), sizeof(PIXEL_t));
    if (!new_img->img)
    {
        free(rgb);
        free_img(new_img); 
        return NULL; 
    }

    for (line = 0; line < height; line++)
        load_pixelscan(new_img, rgb + line * width * 3, line + 1);

    free(rgb);
    return new_img; 
}


// * Free the IMG_t structure correctly.
// * param: img: the image that need to be freed. 
void free_img(IMG_t* img)
{
    if (!img)
        return; 

    if (img->img)
    {
        free(img->img);
        img->img = NULL; 
    }

    free(img); 
    return; 
}


// * Detect the format of an image file by reading its signature.
// * param: filename: path to the image.
// * return: the format of the image or FORMAT_UNKNOWN.
IMG_FORMAT_t get_img_format(char* filename)
{
    FILE*           file;
    unsigned char   header[PNG_SIGNATURE_LEN];
    size_t          len;

    file = fopen(filename, "rb");
    if (!file)
    {
        printf("File \"%s\" not found, please check your path.\n", filename);
        return FORMAT_UNKNOWN;
    }

    len = fread(header, 1, sizeof(header), file);
    fclose(file);

    if (is_jpeg(header, len))
        return FORMAT_JPEG;

    if (is_png(header, len))
        return FORMAT_PNG;

    printf("File \"%s\" is not a supported image (JPEG or PNG).\n", filename);
    return FORMAT_UNKNOWN;
}


// * Resize a RGB buffer using area averaging.
// * param: src: the RGB buffer to resize.
// * param: src_width: width of the source buffer.
// * param: src_height: height of the source buffer.
// * param: dst_width: width of the resized buffer.
// * param: dst_height: height of the resized buffer.
// * return: the resized RGB buffer (that need to be freed) or NULL in case of
// *         errors.
unsigned char* resize_rgb(unsigned char* src, size_t src_width, size_t src_height, size_t dst_width, size_t dst_height)
{
    unsigned char*  dst;
    unsigned char*  src_pixel;
    size_t          x, y;
    size_t          sx, sy;
    size_t          x0, x1, y0, y1;
    size_t          sum[3];
    size_t          count;
    size_t          c;

    dst = malloc(sizeof(unsigned char) * dst_width * dst_height * 3);
    if (!dst)
        return NULL; 

    for (y = 0; y < dst_height; y++)
    {
        // Source rows covered by this destination row.
        y0 = y * src_height / dst_height;
        y1 = (y + 1) * src_height / dst_height;
        if (y1 <= y0)
            y1 = y0 + 1;

        for (x = 0; x < dst_width; x++)
        {
            // Source columns covered by this destination column.
            x0 = x * src_width / dst_width;
            x1 = (x + 1) * src_width / dst_width;
            if (x1 <= x0)
                x1 = x0 + 1;

            sum[0] = sum[1] = sum[2] = 0;
            count = (x1 - x0) * (y1 - y0);

            for (sy = y0; sy < y1; sy++)
            {
                for (sx = x0; sx < x1; sx++)
                {
                    src_pixel = src + (sy * src_width + sx) * 3;
                    for (c = 0; c < 3; c++)
                        sum[c] += src_pixel[c];
                }
            }

            for (c = 0; c < 3; c++)
                dst[(y * dst_width + x) * 3 + c] = (sum[c] + count / 2) / count;
        }
    }

    return dst;
}


// * Load a pixel row scan into the correct pixel row of the IMG_t struct. 
// * param: img: the img where the pixel row need to be inserted. 
// * param: pixel_row_t: the RGB pixel row that need to be inserted
// *                     (unsigned char ptr). 
// * param: line: the row where the pixel row need to be inserted. 
void load_pixelscan(IMG_t* img, pixel_row_t* pixel_scanline, unsigned int line)
{
    size_t              i; 
    int                 hex; 
    unsigned short int  r; 
    unsigned short int  g; 
    unsigned short int  b; 

    for (i = 0; i < img->width; i++)
    {
        r = pixel_scanline[i * 3 + 0];
        g = pixel_scanline[i * 3 + 1];
        b = pixel_scanline[i * 3 + 2];

        rgb_to_hex(r, g, b, &hex); 
        change_pixel_color((img->img + i + (line - 1) / 2 * img->width), hex, (line - 1) % 2); 
    }

    return; 
}


// * Draw the IMG_t pixel matrix to the stdout. 
// * param: img: image that will be drawn to the screen. 
void draw_image(IMG_t *img)
{
    if (draw_pixels(img->img, img->width, (img->height + 1) / 2))
        return; 

    // finish the print with some information. 
    if (img->width != img->src_width || img->height != img->src_height)
        printf(
            "\x1b[1mIMG SIZE : %ldx%ld (resized to %ldx%ld) with %ld channel(s) color.\x1b[0m\n\n",
            img->src_width,
            img->src_height,
            img->width,
            img->height,
            img->channel_count
        );

    else
        printf(
            "\x1b[1mIMG SIZE : %ldx%ld with %ld channel(s) color.\x1b[0m\n\n",
            img->width,
            img->height,
            img->channel_count
        );
}
