// bmp-utils.hxx
#ifndef _BMP_UTILS_HXX_
#define _BMP_UTILS_HXX_
#ifdef   __cplusplus
extern "C" {
#endif

extern int writeBMPHeaders(FILE *bmp_ptr, int cols, int rows, int nBPP);
// passed a buffer - byte array - either 255 or 0 for each pixel
extern int writeBMPImage(const char *file, int x, int y, unsigned char *buffer, size_t sbsiz);

// passed a buffer - which is the bit image data - so the size of this MUST be exactly
// bpp = 1;
// BytesPerRow = (((Width * bpp)+31)/32)*4; 
// BytesSize = BytesPerRow * Height;
// PaletteSize = (unsigned long)(pow(2.0,(double)bpp)*4), // = 8 = number of bytes in palette
// This would result in a total file size of -
// FileSize = FileHeaderSize + InfoHeaderSize + PaletteSize + BytesSize;
// The size_t bufsize is only a 'check-sum', and call will FAILED if NOT in agreement with calculated
// size from the w, h, bpp passed.
extern int writeBMP1bit( const char *file, int w, int h, int bpp, unsigned char *bytes,
    size_t bufsize);

#ifdef   __cplusplus
}
#endif
#endif // #ifndef _BMP_UTILS_HXX_
// eof

