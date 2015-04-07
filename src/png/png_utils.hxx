// png_utils.hxx
#ifndef _PNG_UTILS_HXX_
#define _PNG_UTILS_HXX_

extern void stub();
#ifdef USE_PNG_LIB

/*\
 * int writePNGImage(char *file, int width, int height, int bit_depth, int color_type,
 *   unsigned char *buf ) 
 *
 * NOTE: The buffer passed is a simple char array, of width * height
 * The char is either ON (255 - white) or OFF (0 - black)
 *
\*/
extern int writePNGImage(char *file, int width, int height, int bit_depth, int color_type,
    unsigned char *buf );


#endif // #ifdef USE_PNG_LIB
#endif // #ifndef _PNG_UTILS_HXX_
// eof
