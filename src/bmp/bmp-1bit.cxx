/* 
  C console program
  
  A plain black and white bitmap requires only a single bit to document each pixel. 
  the bitmap is monochrome, and the palette contains two entries. 
  each bit in the bitmap array represents a pixel. if the bit is clear, 
  the pixel is displayed with the color of the first entry in the palette; 
  if the bit is set, the pixel has the color of the second entry in the table.
  This type of bitmap has a very small file size, and doesn't require a palette.
  
  based on program bmpsuite.c by Jason Summers
  http://entropymine.com/jason/bmpsuite/
  
  Adam Majewski
  fraktal.republika.pl
       
*/        

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "sprtf.hxx"

static const char *def_out = "temp1bit.bmp";
static char out_file[264];

static void set_bmp_def_out()
{
    strcpy(out_file, get_log_path());
    strcat(out_file, def_out);
}

unsigned char *genImage( int w, int h, int bpp )
{
    unsigned char bit;
    unsigned long iByte, iy, ix, offSet, bitNumber; /* bits are numbered from 0 to 7 */
    unsigned long BytesPerRow = (((w * bpp)+31)/32)*4; 
    unsigned long BytesSize = BytesPerRow * h;
    unsigned char *bytes = (unsigned char *)malloc(BytesSize);
    if (!bytes) {
        printf("Memory allocation FAILED!\n");
        exit(1);
    }

    /* clear all bits */
    for(iByte=0; iByte< BytesSize; iByte++)
        bytes[iByte]=0;
        
    /* compute color of pixel and save it to bytes array */
    srand((unsigned int)time(0));
    for(iy = 0 ; iy < (unsigned long) h; iy++) {
        ix = 0;
        for(iByte = 0; iByte < BytesPerRow; iByte++) {
           bitNumber = 7;
           bit = 1;
           offSet = (iy * BytesPerRow) + iByte;
           if (offSet >= BytesSize) {
               printf("Internal ERROR!\n");
               exit(1);
           }
           while (bitNumber<8) { 
                //if (sqrt((double)((ix*ix)+(iy*iy)))>500.0)
                //    bytes[iByte]+=(int)pow(2.0,(double)bitNumber);
               if (rand() >= (RAND_MAX / 2))
                   bytes[offSet] |= bit;
                bitNumber-=1;
                ix += 1;
                bit = bit << 1;
           }
        }
    }
    return bytes;
}
        
int writeBMP1bit( const char *file, int w, int h, int bpp, unsigned char *bytes )
{
    unsigned char byte[1];
    unsigned short word[1]; /* 2 bytes */
    unsigned long  dword[1], /* 4 bytes */
                   /* bpp=1, */
                   /* in bytes */
                   FileHeaderSize=14, 
                   InfoHeaderSize=40, // header_bytes
                   PaletteSize=(unsigned long)pow(2.0,(double)bpp)*4, // = 8 = number of bytes in palette
                   BytesPerRow,
                   FileSize,
                   OffBits,
                   BytesSize, // bytes in image portion
                   /* in pixels */
                   Width= w, 
                   Height=h;
    //-----------------------   
    printf("Image width %d, height %d, bpp %d\n", w, h, bpp);
    BytesPerRow = (((Width * bpp)+31)/32)*4; 
    printf("BytesPerRow = %d\n", (int)BytesPerRow);
    BytesSize = BytesPerRow * Height;
    printf("BytesSize   = %d\n", (int)BytesSize);
    FileSize = FileHeaderSize+InfoHeaderSize+PaletteSize+BytesSize;
    printf("FileSize    = %d\n", (int)FileSize);
    OffBits= FileHeaderSize+ InfoHeaderSize+ PaletteSize;
    printf("OffBits     = %d\n", (int)OffBits);

    //--------------------------      
    FILE *fp = fopen(file, "wb"); /* b - binary mode */
    if (!fp) {
        printf("Failed to open/create file %s\n",file);
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
    fwrite(bytes,1,BytesSize,fp);
    // free(bytes);
    fclose(fp);
    printf("file %s written...\n", file);
    //getchar();
    return 0;
 }

 int main(int argc, char **argv)
{
    int w = 1000;
    int h = 1000;
    int bpp = 1;
    int i;
    char *arg;
    int iret;
    unsigned char *bytes;
    set_bmp_def_out();
    if (argc < 2) {
        printf("Generating a random %d x %d monochrome bitmap file,\n", w, h );
        printf("written to the output '%s'.\n", out_file);
    } else {
        for (i = 1; i < argc; i++) {
            arg = argv[i];
            if ((*arg == '-')||(*arg == '?')) {
                printf("Only optional input allowed is the name of an output bmp file.\n");
                printf("The default output is '%s'\n", out_file);
                printf("Generates a random %d x %d monochrome bitmap file,\n", w, h );
                printf("written to the output '%s'.\n", out_file);
                return 1;
            } else {
                strcpy(out_file, arg);
            }
        }
    }

    bytes = genImage(w, h, bpp );
    iret = writeBMP1bit( out_file, w, h, bpp, bytes );
    free(bytes);
    return iret;
 }

// eof
