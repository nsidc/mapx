/*------------------------------------------------------------------------
 * albers_conic_equal_area
 *------------------------------------------------------------------------*/
#include "define.h"
#include "mapx.h"

int init_albers_conic_equal_area(mapx_class *current)
{
  current->sin_phi0 = sin(RADIANS(current->center_lat));
  current->sin_phi1 = sin(RADIANS(current->lat0));
  current->cos_phi1 = cos(RADIANS (current->lat0));

  if (999 == current->lat1 || current->lat0 == current->lat1)
    current->n = current->sin_phi1;
  else
    current->n = (current->sin_phi1 + sin(RADIANS(current->lat1)))/2;

  current->C = (current->cos_phi1*current->cos_phi1
		+ 2*current->n*current->sin_phi1);

  current->rho0 = 
    current->Rg*sqrt(current->C - 2*current->n*current->sin_phi0)/current->n;

  return 0;
}

int albers_conic_equal_area(mapx_class *current, float lat, float lon, 
			    float *u, float *v)
{
  float x, y;
  double phi, lam, sin_phi, rho, theta;
  
  phi = RADIANS(lat);
  lam = RADIANS(lon - current->lon0);

  sin_phi = sin(phi);
  rho = current->Rg*sqrt(current->C - 2*current->n*sin_phi)/current->n;
  theta = current->n*lam;

  x = rho*sin(theta);
  y = current->rho0 - rho*cos(theta);

  *u = current->T00*x + current->T01*y - current->u0;
  *v = current->T10*x + current->T11*y - current->v0;
  
  return 0;
}

int inverse_albers_conic_equal_area(mapx_class *current, float u, float v, 
				    float *lat, float *lon)
{
  double phi, lam, rho, rmy, theta, chi, x, y;

  x =  current->T00*(u+current->u0) - current->T01*(v + current->v0);
  y = -current->T10*(u+current->u0) + current->T11*(v + current->v0);

  rmy = current->rho0 - y;
  rho = sqrt(x*x + rmy*rmy);
  theta = atan2(x, rmy);

  chi = rho*current->n/current->Rg;
  phi = asin((current->C - chi*chi)/(2*current->n));
  lam = theta/current->n;

  *lat = DEGREES(phi);
  *lon = DEGREES(lam) + current->lon0;
  NORMALIZE(*lon);
  
  return 0;
}
