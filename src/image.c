#include "image.h"


// * Create an IMG_t struct from a JPEG or PNG file. 
// * param: filename: path to the image.
// * return: the pointer to the IMG_t struct of NULL in case of errors. 
IMG_t* create_img(char* filename)
{
    IMG_t*          new_img; 
    unsigned char*  rgb;
    size_t          width; 
    size_t          height; 
    size_t          channel_count; 
    size_t          line;

    switch (get_img_format(filename))
    {
        case FORMAT_JPEG:
            rgb = decode_jpeg(filename, &width, &height, &channel_count);
            break;

        case FORMAT_PNG:
            rgb = decode_png(filename, &width, &height, &channel_count);
            break;

        default:
            return NULL;
    }

    if (!rgb)
        return NULL; 

    new_img = malloc(sizeof(IMG_t));
    if (!new_img)
    {
        free(rgb);
        return NULL; 
    }

    // Initialize new_img width, height and channel count.
    new_img->height = height; 
    new_img->width = width; 
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
    size_t i; 
    size_t total_matrix_len; 

    total_matrix_len = img->width * ((img->height + 1) / 2);

    for (i = 0; i < total_matrix_len; i++)
    {
        draw_pixel((img->img + i)); 

        // Go to the next line if we reach the end of the pixel row. 
        if (i % img->width == img->width - 1)
            printf("\n"); 
    }

    // finish the print with some information. 
    printf(
        "\x1b[1mIMG SIZE : %ldx%ld with %ld channel(s) color.\x1b[0m\n\n", 
        img->width, 
        img->height, 
        img->channel_count
    ); 
}
