/*========================================================================
 * map projections - convert geographic to map coordinates
 *
 *	2-July-1991 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 *	16-Dec-1991 K.Knowles - put all global parameters into mapx_struct
 *	2-July-1992 K.Knowles - init returns pointer to struct
 *========================================================================*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <define.h>
#include <mapx.h>

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
  double theta;
  float f1, f2, f3, f4;
  int i1, i2, i3, ios;
  char projection[80], readln[80];
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
  sscanf (readln, "%s", projection);

  fgets (readln, sizeof(readln), this->mpp_file);
  ios = sscanf (readln, "%f %f %f &f", &f1, &f2, &f3, &f4);
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


  if (strcmp (projection, "Azimuthal_Equal_Area") == 0)
  { this->geo_to_map = azimuthal_equal_area;
    this->map_to_geo = inverse_azimuthal_equal_area;
    this->sin_phi1 = sin (RADIANS (this->lat0));
    this->cos_phi1 = cos (RADIANS (this->lat0));
  }
  else if (strcmp (projection, "Equal_Area_Cylindrical") == 0)
  { this->geo_to_map = equal_area_cylindrical;
    this->map_to_geo = inverse_equal_area_cylindrical;
    if (this->lat1 == 999) this->lat1 = 30.00;
    this->cos_phi1 = cos (RADIANS (this->lat1));
  }
  else if (strcmp (projection, "Mercator") == 0)
  { this->geo_to_map = mercator;
    this->map_to_geo = inverse_mercator;
    if (this->lat1 == 999) this->lat1 = 30.00;
    this->cos_phi1 = cos (RADIANS (this->lat1));
  }
  else if (strcmp (projection, "Mollweide") == 0)
  { this->geo_to_map = mollweide;
    this->map_to_geo = inverse_mollweide;
  }
  else if (strcmp (projection, "Orthographic") == 0)
  { this->geo_to_map = orthographic;
    this->map_to_geo = inverse_orthographic;
    this->cos_phi1 = cos(this->lat0);
    this->sin_phi1 = sin(this->lat0);
  }
  else if (strcmp (projection, "Sinusoidal") == 0)
  { this->geo_to_map = sinusoidal;
    this->map_to_geo = inverse_sinusoidal;
  }
  else if (strcmp (projection, "Cylindrical_Equidistant") == 0)
  { this->geo_to_map = cylindrical_equidistant;
    this->map_to_geo = inverse_cylindrical_equidistant;
    this->cos_phi1 = cos (RADIANS (this->lat0));
  }
  else if (strcmp (projection, "Polar_Stereographic") == 0)
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
  else
  { fprintf (stderr, "init_mapx: unknown projection %s\n", projection);
    fprintf (stderr, "valid types are:\n");
    fprintf (stderr, " Azimuthal_Equal_Area\n");
    fprintf (stderr, " Equal_Area_Cylindrical\n");
    fprintf (stderr, " Mercator\n");
    fprintf (stderr, " Mollweide\n");
    fprintf (stderr, " Orthographic\n");
    fprintf (stderr, " Sinusoidal\n");
    fprintf (stderr, " Cylindrical_Equidistant\n");
    fprintf (stderr, " Polar_Stereographic\n");
    close_mapx (this);
    return NULL;
  }

  this->Rg = Re / this->scale;

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
  current = this;
  return (*(this->geo_to_map))(lat, lon, u, v);
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
  current = this;
  return (*(this->map_to_geo))(u, v, lat, lon);
}

/*========================================================================
 * projections 
 *
 *	input : lat,lon - geographic coords. (decimal degrees)
 *
 *	output: u,v - map coordinates (map units)
 *
 *	result: 0 = valid coordinates
 *		-1 = coordinates off map
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
