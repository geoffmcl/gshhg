// png_utils.cxx

#include <stdio.h>
#include <stdlib.h> // malloc()...
#include "png_utils.hxx"

void stub()
{

}

#ifdef USE_PNG_LIB
#include <png.h>
#include "sprtf.hxx"

/*\
 * int writePNGImage(char *file, int width, int height, int bit_depth, int color_type,
 *   unsigned char *buf ) 
 *
 * NOTE: The buffer passed is a simple char array, of width * height
 * The char is either ON (255 - white) or OFF (0 - black)
 *
\*/
int writePNGImage(char *file, int width, int height, int bit_depth, int color_type,
    unsigned char *buf ) 
{
    int y, x, z;
    unsigned char ch;
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr) {
        SPRTF("Error: failed to create write structure!\n");
        return 1;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        SPRTF("Error: failed to create info structure!\n");
        return 1;
    }
    FILE *fp = fopen(file, "wb");
    if (!fp) {
        SPRTF("Can NOT create file %s!\n", file);
        return 1;
    }

    SPRTF("\nWriting PNG to %s, width %d, height %d, bit %d, color %d\n", file, width, height, bit_depth, color_type);

    png_init_io(png_ptr, fp);

    /* write header */
    png_set_IHDR(png_ptr, info_ptr, width, height,
        bit_depth, color_type, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    /* write bytes */
    png_bytep *row_pointers;
    // <allocate row_pointers and store each row of your image in it>
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    if (!row_pointers) {
        SPRTF("Memory allocation failed!\n");
        return 1;
    }

    // start first, work upwards
    for (y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
        if (!row_pointers[y]) {
            SPRTF("Memory allocation failed!\n");
            return 1;
        }
        // copy the data into the row pointer
        png_byte* row = row_pointers[y];
        // start first column, work upwards
        for (x = 0; x < width; x++) {
            png_byte* dst = &row[x*4];
            // this is upside down
            //png_byte* src = &buf[(y*width)+x];
            // try inverting the source pointer
            // png_byte* src = &buf[((height - 1 - y) * width) + (width - 1 - x)];
            png_byte* src = &buf[((height - 1 - y) * width) + x];
            // Set the RGBA values - but sets lines to soft gray
            ch = *src;
            //for (z = 0; z < 4; z++)
            // Maybe only set RGB
            for (z = 0; z < 3; z++)
                dst[z] = ch;
            // now lines a black, but background is soft gray
            // try if (ch) dst[3] = 0; // NO sent the whole image gray UGLY ;=((
        }
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation */
    for (y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);
    fclose(fp); // close file
    SPRTF("Written PNG to %s, width %d, height %d, bit %d, color %d\n", file, width, height, bit_depth, color_type);

    return 0;
}




#endif //#ifdef USE_PNG_LIB
// eof
