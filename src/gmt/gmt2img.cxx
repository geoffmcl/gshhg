// gmt2img.cxx

#include <stdlib.h>
#include <stdio.h>
#include <memory.h> // for memcpy()
#include <vector>
#ifdef WIN32
#include <direct.h> // for _getcwd()
#else
#include <unistd.h> // got getcwd(), ...
#endif
#include "sprtf.hxx"
#include "utils.hxx"
#include "bmp_utils.hxx"    // write a BMP, using a byte array width x height - filled 255 or 0
#include "png_utils.hxx"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

//#define DO_DEBUG
//static const char *def_file = "C:\\Users\\user\\Downloads\\GSHHS_shp\\NOAA\\15487.dat";
static const char *def_file = "gshhs.csv";
#define MX_LINE 256

#ifdef WIN32
#define GETCWD _getcwd
#else
#define GETCWD getcwd
#endif

char *out_png = "tempout5.png";
char *out_bmp = "tempout5.bmp";
char *out_1bit = "temp1bit5.bmp";
char *out_unique = "tempun5.csv";

double max_lat = -90.0;
double min_lat = 180.0;
double max_lon = -180.0;
double min_lon = 360.0;

int image_x, image_y;
size_t filesize;

typedef struct tagPT {
    int seg;
    double lat,lon;
}PT, *PPT;

typedef std::vector<PT> vPT;

static vPT vPts;
static vPT vUniquePts;

static void check_me()
{
    int i;
    SPRTF("ENTER key to continue!\n");
    getchar();
    i = 0;
}

int load_file( char *file )
{
    int line_number = 0;
    int iret = 0;
    char *cp;
    char *end;
    char *ccp;
    char str[MX_LINE];
    size_t len;
    int segment = 0;
    float lat,lon;
    PT pt;
    int oor_count = 0;
    char *form = "%f %f";
    char *ft = "GMT ASCII";

    FILE *fp = fopen(file,"r");
    if (!fp) {
        SPRTF("Failed to open file %s\n",file);
        return 1;
    }
    len = strlen(file);
    if (len > 4) {
        if (strcmp(&file[len-4],".csv") == 0) {
            form = "%f,%f";
            ft = "CSV";
        }
    }
    SPRTF("Processing file %s, of %d bytes... type %s\n",file, (int)filesize,ft);
    while (!feof(fp)) {
        line_number++;
        cp = fgets(str,MX_LINE,fp);
        if (cp) {
            len = strlen(cp);
            while (len) {
                len--;
                if (cp[len] > ' ') {
                    len++;
                    break;
                }
                cp[len] = 0;
            }
            end = &cp[len];
            while ((*cp <= ' ')&&(cp < end)) {
                if (len == 0) break;
                len--;
                cp++;
            }
            ccp = cp;
            if (len) {
                lat = 200;
                lon = 200;
                if (*cp == '>') {
                    segment++;
                } else if (sscanf(cp,form, &lon, &lat) == 2) {
                    if (in_world_range(lat,lon)) {
                        pt.lat = lat;
                        pt.lon = lon;
                        pt.seg = segment;
                        vPts.push_back(pt);
                        if (lat > max_lat) max_lat = lat;
                        if (lat < min_lat) min_lat = lat;
                        if (lon > max_lon) max_lon = lon;
                        if (lon < min_lon) min_lon = lon;
                    } else {
                        oor_count++;
                        SPRTF("Ln %d: UGH! Out of range %f,%f (%d)\n", line_number, lat, lon,
                            oor_count);
                    }
                }

            }
        }
    }
    size_t max = vPts.size();
    if (oor_count) SPRTF("Found %d out of range!\n", oor_count);
    SPRTF("Collected %d points, with range min %f,%f max %f,%f\n",
        (int)max, min_lat, min_lon, max_lat, max_lon );

    fclose(fp);
    return iret;

}

// Actually NOT really generate image... more convert the linear on/off byte buffer
// to the appropriate 1-bit pixel array
unsigned char *genImage( int w, int h, int bpp, unsigned char *buffer, size_t bsiz,
                         size_t *psiz )
{
    unsigned char bit;
    unsigned long iByte, iy, ix, offSet, bitNumber; /* bits are numbered from 0 to 7 */
    unsigned long BytesPerRow = (((w * bpp)+31)/32)*4; 
    unsigned long BytesSize = BytesPerRow * h;
    size_t soff;
    unsigned char *bytes = (unsigned char *)malloc(BytesSize);

    *psiz = BytesSize;
    if (!bytes) {
        printf("Memory allocation FAILED!\n");
        check_me();
        exit(1);
    }

    /* clear all bits */
    for(iByte=0; iByte< BytesSize; iByte++) {
        bytes[iByte]=0; // to white
    }
        
    /* get color of pixel and save it to bits array */
    for(iy = 0 ; iy < (unsigned long) h; iy++) {
        ix = 0; // start source column
        for(iByte = 0; iByte < BytesPerRow; iByte++) {
           bit = 0x80;
           offSet = (iy * BytesPerRow) + iByte;
           if (offSet >= BytesSize) {
               SPRTF("Yeek! Row %d, Col %d, Offset %d GTE ByteSize %d\n",
                   iy, ix, offSet, BytesSize );
               check_me();
               exit(1);
           }
            if (ix >= (unsigned long)w) {
                break;   // run out of source
            }
           for (bitNumber = 0; bitNumber < 8; bitNumber++) {
               soff = (iy * w) + ix;
               if (soff >= bsiz) {
                   SPRTF("Yeek! srow %d, scol %d, soff %d GTE buf size %d\n",
                       iy, ix, (int)soff, (int)bsiz );
                   check_me();
                   exit(1);
               }
               if (buffer[soff] == 0) {
                   bytes[offSet] |= bit;    // if off(black), set bit
               }
               ix++; // to next source column
               bit = bit >> 1;  // shift to next bit in destination bit array
               if (ix >= (unsigned long)w) {
                   break;   // run out of source
               }
           }
        }
    }
    return bytes;
}



int write_image() 
{
    int iret = 0;
    int bit_depth = 8;
    int color_type = 6; // RGBA (I think)
    double cmax_lat = max_lat + 90.0;
    double cmin_lat = min_lat + 90.0;
    double cmax_lon = max_lon + 180.0;
    double cmin_lon = min_lon + 180.0;
    int x = (int)((cmax_lon - cmin_lon) + 0.5);
    int y = (int)((cmax_lat - cmin_lat) + 0.5);
    if ((x <= 0) || (y <= 0)) {
        SPRTF("Bad image size %d x %d!\n", x, y );
        return 1;
    }
    size_t len = vPts.size();
    size_t ii, bsiz, offset;
    if (!len) {
        SPRTF("No points to write!\n");
        return 1;
    }

    // not sure why need to bump these...
    // ================================
    x++;    // but it works
    y++;
    // =================================
    x *= 10;    // row width
    y *= 10;    // row height
    bsiz = x * y; // array of x X y - either on or off
    image_x = x;
    image_y = y;
    SPRTF("Image size x,y = %d,%d buf %d bytes\n", x, y, (int)bsiz );
    unsigned char *buffer = (unsigned char *)malloc(bsiz+4);
    if (!buffer) {
        SPRTF("Buffer allocation failed on %d bytes!\n", (int)(bsiz));
        check_me();
        return 1;
    }
    memset(buffer,255,bsiz);   // set all ON (white)
    int lat,lon;
    double dlat,dlon;
    for (ii = 0; ii < len; ii++) {
        PT pt = vPts[ii];
        dlat = pt.lat;
        dlon = pt.lon;
        // put in positive territory
        dlat += 90.0;
        dlon += 180.0;
        // remove the min values for all - reduce to 0 to (max_xxx - min_xxx)
        dlat -= cmin_lat;
        dlon -= cmin_lon;
        // convert to an integer
        lat = (int)( (dlat + 0.05) * 10 );
        lon = (int)( (dlon + 0.05) * 10 );
        // offset = row(lat) * width(x*4) + col(lon) * 4;
        offset = (lat * x) + lon;
        if (offset < bsiz) {
            unsigned char *p = &buffer[offset];
            if (*p) {
                *p = 0; // set OFF (black)
                vUniquePts.push_back(pt);
            }
        } else {
            SPRTF("Off %d GTT %d! row %d on %d, col %d on %d ***FIX ME***\n",
                (int)offset, (int)bsiz,
                lat, y, lon, x );
            check_me();
            exit(1);
        }
    }
#ifdef USE_PNG_LIB
    // Also using a bit depth of 8, but this seems more that needed for simple BW image
    // but need to understand more of PNG creation...
    iret |= writePNGImage(out_png, x, y, bit_depth, color_type, buffer );
#endif
#ifdef WIN32
    iret |= writeBMPImage(out_bmp, x, y, buffer, bsiz);
    size_t bitlen;
    unsigned char *bitarray = genImage( x, y, 1, buffer, bsiz, &bitlen );
    if (bitarray) {
        iret = writeBMP1bit( out_1bit, x, y, 1, bitarray, bitlen );
    }
#endif
    free(buffer);
    return iret;
}

int show_unique()
{
    PT pt;
    size_t len = vPts.size();
    size_t ii = vUniquePts.size();
    SPRTF("At this resolution x,y %d,%d, got %d unique pts of total %d\n",
        image_x, image_y, (int)ii, (int)len );
    len = ii;
    FILE *fp = fopen(out_unique,"w");
    if (!fp) {
        SPRTF("Error: Unable to create %f\n",out_unique);
        return 1;
    }
    fprintf(fp,"%s,%s\n", "latitude", "longitude");
    for (ii = 0; ii < len; ii++) {
        pt = vUniquePts[ii];
        fprintf(fp,"%f,%f\n",pt.lon, pt.lat); 
    }
    fclose(fp);
    SPRTF("Unique points wrttien to %s\n",out_unique);
    return 0;
}

void give_help(char *name)
{
    char cwd[MAX_PATH];
    if( !GETCWD(cwd,MAX_PATH) )
        strcpy(cwd,"Not available!");
    printf("Usage: %s in_gmt_file\n",name);
    printf("\n");
    printf("Given the path to a GMT ASCII file to load %s will\n",name);
    printf("create image files for it. The image outputs will be\n" );
    printf("24-bit BMP %s, 1-bit BMP %s, and an 8-bit PNG %s if libpng.lib is found.\n", 
            out_bmp, out_1bit, out_png );
    printf("These will be written in the currrent work directory [%s].\n",cwd);
    printf("The width and height of the images will be based on 10 times the degrees\n");
    printf("between the maximum and minimum latitudes and longitudes found in the input.\n");
    printf("And the minimum number of unqiue points at this resolution will be written to %s\n",
        out_unique );
    printf("The format of an GMT ASCII is a 'lat lon' on each line. If passed a file with\n");
    printf("a .csv extension it expects each line to be 'lat,lon'. Other lines are ignored.\n");

}


int main( int argc, char **argv )
{
    char *file;
    int i, iret = 0;
    set_log_file("tempgmt.txt",false);
    i = argc;
#ifdef DO_DEBUG
    file = (char *)def_file;
    if (is_file_or_directory(file) != 1) {
        SPRTF("Unable to 'stat' file %s\n",file);
        return 1;
    }
#else
    if (i < 2) {
        give_help( get_file_name(argv[0]) );
        return 2;
    }
    for (i = 1; i < argc; i++) {
        file = argv[i];
        if (*file == '-') {
            printf("Unknown argument [%s]\n",file);
            give_help( get_file_name(argv[0]) );
            return 2;
        }
        if (is_file_or_directory(file) != 1) {
            SPRTF("Unable to 'stat' file %s\n",file);
            return 1;
        }
        break;
    }
#endif
    filesize = get_last_file_size();
    iret = load_file(file);
    if (!iret) iret = write_image();
    if (!iret) show_unique();
    vPts.clear();
    vUniquePts.clear();
    close_log_file();
    return iret;
}

// eof
