// ========================================================================
// IntToBMP.cpp : Defines the entry point for the console application.
// from : http://stackoverflow.com/questions/12200201/c-convert-text-file-of-integers-into-a-bitmap-image-file-in-bmp-format
// 20140304
//Assumption: 32-bit signed integers
//Assumption: Distribution of values range from INT32_MIN through INT32_MAX, inclusive
//Assumption: number of integers contained in file are unknown
//Assumption: source file of integers is a series of space-delimitied strings representing integers
//Assumption: source file's contents are valid
//Assumption: non-rectangular numbers of integers yield non-rectangular bitmaps (final scanline may be short)
//            This may cause some .bmp parsers to fail; others may pad with 0's.  For simplicity, this implementation
//            attempts to render square bitmaps.

// NOT USING pecompile headers!!! #include "stdafx.h"
#ifdef _MSC_VER
#include <SDKDDKVer.h>
#include <tchar.h>
#else
#include <string.h> // for strlen, ...
#endif
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <memory>

#define ADD_RAND_FILE
//#define USE_STD_MT19937 // to use std::mt19937 rng;
static bool do_auto_gen = false;
static int exit_value = 0;

#pragma pack( push, 1 ) 
struct BMP
{
    BMP();
    struct
    {
        uint16_t ID;
        uint32_t fileSizeInBytes;
        uint16_t reserved1;
        uint16_t reserved2;
        uint32_t pixelArrayOffsetInBytes;
    } FileHeader;

    enum CompressionMethod : uint32_t {   BI_RGB              = 0x00, 
                                          BI_RLE8             = 0x01,
                                          BI_RLE4             = 0x02,
                                          BI_BITFIELDS        = 0x03,
                                          BI_JPEG             = 0x04,
                                          BI_PNG              = 0x05,
                                          BI_ALPHABITFIELDS   = 0x06 };

    struct
    {
        uint32_t headerSizeInBytes;
        uint32_t bitmapWidthInPixels;
        uint32_t bitmapHeightInPixels;
        uint16_t colorPlaneCount;
        uint16_t bitsPerPixel;
        CompressionMethod compressionMethod;
        uint32_t bitmapSizeInBytes;
        int32_t horizontalResolutionInPixelsPerMeter;
        int32_t verticalResolutionInPixelsPerMeter;
        uint32_t paletteColorCount;
        uint32_t importantColorCount;
    } DIBHeader;
};
#pragma pack( pop )

BMP::BMP()
{
    //Initialized fields
    FileHeader.ID                                   = 0x4d42; // == 'BM' (little-endian)
    FileHeader.reserved1                            = 0;
    FileHeader.reserved2                            = 0;
    FileHeader.pixelArrayOffsetInBytes              = sizeof( FileHeader ) + sizeof( DIBHeader );
    DIBHeader.headerSizeInBytes                     = 40;
    DIBHeader.colorPlaneCount                       = 1;
    DIBHeader.bitsPerPixel                          = 32;
    DIBHeader.compressionMethod                     = BI_RGB;
    DIBHeader.horizontalResolutionInPixelsPerMeter  = 2835; // == 72 ppi
    DIBHeader.verticalResolutionInPixelsPerMeter    = 2835; // == 72 ppi
    DIBHeader.paletteColorCount                     = 0;
    DIBHeader.importantColorCount                   = 0;
}

void Exit( void )
{
#if (defined(WIN32) && !defined(NDEBUG))
    std::cout << "Press a key to exit...";
    std::getchar();
#endif
    exit( exit_value );
}

// === *** DO WE REALLY WANT A RANDOM FILE ???? ***
void MakeIntegerFile( const std::string& integerFilename )
{
    int num;
    const uint32_t intCount = 1 << 20; //Generate 1M (2^20) integers
    std::unique_ptr< int32_t[] > buffer( new int32_t[ intCount ] ); 

#ifdef USE_STD_MT19937 // std::mt19937 rng;
    // UGH: could NOT get this to compile!!!!!!!!!!!!
    std::mt19937 rng;
    uint32_t rngSeed = static_cast< uint32_t >( time( NULL ) );
    rng.seed9( rngSeed );

    std::uniform_int_distribution< int32_t > dist( INT32_MIN, INT32_MAX );

    for( size_t i = 0; i < intCount; ++i )
    {
        buffer[ i ] = dist( rng );
    }
#else // !#ifdef USE_STD_MT19937 // std::mt19937 rng;
    srand((unsigned)time(0));
    int range_min = INT32_MIN / 2;
    int range_max = INT32_MAX / 2;
    for( size_t i = 0; i < intCount; ++i ) {
        // buffer[ i ] = rand(); // 0 to RAND_MAX (32767)
        num = (int)((double)rand() / ((double)RAND_MAX + 1) * (range_max - range_min) + range_min);
        buffer[ i ] = num;
    }

#endif // #ifdef USE_STD_MT19937 // std::mt19937 rng;

    std::ofstream writeFile( integerFilename, std::ofstream::binary );

    if( !writeFile || !writeFile.is_open() )
    {
        std::cout << "Error writing " << integerFilename << ".\n";
        exit_value = 1;
        Exit();
    }
    int max_num = INT32_MIN;
    int min_num = INT32_MAX;
    num = buffer[ 0 ];
    if (num < min_num)
        min_num = num;
    if (num > max_num)
        max_num = num;
    writeFile << num;
    for( size_t i = 1; i < intCount; ++i )
    {
        num = buffer[ i ];
        writeFile << " " << num;
        if (num < min_num)
            min_num = num;
        if (num > max_num)
            max_num = num;
    }
    std::cout << "Written " << intCount << " integers to '" << integerFilename << "'\n";
    std::cout << "Range min " << min_num << ", to max " << max_num << ".\n";

}

static const char *in_file = "temp-integers.txt";
static const char *out_file = "temp-bitmap.bmp";

static char info[] =
    " Will read the in_file as an array of integers, space separated,\n"
    " values range from INT32_MIN through INT32_MAX\n"
    " and write a bitmap file, in uncompressed BI_RGB format.\n"
    " The width and height of the image is assumed to be the sqrt of\n"
    " the total number of integers found in the input.\n"
    " A non-rectangular numbers of integers yield non-rectangular bitmaps (final scanline may be short)\n"
    " This may cause some .bmp parsers to fail; others may pad with 0's.  For simplicity,\n"
    " this implementation attempts to render square bitmaps.\n\n"
    " In essence is a SIMPLE example of converting an array of integers to a BMP\n"
    " See http://stackoverflow.com/questions/12200201/c-convert-text-file-of-integers-into-a-bitmap-image-file-in-bmp-format\n"
    " for the original code. And as pointed out in that post, this is close to converting a PPM file\n"
    " to a BMP file.\n\n";

static char *get_name_only( char *name )
{
    char *n = name;
    int c, i, len = (int)strlen(name);
    for (i = 0; i < len; i++) {
        c = name[i];
        if (( c == '/' )||( c == '\\' )) {
            if ((( i + 1 ) < len) && name[i+1])
                n = &name[i+1];
        }
    }
    return n;
}

void give_help( char *name )
{
    printf("\nusage: %s [options]\n\n", get_name_only(name));
    printf("options:\n");
    printf(" --help (-h or -?) = This brief help and exit(0)\n");
    printf(" --auto       (-a) = Auto-generate a random integer 1024x1024 input file. (def=%s)\n",
        do_auto_gen ? "on" : "off");
    printf(" --in <file>  (-i) = Set the input file. (def=%s)\n", in_file);
    printf(" --out <file> (-o) = Set the output file. (def=%s)\n", out_file);
    printf("\n");
    printf("%s", info);
}

// int _tmain(int argc, _TCHAR* argv[])  //Replace with int main( int argc, char* argv[] ) if you're not under Visual Studio
int main( int argc, char* argv[] )
{
    int c, i, i2;
    char *arg, *sarg;
    for (i = 1; i < argc; i++) {
        i2 = i + 1;
        arg = argv[i];
        c = *arg;
        if (c == '-') {
            sarg = &arg[1];
            while (*sarg == '-')
                sarg++;
            c = *sarg;
            switch (c) {
            case 'h':
            case '?':
                give_help(argv[0]);
                return 0;
                break;
            case 'a':
                do_auto_gen = true;
                break;
            case 'i':
                if (i2 < argc) {
                    i++;
                    in_file = argv[i];
                } else {
                    printf("Error: Expected input file name to follow '%s'\n", arg);
                    return 1;
                }
                break;
            case 'o':
                if (i2 < argc) {
                    i++;
                    out_file = argv[i];
                } else {
                    printf("Error: Expected output file name to follow '%s'\n", arg);
                    return 1;
                }
                break;
            default:
                printf("Error: Unknown argument '%s'! Try -? for help.\n", arg );
                return 1;
            }
        } else {
            give_help(argv[0]);
            printf("Error: Unknown input '%s'!\n", arg );
            return 1;
        }
    }

    const std::string integerFilename = in_file;
    const std::string bitmapFilename = out_file;
#ifdef ADD_RAND_FILE
    if (do_auto_gen) {
        std::cout << "Creating file '" << integerFilename << "' of random integers...\n";
        MakeIntegerFile( integerFilename );
    }
#endif // #ifdef ADD_RAND_FILE

    std::vector< int32_t >integers; //If quantity of integers being read is known, reserve or resize vector or use array

    //Read integers from file
    std::cout << "Reading integers from file '" << integerFilename << "'...\n";
    {   //Nested scope will release ifstream resource when no longer needed
        std::ifstream readFile( integerFilename );

        if( !readFile || !readFile.is_open() )
        {
            std::cout << "Error reading " << integerFilename << ".\n";
            exit_value = 1;
            Exit();
        }

        std::string number;
        int max_num = INT32_MIN;
        int min_num = INT32_MAX;
        while( readFile.good() )
        {
            std::getline( readFile, number, ' ' );
            int num = std::stoi( number );
            integers.push_back(num);
            if (num < min_num)
                min_num = num;
            if (num > max_num)
                max_num = num;
        }
        size_t icnt = integers.size();
        if(icnt  == 0 )
        {
            std::cout << "No integers read from " << integerFilename << ".\n";
            exit_value = 1;
            Exit();
        }
        std::cout << "Collected " << icnt << " integers, range min " << min_num << ", to max " << max_num << ".\n";
    }

    //Construct .bmp
    std::cout << "Constructing BMP...\n";
    BMP bmp;
    size_t intCount = integers.size();
    bmp.DIBHeader.bitmapSizeInBytes = intCount * sizeof( integers[ 0 ] );
    bmp.FileHeader.fileSizeInBytes = bmp.FileHeader.pixelArrayOffsetInBytes + bmp.DIBHeader.bitmapSizeInBytes;
    bmp.DIBHeader.bitmapWidthInPixels = static_cast< uint32_t >( ceil( sqrt( (double)intCount ) ) );
    bmp.DIBHeader.bitmapHeightInPixels = static_cast< uint32_t >( ceil( intCount / static_cast< float >( bmp.DIBHeader.bitmapWidthInPixels ) ) );

    //Write integers to .bmp file
    std::cout << "Writing BMP file '" << bitmapFilename << 
                 "', width " << bmp.DIBHeader.bitmapWidthInPixels << 
                 ", height " << bmp.DIBHeader.bitmapHeightInPixels << ".\n";
    {
        std::ofstream writeFile( bitmapFilename, std::ofstream::binary );

        if( !writeFile || !writeFile.is_open() )
        {
            std::cout << "Error writing " << bitmapFilename << ".\n";
            exit_value = 1;
            Exit();
        }

        writeFile.write( reinterpret_cast< char * >( &bmp ), sizeof( bmp ) );
        writeFile.write( reinterpret_cast< char * >( &integers[ 0 ] ), bmp.DIBHeader.bitmapSizeInBytes );
    }

    Exit();
} 

// eof
