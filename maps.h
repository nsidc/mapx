/*========================================================================
 * maps - map utility functions
 *
 *	18-Aug-1992 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 *========================================================================*/
#ifndef maps_h_
#define maps_h_

#include <mapx.h>
#include <grids.h>
#include <cdb.h>

/*
 *	function prototypes
 */
void draw_graticule(mapx_class *mapx, void (*move_pu)(float lat, float lon),
		    void (*draw_pd)(float lat, float lon),
		    void (*label)(char *string, float lat, float lon));
double arc_length_km (float lat1, float lon1, float lat2, float lon2);
void bisect(float lat1, float lon1, float lat2, float lon2, 
	    float *lat, float *lon);
int sscanf_lat_lon(char *readln, double *lat, double *lon);

#endif
