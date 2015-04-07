// winBMP.cxx
// from : http://stackoverflow.com/questions/12200201/c-convert-text-file-of-integers-into-a-bitmap-image-file-in-bmp-format
/*\
 from :
 If biCompression equals BI_RGB and the bitmap uses 8 bpp or less, 
 the bitmap has a color table immediatelly following the BITMAPINFOHEADER structure. 
 The color table consists of an array of RGBQUAD values. The size of the array is given 
 by the biClrUsed member. If biClrUsed is zero, the array contains the maximum number of 
 colors for the given bitdepth; that is, 2^biBitCount colors.

 For uncompressed RGB formats, the minimum stride is always the image width in bytes, 
 rounded up to the nearest DWORD. You can use the following formula to calculate the stride:
    stride = ((((biWidth * biBitCount) + 31) & ~31) >> 3)

\*/

#include <windows.h>
#include <stdio.h>
#include <fstream>
#include "sprtf.hxx"
#include "bmp_utils.hxx"

static void check_me()
{
    int i;
    SPRTF("Any key to continue!\n");
    getchar();
    i = 0;
}


//////////////////////////////////////////////////////////////////////////////
// found in tools\BglView2\FsTrack.c, tools\dv32\DvRGB.c tools\dv32\ShowDIB2\SHOWDIB\Rgb.c
// tools\OpenAir\OA_BMP.cxx
// tools\poly-view\poly-view\poly-bmp.cxx, poly-utils.cxx, src\poly-bmp.cxx
// src\poly-utils.cxx, shpdisp\shp_bmp.cxx
// tools\Sudoku\Sudo_BMP.cxx, tools\testap3\testBmp.cxx
// tools\dv32\Dvdib.c - (DWORD)(BPP) + 31) >> 5)) << 2)
#define WIDTHBYTES(bits)      (((bits) + 31) / 32 * 4)

int get_BPP( int nBPP )
{
    int iBPP = 24;
    	// Fix Bits per Pixel
	if( nBPP <= 1 )
		iBPP = 1;
	else if( nBPP <= 4 )
		iBPP = 4;
	else if( nBPP <= 8 )
		iBPP = 8;
	else
		iBPP = 24;
    return iBPP;
}

int writeBMPHeaders(FILE *bmp_ptr, int cols, int rows, int nBPP)
{
    int iret = 0;
    size_t siz,res,total;
    nBPP = get_BPP(nBPP);
    int width = WIDTHBYTES(cols * nBPP);
    DWORD dwSizeInBytes = rows * width; // when your matrix contains RGB data)

    // fill in the headers
    BITMAPFILEHEADER bmfh;
    memset(&bmfh,0,sizeof(BITMAPFILEHEADER));
    bmfh.bfType = 0x4D42; // 'BM'
    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwSizeInBytes;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bmih;
    memset(&bmih,0,sizeof(BITMAPINFOHEADER));
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = cols;
    bmih.biHeight = rows;
    bmih.biPlanes = 1;
    bmih.biBitCount = nBPP;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = dwSizeInBytes; // have read can be ZERO for BI_RGB uncompressed
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    total = 0;
    siz = sizeof(bmfh);
    res = fwrite(&bmfh, 1, siz, bmp_ptr);
    if (res != siz) {
        SPRTF("Failed to write bmp file header!\n");
        return 1;
    }
    total += siz;
    siz = sizeof(bmih);
    res = fwrite(&bmih, 1, siz, bmp_ptr);
    if (res != siz) {
        SPRTF("Failed to write bmp file header!\n");
        return 1;
    }
    total += siz;
    //fwrite(&intmatrix, size, sizeof(int), bmp_ptr);
    SPRTF("Written %d bytes. BITMAPFILEHEADER %d + BITMAPINFOHEADER %d. Indicated file size %d\n",
        (int)total, sizeof(BITMAPFILEHEADER), bmih.biSize, bmfh.bfSize);
    total += dwSizeInBytes;
    SPRTF("plus image size %d, sets file size at %d bytes\n",
        dwSizeInBytes, (int)total );
    return iret;

}

// passed a buffer - byte array - either 255 or 0 for each pixel
int writeBMPImage(const char *file, int x, int y, unsigned char *buffer, size_t sbsiz)
{
    int iret = 0;
    int nBPP = 24;
    int row,col,soff,doff,z;
    //int stride = ((((x * 32) + 31) & ~31) >> 3);
    int width = WIDTHBYTES( x * nBPP );
    size_t bsiz = width * y;   // 3 bytes per pixel
    size_t res;
    unsigned char *src;
    unsigned char *dst;
    unsigned char *cbuf = (unsigned char *)malloc(bsiz);
    if (!cbuf) {
        SPRTF("Allocation of %d bytes FAILED!\n", (int)bsiz);
        return 1;
    }
    // fill to white
    memset(cbuf,255,bsiz);
    SPRTF("\nWriting BMP file %s, x=%d, y=%d, buf %d bytes\n", file, x, y, (int)bsiz);
    for (row = 0; row < y; row++) {
        for (col = 0; col < x; col++) {
            soff = (row * x) + col;
            if (soff > (int)sbsiz) {
                SPRTF( "ERROR: SOURCE: Doing row %d of %d, col %d of %d, got %d on %d offset\n",
                    row, y, col, x, (int)soff, (int)sbsiz);
                check_me();
                exit(1);
            }
            doff = (row * width) + (col * 3);
            if ((doff + 3) > (int)bsiz) {
                SPRTF( "ERROR: DESTINATION: Doing row %d of %d, col %d of %d, got %d on %d offset\n",
                    row, y, col, x, (int)doff, (int)bsiz);
                check_me();
                exit(1);
            }
            src = &buffer[soff];    // source
            dst = &cbuf[doff];      // destination
            for (z = 0; z < 3; z++) {
                dst[z] = *src;    // copy 3 bytes of color - 255 or 0
            }
        }
    }

    FILE *fp = fopen(file, "wb");
    if (!fp) {
        SPRTF("Can NOT create file %s!\n", file);
        iret = 1;
        goto cleanup;
    }
    iret = writeBMPHeaders( fp, x, y, nBPP ); 
    if (iret) {
        goto cleanup;
    }
    res = fwrite(cbuf,1,bsiz,fp);
    if (res != bsiz) {
        SPRTF("Failed to write color bits block!\n");
        iret = 1;
        goto cleanup;
    }
    SPRTF("Written BMP file %s, x=%d, y=%d\n", file, x, y);
cleanup:
    fclose(fp);
    free(cbuf);
    return iret;
}


int writeBMPHeaders_ORG(FILE *bmp_ptr, int rows, int cols)
{
    int iret = 0;
    size_t siz,res;

    DWORD dwSizeInBytes = rows*cols*4; // when your matrix contains RGBX data)

    // fill in the headers
    BITMAPFILEHEADER bmfh;
    bmfh.bfType = 0x4D42; // 'BM'
    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwSizeInBytes;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bmih;
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = cols;
    bmih.biHeight = rows;
    bmih.biPlanes = 1;
    bmih.biBitCount = 32;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = 0;
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    siz = sizeof(bmfh);
    res = fwrite(&bmfh, 1, siz, bmp_ptr);
    if (res != siz) {
        SPRTF("Failed to write bmp file header!\n");
        return 1;
    }

    siz = sizeof(bmih);
    res = fwrite(&bmih, 1, siz, bmp_ptr);
    if (res != siz) {
        SPRTF("Failed to write bmp file header!\n");
        return 1;
    }

    //fwrite(&intmatrix, size, sizeof(int), bmp_ptr);

    return iret;

}

// from : http://stackoverflow.com/questions/12200201/c-convert-text-file-of-integers-into-a-bitmap-image-file-in-bmp-format
// read in the matrix using  arma::Mat 
// maybe from http://arma.sourceforge.net/docs.html
// API Reference for Armadillo 4.100 a C++ linear algebra library
int process_matrix( const char *file )
{
    // std::ifstream f("MYFILE.TXT", std::ios::in); 
    // intmatrix.load(f);
    // or
    //int x;
    std::ifstream f(file, std::ios::in);
    // while(f << x) intmatrix >> x;
    // and resize the matrix afterwards with 
    // intmatrix.reshape(2,3);
    // 
    //fwrite(&intmatrix, size, sizeof(int), bmp_ptr);
    return 0;
}

        
int writeBMP1bit( const char *file, int w, int h, int bpp, unsigned char *bytes,
        size_t bufsize)
{
    size_t res;
    unsigned char  byte[1];  /* 1 byte  */
    unsigned short word[1];  /* 2 bytes */
    unsigned long  dword[1], /* 4 bytes */
                   FileHeaderSize=14, 
                   InfoHeaderSize=40, // header_bytes
                   PaletteSize,
                   BytesPerRow,
                   FileSize,
                   OffBits,
                   BytesSize, // bytes in image portion
                   /* in pixels */
                   Width= w, 
                   Height=h;

    //-----------------------  
    PaletteSize = (unsigned long)(pow(2.0,(double)bpp)*4); // = 8 = number of bytes in palette
    BytesPerRow = (((Width * bpp)+31)/32)*4; 
    BytesSize   = BytesPerRow * Height;
    FileSize    = FileHeaderSize+InfoHeaderSize+PaletteSize+BytesSize;
    OffBits     = FileHeaderSize+ InfoHeaderSize+ PaletteSize;

    SPRTF("Image width %d, height %d, bpp %d\n", w, h, bpp);
    SPRTF("FileHeader :\t%d\n", FileHeaderSize);
    SPRTF("InfoHeader :\t%d\n", InfoHeaderSize);
    SPRTF("PaletteSize:\t%d\n", PaletteSize);
    SPRTF("OffBits    :\t%d\n", OffBits);
    SPRTF("BytesPerRow:\t%d\n", BytesPerRow);
    SPRTF("BytesSize  :\t%d\n", BytesSize);
    SPRTF("FileSize   :\t%d\n", FileSize);
    if (BytesSize != (unsigned long)bufsize) {
        SPRTF("Internal Error: Passed buffer size %d not agree with calculated size %d!\n",
            (int)bufsize, BytesSize );
        return 1;
    }
    //--------------------------      
    FILE *fp = fopen(file, "wb"); /* b - binary mode */
    if (!fp) {
        SPRTF("Failed to open/create file %s\n",file);
        return 1;
    }
    /* bmp file header */
    word[0]=19778;                                         fwrite(word,1,2,fp); /* file Type signature = BM */
    dword[0]=FileSize;                                     fwrite(dword,1,4,fp); /* FileSize */
    word[0]=0;                                             fwrite(word,1,2,fp); /* reserved1 */
    word[0]=0;                                             fwrite(word,1,2,fp); /* reserved2 */
    dword[0]=OffBits;                                      fwrite(dword,1,4,fp); /* OffBits */
    dword[0]=InfoHeaderSize;                               fwrite(dword,1,4,fp); 
    dword[0]=Width;                                        fwrite(dword,1,4,fp); 
    dword[0]=Height;                                       fwrite(dword,1,4,fp); 
    word[0]=1;                                             fwrite(word,1,2,fp); /* planes */
    word[0]=1;                                             fwrite(word,1,2,fp); /* Bits of color per pixel */
    dword[0]=0;                                            fwrite(dword,1,4,fp); /* compression type */
    dword[0]=0;                                            fwrite(dword,1,4,fp); /* Image Data Size, set to 0 when no compression */
    dword[0]=0;                                            fwrite(dword,1,4,fp); /*  */
    dword[0]=0;                                            fwrite(dword,1,4,fp); /*  */
    dword[0]=2;                                            fwrite(dword,1,4,fp); /*  number of used coloors*/
    dword[0]=0;                                            fwrite(dword,1,4,fp); /*  */
               
    /*  color table (palette) = 2 colors as a RGBA */
    /* color 0 = white */
    byte[0]=255;                                            fwrite(byte,1,1,fp); /* R */                                     
    byte[0]=255;                                            fwrite(byte,1,1,fp); /* G */    
    byte[0]=255;                                            fwrite(byte,1,1,fp); /* B */ 
    byte[0]=255;                                            fwrite(byte,1,1,fp); /* A */  
    /* color 1 = black */   
    byte[0]=0;                                              fwrite(byte,1,1,fp); /* R */                                     
    byte[0]=0;                                              fwrite(byte,1,1,fp); /* G */    
    byte[0]=0;                                              fwrite(byte,1,1,fp); /* B */    
    byte[0]=255;                                            fwrite(byte,1,1,fp); /* A */ 

    /* pixel color  data  */
    res = fwrite(bytes,1,bufsize,fp);
    if (res != bufsize) {
        fclose(fp);
        SPRTF("Error: Failed to write %d bytes pixel data! Got result %d\n",
            (int)bufsize, (int)res );
        return 1;
    }

    fclose(fp);
    SPRTF("BMP 1-bit file %s written...\n", file);
    // getchar();
    return 0;
}


// eof

