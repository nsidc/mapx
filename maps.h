/*========================================================================
 * maps - map utility functions
 *
 * 18-Aug-1992 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *========================================================================*/
#ifndef maps_h_
#define maps_h_

#include <math.h>
#include "mapx.h"

#define LAT_DESIGNATORS "NSns"
#define LON_DESIGNATORS "EWew"

/*
 *	printf latitude north south, use %5.2f%c format
 *	for example printf("%5.2f%c", PF_LAT_NS(lat));
 */

#define PF_LAT_NS(lat) fabs((float)lat), ((lat) >= 0 ? 'N' : 'S')

/*
 *	printf longitude east west, use %6.2f%c format
 *	for example printf("%6.2f%c", PF_LON_EW(lon));
 */

#define PF_LON_EW(lon) fabs((float)lon), ((lon) >= 0 ? 'E' : 'W')

/*
 *	function prototypes
 */
void draw_graticule(mapx_class *mapx, int (*move_pu)(float lat, float lon),
		    int (*draw_pd)(float lat, float lon),
		    int (*label)(char *string, float lat, float lon));
float arc_length_km (float lat1, float lon1, float lat2, float lon2);
float west_azimuth(float lat1, float lon1, float lat2, float lon2);
void bisect(float lat1, float lon1, float lat2, float lon2, 
	    float *lat, float *lon);
int sscanf_lat_lon(char *readln, float *lat, float *lon);
int lat_lon_decode(const char *readln, const char *designators, float *value);

FILE *search_path_fopen(char *filename, const char *pathvar, const char *mode);
#endif
