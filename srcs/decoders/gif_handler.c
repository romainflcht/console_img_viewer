#include "decoders/gif_handler.h"


// * Check if the header bytes match the GIF signature.
// * param: header: the first bytes of the file.
// * param: len: the number of bytes available in header.
// * return: 1 if the header is a GIF signature and 0 otherwise.
int is_gif(const unsigned char* header, size_t len)
{
    if (len < GIF_SIGNATURE_LEN)
        return 0;

    return !memcmp(header, GIF87_STAMP, GIF_SIGNATURE_LEN)
        || !memcmp(header, GIF89_STAMP, GIF_SIGNATURE_LEN);
}


// * Get a readable message for a giflib error code. 
// * param: error: the giflib error code. 
// * return: the error message. 
static const char* gif_error(int error)
{
    const char* message; 

    message = GifErrorString(error); 
    return message ? message : "Unknown error"; 
}


// * Fill a rectangle of the canvas with black.
// * param: canvas: the RGB canvas.
// * param: canvas_width: width of the canvas.
// * param: left, top, right, bottom: the rectangle, already clipped.
static void clear_rect(unsigned char* canvas, size_t canvas_width, size_t left, size_t top, size_t right, size_t bottom)
{
    size_t y;

    for (y = top; y < bottom; y++)
        memset(canvas + (y * canvas_width + left) * 3, 0, (right - left) * 3);
}


// * Draw a GIF frame over the canvas, transparent pixels are skipped.
// * param: gif: the GIF file.
// * param: frame: the frame to draw.
// * param: canvas: the RGB canvas.
// * param: transparent: palette index of the transparent color or -1.
// * param: left, top, right, bottom: the frame rectangle, already clipped.
static void draw_frame(GifFileType* gif, SavedImage* frame, unsigned char* canvas, int transparent, size_t left, size_t top, size_t right, size_t bottom)
{
    ColorMapObject* color_map;
    GifColorType*   color;
    unsigned char*  pixel;
    size_t          x, y;
    int             index;

    // A frame uses its own palette or the global one.
    color_map = frame->ImageDesc.ColorMap ? frame->ImageDesc.ColorMap : gif->SColorMap;
    if (!color_map || !frame->RasterBits)
        return;

    for (y = top; y < bottom; y++)
    {
        for (x = left; x < right; x++)
        {
            index = frame->RasterBits[(y - frame->ImageDesc.Top) * frame->ImageDesc.Width + (x - frame->ImageDesc.Left)];
            if (index == transparent || index >= color_map->ColorCount)
                continue;

            color = color_map->Colors + index;
            pixel = canvas + (y * gif->SWidth + x) * 3;
            pixel[0] = color->Red;
            pixel[1] = color->Green;
            pixel[2] = color->Blue;
        }
    }
}


// * Decode every frame of a GIF image. Each frame is composed over the
// * previous ones (following its disposal mode) and given to the callback.
// * Transparent pixels are black.
// * param: filename: path to the GIF image.
// * param: callback: function called with each decoded frame.
// * param: user: pointer given to the callback.
// * return: 0 if at least one frame was decoded and 1 otherwise.
int decode_gif(char* filename, FRAME_CALLBACK_t callback, void* user)
{
    GifFileType*            gif;
    GraphicsControlBlock    gcb;
    SavedImage*             frame;
    unsigned char*          canvas;
    unsigned char*          previous;
    size_t                  canvas_len;
    size_t                  left, top, right, bottom;
    int                     error;
    int                     delay_ms;
    int                     i;
    int                     retval;

    gif = DGifOpenFileName(filename, &error);
    if (!gif)
    {
        printf("Unable to read GIF \"%s\": %s\n", filename, gif_error(error));
        return 1;
    }

    // A truncated GIF still keeps the frames decoded before the error.
    if (DGifSlurp(gif) != GIF_OK && gif->ImageCount == 0)
    {
        printf("Unable to decode GIF \"%s\": %s\n", filename, gif_error(gif->Error));
        DGifCloseFile(gif, &error);
        return 1;
    }

    canvas_len = (size_t)gif->SWidth * gif->SHeight * 3;
    canvas = calloc(canvas_len, 1);
    previous = malloc(canvas_len);
    if (!canvas_len || !canvas || !previous)
    {
        free(canvas);
        free(previous);
        DGifCloseFile(gif, &error);
        return 1;
    }

    retval = 1;
    for (i = 0; i < gif->ImageCount; i++)
    {
        frame = gif->SavedImages + i;

        // Frames without a graphics control block have no transparency and
        // no delay.
        if (DGifSavedExtensionToGCB(gif, i, &gcb) != GIF_OK)
        {
            gcb.DisposalMode = DISPOSAL_UNSPECIFIED;
            gcb.DelayTime = 0;
            gcb.TransparentColor = NO_TRANSPARENT_COLOR;
        }

        // Clip the frame rectangle to the canvas.
        left = frame->ImageDesc.Left < gif->SWidth ? frame->ImageDesc.Left : gif->SWidth;
        top = frame->ImageDesc.Top < gif->SHeight ? frame->ImageDesc.Top : gif->SHeight;
        right = frame->ImageDesc.Left + frame->ImageDesc.Width;
        bottom = frame->ImageDesc.Top + frame->ImageDesc.Height;
        right = right < (size_t)gif->SWidth ? right : (size_t)gif->SWidth;
        bottom = bottom < (size_t)gif->SHeight ? bottom : (size_t)gif->SHeight;

        if (gcb.DisposalMode == DISPOSE_PREVIOUS)
            memcpy(previous, canvas, canvas_len);

        draw_frame(gif, frame, canvas, gcb.TransparentColor, left, top, right, bottom);

        delay_ms = gcb.DelayTime * 10;
        if (delay_ms < GIF_MIN_DELAY_MS)
            delay_ms = GIF_DEFAULT_DELAY_MS;

        if (callback(canvas, gif->SWidth, gif->SHeight, 3, delay_ms, user))
        {
            retval = 1;
            break;
        }

        retval = 0;

        // Prepare the canvas for the next frame.
        if (gcb.DisposalMode == DISPOSE_BACKGROUND)
            clear_rect(canvas, gif->SWidth, left, top, right, bottom);

        else if (gcb.DisposalMode == DISPOSE_PREVIOUS)
            memcpy(canvas, previous, canvas_len);
    }

    free(canvas);
    free(previous);
    DGifCloseFile(gif, &error);
    return retval;
}
