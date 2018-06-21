// utils.cxx

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale>
//#include <iostream>
#include "sprtf.hxx"
#include "utils.hxx"
#ifdef WIN32
#define ITOA _itoa
#else
#define ITOA itoa
#endif

using namespace std;

char *get_file_name(char *file)
{
	char *name = file;
	size_t len = strlen(file);
	size_t ii;
	int c;
	for (ii = 0; ii < len; ii++) {
		c = file[ii];
		if ((c == '\\')||(c == '/')) {
			if ((ii + 1) < len)
				name = &file[ii+1];
		}
	}
	return name;
}

static struct stat stat_buf;
// return: 0 = not file or directory, 1 = file, 2 = directory, 
int is_file_or_directory( char *file )
{
    int iret = 0;
    if (stat(file,&stat_buf) == 0) {
        if (stat_buf.st_mode &  M_IS_DIR) 
            iret = 2;
        else
            iret = 1;
    }
    return iret;
}

size_t get_last_file_size() { return stat_buf.st_size; }

int is_an_integer( char *arg )
{
    int len = (int)strlen(arg);
    if (!len) return 0;
    int i, c;
    for (i = 0; i < len; i++) {
        c = arg[i];
        if (!ISDIGIT(c))
            return 0;
    }
    return 1;   // is a positive integer
}

/////////////////////////////////////////////////////////////////////////
// Number of hex values displayed per line
#define HEX_DUMP_WIDTH 16
//
// Dump a region of memory in a hexadecimal format
//
void HexDump(unsigned char *ptr, int length, bool addhdr, bool addascii, bool addspace)
{
    char buffer[256];
    char *buffPtr, *buffPtr2;
    unsigned cOutput, i;
    int bytesToGo = length;

    while ( bytesToGo  )
    {
        memset(buffer,0,256);
        cOutput = bytesToGo >= HEX_DUMP_WIDTH ? HEX_DUMP_WIDTH : bytesToGo;

#if 0 // chop this
        if( Try_HD_Width( ptr, cOutput ) )
        {
           sprtf( "WARNING: Abandoning HEX DUMP, bad ptr %p, length %d!\n",
               ptr, cOutput );
            return;
        }
        if (add_2_ptrlist(ptr)) {
            bytesToGo -= cOutput;
            ptr += HEX_DUMP_WIDTH;
            continue;
        }
#endif // 0 - chopped code

        buffPtr = buffer;
        if (addhdr) {
            buffPtr += sprintf(buffPtr, "%08X:  ", length-bytesToGo );
        }
        buffPtr2 = buffPtr + (HEX_DUMP_WIDTH * 3) + 1;
        *buffPtr2 = 0;

        for ( i=0; i < HEX_DUMP_WIDTH; i++ )
        {
            unsigned char value = *(ptr+i);

            if ( i >= cOutput )
            {
                // On last line.  Pad with spaces
                *buffPtr++ = ' ';
                *buffPtr++ = ' ';
                *buffPtr++ = ' ';
            }
            else
            {
                if ( value < 0x10 )
                {
                    *buffPtr++ = '0';
#ifdef WIN32
                    ITOA( value, buffPtr++, 16);
#else
                    sprintf(buffPtr,"%d",value);
                    buffPtr++;
#endif
                }
                else
                {
#ifdef WIN32
                    ITOA( value, buffPtr, 16);
#else
                    sprintf(buffPtr,"%d",value);
#endif
                    buffPtr+=2;
                }
 
                *buffPtr++ = ' ';
                if (addascii) {
                    *buffPtr2++ = isprint(value) ? value : '.';
                    if ( value == '%' )
                       *buffPtr2++ = '%';  // insert another
                }
            }
            
            // Put an extra space between the 1st and 2nd half of the bytes
            // on each line.
            if (addspace) {
                if ( i == (HEX_DUMP_WIDTH/2)-1 )
                    *buffPtr++ = ' ';
            }
        }

        *buffPtr2 = 0;  // Null terminate it.
        //puts(buffer);   // Can't use sprtf(), since there may be a '%'
                        // in the string.
        SPRTF("%s\n", buffer);   // Have add 2nd % if one found in ASCII

        bytesToGo -= cOutput;
        ptr += HEX_DUMP_WIDTH;
    }
}

////////////////////////////////////////////////////////////////////////////

int in_world_range(double lat, double lon)
{
    if ((lat >= -90.0) && (lat <= 90.0) &&
        (lon >= -180.0) && (lon <= 180.0)) {
            return 1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////
//// distance - Haversine formula
//// from : https://stackoverflow.com/questions/10198985/calculating-the-distance-between-2-latitudes-and-longitudes-that-are-saved-in-a

#define earthRadiusKm 6371.0

// This function converts decimal degrees to radians
double deg2rad(double deg) {
    return (deg * M_PI / 180);
}

//  This function converts radians to decimal degrees
double rad2deg(double rad) {
    return (rad * 180 / M_PI);
}

/**
* Returns the distance between two points on the Earth.
* Direct translation from http://en.wikipedia.org/wiki/Haversine_formula
* @param lat1d Latitude of the first point in degrees
* @param lon1d Longitude of the first point in degrees
* @param lat2d Latitude of the second point in degrees
* @param lon2d Longitude of the second point in degrees
* @return The distance between the two points in kilometers
*/
double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
    double lat1r, lon1r, lat2r, lon2r, u, v;
    lat1r = deg2rad(lat1d);
    lon1r = deg2rad(lon1d);
    lat2r = deg2rad(lat2d);
    lon2r = deg2rad(lon2d);
    u = sin((lat2r - lat1r) / 2);
    v = sin((lon2r - lon1r) / 2);
    return 2.0 * earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}

// eof
