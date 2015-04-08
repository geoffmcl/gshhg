# GSHHG Project

What a great name - **A Global Self-consistent, Hierarchical, High-resolution Geography Database**

For the original source data see <a target="_blank" href="http://www.soest.hawaii.edu/pwessel/gshhg/">http://www.soest.hawaii.edu/pwessel/gshhg/</a>, and perhaps updates.

For another project related to flight planning I needed to be able to extract 
a bounding box of the shoreline, and so this small project was born.

The final executable is gshhg.exe, Running it with -? will give details of its usage. The area extracted can be written to an 'xg' file, which can be viewed by a PolyView2D app, which is part of this repo - https://gitlab.com/fgtools/osm2xg.

But then the porject expanded, exploring and testing reading/writing various image files.

### Prerequisites:

 1. git - http://git-scm.com/downloads
 2. cmake - http://www.cmake.org/download/
 3. Native build tools to suit genertor used.
 4. PNG (and ZLIB) - Optionals. Only some tools require this.

### Building:

This project uses the cmake build file generator.

#### In Unix/Linux

 1. cd build
 2. cmake ..
 3. make
 
#### In Windows

 1. cd build
 2. cmake ..
 3. cmake --build . --config Release
 
The 'build' directory contains convenient build scripts - build-me.bat and build-me.sh - It should be relatively easy to modify these to suit your particular environment.
 
Of course the cmake GUI can also be used, setting the source directory, and the binary directory to the 'build' folder. And in Windows, the MSVC IDE can be used if this is the chosen generator.


### Bitmap Tests (BMP)

bmp_utils - A bitmap utility library

A whole bunch of little utilities, all dealing with bitmaps.

 1. bmp-test
 2. bmp-1bit
 3. bmp_io
 4. Int2BMP
 5. write-bmp1
 6. write-bmp2

#### 1. bmp-test

Test program for reading bitmap files.  It accepts an input file and an
output file on the command line.  It will read and process the input file
and dump an ASCII representation of the contents to the output file.  The
dump will consist of the color image and two masks.  Missing parts will be
indicated as such (BMP files have no masks and monochrome ICO/PTR files
have no color data.  In the color image, the dump will be a series of RGB
values (in hexadecimal).  In the masks, the dump will be represented by "."
symbols representing zeros and "@" symbols representing ones.

Reprinted courtesy Dr. Dobb's Journal, (C) 1995.

This is a good example of crossplatfom bitmap handling, since no Windows API is used,
and it defines all it own structures.
 
#### 2. bmp-1bit

Generates a random 1000 x 1000 monchrome bitmap, again NOT using any Windows API. Only optional input is the name of the output random bitmap file.

Is just an example of how to generate a monochrome bitmap, from a randomly generate byte array, in a cross platform way.

#### 3. bmp_io

This application does NOTHING!
It is just some crossplatform Windows BMP functions.
This the full list from bmp_io.hpp

bool **bmp_byte_swap_get** ( void );  
void **bmp_byte_swap_set** ( bool value );  

bool **bmp_08_data_read** ( ifstream &file_in, unsigned long int width, long int height, 
  unsigned char *rarray );  
void **bmp_08_data_write** ( ofstream &file_out, unsigned long int width, 
  long int height, unsigned char *rarray );  

bool **bmp_24_data_read** ( ifstream &file_in, unsigned long int width, 
  long int height, unsigned char *rarray, unsigned char *garray, unsigned char *barray );  
void **bmp_24_data_write** ( ofstream &file_out, unsigned long int width, 
  long int height, unsigned char *rarray, unsigned char *garray, unsigned char *barray );  

void **bmp_header1_print** ( unsigned short int filetype, 
  unsigned long int filesize, unsigned short int reserved1, 
  unsigned short int reserved2, unsigned long int bitmapoffset );  
bool **bmp_header1_read** ( ifstream &file_in, unsigned short int *filetype, 
  unsigned long int *filesize, unsigned long int *reserved1, 
  unsigned short int *reserved2, unsigned long int *bitmapoffset );  
void **bmp_header1_write** ( ofstream &file_out, unsigned short int filetype,
  unsigned long int filesize, unsigned long int reserved1, 
  unsigned short int reserved2, unsigned long int bitmapoffset );  

void **bmp_header2_print** ( unsigned long int size, unsigned long int width, 
  long int height, 
  unsigned short int planes, unsigned short int bitsperpixel, 
  unsigned long int compression, unsigned long int sizeofbitmap,
  unsigned long int horzresolution, unsigned long int vertresolution,
  unsigned long int colorsused,  unsigned long int colorsimportant );  
bool **bmp_header2_read** ( ifstream &file_in, unsigned long int *size,
  unsigned long int *width, long int *height, 
  unsigned short int *planes, unsigned short int *bitsperpixel,
  unsigned long int *compression, unsigned long int *sizeofbitmap,
  unsigned long int *horzresolution, unsigned long int *vertresolution,
  unsigned long int *colorsused, unsigned long int *colorsimportant );  
void **bmp_header2_write** ( ofstream &file_out, unsigned long int size,
  unsigned long int width, long int height, 
  unsigned short int planes, unsigned short int bitsperpixel,
  unsigned long int compression, unsigned long int sizeofbitmap,
  unsigned long int horzresolution, unsigned long int vertresolution,
  unsigned long int colorsused, unsigned long int colorsimportant );  

void **bmp_palette_print** ( unsigned long int colorsused, 
  unsigned char *rparray, unsigned char *gparray, unsigned char *bparray,
  unsigned char *aparray );  
bool **bmp_palette_read** ( ifstream &file_in, unsigned long int colorsused,
  unsigned char *rparray, unsigned char *gparray, unsigned char *bparray, 
  unsigned char *aparray );  
void **bmp_palette_write** ( ofstream &file_out, unsigned long int colorsused, 
  unsigned char *rparray, unsigned char *gparray, unsigned char *bparray,
  unsigned char *aparray );  

bool **bmp_print_test** ( char *file_in_name );  

bool **bmp_read** ( char *file_in_name, unsigned long int *width, long int *height, 
  unsigned char **rarray, unsigned char **garray, unsigned char **barray );  
bool **bmp_read_test** ( char *file_in_name );  

bool **bmp_08_write** ( char *file_out_name, unsigned long int width, long int height, 
  unsigned char *rarray, unsigned char *garray, unsigned char *barray );  
bool **bmp_08_write_test** ( char *file_out_name );  

bool **bmp_24_write** ( char *file_out_name, unsigned long int width, long int height, 
  unsigned char *rarray, unsigned char *garray, unsigned char *barray );  
bool **bmp_24_write_test** ( char *file_out_name );  

bool **long_int_read** ( long int *long_int_val, ifstream &file_in );  
void **long_int_write** ( long int long_int_val, ofstream &file_out );  

bool **u_long_int_read** ( unsigned long int *u_long_int_val, ifstream &file_in );  
void **u_long_int_write** ( unsigned long int u_long_int_val, ofstream &file_out );  

bool **u_short_int_read** ( unsigned short int *u_short_int_val, ifstream &file_in );  
void **u_short_int_write** ( unsigned short int u_short_int_val, ofstream &file_out );  

Although the file has a C++ extension, there is not really anything
that is really C++. Most, if not all functions could be extracted and
quite easily be used in a C program.

The source is from : http://people.sc.fsu.edu/~jburkardt/cpp_src/bmp_io/bmp_io.html

The Licensing: This code is distributed under the GNU LGPL license. 
Asside from this main() added by me, it was last modified: 13 August 2007, with an
earliest date of 26 February 2003

The original Author: John Burkardt, and my thanks to him for putting them together.

Of course it could be quite easily cast into a library to attach to your own app.
The important thing is that it is quite crossplatform, having NO dependency on the
windows GDI header files.


### Targa Tests (TGA)

tga-test -

### Portable PixMap (PPM) GrayMap (PGM) BitMap (PBM)

pgm-test -

Have FUN ;=))

Geoff.   
20150407

; eof
