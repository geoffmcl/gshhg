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
        if (stat_buf.st_mode &  _S_IFDIR) 
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
                    _itoa( value, buffPtr++, 16);
                }
                else
                {
                    _itoa( value, buffPtr, 16);
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

// eof
