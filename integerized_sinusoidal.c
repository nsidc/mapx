/*------------------------------------------------------------------------
 * integerized sinusoidal
 *------------------------------------------------------------------------*/
static const char integerized_sinusoidal_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/integerized_sinusoidal.c,v 1.2 2003-06-24 22:08:02 haran Exp $";

#include "isin.h"
#include "define.h"
#include "mapx.h"

char *id_integerized_sinusoidal(void)
{
  return((char *)integerized_sinusoidal_c_rcsid);
}

int init_integerized_sinusoidal(mapx_class *current)
{
  current->isin_data = 
    (void *)Isin_inv_init(current->equatorial_radius,
			  RADIANS(current->lon0), 
			  current->false_easting, current->false_northing, 
			  current->isin_nzone, current->isin_justify);

  if (NULL == current->isin_data) return -1;

  return 0;
}

int integerized_sinusoidal(mapx_class *current,
			   double lat, double lon, double *x, double *y)
{
  int status;
  double phi, lam;
  
  NORMALIZE(lon);
  
  phi = RADIANS(lat);
  lam = RADIANS(lon);

  status = Isin_fwd((Isin_t *)(current->isin_data), lam, phi, x, y);
  
  *x += current->false_easting;
  *y += current->false_northing;
  
  return status;
}

int inverse_integerized_sinusoidal(mapx_class *current,
				   double x, double y, double *lat, double *lon)
{
  int status;
  double phi, lam;
  
  x -= current->false_easting;
  y -= current->false_northing;

  status = Isin_inv((Isin_t *)(current->isin_data), x, y, &lam, &phi);

  *lat = DEGREES(phi);
  *lon = DEGREES(lam);
  NORMALIZE(*lon);
  
  return status;
}
