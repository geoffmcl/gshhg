// utils.hxx
#ifndef _UTILS_HXX_
#define _UTILS_HXX_
/* Inhibit C++ name-mangling for util functions */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern char *get_file_name(char *file);
extern int is_file_or_directory( char *file ); // 1=file 2=directory, else 0
extern size_t get_last_file_size(); // last file stat'ed above

#define ISDIGIT(a) (( a >= '0' )&&( a <= '9'))
extern int is_an_integer( char *arg );
extern void HexDump(unsigned char *ptr, int length, bool addhdr = false, bool addascii = true, bool addspace = true);

extern int in_world_range(double lat, double lon);

#ifdef _MSC_VER
#define M_IS_DIR _S_IFDIR
#else // !_MSC_VER
#define M_IS_DIR S_IFDIR
#endif

// return The distance between the two points in kilometers
extern double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // #ifndef _UTILS_HXX_
// eof

