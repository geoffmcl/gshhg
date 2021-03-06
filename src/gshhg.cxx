/*\
 * gshhg.cxx
 *
 * Copyright (c) 2015 - 2020 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
 * from : http://www.soest.hawaii.edu/pwessel/gshhg/
 *
 * The geography data come in five resolutions:
 *  full resolution: Original (full) data resolution.
 *  high resolution: About 80 % reduction in size and quality.
 *  intermediate resolution: Another ~80 % reduction.
 *  low resolution: Another ~80 % reduction.
 *  crude resolution: Another ~80 % reduction.
 * Unlike the shoreline polygons at all resolutions, the lower resolution rivers 
 * are not guaranteed to be free of intersections.
 *
 * Shorelines are furthermore organized into 6 hierarchical levels:
 *  L1: boundary between land and ocean, except Antarctica.
 *  L2: boundary between lake and land.
 *  L3: boundary between island-in-lake and lake.
 *  L4: boundary between pond-in-island and island.
 *  L5: boundary between Antarctica ice and ocean.
 *  L6: boundary between Antarctica grounding-line and ocean.
 *
 * Rivers are organized into 10 classification levels:
 *  L0: Double-lined rivers (river-lakes).
 *  L1: Permanent major rivers.
 *  L2: Additional major rivers.
 *  L3: Additional rivers.
 *  L4: Minor rivers.
 *  L5: Intermittent rivers - major.
 *  L6: Intermittent rivers - additional.
 *  L7: Intermittent rivers - minor.
 *  L8: Major canals.
 *  L9: Minor canals.
 *  L10: Irrigation canals.
 *
 * Finally, borders are organized into three levels:
 *  L1: National boundaries.
 *  L2: State boundaries within the Americas.
 *  L3: Marine boundaries.
 *
\*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <locale>
#include <iostream>
#include <sstream>      // std::stringstream
#include <vector>
#include "sprtf.hxx"
#include "utils.hxx"
#include "gshhg.hxx"

using namespace std;

#ifndef SPRTF
#define SPRTF printf
#endif

#ifndef GSHHG_VERSION
#define GSHHG_VERIONS "No version"
#endif
#ifndef GSHHG_DATE
#define GSHHG_DATE "Unknown Date"
#endif

typedef struct tagPointD {	/* Each lon, lat pair is stored in micro-degrees in 4-byte integer format */
	double x, y;
}PointD, *PPointD;

typedef std::vector<PointD> vPTS;

static const char *module = "gshhg";
static const char *gshhg_version = GSHHG_VERSION;
static const char *gshhg_date = GSHHG_DATE;

#ifdef WORDS_BIGENDIAN
static const bool must_swab = false;
#else
static const bool must_swab = true;
#endif

static const char *def_log = "tempgshhg.txt";
// static char curr_log[264];
static const char *in_file = 0;
static const char *def_xg = "templist.xg";
static char curr_xg[264];
static const char *out_color = "blue";
static double fudge = 0.0;
static bool add_boundary = false;
static const char *bnd_color = "gray";
static int verbosity = 0;

#define VERB1 (verbosity >= 1)
#define VERB2 (verbosity >= 2)
#define VERB5 (verbosity >= 5)
#define VERB9 (verbosity >= 9)

static struct POINT p;
static struct GSHHG h;
static bool headers_only = false;
static bool whole_features = false;
static bool add_point_fix = true;
static bool check_wrap = true;

enum geomtype {
    IS_POLY,
    IS_LINE
};

static char header[1024];
static double min_lon, min_lat, max_lon, max_lat;
static bool got_bbox = false;

#ifndef _UTILS_HXX_
bool in_world_range( double lat, double lon )
{
    if ((lat < -90.0) ||
        (lat >  90.0) ||
        (lon < -180.0) ||
        (lon >  180.0))
        return false;
    return true;
}
#endif // !#ifndef _UTILS_HXX_

bool in_bbox( double lat, double lon )
{
    if (!got_bbox)
        return true;
    if ((lat < min_lat) ||
        (lat > max_lat) ||
        (lon < min_lon) ||
        (lon > max_lon))
        return false;
    return true;
}

void show_version()
{
    SPRTF("%s version %s, date %s\n", module, gshhg_version, gshhg_date);
}

void give_help( char *name )
{
#define INDENT "    "
    SPRTF("\n");
    show_version();
    SPRTF(INDENT "%s [options] in_gshhg_file\n", module );
    SPRTF("\n");
    SPRTF(INDENT "Options:\n");
    SPRTF(INDENT " --help    (-h or -?) = This help and exit(0)\n");
    SPRTF(INDENT " --v[vvv]   or -v[nn] = Bump or set verbosity (def=%d)\n", verbosity);
    SPRTF(INDENT " --bbox <bbox>   (-b) = Only ouput the points in this bbox. -B to add boundary.\n");
    SPRTF(INDENT " --color <color> (-c) = Set the output line color. (def=%s)\n", out_color);
    SPRTF(INDENT " --fudge <degs>  (-f) = Expand the bounding box, by by this 'fudge' factor. (def=%lf)\n", fudge );
    SPRTF(INDENT " --whole[-]      (-w) = Add whole feature if a single point is in this box. (def=%s)\n",
        (whole_features ? "On" : "Off"));
    SPRTF(INDENT " --xg <file>     (-x) = Set xg output file. 'none' for no xg. (def=%s)\n", curr_xg);
    SPRTF(INDENT " --display[-]    (-d) = Display headers only. Implies verbosity. (def=%s)\n",
        (headers_only ? "on" : "off") );
    SPRTF(INDENT " --log <file>    (-l) = Set the log file. (def=%s)\n", get_log_file());
    SPRTF(INDENT " --Fix[-]        (-F) = Try to FIX out of world points. (def=%s)\n",
        (add_point_fix ? "On" : "Off"));
    SPRTF(INDENT " --Wrap[-]       (-W) = Try to avoid E/W screen wrap. (def=%s)\n",
        (check_wrap ? "On" : "Off"));
    SPRTF("\n");
    SPRTF(INDENT "Notes:\n");
    SPRTF(INDENT " Verbosity points are 0, 1, 2, 5, and 9 for all messages.\n");
    SPRTF(INDENT " If -w not given, output if 2 or more consecutive points are within the bbox. (def=%s)\n",
        (whole_features ? "on" : "off") );
    SPRTF(INDENT " The bbox = min_lon,min_lat,max_lon,max_lat. Separated by any non-digit char.\n");
    SPRTF(INDENT " The xg file is a small subset of XGraph 2D display, as can be viewed\n");
    SPRTF(INDENT " by https://github.com/geoffmcl/PolyView, a Qt5 app.\n");
    SPRTF(INDENT " Boolean options, like -W,-F, ... support a trailing '-', to set the option OFF.\n");
    SPRTF(INDENT " All output is written to both stdout, and the log file.\n");
    SPRTF("\n");
    SPRTF(INDENT "Will extract all points from the gshhs bin file, and write the results\n");
    SPRTF(INDENT "to the xg file for viewing.\n");
    SPRTF("                                                     Happy GSHHG data extraction.\n");
}

#ifndef _UTILS_HXX_
#define ISDIGIT(a) (( a >= '0' ) && ( a <= '9' ))
#endif // !#ifndef _UTILS_HXX_
#define ISEXTDIGIT(a) ISDIGIT(a) || ( a == '.' ) || ( a == '-' )

static int is_digits(char * arg)
{
    size_t len,i;
    len = strlen(arg);
    for (i = 0; i < len; i++) {
        if ( !ISDIGIT(arg[i]) )
            return 0;
    }
    return 1; /* is all digits */
}

int get_bbox( char *bbox )
{
    int c;
    size_t ii, len = strlen(bbox);
    int off = 0;
    int cnt = 0;
    double d;
    for (ii = 0; ii < len; ii++) {
        c = bbox[ii];
        if (c <= ' ')
            continue;
        if (ISEXTDIGIT(c)) {
            header[off++] = (char)c;
        } else {
            if (off) {
                header[off] = 0;
                d = atof(header);
                switch(cnt) {
                case 0:
                    min_lon = d;
                    break;
                case 1:
                    min_lat = d;
                    break;
                case 2:
                    max_lon = d;
                    break;
                case 3:
                    max_lat = d;
                    break;
                default:
                    SPRTF("%s: bbox '%s' FAILED! Too many values\n", module, bbox);
                    return 0;   // failed
                }
                cnt++;
                off = 0;
            }
        }
    }
    if (off) {
        header[off] = 0;
        d = atof(header);
        switch(cnt) {
        case 0:
            min_lon = d;
            break;
        case 1:
            min_lat = d;
            break;
        case 2:
            max_lon = d;
            break;
        case 3:
            max_lat = d;
            break;
        default:
            SPRTF("%s: bbox '%s' FAILED! Too many values\n", module, bbox);
            return 0;   // failed
        }
        cnt++;
        off = 0;
    }
    if (cnt < 4) {
        SPRTF("%s: bbox '%s' FAILED! Not enough values\n", module, bbox);
        return 0;
    }
    if (min_lon > max_lon) {
        SPRTF("%s: bbox '%s' FAILED! Min lon GT max lon!\n", module, bbox);
        return 0;
    }
    if (min_lat > max_lat) {
        SPRTF("%s: bbox '%s' FAILED! Min lat GT max lat!\n", module, bbox);
        return 0;
    }
    if ( !(min_lon < max_lon) ) {
        SPRTF("%s: bbox '%s' FAILED! Min lon EQUALS max lon!\n", module, bbox);
        return 0;
    }
    if ( !(min_lat < max_lat) ) {
        SPRTF("%s: bbox '%s' FAILED! Min lat EQUALS max lat!\n", module, bbox);
        return 0;
    }

    if (!in_world_range(min_lat,min_lon)) {
        SPRTF("%s: bbox '%s' FAILED! Min lon or lat out of range!\n", module, bbox);
        return 0;
    }
    if (!in_world_range(max_lat,max_lon)) {
        SPRTF("%s: bbox '%s' FAILED! Max lon or lat out of range!\n", module, bbox);
        return 0;
    }
    got_bbox = true;
    return 1; // success
}

int parse_log_file( int argc, char **argv )
{
    int i, c, i2;
    char *arg, *sarg;
    set_def_log(def_log);
    for (i = 1; i < argc; i++) {
        i2 = i + 1;
        arg = argv[i];
        if (*arg == '-') {
            sarg = &arg[1];
            while (*sarg == '-') sarg++;
            c = *sarg;
            switch (c) {
            case 'l':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    set_log_file((char*)sarg, false);
                    if (VERB1) SPRTF("%s: Set log file to '%s'\n", module, get_log_file());
                } else {
                    SPRTF("%s: Expected log file name to follow %s! Aborting...\n", module, arg);
                    return 1;
                }
                break;
            }
        }
    }
    strcpy(curr_xg, get_log_path());
    strcat(curr_xg, def_xg);
    return 0;
}


int parse_args( int argc, char **argv )
{
    int i, c, i2;
    char *arg, *sarg;
    size_t len = 0;
    bool opt = false;
    i = parse_log_file(argc,argv);    // deal with the log file first
    if (i)
        return i;
    for (i = 1; i < argc; i++) {
        i2 = i + 1;
        arg = argv[i];
        if (*arg == '-') {
            if (strcmp(arg, "--version") == 0) {
                show_version();
                return 2;
            }
            sarg = &arg[1];
            while (*sarg == '-') sarg++;
            c = *sarg;
            len = strlen(sarg);
            switch (c) {
            case '?':
            case 'h':
                give_help(argv[0]);
                return 2;
                break;
            case 'b':
            case 'B':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    if (c == 'B')
                        add_boundary = true;
                    if (get_bbox(sarg)) {
                        if (VERB1) SPRTF("%s: Set BBOX to '%s'\n", module, sarg);
                    } else {
                        SPRTF("%s: Failed to get bbox from '%s'\n", module, sarg);
                        return 1;
                    }
                } else {
                    SPRTF("%s: Expected bbox to follow %s! Aborting...\n", module, arg);
                    return 1;
                }
                break;
            case 'c':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    out_color = strdup(sarg);
                    if (VERB1) SPRTF("%s: Set out 'color' to '%s'.\n", module, out_color);
                }
                else {
                    SPRTF("%s: Expected 'color' to follow %s! Aborting...\n", module, arg);
                    return 1;
                }
                break;
            case 'd':
                opt = true;
                if ((len > 1) && sarg[len - 1] == '-')
                    opt = false;
                headers_only = opt;
                if (VERB1) {
                    SPRTF("%s: Set display headers only %s.\n", module,
                        (headers_only ? "On, skipping points" : "Off"));
                }
                break;
            case 'f':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    fudge = atof(sarg);
                    if (VERB1) SPRTF("%s: Set 'fudge' factor to '%s'\n", module, sarg);
                } else {
                    SPRTF("%s: Expected fudge 'degrees' to follow %s! Aborting...\n", module, arg);
                    return 1;
                }
                break;
            case 'l':
                i++;    // already handled
                break;
            case 'v':
                sarg++; // skip the -v
                if (*sarg) {
                    // expect digits
                    if (is_digits(sarg)) {
                        verbosity = atoi(sarg);
                    } else if (*sarg == 'v') {
                        verbosity++; /* one inc for first */
                        while(*sarg == 'v') {
                            verbosity++;
                            arg++;
                        }
                    } else
                        goto Bad_CMD;
                } else
                    verbosity++;
                if (VERB1) SPRTF("%s: Set verbosity to %d\n", module, verbosity);
                break;
            case 'w':
                opt = true;
                if ((len > 1) && sarg[len - 1] == '-')
                    opt = false;
                whole_features = opt;
                if (VERB1) {
                    SPRTF("%s: Set whole feature %s, if any point in bbox.\n", module,
                        (whole_features ? "On" : "Off"));
                }
                break;
            case 'x':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    if (strcmp(sarg,"none")) {
                        strcpy(curr_xg, sarg);
                        if (VERB1) SPRTF("%s: Set xg out file to '%s'\n", module, curr_xg);
                    } else {
                        strcpy(curr_xg, sarg);
                        if (VERB1) SPRTF("%s: Set xg out file is disabled. (%s)\n", module, curr_xg);
                    }
                } else {
                    SPRTF("%s: Expected xg out file (or none) to follow %s! Aborting...\n", module, arg);
                    return 1;
                }
                break;
            case 'F':
                opt = true;
                if ((len > 1) && sarg[len - 1] == '-')
                    opt = false;
                add_point_fix = opt;
                if (VERB1) {
                    SPRTF("%s: Set add point fix %s.\n", module,
                        (add_point_fix ? "On" : "Off"));
                }
                break;
            case 'W':
                opt = true;
                if ((len > 1) && sarg[len - 1] == '-')
                    opt = false;
                check_wrap = opt;
                if (VERB1) {
                    SPRTF("%s: Set check screen wrap %s.\n", module,
                        (check_wrap ? "On" : "Off"));
                }
                break;
            default:
Bad_CMD:
                SPRTF("%s: Unknown argument '%s'! Aborting...\n", module, arg );
                return 1;
            }
        } else {
            if (in_file) {
                SPRTF("%s: Already have input file '%s'\n", module, in_file);
                SPRTF("What is this '%s'! Aborting...\n", arg );
                return 1;
            }
            in_file = strdup(arg);
        }
    }
    return 0;
}

int screen_wrap(PointD pt1, PointD pt2)
{
    if ((pt1.x < -170) && (pt2.x > 170))
        return 1;

    if ((pt2.x < -170) && (pt1.x > 170))
        return 1;

    return 0;   // noscreen wrap
}

// main() OS entry
int main( int argc, char **argv )
{
   	size_t n_read;
    unsigned int m, this_id, is_line, level, n_seg = 0, seg_no = 0;
   	int version, greenwich, is_river, src;
   	double w, e, s, n, area, f_area, scale = 10.0;
    bool OK, first = true;
    char source;
   	int32_t max_east = 270000000;
    geomtype geometry = IS_POLY;
    char container[8];
    char ancestor[8];
    uint32_t row, pt_count = 0;
    char *name[2] = {"polygon", "line"};
    std::stringstream xg;
    PointD pt, pt2;
    vPTS vPoints;
    bool inbbox = false;
    bool inworld = false;
    int outworld = 0;
    int wrapped = 0;
    const char *title = "    id points lvso       area     f_area       west       east     south     north contai ancest";
    int iret = parse_args(argc,argv);
    if (iret) {
        if (iret == 2)
            iret = 0;
        return iret;
    }
    if (in_file == 0) {
        give_help(argv[0]);
        SPRTF("%s: ERROR: No input file found in command! Aborting...\n", module );
        return 1;
    }
    FILE *fp = fopen(in_file,"rb");
    if (!fp) {
        SPRTF("%s: Unable to open file '%s'! Aborting...\n", module, in_file );
        return 1;
    }
    //////////////////////////////////////////////////////////////
   	sprintf(header, "# Data extracted from GSHHG file %s", in_file);
    xg << header;
    xg << std::endl;
    if (VERB1)
        SPRTF("%s\n",header);
    if (got_bbox) {
        if (fudge > 0.0) {
            sprintf(header,"# bbox=%lf,%lf,%lf,%lf (min lon,lat max lon,lat), adjusted by fudge %lf\n",
                min_lon, min_lat, max_lon, max_lat, fudge );
            min_lon -= fudge;
            max_lon += fudge;
            min_lat -= fudge;
            max_lat += fudge;
        } else {
            sprintf(header,"# bbox=%lf,%lf,%lf,%lf (min lon,lat max lon,lat)\n", min_lon, min_lat, max_lon, max_lat );
        }
        xg << header;
        if (VERB1)
            SPRTF("%s",header);
        if (add_boundary)
        {
            xg << "color " << bnd_color << std::endl; // boundary in gray
            xg << min_lon << " " << min_lat << " # BL" << std::endl;
            xg << min_lon << " " << max_lat << " # TL" << std::endl;
            xg << max_lon << " " << max_lat << " # TR" << std::endl;
            xg << max_lon << " " << min_lat << " # BR" << std::endl;
            xg << min_lon << " " << min_lat << " # BL" << std::endl;
            xg << "NEXT" << std::endl;
        }
    }
    xg << "color " << out_color << std::endl; // outline in blue
    //////////////////////////////////////////////////////////////
	n_read = fread (&h, sizeof (struct GSHHG), 1U, fp);
    while (n_read == 1) {
		n_seg++;
		if (must_swab) /* Must deal with different endianness */
			bswap_GSHHG_struct (&h);

		/* OK, we want to return info for this feature */
		level = h.flag & 255;				/* Level is 1-4 [5-6 for Antarctica] */
		version = (h.flag >> 8) & 255;			/* Version is 1-255 */
		if (first) {
            if (VERB2)
                SPRTF("%s: Found GSHHG/WDBII version %d in file %s\n", module, version, in_file);
        }
		first = false;
		greenwich = (h.flag >> 16) & 3;			/* Greenwich is 0-3 */
		src = (h.flag >> 24) & 1;			/* Source is 0 (WDBII) or 1 (WVS) */
		is_river = (h.flag >> 25) & 1;			/* River is 0 (not river) or 1 (is river) */
		m = h.flag >> 26;				/* Magnitude for area scale */
		w = h.west  * GSHHG_SCL;			/* Convert region from microdegrees to degrees */
		e = h.east  * GSHHG_SCL;
		s = h.south * GSHHG_SCL;
		n = h.north * GSHHG_SCL;
		source = (level > 4) ? 'A' : ((src == 1) ? 'W' : 'C');		/* Either Antarctica, WVS or CIA (WDBII) pedigree */
		if (is_river) source = (char)tolower ((int)source);	/* Lower case c means river-lake */
		is_line = (h.area) ? 0 : 1;			/* Either Polygon (0) or Line (1) (if no area) */
		if (is_line && geometry == IS_POLY) geometry = IS_LINE;	/* Change from polygon to line geometry */
		if (version >= 9) {				/* Magnitude for area scale */
			m = h.flag >> 26;
			scale = pow (10.0, (double)m);	/* Area scale */
		}
		area = h.area / scale;				/* Now im km^2 */
		f_area = h.area_full / scale;		/* Now im km^2 */
		this_id = h.id;
        OK = true;
		if (!OK) {	/* Not what we are looking for, skip to next */
			fseek (fp, (off_t)(h.n * sizeof(struct POINT)), SEEK_CUR);
			n_read = fread (&h, sizeof (struct GSHHG), 1U, fp);	/* Get the next GSHHG header */
			continue;	/* Back to top of loop */
		}
		/* Create the segment/polygon header record */
		if (is_line) {	/* River or border line-segment */
			sprintf (header, "%6d%8d%3d%2c%11.5f%10.5f%10.5f%10.5f", h.id, h.n, level, source, w, e, s, n);
			max_east = 180000000;	/* For line segments we always use -180/+180  */
		}
		else {		/* Island or lake polygon */
			(h.container == -1) ? sprintf(container, "%6s", "-") : sprintf (container, "%6d", h.container);
			(h.ancestor == -1) ? sprintf (ancestor, "%5s","-") : sprintf (ancestor, "%6d", h.ancestor);
			sprintf (header, "%6d%8d%2d%2c %.12g %.12g%11.5f%11.5f%10.5f%10.5f %s %s", h.id, h.n, level, source, area, f_area, w, e, s, n, container, ancestor);
		}
        if (VERB5 || headers_only) {
            if (title)
                SPRTF("%s\n",title);
            SPRTF("%s\n", header);
            title = 0;
        }
        if (headers_only) {
			fseek (fp, (off_t)(h.n * sizeof(struct POINT)), SEEK_CUR);
        } else if (whole_features) {
            vPoints.clear();
            inbbox = got_bbox ? false : true;
			for (row = 0; row < h.n; row++) {
				if (fread (&p, sizeof (struct POINT), 1U, fp) != 1) {
					SPRTF("%s: Error reading file %s for %s %d, point %d.\n", module,
                        in_file, name[is_line], h.id, row);
					return 1;
				}
				if (must_swab) /* Must deal with different endianness */
					bswap_POINT_struct (&p);
				pt.x = p.x * GSHHG_SCL;
				if ((greenwich && p.x > max_east) || (h.west > 180000000))
                    pt.x -= 360.0;
				pt.y = p.y * GSHHG_SCL;
                inworld = in_world_range(pt.y, pt.x);
                if (!inworld) {
                    if (VERB5) {
                        SPRTF("%s: File %s for %s %d, point %d - warn %f,%f NOT IN WORLD.\n", module,
                            in_file, name[is_line], h.id, row, pt.y, pt.x);
                    }
                    outworld++;
                    if (add_point_fix) {
                        if (pt.x > 180.0)
                            pt.x -= 360.0;
                        else if (pt.x < -180.0)
                            pt.x += 360.0;
                        if (pt.y > 90.0)
                            pt.y -= 180.0;
                        else if (pt.y < -90.0)
                            pt.y += 180.0;
                        if (in_world_range(pt.y, pt.x)) {
                            if (VERB5) {
                                SPRTF("Point ajusted to %f,%f\n", pt.y, pt.x);
                            }
                        }
                        else {
                            if (VERB5) {
                                SPRTF("Failed in adj to %f,%f\n", pt.y, pt.x);
                            }
                        }
                    }
                }
                vPoints.push_back(pt);
                if (got_bbox && !inbbox) {
                    if (in_bbox( pt.y, pt.x )) {
                        inbbox = true;
                    }
                }
			}
            if (h.n && inbbox) {
                size_t jj, max = vPoints.size();
                for (jj = 0; jj < max; jj++) {
                    pt = vPoints[jj];
                    if (jj && screen_wrap(pt2, pt)) {
                        if (check_wrap)
                            xg << "NEXT" << std::endl;
                        wrapped++;
                    }
                    xg << pt.x << " " << pt.y << std::endl;
                    pt_count++;
                    pt2 = pt;
                }
                xg << "NEXT" << std::endl;
            }
        } else if (h.n) {
            // here only if two or more points of a this feature are within the bbox
            // if there is a bounding box given
            size_t cnt = 0;
            vPoints.clear();
            inbbox = got_bbox ? false : true;
			for (row = 0; row < h.n; row++) {
				if (fread (&p, sizeof (struct POINT), 1U, fp) != 1) {
					SPRTF("%s: Error reading file %s for %s %d, point %d.\n", module,
                        in_file, name[is_line], h.id, row);
					return 1;
				}
				if (must_swab) /* Must deal with different endianness */
					bswap_POINT_struct (&p);
				pt.x = p.x * GSHHG_SCL;
				if ((greenwich && p.x > max_east) || (h.west > 180000000))
                    pt.x -= 360.0;
				pt.y = p.y * GSHHG_SCL;
                inworld = in_world_range(pt.y, pt.x);
                if (!inworld) {
                    if (VERB5) {
                        SPRTF("%s: File %s for %s %d, point %d - warn %f,%f NOT IN WORLD.\n", module,
                            in_file, name[is_line], h.id, row, pt.y, pt.x);
                    }
                    outworld++;
                    if (add_point_fix) {
                        if (pt.x > 180.0)
                            pt.x -= 360.0;
                        else if (pt.x < -180.0)
                            pt.x += 360.0;
                        if (pt.y > 90.0)
                            pt.y -= 180.0;
                        else if (pt.y < -90.0)
                            pt.y += 180.0;
                        if (in_world_range(pt.y, pt.x)) {
                            if (VERB5) {
                                SPRTF("Point ajusted to %f,%f\n", pt.y, pt.x);
                            }
                        }
                        else {
                            if (VERB5) {
                                SPRTF("Failed in adj to %f,%f\n", pt.y, pt.x);
                            }
                        }
                    }
                }
                vPoints.push_back(pt);  // collect ALL points of this feature
                inbbox = in_bbox( pt.y, pt.x );
                if (inbbox) {
                    if (cnt) {
                        if (cnt == 1) {
                            xg << pt2.x << " " << pt2.y << std::endl;
                            pt_count++;
                        }
                        if (screen_wrap(pt2, pt)) {
                            wrapped++;
                            if (check_wrap) {
                                xg << "NEXT" << std::endl;  // close line
                                if (VERB5) {
                                    SPRTF("%s: Found possible wrap %f vs %f, and fixed adding a 'next'!\n",
                                        module, pt2.x, pt.x);
                                }
                            }
                            else {
                                if (VERB5) {
                                    SPRTF("%s: Found possible wrap %f vs %f! But Fix wrap is off.\n",
                                        module, pt2.x, pt.x);
                                }
                            }
                        }
                        xg << pt.x << " " << pt.y << std::endl;
                        pt_count++;
                    }
                    pt2 = pt;
                    cnt++;
                } else {
                    if (cnt > 1) {
                        xg << "NEXT" << std::endl;  // close line
                    }
                    cnt = 0;
                }
			}
            if (cnt) {
                 xg << "NEXT" << std::endl; // close line
            }
        }
		seg_no++;
		max_east = 180000000;	/* Only Eurasia (the first polygon) needs 270 */
		n_read = fread (&h, sizeof (struct GSHHG), 1U, fp);	/* Get the next GSHHG header */
    }
    fclose(fp);
    if (pt_count && (strcmp(curr_xg,"none"))) {
        if (is_file_or_directory((char *)curr_xg) == 1) {
            char* tb = GetNxtBuf();
            strcpy(tb, curr_xg);
            strcat(tb, ".bak");
            if (is_file_or_directory(tb) == 1) {
                if (delete_file(tb)) {
                    if (VERB1) SPRTF("%s: FAILED to delete previous backup file '%s'\n", module, tb);
                }
            }
            if (rename_file(curr_xg, tb)) {
                if (VERB1) SPRTF("%s: FAILED to RENAME file '%s' to '%s'\n", module, curr_xg, tb);
            }
        }
        fp = fopen(curr_xg,"w");
        if (fp) {
            n_read = fwrite( xg.str().c_str(), 1, xg.str().size(), fp);
            fclose(fp);
            if (n_read == xg.str().size()) {
                if (VERB1) SPRTF("%s: Written %d points to '%s'\n", module, pt_count, curr_xg);
            }
            else {
                if (VERB1) SPRTF("%s: FAILED to write %d points to '%s'\n", module, pt_count, curr_xg);
            }
        }
        else {
            if (VERB1) SPRTF("%s: FAILED to write %d points to '%s'\n", module, pt_count, curr_xg);
        }
    } else if (strcmp(curr_xg,"none") && !headers_only) {
        if (VERB1) SPRTF("%s: Got NO points!\n", module );
    }
    if (VERB1) {
        if (outworld) {
            SPRTF("%s: Had %d points out of world range, %s.\n", module, outworld,
                (add_point_fix ? "fixed" : "left as is"));
        }
        if (wrapped) {
            SPRTF("%s: Had %d points appear to wrap screen, %s.\n", module, wrapped,
                (check_wrap ? "fixed" : "left as is"));

        }
    }
    return iret;
}

// eof = gshhg.cxx
