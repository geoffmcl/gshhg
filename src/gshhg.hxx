/*\
 * gshhg.hxx
 *
 * Copyright (c) 2014 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#ifndef _GSHHG_HXX_
#define _GSHHG_HXX_

#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

/*
 * Default inline functions that the compiler should optimize properly. Use
 * these functions if you know that you are dealing with constant values.
 */
static inline uint16_t bswap16 (uint16_t x) {
	return
		(((x & 0x00FFU) << 8) |
		 ((x & 0xFF00U) >> 8));
}

static inline uint32_t bswap32 (uint32_t x) {
	return
		(((x & 0xFF000000U) >> 24) |
		 ((x & 0x00FF0000U) >>  8) |
		 ((x & 0x0000FF00U) <<  8) |
		 ((x & 0x000000FFU) << 24));
}

static inline uint64_t bswap64 (uint64_t x) {
	return
		(((x & 0x00000000000000FFULL) << 56) |
		 ((x & 0x000000000000FF00ULL) << 40) |
		 ((x & 0x0000000000FF0000ULL) << 24) |
		 ((x & 0x00000000FF000000ULL) << 8) |
		 ((x & 0x000000FF00000000ULL) >> 8) |
		 ((x & 0x0000FF0000000000ULL) >> 24) |
		 ((x & 0x00FF000000000000ULL) >> 40) |
		 ((x & 0xFF00000000000000ULL) >> 56));
}


#define GSHHG_MAXPOL	200000	/* Should never need to allocate more than this many polygons */
#define GSHHG_SCL	1.0e-6	/* Convert micro-degrees to degrees */

struct GSHHG {	/* Global Self-consistent Hierarchical High-resolution Shorelines */
	uint32_t id;		/* Unique polygon id number, starting at 0 */
	uint32_t n;		/* Number of points in this polygon */
	uint32_t flag;	/* = level + version << 8 + greenwich << 16 + source << 24 + river << 25 + p << 26 */
	/* flag contains 6 items, as follows:
	 * low byte:	level = flag & 255: Values: 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake
	 * 2nd byte:	version = (flag >> 8) & 255: Values: Should be 9 for GSHHG release 9.
 	 * 3rd byte:	greenwich = (flag >> 16) & 3: Values: 0 if Greenwich nor Dateline are crossed,
	 *		1 if Greenwich is crossed, 2 if Dateline is crossed, 3 if both is crossed.
	 * 4th byte:	source = (flag >> 24) & 1: Values: 0 = CIA WDBII, 1 = WVS
	 * 4th byte:	river = (flag >> 25) & 1: Values: 0 = not set, 1 = river-lake and GSHHG level = 2 (or WDBII class 0)
	 * 4th byte:	area magnitude scale p (as in 10^p) = flag >> 26.  We divide area by 10^p.
	 */
	int32_t west, east, south, north;	/* Signed min/max extent in micro-degrees */
	uint32_t area;		/* Area of polygon in km^2 * 10^p for this resolution file */
	uint32_t area_full;	/* Area of corresponding full-resolution polygon in km^2 * 10^p */
	int32_t container;	/* Id of container polygon that encloses this polygon (-1 if none) */
	int32_t ancestor;	/* Id of ancestor polygon in the full resolution set that was the source of this polygon (-1 if none) */
};

/* byteswap all members of GSHHG struct */
#define GSHHG_STRUCT_N_MEMBERS 11
static inline void bswap_GSHHG_struct (struct GSHHG *h) {
	uint32_t unsigned32[GSHHG_STRUCT_N_MEMBERS];
	uint32_t n;

	/* since all members are 32 bit words: */
	memcpy (&unsigned32, h, sizeof(struct GSHHG));

	for (n = 0; n < GSHHG_STRUCT_N_MEMBERS; ++n)
		unsigned32[n] = bswap32 (unsigned32[n]);

	memcpy (h, &unsigned32, sizeof(struct GSHHG));
}

struct	POINT {	/* Each lon, lat pair is stored in micro-degrees in 4-byte integer format */
	int32_t x;
	int32_t y;
};

/* byteswap members of POINT struct */
static inline void bswap_POINT_struct (struct POINT *p) {
	uint32_t unsigned32;
	memcpy (&unsigned32, &p->x, sizeof(uint32_t));
	unsigned32 = bswap32 (unsigned32);
	memcpy (&p->x, &unsigned32, sizeof(uint32_t));
	memcpy (&unsigned32, &p->y, sizeof(uint32_t));
	unsigned32 = bswap32 (unsigned32);
	memcpy (&p->y, &unsigned32, sizeof(uint32_t));
}


#endif // #ifndef _GSHHG_HXX_
// eof - gshhg.hxx
