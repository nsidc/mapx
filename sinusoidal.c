/*------------------------------------------------------------------------
 * sinusoidal
 *------------------------------------------------------------------------*/
static const char sinusoidal_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/sinusoidal.c,v 1.2 2003-06-24 23:05:35 haran Exp $";

#include "define.h"
#include "mapx.h"

char *id_sinusoidal(void)
{
  return((char *)sinusoidal_c_rcsid);
}

int init_sinusoidal(mapx_class *current)
{
  /* no variables require initialization */
  return 0;
}

int sinusoidal(mapx_class *current,
	       double lat, double lon, double *x, double *y)
{
  double dlon;
  double phi, lam;
  
  dlon = lon - current->lon0;
  NORMALIZE(dlon);
  
  phi = RADIANS (lat);
  lam = RADIANS (dlon);
  
  *x =  current->Rg * lam * cos (phi);
  *y =  current->Rg * phi;
  
  *x += current->false_easting;
  *y += current->false_northing;
  
  return 0;
}

int inverse_sinusoidal(mapx_class *current,
		       double x, double y, double *lat, double *lon)
{
  double phi, lam;
  
  x -= current->false_easting;
  y -= current->false_northing;

  phi = y/current->Rg;
  lam = x/(current->Rg*cos(phi));
  
  *lat = DEGREES(phi);
  *lon = DEGREES(lam) + current->lon0;
  NORMALIZE(*lon);
  
  return 0;
}
