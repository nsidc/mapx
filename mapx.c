/*========================================================================
 * map projections - convert geographic to map coordinates
 *
 *	2-July-1991 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 *	16-Dec-1991 K.Knowles - put all global parameters into mapx_struct
 *	2-July-1992 K.Knowles - init returns pointer to struct
 *      10-Dec-1992 R.Swick - added error checking and flexible names
 *      15-Dec-1992 R.Swick - added ellipsoid projections.
 *========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <define.h>
#include <mapx.h>
#include <errno.h>
#include <ctype.h>
/*
 * local mapx parameters for map transformation functions
 */
static mapx_class *current;

/*
 * function prototypes for map transformations
 */
static int azimuthal_equal_area(float,float,float*,float*);
static int inverse_azimuthal_equal_area(float,float,float*,float*);
static int cylindrical_equidistant(float,float,float*,float*);
static int inverse_cylindrical_equidistant(float,float,float*,float*);
static int equal_area_cylindrical(float,float,float*,float*);
static int inverse_equal_area_cylindrical(float,float,float*,float*);
static int mercator(float,float,float*,float*);
static int inverse_mercator(float,float,float*,float*);
static int mollweide(float,float,float*,float*);
static int inverse_mollweide(float,float,float*,float*);
static int orthographic(float,float,float*,float*);
static int inverse_orthographic(float,float,float*,float*);
static int polar_stereographic(float,float,float*,float*);
static int inverse_polar_stereographic(float,float,float*,float*);
static int sinusoidal(float,float,float*,float*);
static int inverse_sinusoidal(float,float,float*,float*);
static int azimuthal_equal_area_ellipsoid(float,float,float*,float*);
static int inverse_azimuthal_equal_area_ellipsoid(float,float,float*,float*);
static int equal_area_cylindrical_ellipsoid(float,float,float*,float*);
static int inverse_equal_area_cylindrical_ellipsoid(float,float,float*,float*);

/*----------------------------------------------------------------------
 * init_mapx - initialize map projection 
 *
 *	input : map_file_name - map parameters file name
 *			format as follows:
 *			 Map_Projection_Name
 *			 lat0 lon0 [lat1 lon1] (decimal degrees)
 *			 rotation (portrait=0, landscape=90)
 *			 scale (kilometers per map unit)
 *			 center_lat center_lon (for map)
 *			 south north
 *			 west east
 *			 lat_interval lon_interval (for graticule)
 *			 label_lat label_lon (for graticule)
 *			 cil_detail bdy_detail riv_detail (see database_info)
 *	    [optional]   equitorial_radius inverse_flattening  (in Kilometers)
 *			                  
 *
 *			valid projection names are:
 *			 Azimuthal_Equal_Area
 *			 Equal_Area_Cylindrical
 *			 Mercator
 *			 Mollweide
 *			 Orthographic
 *			 Sinusoidal
 *			 Cylindrical_Equidistant
 *			 Polar_Stereographic
 *			 Azimuthal_Equal_Area_Ellipsoid			 
 *			 Equal_Area_Cylindrical_Ellipsoid
 *
 *	The parameter lat1 is currently used for the Polar_Stereographic
 *	projection to define the "true at" parallel (default pole) and
 *	the Equal_Area_Cylindrical projection to define the latitude of
 *	1:1 aspect ratio (default 30.00).
 *
 *	result: pointer to new mapx_class instance for this map
 *		or NULL if an error occurs during initialization
 *
 *----------------------------------------------------------------------*/
mapx_class *init_mapx (char *map_file_name)
{
  double theta, d1, d2;
  float f1, f2, f3, f4;
  int i1, i2, i3, ios;
  char projection[80], readln[80], old_projection[80];
  char *change(char *);
  mapx_class *this;

  this = (mapx_class *) malloc(sizeof(mapx_class));
  if (this == NULL)
  { perror("init_mapx");
    return NULL;
  }

  this->mpp_file_name = strdup(map_file_name);

  this->mpp_file = fopen (map_file_name, "r");
  if (this->mpp_file == NULL)
  { fprintf (stderr,"init_mapx: error opening parameters file.\n");
    perror(map_file_name);
    close_mapx(this);
    return NULL;
  }

  fgets (readln, sizeof(readln), this->mpp_file);
  sscanf (readln, "%s", old_projection);
  strcpy(projection, change(old_projection));

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f %f %f %f", &f1, &f2, &f3, &f4);
  this->lat0 = (ios >= 1) ? f1 : 0.0;
  this->lon0 = (ios >= 2) ? f2 : 0.0;
  this->lat1 = (ios >= 3) ? f3 : 999;
  this->lon1 = (ios >= 4) ? f4 : 999;

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f", &f1);
  this->rotation = (ios >= 1) ? f1 : 0.0;

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f", &f1);
  this->scale = (ios >= 1) ? f1 : 1.0;

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f %f", &f1, &f2);
  this->center_lat = (ios >= 1) ? f1 : 0.0;
  this->center_lon = (ios >= 2) ? f2 : 0.0;

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f %f", &f1, &f2);
  this->south = (ios >= 1) ? f1 : -90.;
  this->north = (ios >= 2) ? f2 :  90.;

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f %f", &f1, &f2);
  this->west = (ios >= 1) ? f1 : -180.;
  this->east = (ios >= 2) ? f2 :  180.;

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f %f", &f1, &f2);
  this->lat_interval = (ios >= 1) ? f1 : 30.;
  this->lon_interval = (ios >= 2) ? f2 : 30.;

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f %f", &f1, &f2);
  this->label_lat = (ios >= 1) ? f1 : 0.0;
  this->label_lon = (ios >= 2) ? f2 : 0.0;

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%d %d %d", &i1, &i2, &i3);
  this->cil_detail = (ios >= 1) ? i1 : 1;
  this->bdy_detail = (ios >= 2) ? i2 : 0;
  this->riv_detail = (ios >= 3) ? i3 : 0;
  
  if (ferror(this->mpp_file) || feof(this->mpp_file))
  { fprintf (stderr,"init_mapx: error reading parameters file.\n");
    if (feof(this->mpp_file))
      fprintf(stderr,"%s: unexpected end of file.\n", map_file_name);
    else
      perror(map_file_name);
    close_mapx(this);
    return NULL;
  }

  fgets (readln, sizeof(readln), this->mpp_file);       /******************/
  if feof(this->mpp_file)
  {this->equitorial_radius = 6378.2064;
   this->flattening = (1.0/294.98);
 }
  else
  {ios = sscanf (readln, "%lf %lf", &d1, &d2);             /****************/
  this->equitorial_radius = (ios >= 1) ? d1 : 6378.2064; /*****Clarke******/
  this->flattening = (ios >= 2) ? d2 : (1.0/294.98);     /******1866******/
 }

  if ferror(this->mpp_file)
  {fprintf (stderr, "init_mapx: error reading parameters file.\n");
   perror(map_file_name);
   close_mapx(this);
   return NULL;
 }

/*
 *	Some important notes on specifying longitudes:
 *	All longitudes should be >= -180 and <= 360.
 *	West to east should not span more than 360 degrees.
 *	West specifies the left side of the map and east the right,
 *	not necessarily the minimum and maximum longitudes.
 *	For purposes of bounds checking all longitudes are 
 *	normalized -180 to 180.
 */
  if (this->east < -180 || this->east > 360 
      || this->west < -180 || this->west > 360)
  { fprintf(stderr,"init_mapx: illegal bounds: west=%f, east=%f\n",
	    this->west, this->east);
    fprintf(stderr,"           should be >= -180 and <= 360\n");
    close_mapx(this);
    return NULL;
  }

  if (fabs(this->east - this->west) > 360)
  { fprintf(stderr,"init_mapx: illegal bounds: west=%f, east=%f\n",
	    this->west, this->east);
    fprintf(stderr,"           bounds cannot span > 360 degrees.\n");
    close_mapx(this);
    return NULL;
  }

  if (this->east > 180 && this->west > 180)
  { this->east -=360;
    this->west -=360;
  }

/*
 *	set flag for bounds checking
 */
  if (this->east < this->west || this->east > 180)
    this->map_stradles_180 = TRUE;
  else
    this->map_stradles_180 = FALSE;

  NORMALIZE(this->east);
  NORMALIZE(this->west);


  if ((strcmp (projection, "AZIMUTHALEQUALAREA") == 0) || 
      (strcmp (projection, "AZIMUTHALEQUALAREASPHERICAL") == 0) || 
      (strcmp (projection, "SPHERICALAZIMUTHALEQUALAREA") == 0) || 
      (strcmp (projection, "SPHERICALEQUALAREAAZIMUTHAL") == 0) || 
      (strcmp (projection, "EQUALAREAAZIMUTHALSPHERICAL") == 0) || 
      (strcmp (projection, "EQUALAREASPHERICALAZIMUTHAL") == 0) || 
      (strcmp (projection, "EQUALAREAAZIMUTHAL") == 0))
  { this->geo_to_map = azimuthal_equal_area;
    this->map_to_geo = inverse_azimuthal_equal_area;
    this->sin_phi1 = sin (RADIANS (this->lat0));
    this->cos_phi1 = cos (RADIANS (this->lat0));
  }
  else if ((strcmp (projection, "EQUALAREACYLINDRICAL") == 0) || 
	   (strcmp (projection, "CYLINDRICALEQUALAREA") == 0))
  { this->geo_to_map = equal_area_cylindrical;
    this->map_to_geo = inverse_equal_area_cylindrical;
    if (this->lat1 == 999) this->lat1 = 30.00;
    this->cos_phi1 = cos (RADIANS (this->lat1));
  }
  else if (strcmp (projection, "MERCATOR") == 0)
  { this->geo_to_map = mercator;
    this->map_to_geo = inverse_mercator;
    if (this->lat1 == 999) this->lat1 = 30.00;
    this->cos_phi1 = cos (RADIANS (this->lat1));
  }
  else if (strcmp (projection, "MOLLWEIDE") == 0)
  { this->geo_to_map = mollweide;
    this->map_to_geo = inverse_mollweide;
  }
  else if (strcmp (projection, "ORTHOGRAPHIC") == 0)
  { this->geo_to_map = orthographic;
    this->map_to_geo = inverse_orthographic;
    this->cos_phi1 = cos (RADIANS (this->lat0));
    this->sin_phi1 = sin (RADIANS (this->lat0));
  }
  else if (strcmp (projection, "SINUSOIDAL") == 0)
  { this->geo_to_map = sinusoidal;
    this->map_to_geo = inverse_sinusoidal;
  }
  else if ((strcmp (projection, "CYLINDRICALEQUIDISTANT") == 0) || 
	   (strcmp (projection, "EQUIDISTANTCYLINDRICAL") == 0))
  { this->geo_to_map = cylindrical_equidistant;
    this->map_to_geo = inverse_cylindrical_equidistant;
    this->cos_phi1 = cos (RADIANS (this->lat0));
  }
  else if ((strcmp (projection, "POLARSTEREOGRAPHIC") == 0)|| 
	   (strcmp (projection, "STEREOGRAPHICPOLAR") == 0))
  { this->geo_to_map = polar_stereographic;
    this->map_to_geo = inverse_polar_stereographic;
    if (this->lat1 == 999) this->lat1 = this->lat0;
    this->sin_phi1 = sin (RADIANS (this->lat1));
    if (this->lat0 != 90.00 && this->lat0 != -90.00)
    { fprintf(stderr,"only polar aspects allowed: lat0 = %7.2f\n", this->lat0);
      close_mapx(this);
      return NULL;
    }
  }

  else if ((strcmp (projection, "AZIMUTHALEQUALAREAELLIPSOID") == 0) || 
	   (strcmp (projection, "ELLIPSOIDAZIMUTHALEQUALAREA") == 0) || 
	   (strcmp (projection, "EQUALAREAELLIPSOIDAZIMUTHAL") == 0) || 
	   (strcmp (projection, "EQUALAREAAZIMUTHALELLIPSOID") == 0) || 
	   (strcmp (projection, "AZIMUTHALELLIPSOIDEQUALAREA") == 0) || 
	   (strcmp (projection, "ELLIPSOIDEQUALAREAAZIMUTHAL") == 0) )
  { this->geo_to_map = azimuthal_equal_area_ellipsoid;
    this->map_to_geo = inverse_azimuthal_equal_area_ellipsoid;
    this->eccentricity = sqrt((2.0*this->flattening) - (this->flattening * this->flattening));
    this->e2 = (this->eccentricity) * (this->eccentricity);
    this->e4 = (this->e2) * (this->e2);
    this->e6 = (this->e4) * (this->e2);
    this->qp = (1.0-(((1.0-(this->e2))/(2.0*(this->eccentricity)))*
		     (log((1.0-(this->eccentricity))/(1.0+(this->eccentricity))))));
    this->Rg = this->equitorial_radius / this->scale;
    this->Rq = this->Rg *(sqrt(this->qp/2.0));
    this->cos_phi1 = cos (RADIANS (this->lat0));
    this->sin_phi1 = sin (RADIANS (this->lat0));
    this->q1 = (1.0-(this->e2)) * ((this->sin_phi1) / 
				   (1.0-((this->e2) * (this->sin_phi1*this->sin_phi1)))-
				   (1.0/(2.0*(this->eccentricity))) * 
				   log((1.0-((this->eccentricity) * (this->sin_phi1))) / 
				       (1.0+((this->eccentricity) * (this->sin_phi1)))));
    this->beta1 = asin(this->q1/this->qp);
    this->sin_beta1 = sin(this->beta1);
    this->cos_beta1 = cos(this->beta1);
    this->m1 = (this->cos_phi1)/sqrt(1.0-((this->e2) * ((this->sin_phi1)*(this->sin_phi1))));
    this->D = ((this->Rg) * (this->m1)) / ((this->Rq) * (this->cos_beta1));

  }

  else if ((strcmp (projection, "CYLINDRICALEQUALAREAELLIPSOID") == 0) || 
	   (strcmp (projection, "ELLIPSOIDCYLINDRICALEQUALAREA") == 0) || 
	   (strcmp (projection, "EQUALAREAELLIPSOIDCYLINDRICAL") == 0) || 
	   (strcmp (projection, "EQUALAREACYLINDRICALELLIPSOID") == 0) || 
	   (strcmp (projection, "CYLINDRICALELLIPSOIDEQUALAREA") == 0) || 
	   (strcmp (projection, "ELLIPSOIDEQUALAREACYLINDRICAL") == 0) )
  { this->geo_to_map = equal_area_cylindrical_ellipsoid;
    this->map_to_geo = inverse_equal_area_cylindrical_ellipsoid;
    this->Rg = this->equitorial_radius / this->scale;
    this->phis = RADIANS(this->lat0);
    this->eccentricity = sqrt((2.0*this->flattening) - (this->flattening * this->flattening));
    this->e2 = (this->eccentricity) * (this->eccentricity);
    this->e4 = (this->e2) * (this->e2);
    this->e6 = (this->e4) * (this->e2);
    this->kz = cos(this->phis)/(sqrt(1.0 - ((this->e2)*sin(this->phis)*sin(this->phis))));
    this->qp = (1.0 - (this->e2)) * ((1.0/(1.0 - (this->e2)))
			     - (1.0/(2.0*(this->eccentricity))) * 
				log((1.0 - (this->eccentricity))/(1.0 + (this->eccentricity))));
  }
  else
  { fprintf (stderr, "init_mapx: unknown projection %s\n", old_projection);
    fprintf (stderr, "valid types are:\n");
    fprintf (stderr, " Azimuthal_Equal_Area\n");
    fprintf (stderr, " Equal_Area_Cylindrical\n");
    fprintf (stderr, " Mercator\n");
    fprintf (stderr, " Mollweide\n");
    fprintf (stderr, " Orthographic\n");
    fprintf (stderr, " Sinusoidal\n");
    fprintf (stderr, " Cylindrical_Equidistant\n");
    fprintf (stderr, " Polar_Stereographic\n");
    fprintf (stderr, " Azimuthal_Equal_Area_Ellipsoid\n");
    fprintf (stderr, " Equal_Area_Cylindrical_Ellipsoid\n");
    close_mapx (this);
    return NULL;
  }

  this->Rg = this->equitorial_radius / this->scale;

  theta = RADIANS(this->rotation);

  this->T00 =  cos(theta);
  this->T01 =  sin(theta);
  this->T10 = -sin(theta);
  this->T11 =  cos(theta);

  this->u0 = this->v0 = 0.0;
  forward_mapx (this, this->center_lat, this->center_lon, &f1, &f2);
  this->u0 = f1;
  this->v0 = f2;

  return this;
}

/*------------------------------------------------------------------------
 * close_mapx - free resources associated with active mapx struct
 *
 *	input : this - pointer to map data structure (returned by init_mapx)
 *
 *------------------------------------------------------------------------*/
void close_mapx (mapx_class *this)
{
  if (this == NULL) return;
  if (this->mpp_file != NULL) fclose (this->mpp_file);
  if (this->mpp_file_name != NULL) free (this->mpp_file_name);
  free (this);
}

/*------------------------------------------------------------------------
 * within_mapx - test if lat,lon are within map transformation bounds
 *
 *	input : this - pointer to map data structure (returned by init_mapx)
 *		lat,lon - geographic coordinates in decimal degrees
 *
 *	result: TRUE iff lat,lon are within specified mapx min,max
 *
 *------------------------------------------------------------------------*/
int within_mapx (mapx_class *this, float lat, float lon)
{
  if (lat < this->south || lat > this->north) return FALSE;

  NORMALIZE(lon);

  if (this->map_stradles_180)
  { if (lon > this->east && lon < this->west)
      return FALSE;
  }
  else
  { if (lon < this->west || lon > this->east)
      return FALSE;
  }

  return TRUE;
}

/*------------------------------------------------------------------------
 * forward_mapx - forward map transformation
 *
 *	input : this - pointer to map data structure (returned by init_mapx)
 *		lat,lon - geographic coordinates in decimal degrees
 *
 *	output: u,v - map coordinates in map units
 *
 *------------------------------------------------------------------------*/
int forward_mapx (mapx_class *this, float lat, float lon, float *u, float *v)
{
  int status;
  errno = 0;
  current = this;
  status = (*(this->geo_to_map))(lat, lon, u, v);
  if (errno != 0) 
    return -1; 
  else
    return status;
}

/*------------------------------------------------------------------------
 * inverse_mapx - inverse map transformation
 *
 *	input : this - pointer to map data structure (returned by init_mapx)
 *		u,v - map coordinates in map units
 *
 *	output: lat,lon - geographic coordinates in decimal degrees
 *
 *------------------------------------------------------------------------*/
int inverse_mapx (mapx_class *this, float u, float v, float *lat, float *lon)
{
  int status;
  errno = 0;
  current = this;
  status = (*(this->map_to_geo))(u, v, lat, lon);
  if (errno != 0) 
    return -1; 
  else
    return status;
}

/*========================================================================
 * projections 
 *
 *	input : lat,lon - geographic coords. (decimal degrees)
 *
 *	output: u,v - map coordinates (map units)
 *
 *	result: 0 = valid coordinates
 *		-1 = invalid point
 *
 *========================================================================*/

/*------------------------------------------------------------------------
 * azimuthal_equal_area
 *------------------------------------------------------------------------*/
static int azimuthal_equal_area (float lat, float lon, float *u, float *v)
{
	float x, y;
	double kp, phi, lam, rho;

	phi = RADIANS (lat);
	lam = RADIANS (lon - current->lon0);

	if (current->lat0 == 90)
	{ rho = 2*current->Rg * sin(PI/4 - phi/2);
	  x =  rho * sin(lam);
	  y = -rho * cos(lam);
	}
	else if (current->lat0 == -90)
	{ rho = 2*current->Rg * cos(PI/4 - phi/2);
	  x =  rho * sin(lam);
	  y =  rho * cos(lam);
	}
	else
	{ kp = sqrt(2./(1+current->sin_phi1*sin(phi) 
	    + current->cos_phi1*cos(phi)*cos(lam)));
	  x = current->Rg*kp*cos(phi)*sin(lam);
	  y = current->Rg*kp*(current->cos_phi1*sin(phi) 
	    - current->sin_phi1*cos(phi)*cos(lam));
	}

	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;

	return(0);
      }

static int inverse_azimuthal_equal_area (float u, float v, float *lat, float *lon)
{
	double phi, lam, rho, c, x, y;

	x =  current->T00*(u+current->u0) - current->T01*(v + current->v0);
	y = -current->T10*(u+current->u0) + current->T11*(v + current->v0);

	rho = sqrt(x*x + y*y);

	if (rho != 0.0)
	{ c = 2*asin( rho/(2*current->Rg) );

	  phi = asin( cos(c)*current->sin_phi1 + y*sin(c)*current->cos_phi1/rho );

	  if (current->lat0 == 90)
	    lam = atan2(x, -y);
	  else if (current->lat0 == -90)
	    lam = atan2(x, y);
	  else
	    lam = atan2(x*sin(c), rho*current->cos_phi1*cos(c)
		 - y*current->sin_phi1*sin(c));
	}
	else
	{ phi = RADIANS(current->lat0);
	  lam = 0.0;
	}

	*lat = DEGREES(phi);
	*lon = DEGREES(lam) + current->lon0;
	NORMALIZE(*lon);

	return(0);
      }

/*-----------------------------------------------------------------------
 * Azimuthal_Equal_Area_Ellipsoid
 *------------------------------------------------------------------------*/

static int azimuthal_equal_area_ellipsoid (float lat, float lon, float *u, float *v)
{
  	float x,y;
	double phi, lam, rho, beta; 
	double sin_phi, cos_phi, sin_beta, cos_beta, q, B;
	
	phi = RADIANS (lat);
	lam = RADIANS (lon - current->lon0);
	sin_phi = sin(phi);
	cos_phi = cos(phi);

	q = (1.0-(current->e2)) * (((sin_phi)/(1.0-((current->e2)*(sin_phi * sin_phi))))-
				   (1.0/(2.0 * (current->eccentricity))) * 
				   log((1-(current->eccentricity * sin_phi)) /
				       (1.0+(current->eccentricity * sin_phi))));

	if (current->lat0 == 90.00)
	{ rho = (current->Rg) * (sqrt((current->qp) - q));
	  x = rho * (sin(lam));
	  y = -rho * (cos(lam));
	}
	else if (current->lat0 == -90.00)
	{ rho = (current->Rg) * (sqrt((current->qp) + q));
	  x = rho * (sin(lam));
	  y = rho * (cos(lam));
	}

	else
	{ if(fabs(fabs(q)-fabs(current->qp))<0.00000001)
	    beta = RADIANS(90.0)*(fabs(q)/q);
	  else
	    beta = asin(q/(current->qp));
	  sin_beta = sin(beta);
	  cos_beta = cos(beta);
	  B = (current->Rq) * (sqrt(2.0/(1.0+((current->sin_beta1) * sin_beta)+
					 ((current->cos_beta1) * cos_beta * 
					  cos(lam)))));
	  x = B * (current->D) * cos_beta * sin(lam);
	  y = (B/(current->D)) * (((current->cos_beta1) * sin_beta)-
				  ((current->sin_beta1) * cos_beta * 
				   cos(lam)));
	}
	
	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;
	
	return(0);
      
      }   

static int inverse_azimuthal_equal_area_ellipsoid (float u, float v, float *lat, float *lon)
{
  double phi, lam, rho, x, y, ce, beta;

  x = current->T00 * (u + current->u0) - (current->T01) * (v + current->v0);
  y = -current->T10 * (u + current->u0) + (current->T11) * (v + current->v0);
  
  if (current->lat0 == 90.00)
  { rho = sqrt((x*x)+(y*y));
    beta = asin(1.0-(rho*rho)/((current->Rg)*(current->Rg)*
			       (1.0-(((1.0-(current->e2))/(2.0 * (current->eccentricity)))*
				(log((1.0-(current->eccentricity))/
				     (1.0+(current->eccentricity))))))));
    phi = beta + ((((current->e2)/3.0)+((31.0/180.0)*(current->e4))+
			((517.0/5040.0)*(current->e6)))*sin(2.0*beta))+
			  ((((23.0/360.0)*(current->e4))+
			    ((251.0/3780.0)*(current->e6)))*sin(4.0*beta))+
			      (((761.0/45360.0)*(current->e6))*sin(6.0*beta));
    lam = atan2(x,(-y));
  }
  else if (current->lat0 == -90.00)
  { rho = sqrt((x*x) + (y*y));
    beta = asin(1.0-(rho*rho)/((current->Rg)*(current->Rg)*
			       (1.0-(((1.0-(current->e2))/(2.0 * (current->eccentricity)))*
				(log((1.0-(current->eccentricity))/
				     (1.0+(current->eccentricity))))))));
    lam = atan2(x,y);
    phi = (-beta) +((((current->e2)/3.0)+((31.0/180.0)*(current->e4))+
			 ((517.0/5040.0)*(current->e6)))*sin(2.0*beta))+
			   ((((23.0/360.0)*(current->e4))+
			     ((251.0/3780.0)*(current->e6)))*sin(4.0*beta))+
			       (((761.0/45360.0)*(current->e6))*sin(6.0*beta));
  }
  else 
  { rho = sqrt(((x/(current->D))*(x/(current->D)))+
	       (((current->D)*y)*((current->D)*y)));
    ce = 2*asin(rho/(2.0*(current->Rq)));
    
    if (fabs(rho)<0.00000001)
      beta = current->beta1;
    else
      beta = asin ((cos(ce)*(current->sin_beta1))+
		   ((current->D)*y*sin(ce)*(current->cos_beta1)/rho));
    phi = beta +((((current->e2)/3.0)+((31.0/180.0)*(current->e4))+
		  ((517.0/5040.0)*(current->e6)))*sin(2.0*beta))+
		    ((((23.0/360.0)*(current->e4))+
		      ((251.0/3780.0)*(current->e6)))*sin(4.0*beta))+
			(((761.0/45360.0)*(current->e6))*sin(6.0*beta));
    lam = atan2((x*sin(ce)),(((current->D)*rho*(current->cos_beta1)*cos(ce))-
			     (((current->D)*(current->D))*y*(current->sin_beta1)*sin(ce))));

 }
  *lat = DEGREES(phi);
  *lon = DEGREES(lam) + (current->lon0);
  NORMALIZE(*lon);

  return(0);

}

/*------------------------------------------------------------------------
 * polar_stereographic
 *------------------------------------------------------------------------*/
static int polar_stereographic (float lat, float lon, float *u, float *v)
{
	float x, y;
	float phi, lam, rho;

	phi = RADIANS (lat);
	lam = RADIANS (lon - current->lon0);

	if (current->lat0 == 90)
	{ rho = 2*current->Rg * cos(phi) 
		* (1 + current->sin_phi1) / (1 + sin(phi));
	  x =  rho * sin(lam);
	  y = -rho * cos(lam);
	}
	else if (current->lat0 == -90)
	{ rho = 2*current->Rg * cos(phi) 
		* (1 - current->sin_phi1) / (1 - sin(phi));
	  x = rho * sin(lam);
	  y = rho * cos(lam);
	}

	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;

	return(0);
}

static int inverse_polar_stereographic (float u, float v, float *lat, float *lon)
{
	double phi, lam, rho, c, q, x, y;

	x =  current->T00*(u+current->u0) - current->T01*(v + current->v0);
	y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);

	rho = sqrt(x*x + y*y);
	q = 2*current->Rg*(1 + current->sin_phi1);
	c = 2*atan2(rho,q);

	if (current->lat0 == 90)
	{ phi = asin(cos(c));
	  lam = atan2(x, -y);
	}
	else if (current->lat0 == -90)
	{ phi = asin(-cos(c));
	  lam = atan2(x, y);
	}

	*lat = DEGREES(phi);
	*lon = DEGREES(lam) + current->lon0;
	NORMALIZE(*lon);

	return(0);
}

/*------------------------------------------------------------------------
 * orthographic
 *------------------------------------------------------------------------*/
static int orthographic (float lat, float lon, float *u, float *v)
{
	float x, y;
	float phi, lam, sin_phi, cos_phi, sin_lam, cos_lam, cos_beta;

	phi = RADIANS (lat);
	lam = RADIANS (lon - current->lon0);

	sin_phi = sin(phi);
	cos_phi = cos(phi);
	cos_lam = cos(lam);

	cos_beta = current->sin_phi1 * sin_phi
		+ current->cos_phi1 * cos_phi * cos_lam;

	if (cos_beta < 0.0) return(-1);

	sin_lam = sin(lam);
	x = current->Rg * cos_phi * sin_lam;
	y = current->Rg * (current->cos_phi1*sin_phi 
			- current->sin_phi1*cos_phi*cos_lam);

	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;

	return(0);
}

static int inverse_orthographic (float u, float v, float *lat, float *lon)
{
	double phi, lam, rho, x, y, cos_beta, sin_beta;

	x =  current->T00*(u+current->u0) - current->T01*(v+current->v0);
	y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);

	rho = sqrt(x*x + y*y);
	if (rho == 0.0)
	{ phi = RADIANS (current->lat0);
	  lam = 0.0;
	}
	else
	{ sin_beta = rho/current->Rg;
	  cos_beta = sqrt(1 - sin_beta*sin_beta);
	  phi = asin(cos_beta*current->sin_phi1 
		+ y*sin_beta*current->cos_phi1/rho);
	  if (current->lat0 == 90)
	  { lam = atan2(x, -y);
	  }
	  else if (current->lat0 == -90)
	  { lam = atan2(x, y);
	  }
	  else
	  { lam = atan2(x*sin_beta, rho*current->cos_phi1*cos_beta
					- y*current->sin_phi1*sin_beta);
	  }
	}

	*lat = DEGREES(phi);
	*lon = DEGREES(lam) + current->lon0;
	NORMALIZE(*lon);

	return(0);
}

/*------------------------------------------------------------------------
 * equal_area_cylindrical
 *------------------------------------------------------------------------*/
static int equal_area_cylindrical (float lat, float lon, float *u, float *v)
{
	float x, y, dlon;
	double phi, lam;

	dlon = lon - current->lon0;
	NORMALIZE(dlon);

	phi = RADIANS (lat);
	lam = RADIANS (dlon);

	x =  current->Rg * lam * current->cos_phi1;
	y =  current->Rg * sin (phi) / current->cos_phi1;

	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;

	return(0);
}

static int inverse_equal_area_cylindrical (float u, float v, float *lat, float *lon)
{
	double phi, lam, x, y;

	x =  current->T00*(u+current->u0) - current->T01*(v+current->v0);
	y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);

	phi = asin(y*current->cos_phi1/current->Rg);
	lam = x/current->cos_phi1/current->Rg;

	*lat = DEGREES(phi);
	*lon = DEGREES(lam) + current->lon0;
	NORMALIZE(*lon);

	return(0);
}

/*--------------------------------------------------------------------------
 * equal_area_cylindrical_ellipsoid (normal aspect)
 *--------------------------------------------------------------------------*/

static int equal_area_cylindrical_ellipsoid (float lat, float lon, float *u, float *v)
{
  float x, y, dlon;
  double phi, lam, kz, q, sin_phi;

  dlon = (lon - current->lon0);
  NORMALIZE (dlon);
  
  phi = RADIANS(lat);
  lam = RADIANS(dlon);

  sin_phi = sin(phi);
  q = (1.0 - (current->e2)) * ((sin_phi/(1.0 - ((current->e2) * sin_phi * sin_phi)))
			    - (1.0/(2.0*(current->eccentricity))) * 
				log((1.0 - ((current->eccentricity)*sin_phi))/
				    (1.0 + ((current->eccentricity)*sin_phi))));

  x = ((current->Rg) * (current->kz) * lam);
  y = ((current->Rg) * q)/(2.0 * (current->kz));

  *u = current->T00*x + current->T01*y - current->u0;
  *v = current->T10*x + current->T11*y - current->v0;

  return(0);
}


static int inverse_equal_area_cylindrical_ellipsoid (float u, float v, float *lat, float *lon)
{
  double phi, lam, x, y, kz, f, e, phis, qp, beta, alpha;

  x =  current->T00*(u+current->u0) - current->T01*(v+current->v0);
  y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);
  
  beta = asin(2.0 * y * (current->kz)/((current->Rg) * (current->qp)));

  phi = beta +((((current->e2)/3.0)+((31.0/180.0)*(current->e4))+
		((517.0/5040.0)*(current->e6)))*sin(2.0*beta))+
		  ((((23.0/360.0)*(current->e4))+
		    ((251.0/3780.0)*(current->e6)))*sin(4.0*beta))+
		      (((761.0/45360.0)*(current->e6))*sin(6.0*beta));
  lam = (x/((current->Rg) * (current->kz)));

  *lat = DEGREES(phi);
  *lon = DEGREES(lam) + current->lon0;
  NORMALIZE(*lon);

  return(0);
}


/*------------------------------------------------------------------------
 * mercator
 *------------------------------------------------------------------------*/
static int mercator (float lat, float lon, float *u, float *v)
{
	float x, y, dlon;
	double phi, lam;

	dlon = lon - current->lon0;
	NORMALIZE(dlon);

	phi = RADIANS (lat);
	lam = RADIANS (dlon);

	x =  current->Rg * lam;
	y =  current->Rg * log (tan (PI/4 + phi/2));

	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;

	return(0);
}

static int inverse_mercator (float u, float v, float *lat, float *lon)
{
	double phi, lam, x, y;

	x =  current->T00*(u+current->u0) - current->T01*(v+current->v0);
	y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);

	phi = PI/2 - 2*atan(exp(-y/current->Rg));
	lam = x/current->Rg;

	*lat = DEGREES(phi);
	*lon = DEGREES(lam) + current->lon0;

	return(0);
}

/*------------------------------------------------------------------------
 * mollweide
 *------------------------------------------------------------------------*/
static int mollweide (float lat, float lon, float *u, float *v)
{
	float x, y, dlon;
	double phi, lam, theta, delta;
	double sin_theta, cos_theta, psi, epsilon=.0025;
	int it, maxit=10;

	dlon = lon - current->lon0;
	NORMALIZE(dlon);

	phi = RADIANS (lat);
	lam = RADIANS (dlon);

	delta = 0.0;
	theta = phi;
	sin_theta = sin(theta);
	cos_theta = cos(theta);
	if (fabs(cos_theta) > epsilon)
	{ psi = PI*sin(phi);
	  it = 0;
	  repeat
	  { delta = -(theta + sin_theta - psi) / (1 + cos_theta);
	    theta += delta;
	    sin_theta = sin(theta);
	    cos_theta = cos(theta);
	    if (++it >= maxit) break;
	  } until (fabs(delta) <= epsilon);
	  theta /= 2.0;
	  sin_theta = sin(theta);
	  cos_theta = cos(theta);
	}

	x =  2*SQRT2/PI * current->Rg * lam * cos_theta;
	y =  SQRT2 * current->Rg * sin_theta;

	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;

	return(0);
}

static int inverse_mollweide (float u, float v, float *lat, float *lon)
{
	double phi, lam, theta, cos_theta, x, y;

	x =  current->T00*(u+current->u0) - current->T01*(v+current->v0);
	y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);

	theta = asin( y / (SQRT2*current->Rg) );
	phi = asin( (2*theta + sin(2*theta)) / PI);
	cos_theta = cos(theta);
	if (cos_theta != 0.0)
	  lam = PI*x / (2*SQRT2*current->Rg*cos_theta);
	else
	  lam = 0.0;


	*lat = DEGREES(phi);
	*lon = DEGREES(lam) + current->lon0;

	return(0);
}

/*------------------------------------------------------------------------
 * cylindrical_equidistant
 *------------------------------------------------------------------------*/
static int cylindrical_equidistant (float lat, float lon, float *u, float *v)
{
	float x, y, dlon;
	double phi, lam;

	dlon = lon - current->lon0;
	NORMALIZE(dlon);

	phi = RADIANS (lat);
	lam = RADIANS (dlon);

	x =  current->Rg * lam * current->cos_phi1;
	y =  current->Rg * phi;

	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;

	return(0);
}

static int inverse_cylindrical_equidistant (float u, float v, float *lat, float *lon)
{
	double phi, lam, x, y;

	x =  current->T00*(u+current->u0) - current->T01*(v+current->v0);
	y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);

	phi = y/current->Rg;
	lam = x/(current->Rg*current->cos_phi1);

	*lat = DEGREES(phi);
	*lon = DEGREES(lam) + current->lon0;

	return(0);
}

/*------------------------------------------------------------------------
 * sinusoidal
 *------------------------------------------------------------------------*/
static int sinusoidal (float lat, float lon, float *u, float *v)
{
	float x, y, dlon;
	double phi, lam;

	dlon = lon - current->lon0;
	NORMALIZE(dlon);

	phi = RADIANS (lat);
	lam = RADIANS (dlon);

	x =  current->Rg * lam * cos (phi);
	y =  current->Rg * phi;

	*u = current->T00*x + current->T01*y - current->u0;
	*v = current->T10*x + current->T11*y - current->v0;

	return(0);
}

static int inverse_sinusoidal (float u, float v, float *lat, float *lon)
{
	double phi, lam, x, y;

	x =  current->T00*(u+current->u0) - current->T01*(v+current->v0);
	y = -current->T10*(u+current->u0) + current->T11*(v+current->v0);

	phi = y/current->Rg;
	lam = x/(current->Rg*cos(phi));

	*lat = DEGREES(phi);
	*lon = DEGREES(lam) + current->lon0;

	return(0);
}


/*--------------------------------------------------------------------------
 * change projection name
 *-------------------------------------------------------------------------*/

char *change(char *s)
{
  static char new_projection[80];
  char *p = new_projection;

  for(; *s != '\0'; ++s)
  {
    if ((*s == '_') || (*s == ' ') ||(*s == '-'))
      ;
    else 
      *p++ = toupper(*s);
  }
  return new_projection;
}

#ifdef MTEST
/*========================================================================
 * mtest - interactive test for mapx routines
 *========================================================================*/
main(int argc, char *argv[])
{
  float lat, lon, u, v;
  char readln[80];
  mapx_class *the_map;
  int status;

  for (;;)
  { printf("\nenter .mpp file name - ");
    gets(readln);
    if (feof(stdin)) { printf("\n"); exit(0);}
    if (*readln == '\0') break;
    the_map = init_mapx(readln);
    if (the_map == NULL) continue;
    
    printf("\nforward_mapx:\n");
    for (;;)
    { printf("enter lat lon - ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%f %f", &lat, &lon);
      status = forward_mapx(the_map, lat, lon, &u, &v);
      printf("u,v = %f %f    %s\n", u, v, 
	     status == 0 ? "valid" : "invalid");
      status = inverse_mapx(the_map, u, v, &lat, &lon);
      printf("lat,lon = %f %f     %s\n", lat, lon, 
	     status == 0 ? "valid" : "invalid");
    }

    printf("\ninverse_mapx:\n");
    for (;;)
    { int temp;
      printf("enter u v - ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%f %f", &u, &v);
      status = inverse_mapx(the_map, u, v, &lat, &lon);
      printf("lat,lon = %f %f    %s\n", lat, lon, 
	     status == 0 ? "valid" : "invalid");
      status = forward_mapx(the_map, lat, lon, &u, &v);
      printf("u,v = %f %f    %s\n", u, v, 
	     status == 0 ? "valid" : "invalid");
    }
    printf("\nwithin_mapx:\n");
    for (;;)
    { printf("enter lat lon - ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%f %f", &lat, &lon);
      printf("%s\n", within_mapx(the_map, lat, lon) ? "INSIDE" : "OUTSIDE");
    }
    
    close_mapx(the_map);
  }
}
#endif

#ifdef MACCT
#define usage "usage: macct mpp_file"
/*========================================================================
 * macct - accuracy test mapx routines
 *========================================================================*/
#define MPMON
static float dist_km(float lat1, float lon1, float lat2, float lon2)
{ register float phi1, lam1, phi2, lam2, beta;
  phi1 = radians(lat1);
  lam1 = radians(lon1);
  phi2 = radians(lat2);
  lam2 = radians(lon2);
  beta = acos(cos(phi1)*cos(phi2)*cos(lam1-lam2) + sin(phi1)*sin(phi2));
  return beta*Re;
}
#endif

#ifdef MPMON
#ifndef usage
#define usage "usage: mpmon mpp_file [num_its]"
#endif
/*========================================================================
 * mpmon - performance test mapx routines
 *========================================================================*/
main(int argc, char *argv[])
{
  register int i_lat, i_lon, npts = 0, bad_pts = 0;
  int status1, status2;
  int ii, its = 1, pts_lat = 319, pts_lon = 319;
  register float lat, lon, dlat, dlon;
  float latx, lonx, u, v;
  mapx_class *the_map;
#ifdef MACCT
  float err=0, sum=0, sum2=0, stdv=0, max_err=0, lat_max=0, lon_max=0;
#endif

  if (argc < 2) 
#ifdef MACCT
  { fprintf(stderr,"#\tmacct can be used to test the accuracy\n");
    fprintf(stderr,"#\tof the mapx routines. It runs the forward and\n");
    fprintf(stderr,"#\tinverse transforms at ~100K points over the whole\n");
    fprintf(stderr,"#\tmap. Error statistics are accumulated in kilometers.\n");
    fprintf(stderr,"#\To run the test type:\n");
    fprintf(stderr,"#\t\tmacct test.mpp\n");
    fprintf(stderr,"\n");
    error_exit(usage);
  }
#else
  { fprintf(stderr,"#\tmpmon can be used to monitor the performance\n");
    fprintf(stderr,"#\tof the mapx routines. It runs the forward and\n");
    fprintf(stderr,"#\tinverse transforms at ~100K points over the whole\n");
    fprintf(stderr,"#\tmap. The optional parameter num_its specifies how\n");
    fprintf(stderr,"#\tmany times to run through the entire map, (the\n");
    fprintf(stderr,"#\tdefault is 1). To run the test type:\n");
    fprintf(stderr,"#\t\tmpmon test.mpp\n");
    fprintf(stderr,"#\t\tprof mpmon\n");
    fprintf(stderr,"\n");
    error_exit(usage);
  }
#endif

  the_map = init_mapx(argv[1]);
  if (the_map == NULL) error_exit(usage);
  if (argc < 3 || sscanf(argv[2],"%d",&its) != 1) its = 1;

  dlat = the_map->north - the_map->south;
  dlon = the_map->east - the_map->west;

  for (ii = 1; ii <= its; ii++)
  { for (i_lat = 0; i_lat <= pts_lat; i_lat++)
    { lat = (float)i_lat/pts_lat * dlat + the_map->south;
      for (i_lon = 0; i_lon <= pts_lon; i_lon++)
      { lon = (float)i_lon/pts_lon * dlon + the_map->west;
	status1 = forward_mapx(the_map, lat, lon, &u, &v);
	status2 = inverse_mapx(the_map, u, v, &latx, &lonx);
	if ((status1 | status2) != 0) ++bad_pts;
	++npts;
#ifdef MACCT
	if ((status1 | status2) == 0)
	{ err = dist_km(lat, lon, latx, lonx);
	  if (err > 0) { sum += err; sum2 += err*err; }
	  if (err > max_err) { max_err=err; lat_max=lat; lon_max=lon; }
	}
#endif
      }
    }
  }
  fprintf(stderr,"%d points,  %d bad points\n", npts, bad_pts);
#ifdef MACCT
  npts -= bad_pts;
  if (npts > 0)
  { err = sum/npts;
    stdv = sqrt((sum2 - npts*err*err)/(npts-1));
  }
  fprintf(stderr,"average error = %10.4e km\n", err);
  fprintf(stderr,"std dev error = %10.4e km\n", stdv);
  fprintf(stderr,"maximum error = %10.4e km\n", max_err);
  fprintf(stderr,"max error was at %4.2f%c %4.2f%c\n",
	  fabs(lat_max), lat_max >= 0 ? 'N' : 'S',
	  fabs(lon_max), lon_max >= 0 ? 'E' : 'W');
#endif
}
#endif
