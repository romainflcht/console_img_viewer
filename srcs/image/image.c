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


// * Make room for one more frame in the IMG_t struct. 
// * param: img: the image. 
// * return: 0 if no error occured and 1 otherwise. 
static int grow_frames(IMG_t* img)
{
    PIXEL_t*    frames; 
    int*        delays; 
    size_t      capacity; 
    size_t      cell_count; 

    if (img->frame_count < img->frame_capacity)
        return 0; 

    capacity = img->frame_capacity ? img->frame_capacity * 2 : 1; 
    cell_count = img->width * ((img->height + 1) / 2); 

    frames = realloc(img->img, sizeof(PIXEL_t) * cell_count * capacity); 
    if (!frames)
        return 1; 

    img->img = frames; 

    delays = realloc(img->delays, sizeof(int) * capacity); 
    if (!delays)
        return 1; 

    img->delays = delays; 
    img->frame_capacity = capacity; 
    return 0; 
}


// * Add a decoded frame to the image (FRAME_CALLBACK_t). The first frame sets 
// * the image size, every frame is resized to fit inside the terminal. 
// * param: rgb: the frame in RGB (3 bytes per pixel). 
// * param: width: width of the frame. 
// * param: height: height of the frame. 
// * param: channel_count: channel count of the original image. 
// * param: delay_ms: how long the frame is displayed, in milliseconds. 
// * param: user: the IMG_t struct. 
// * return: 0 if no error occured and 1 otherwise. 
static int add_frame(unsigned char* rgb, size_t width, size_t height, size_t channel_count, int delay_ms, void* user)
{
    IMG_t*          img; 
    unsigned char*  resized; 
    PIXEL_t*        frame; 
    size_t          cell_count; 
    size_t          line; 

    img = user; 

    // The first frame gives the size of the image. 
    if (!img->frame_count)
    {
        img->src_width = width; 
        img->src_height = height; 
        img->channel_count = channel_count; 
        img->width = width; 
        img->height = height; 
        fit_to_terminal(&img->width, &img->height); 
    }

    // Shrink the frame if it doesn't fit inside the terminal. 
    resized = NULL; 
    if (img->width != width || img->height != height)
    {
        resized = resize_rgb(rgb, width, height, img->width, img->height); 
        if (!resized)
            return 1; 

        rgb = resized; 
    }

    if (grow_frames(img))
    {
        free(resized); 
        return 1; 
    }

    // The missing bottom row stays black if the image height is odd. 
    cell_count = img->width * ((img->height + 1) / 2); 
    frame = img->img + img->frame_count * cell_count; 
    memset(frame, 0, sizeof(PIXEL_t) * cell_count); 

    for (line = 0; line < img->height; line++)
        load_pixelscan(img, frame, rgb + line * img->width * 3, line + 1); 

    img->delays[img->frame_count] = delay_ms; 
    img->frame_count++; 
    free(resized); 
    return 0; 
}


// * Create an IMG_t struct from a JPEG, PNG, GIF or WebP file, resized to 
// * fit inside the current terminal. Animated images keep all their frames. 
// * param: filename: path to the image.
// * return: the pointer to the IMG_t struct of NULL in case of errors. 
IMG_t* create_img(char* filename)
{
    IMG_t*          new_img; 
    unsigned char*  rgb;
    size_t          width; 
    size_t          height; 
    size_t          channel_count; 
    int             retval; 

    new_img = calloc(1, sizeof(IMG_t)); 
    if (!new_img)
        return NULL; 

    switch (get_img_format(filename))
    {
        case FORMAT_JPEG:
            rgb = decode_jpeg(filename, &width, &height, &channel_count);
            retval = !rgb || add_frame(rgb, width, height, channel_count, 0, new_img); 
            free(rgb); 
            break;

        case FORMAT_PNG:
            rgb = decode_png(filename, &width, &height, &channel_count);
            retval = !rgb || add_frame(rgb, width, height, channel_count, 0, new_img); 
            free(rgb); 
            break;

        case FORMAT_GIF:
            retval = decode_gif(filename, add_frame, new_img); 
            break; 

        case FORMAT_WEBP:
            retval = decode_webp(filename, add_frame, new_img); 
            break; 

        default:
            retval = 1; 
            break; 
    }

    if (retval || !new_img->frame_count)
    {
        free_img(new_img); 
        return NULL; 
    }

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

    if (img->delays)
    {
        free(img->delays); 
        img->delays = NULL; 
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
    unsigned char   header[IMG_SIGNATURE_LEN];
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

    if (is_gif(header, len))
        return FORMAT_GIF;

    if (is_webp(header, len))
        return FORMAT_WEBP;

    printf("File \"%s\" is not a supported image (JPEG, PNG, GIF or WebP).\n", filename);
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


// * Load a pixel row scan into the correct pixel row of a frame. 
// * param: img: the img where the pixel row need to be inserted. 
// * param: frame: the pixel matrix of the frame. 
// * param: pixel_row_t: the RGB pixel row that need to be inserted
// *                     (unsigned char ptr). 
// * param: line: the row where the pixel row need to be inserted. 
void load_pixelscan(IMG_t* img, PIXEL_t* frame, pixel_row_t* pixel_scanline, unsigned int line)
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
        change_pixel_color((frame + i + (line - 1) / 2 * img->width), hex, (line - 1) % 2); 
    }

    return; 
}


// * Draw the IMG_t pixel matrix to the stdout, animated images are played 
// * when the output is a terminal. 
// * param: img: image that will be drawn to the screen. 
// * param: loop: 1 to play animations until Ctrl-C, 0 to play them once. 
// * return: 1 if the user pressed Ctrl-C during an animation, 0 otherwise. 
int draw_image(IMG_t *img, int loop)
{
    size_t  rows; 
    int     interrupted; 

    rows = (img->height + 1) / 2; 
    interrupted = 0; 

    // Animations can't be played when the output is redirected, only the 
    // first frame is written. 
    if (img->frame_count > 1 && isatty(STDOUT_FILENO))
        interrupted = play_animation(img->img, img->delays, img->frame_count, img->width, rows, loop); 

    else if (draw_pixels(img->img, NULL, img->width, rows))
        return 0; 

    // finish the print with some information. 
    printf("\x1b[1mIMG SIZE : %ldx%ld", img->src_width, img->src_height); 

    if (img->width != img->src_width || img->height != img->src_height)
        printf(" (resized to %ldx%ld)", img->width, img->height); 

    printf(" with %ld channel(s) color", img->channel_count); 

    if (img->frame_count > 1)
        printf(" and %ld frames", img->frame_count); 

    printf(".\x1b[0m\n\n"); 
    return interrupted; 
}
