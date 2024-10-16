#include "image.h"


// * Create an IMG_t struct but doesn't initialize the PIXEL_t matrix. 
// * return: the pointer to the IMG_t struct of NULL in case of errors. 
IMG_t* create_img(char* filename)
{
    IMG_t*          new_img; 
    JPEG_HANDLER_t* handler; 
    size_t          height; 
    size_t          width; 
    size_t          channel_count; 

    handler = create_jpeg_handler(filename); 
    if (!handler)
        return NULL; 

    height = handler->cinfo.image_height; 
    width = handler->cinfo.image_width; 
    channel_count = handler->cinfo.num_components; 
    
    // TODO: Handle img with odd height. 
    if (height % 2 != 0)
    {
        free_jpeg_handler(handler);
        return NULL; 
    }

    // Allocate memory for the pixel row buffer to read the jpeg line by line. 
    handler->pixel_scan_row = malloc(sizeof(unsigned char) * width * channel_count); 
    if (!handler->pixel_scan_row)
    {
        free_jpeg_handler(handler); 
        return NULL; 
    }

    new_img = malloc(sizeof(IMG_t));
    if (!new_img)
    {
        free_jpeg_handler(handler); 
        return NULL; 
    }

    // Initialize new_img handler, width, height and channel count. 
    new_img->handler = handler; 
    new_img->height = height; 
    new_img->width = width; 
    new_img->channel_count = channel_count; 

    new_img->img = malloc(sizeof(PIXEL_t) * new_img->width * (new_img->height / 2)); 
    
    if (!new_img->img)
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

    if (img->handler)
    {
        free_jpeg_handler(img->handler); 
        img->handler = NULL; 
    }

    free(img); 
    return; 
}


// * Load the jpeg img into the IMG_t data structure. 
// * param: img: the IMG_t structure where JPEG data need to be inserted in. 
// * return: 0 if no error occured and 1 otherwise. 
int load_jpeg(IMG_t* img)
{
    int retval; 

    if(!jpeg_start_decompress(&(img->handler->cinfo)))
        return 1;

    while(img->handler->cinfo.output_scanline < img->handler->cinfo.image_height)
    {
        // Read one row of pixels of the image loaded in JPEG and stores it 
        // inside pixel_scan_row. 
        retval = jpeg_read_scanlines(&(img->handler->cinfo), &(img->handler->pixel_scan_row), 1); 
        if (!retval)
            return 1; 

        // Load the pixel row into the IMG_t struct. 
        load_pixelscan(img, img->handler->pixel_scan_row, img->handler->cinfo.output_scanline); 
    }

    // Free the decompression. 
    jpeg_finish_decompress(&(img->handler->cinfo)); 
    return 0; 
}


// * Load a pixel row scan into the correct pixel row of the IMG_t struct. 
// * param: img: the img where the pixel row need to be inserted. 
// * param: pixel_row_t: the pixel row that need to be inserted 
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
        if (img->channel_count < 3)
            // Only one channel mean grayscale image, r, g and b have the same 
            // value. 
            r = g = b =  pixel_scanline[i * 3];

        else
        {
            r = pixel_scanline[i * img->channel_count + 0]; 
            g = pixel_scanline[i * img->channel_count + 1]; 
            b = pixel_scanline[i * img->channel_count + 2]; 
        }

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

    total_matrix_len = img->width * (img->height / 2);

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
