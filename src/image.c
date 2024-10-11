#include "image.h"


// * Create an IMG_t struct and initialise the image with BLACK pixels. 
// * param: height: height of the image. 
// * param: width: width of the image. 
// * return: the pointer to the IMG_t struct of NULL in case of errors. 
IMG_t *create_img(char* filepath)
{
    size_t      i; 
    size_t      width; 
    size_t      height; 
    size_t      channel_count; 

    IMG_t*      new_img; 
    PIXEL_t*    new_pixels; 
    FILE*       jpeg_file; 
    int         retval; 


    jpeg_file = fopen(filepath, "rb");

    if (!jpeg_file)
        return NULL; 

    // Read the JPEG header to fetch height, width and color channel count 
    // of the image. 
    read_header_jpeg(jpeg_file, &height, &width, &channel_count); 
    fclose(jpeg_file);

    // TODO: Handle img with height no divisible by 2. 
    if (height % 2 != 0)
        return NULL; 

    new_img = malloc(sizeof(IMG_t)); 
    new_pixels = malloc(sizeof(PIXEL_t) * (height / 2) * width); 

    if (!new_img || !new_pixels)
        // TODO: Check which one failed and free the other. 
        return NULL; 

    new_img->height = height;
    new_img->width = width;
    new_img->channel_count = channel_count; 
    
    // Initialise every pixels with black. 
    for (i = 0; i < width * (height / 2); i++)
    {
        (new_pixels + i)->f_color = BLACK; 
        (new_pixels + i)->b_color = BLACK; 
    }

    new_img->img = new_pixels; 

    // Load the JPEG file into every pixel of the IMG_t struct. 
    retval = load_jpeg(new_img, filepath); 

    if (retval)
        // TODO: Free the all structure if the jpeg didn't load 
        // TODO: (Or leave it black? ). 
        return NULL; 

    return new_img; 
}


// * Free the IMG_t structure correctly.
// * param: img: the image that need to be freed. 
// * return: 0 in case of success and 1 in case of an error. 
int free_img(IMG_t* img)
{
    if (img)
    {
        if (img->img)
            free(img->img); 

        free(img); 
        return 0; 
    }

    return 1; 
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

    printf("\x1b[1mIMG SIZE : %ldx%ld with %ld channel(s) color.\x1b[0m\n", img->width, img->height, img->channel_count); 
}


// * Load the jpeg img into the IMG_t data structure. 
// * param: img: the IMG_t structure where JPEG data need to be inserted in. 
// * param: file_path: the file path to the JPEG image. 
// * return: 0 if no error occured and 1 otherwise. 
int load_jpeg(IMG_t* img, char* file_path)
{
    FILE*                         jpeg_file; 
    struct jpeg_decompress_struct jpeg; 
    struct jpeg_error_mgr         jpeg_err_handler; 
    pixel_row_t*                  pixel_scan_row;

    // Link the error handler to the decompressor. 
    jpeg.err = jpeg_std_error(&jpeg_err_handler);
    jpeg_file = fopen(file_path, "rb"); 

    if (!jpeg_file)
        return 1; 

    jpeg_create_decompress(&jpeg);
    jpeg_stdio_src(&jpeg, jpeg_file);
    jpeg_read_header(&jpeg, TRUE);

    jpeg_start_decompress(&jpeg);

    // Allocating memory to store one row of pixels during the scan. 
    pixel_scan_row = malloc(sizeof(pixel_row_t) * jpeg.output_width * jpeg.num_components);

    if (!pixel_scan_row)
        return 1; 

    while(jpeg.output_scanline < jpeg.image_height)
    {
        // Read one row of pixels of the image loaded in JPEG and stores it 
        // inside pixel_scan_row. 
        jpeg_read_scanlines(&jpeg, &pixel_scan_row, 1);

        // Load the pixel row into the IMG_t struct.  
        load_pixelscan(img, pixel_scan_row, jpeg.output_scanline); 
    }

    // Free the allocated memory.
    jpeg_finish_decompress(&jpeg);
    jpeg_destroy_decompress(&jpeg);
    free(pixel_scan_row);
    fclose(jpeg_file);

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

    // TODO: Handling modifing pixel for image with other color channel 
    // TODO: count than 3. 
    for (i = 0; i < img->width; i++)
    { 
        r = pixel_scanline[i * 3]; 
        g = pixel_scanline[i * 3 + 1]; 
        b = pixel_scanline[i * 3 + 2]; 
        rgb_to_hex(r, g, b, &hex); 

        change_pixel_color((img->img + i + (line - 1) / 2 * img->width), hex, (line - 1) % 2); 
    }
}

