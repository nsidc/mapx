/*------------------------------------------------------------------------
 * integerized sinusoidal
 *------------------------------------------------------------------------*/
#include "define.h"
#include "mapx.h"
#include "isin.h"

int init_integerized_sinusoidal(mapx_class *current)
{ 
  long nrows;

  nrows = nint(PI*current->Rg);

  current->isin_data = 
    (void *)Isin_inv_init(current->equatorial_radius,
			  RADIANS(current->lon0), 
			  (double)0, (double)0, 
			  nrows, current->isin_justify);

  if (NULL == current->isin_data) return -1;

  return 0;
}

int integerized_sinusoidal(mapx_class *current, float lat, float lon, float *u, float *v)
{
  int status;
  double x, y, phi, lam;
  
  NORMALIZE(lon);
  
  phi = RADIANS(lat);
  lam = RADIANS(lon);

  status = Isin_fwd((Isin_t *)(current->isin_data), lam, phi, &x, &y);
  
  *u = current->T00*x + current->T01*y - current->u0;
  *v = current->T10*x + current->T11*y - current->v0;
  
  return status;
}

int inverse_integerized_sinusoidal(mapx_class *current, float u, float v, float *lat, float *lon)
{
  int status;
  double phi, lam, x, y;
  
  x =  current->T00*(u+current->u0) - current->T01*(v+current->v0);
  y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);

  status = Isin_inv((Isin_t *)(current->isin_data), x, y, &lam, &phi);

  *lat = DEGREES(phi);
  *lon = DEGREES(lam);
  NORMALIZE(*lon);
  
  return status;
}
