#include "image.h"


IMG_t *create_image(int height, int width)
{
    int         i; 
    int         j; 

    IMG_t*      new_img; 
    PIXEL_t**   pixel_col; 
    PIXEL_t*    pixel_row; 

    if (height % 2 != 0)
        return NULL; 

    new_img = malloc(sizeof(IMG_t)); 
    pixel_col = malloc(sizeof(PIXEL_t*) * height / 2); 

    if (!pixel_col || !new_img)
        return NULL; 

    new_img->width = width; 
    new_img->height = height; 

    for (i = 0; i < height / 2; i++)
    {
        pixel_row = malloc(sizeof(PIXEL_t) * width); 

        if (!pixel_row)
            return NULL; 

        for (j = 0; j < width; j++)
        {
            (pixel_row + j)->b_color = BLACK; 
            (pixel_row + j)->f_color = WHITE; 
        }

        pixel_col[i] = pixel_row; 
    }

    new_img->img = pixel_col; 
    return new_img; 
}

void draw_image(IMG_t *img)
{
    size_t i; 
    size_t j;

    for (i = 0; i < img->height / 2; i++)
    {

        for(j = 0; j < img->width; j++)
            draw_pixel(&(img->img[i][j])); 

        printf("\n"); 
    }
}


