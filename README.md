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
 
### Targa Tests (TGA)

tga-test -

### Portable PixMap (PPM) GrayMap (PGM) BitMap (PBM)

pgm-test -

Have FUN ;=))

Geoff.   
20150407

; eof
