// write-bmp2.cxx
// from : http://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
//#ifdef _MSC_VER
//#include <Windows.h>
//#endif
#include <math.h>       // 
#include <stdio.h>
#include <time.h>
#include <fstream>      // std::ifstream, std::ofstream
#ifndef WIN32
#include <stdlib.h> // for rand, ...
#include <string.h> // for memset, ...
#endif
// mimeType = "image/bmp";
#define width 512
#define height 256
#define color 3

unsigned char waterfall[width][height][color];

unsigned char file[14] = {
    'B','M', // magic
    0,0,0,0, // size in bytes
    0,0, // app data
    0,0, // app data
    40+14,0,0,0 // start of data offset
};

unsigned char info[40] = {
    40,0,0,0, // info hd size
    0,0,0,0, // width
    0,0,0,0, // heigth
    1,0, // number color planes
    24,0, // bits per pixel
    0,0,0,0, // compression is none
    0,0,0,0, // image bits size
    0x13,0x0B,0,0, // horz resoluition in pixel / m
    0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
    0,0,0,0, // #colors in pallete
    0,0,0,0, // #important colors
    };


void write_bmp(std::ofstream &stream )
{
    int w = width;
    int h = height;

    int padSize  = (4 - (w %4)) % 4;
    int sizeData = (w * h * 3) + (h * padSize);
    int sizeAll  = sizeData + sizeof(file) + sizeof(info);

    file[ 2] = (unsigned char)( sizeAll    );
    file[ 3] = (unsigned char)( sizeAll>> 8);
    file[ 4] = (unsigned char)( sizeAll>>16);
    file[ 5] = (unsigned char)( sizeAll>>24);

    info[ 4] = (unsigned char)( w   );
    info[ 5] = (unsigned char)( w>> 8);
    info[ 6] = (unsigned char)( w>>16);
    info[ 7] = (unsigned char)( w>>24);

    info[ 8] = (unsigned char)( h    );
    info[ 9] = (unsigned char)( h>> 8);
    info[10] = (unsigned char)( h>>16);
    info[11] = (unsigned char)( h>>24);

    info[24] = (unsigned char)( sizeData    );
    info[25] = (unsigned char)( sizeData>> 8);
    info[26] = (unsigned char)( sizeData>>16);
    info[27] = (unsigned char)( sizeData>>24);

    stream.write( (char*)file, sizeof(file) );
    stream.write( (char*)info, sizeof(info) );

    unsigned char pad[3] = {0,0,0};

    for ( int y=0; y<h; y++ )
    {
        for ( int x=0; x<w; x++ )
        {
            //long red = lround( 255.0 * waterfall[x][y] );
            //long red = (long)(( 255.0 * waterfall[x][y] ) + 0.5);
            unsigned char red   = waterfall[x][y][0];
            unsigned char green = waterfall[x][y][1];
            unsigned char blue  = waterfall[x][y][2];

            unsigned char pixel[3];
            pixel[0] = (unsigned char)blue;
            pixel[1] = (unsigned char)green;
            pixel[2] = (unsigned char)red;

            stream.write( (char*)pixel, 3 );
        }
        if (padSize)
            stream.write( (char*)pad, padSize );
    }
    printf("Width %d, Height %d, datasize %d, sizeall %d, padsize %d\n",
        w, h,
        sizeData, sizeAll, padSize);

}

static size_t counts[256];
bool show_rand = false;

int main( int argc, char **argv)
{
	int iret = 0;
    int w,h,c;
    unsigned char v;
    int min_c = 9999999;
    int max_c = 0;
    printf("Generating random values...\n");
    memset(&counts,0,sizeof(counts));
    srand( (unsigned)time( NULL ) );
    for (w = 0; w < width; w++) {
        for (h = 0; h < height; h++) {
            for (c = 0; c < color; c++) {
                v = (unsigned char)(rand() % 256);
                waterfall[w][h][c] = v;
                if (v < 256)
                    counts[c]++;
                if (v > max_c)
                    max_c = v;
                if (v < min_c)
                    min_c = v;
            }
        }
    }
    printf("Range of values min %d to max %d\n", min_c, max_c );
    if (show_rand) {
        printf("Counts of values in array...\n");
        h = 0;
        for (w = 0; w < 256; w++) {
            printf("#%3d %3ld ", w, counts[w]);
            h++;
            if (h >= 8) {
                printf("\n");
                h = 0;
            }
        }
        if(h) printf("\n");
    }

    if (argc < 2) {
        printf("Give the name of the output random bmp!\n");
        return 1;
    }
    printf("Writing bitmap to file %s\n", argv[1]);
    std::ofstream stream;
    stream.open( argv[1], std::ofstream::binary ); 
    write_bmp( stream );
    stream.close();
    printf("Written bitmap to file %s\n", argv[1]);

	return iret;
}

// eof
