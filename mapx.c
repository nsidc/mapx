/*========================================================================
 * map projections - convert geographic to map coordinates
 *
 * 2-July-1991 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 * 10-Dec-1992 R.Swick swick@krusty.colorado.edu 303-492-1395
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *========================================================================*/
static const char mapx_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/mapx.c,v 1.30 1999-07-28 22:23:48 knowles Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include "define.h"
#include "maps.h"
#include "mapx.h"


/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * projections 
 *
 *	To add a new projection the projection names should be added
 *	to the standard_name function, the standard name is added to
 *	the if-else-if construct in init_mapx and three private functions
 *	must be defined in a separate file and declared in the prototypes 
 *	section below.
 *
 *	The initialization function sets all pre-computed projection
 *	constants.
 *
 *	The forward function converts geographic to map coordinates.
 *
 *	input : lat,lon - geographic coords. (decimal degrees)
 *
 *	output: u,v - map coordinates (map units)
 *
 *	result: 0 = valid coordinates
 *		-1 = invalid point
 *
 *	The inverse function converts map to geographic coordinates.
 *
 *	input : u,v - map coordinates (map units)
 *
 *	output: lat,lon - geographic coords. (decimal degrees);
 *
 *	result: 0 = valid coordinates
 *		-1 = invalid point
 *
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
int init_azimuthal_equal_area(void *);
int azimuthal_equal_area(void *, float, float, float *, float *);
int inverse_azimuthal_equal_area(void *, float, float, float *, float *);
int init_cylindrical_equidistant(void *);
int cylindrical_equidistant(void *, float, float, float *, float *);
int inverse_cylindrical_equidistant(void *, float, float, float *, float *);
int init_cylindrical_equal_area(void *);
int cylindrical_equal_area(void *, float, float, float *, float *);
int inverse_cylindrical_equal_area(void *, float, float, float *, float *);
int init_mercator(void *);
int mercator(void *, float, float, float *, float *);
int inverse_mercator(void *, float, float, float *, float *);
int init_mollweide(void *);
int mollweide(void *, float, float, float *, float *);
int inverse_mollweide(void *, float, float, float *, float *);
int init_orthographic(void *);
int orthographic(void *, float, float, float *, float *);
int inverse_orthographic(void *, float, float, float *, float *);
int init_polar_stereographic(void *);
int polar_stereographic(void *, float, float, float *, float *);
int inverse_polar_stereographic(void *, float, float, float *, float *);
int init_polar_stereographic_ellipsoid(void *);
int polar_stereographic_ellipsoid(void *, float, float, float *, float *);
int inverse_polar_stereographic_ellipsoid(void *, float, float, float *, float *);
int init_sinusoidal(void *);
int sinusoidal(void *, float, float, float *, float *);
int inverse_sinusoidal(void *, float, float, float *, float *);
int init_azimuthal_equal_area_ellipsoid(void *);
int azimuthal_equal_area_ellipsoid(void *, float, float, float *, float *);
int inverse_azimuthal_equal_area_ellipsoid(void *, float, float, float *, float *);
int init_cylindrical_equal_area_ellipsoid(void *);
int cylindrical_equal_area_ellipsoid(void *, float, float, float *, float *);
int inverse_cylindrical_equal_area_ellipsoid(void *, float, float, float *, float *);
int init_lambert_conic_conformal_ellipsoid(void *);
int lambert_conic_conformal_ellipsoid(void *, float, float, float *, float *);
int inverse_lambert_conic_conformal_ellipsoid(void *, float, float, float *, float *);
int init_interupted_homolosine_equal_area(void *);
int interupted_homolosine_equal_area(void *, float, float, float *, float *);
int inverse_interupted_homolosine_equal_area(void *, float, float, float *, float *);
int init_albers_conic_equal_area(void *);
int albers_conic_equal_area(void *, float, float, float *, float *);
int inverse_albers_conic_equal_area(void *, float, float, float *, float *);
static char *standard_name(char *);

/*----------------------------------------------------------------------
 * init_mapx - initialize map projection 
 *
 *	input : map_filename - map parameters file name
 *			format as follows:
 *			 Map_Projection_Name
 *			 lat0 lon0 [lat1 lon1] (decimal degrees)
 *			 rotation (portrait=0, landscape=90)
 *			 scale (kilometers per map unit)
 *			 center_lat center_lon (for map)
 *			 south north (decimal degrees)
 *			 west east (decimal degrees)
 *			 lat_interval lon_interval (for graticule)
 *			 label_lat label_lon (for graticule)
 *			 cil_detail bdy_detail riv_detail (see database_info)
 *	    [optional]   equatorial_radius (kilometers) 
 *	    [optional]	 eccentricity 
 *			                  
 *
 *			valid projection names are:
 *			 Azimuthal_Equal_Area
 *			 Cylindrical_Equal_Area
 *			 Mercator
 *			 Mollweide
 *			 Orthographic
 *			 Sinusoidal
 *			 Cylindrical_Equidistant
 *			 Polar_Stereographic
 *			 Polar_Stereographic_Ellipsoid
 *			 Azimuthal_Equal_Area_Ellipsoid			 
 *			 Cylindrical_Equal_Area_Ellipsoid
 *                       Lambert_Conic_Conformal_Ellipsoid
 *			 Interupted_Homolosine_Equal_Area
 *                       Albers_Conic_Equal_Area
 *			or anything reasonably similar
 *
 *	The parameter lat1 is currently used for the Polar_Stereographic
 *	projection to define the "true at" parallel (default pole) and
 *	the Cylindrical_Equal_Area projection to define the latitude of
 *	1:1 aspect ratio (default 30.00). In the Lambert Conic Conformal
 *	and Albers Conic Equal-Area lat0 and lat1 are the standard parallels.
 *
 *	result: pointer to new mapx_class instance for this map
 *		or NULL if an error occurs during initialization
 *
 *	note  : if unable to open .mpp file on first attempt then the
 *		value of the search path environment variable is prepended
 *		to the filename and a second attempt is made
 *
 *		Some important notes on specifying longitudes:
 *		All longitudes should be >= -180 and <= 360.
 *		West to east should not span more than 360 degrees.
 *		West specifies the left side of the map and east the right,
 *		not necessarily the minimum and maximum longitudes.
 *		For purposes of bounds checking all longitudes are 
 *		normalized -180 to 180.
 *
 *----------------------------------------------------------------------*/
mapx_class *init_mapx (char *map_filename)
{
  double theta;
  float f1, f2, f3, f4;
  int i1, i2, i3, ios;
  char projection[80], readln[80], original_name[80];
  mapx_class *this;
  
  /*
   *	allocate storage for projection parameters
   */
  this = (mapx_class *) calloc(1, sizeof(mapx_class));
  if (this == NULL)
  { perror("init_mapx");
    return NULL;
  }
  
  /*
   *	open .mpp file
   */
  this->mpp_filename = (char *) malloc((size_t)MAX_STRING);
  if (this->mpp_filename == NULL)
  { perror("init_mapx");
    close_mapx(this);
    return NULL;
  }
  strncpy(this->mpp_filename, map_filename, MAX_STRING);
  this->mpp_file = search_path_fopen(this->mpp_filename, mapx_PATH, "r");
  if (this->mpp_file == NULL)
  { fprintf (stderr,"init_mapx: error opening parameters file.\n");
    perror(map_filename);
    close_mapx(this);
    return NULL;
  }
  
  /*
   *	read in projection parameters
   */
  fgets (readln, sizeof(readln), this->mpp_file);
  strcpy(original_name, readln);
  strcpy(projection, standard_name(original_name));
  this->projection_name = strdup(projection);
  
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
  
  /*
   *	check for errors when reading
   */
  if (ferror(this->mpp_file) || feof(this->mpp_file))
  { fprintf (stderr,"init_mapx: error reading parameters file.\n");
    if (feof(this->mpp_file))
      fprintf(stderr,"%s: unexpected end of file.\n", map_filename);
    else
      perror(map_filename);
    close_mapx(this);
    return NULL;
  }
  
  /*
   *	look for optional parameters
   */
  fgets (readln, sizeof(readln), this->mpp_file);
  if (feof(this->mpp_file))
  { this->equatorial_radius = mapx_Re_km;
    this->eccentricity = 0.082271673;
  }
  else
  { ios = sscanf (readln, "%f", &f1);           
    this->equatorial_radius = (ios >= 1) ? f1 : mapx_Re_km;
    
    fgets (readln, sizeof(readln), this->mpp_file);      
    if (feof(this->mpp_file))
      this->eccentricity = 0.082271673;
    else
    { ios = sscanf (readln, "%f", &f1);            
      this->eccentricity = (ios >= 1) ? f1 : 0.082271673;
    }
  }
  
  if (ferror(this->mpp_file))
  { fprintf (stderr, "init_mapx: error reading parameters file.\n");
    perror(map_filename);
    close_mapx(this);
    return NULL;
  }
  
  
  /*
   *	match projection name and initialize remaining parameters
   */
  if (strcmp (projection, "AZIMUTHALEQUALAREA") == 0)     
  { this->initialize = init_azimuthal_equal_area; 
    this->geo_to_map = azimuthal_equal_area;
    this->map_to_geo = inverse_azimuthal_equal_area;
  }
  else if (strcmp (projection, "CYLINDRICALEQUALAREA") == 0)
  { this->initialize = init_cylindrical_equal_area;
    this->geo_to_map = cylindrical_equal_area;
    this->map_to_geo = inverse_cylindrical_equal_area;
  }
  else if (strcmp (projection, "MERCATOR") == 0)
  { this->initialize = init_mercator;
    this->geo_to_map = mercator;
    this->map_to_geo = inverse_mercator;
  }
  else if (strcmp (projection, "MOLLWEIDE") == 0)
  { this->initialize = init_mollweide;
    this->geo_to_map = mollweide;
    this->map_to_geo = inverse_mollweide;
  }
  else if (strcmp (projection, "ORTHOGRAPHIC") == 0)
  { this->initialize = init_orthographic;
    this->geo_to_map = orthographic;
    this->map_to_geo = inverse_orthographic;
  }
  else if (strcmp (projection, "SINUSOIDAL") == 0)
  { this->initialize = init_sinusoidal;
    this->geo_to_map = sinusoidal;
    this->map_to_geo = inverse_sinusoidal;
  }
  else if (strcmp (projection, "CYLINDRICALEQUIDISTANT") == 0)
  { this->initialize = init_cylindrical_equidistant;
    this->geo_to_map = cylindrical_equidistant;
    this->map_to_geo = inverse_cylindrical_equidistant;
  }
  else if (strcmp (projection, "POLARSTEREOGRAPHIC") == 0)
  { this->initialize = init_polar_stereographic;
    this->geo_to_map = polar_stereographic;
    this->map_to_geo = inverse_polar_stereographic;
  }
  
  else if (strcmp (projection, "POLARSTEREOGRAPHICELLIPSOID") == 0)
  { this->initialize = init_polar_stereographic_ellipsoid;
    this->geo_to_map = polar_stereographic_ellipsoid;
    this->map_to_geo = inverse_polar_stereographic_ellipsoid;
  }
  
  else if (strcmp (projection, "AZIMUTHALEQUALAREAELLIPSOID") == 0) 
  { this->initialize = init_azimuthal_equal_area_ellipsoid;
    this->geo_to_map = azimuthal_equal_area_ellipsoid;
    this->map_to_geo = inverse_azimuthal_equal_area_ellipsoid;
  }
  
  else if (strcmp (projection, "CYLINDRICALEQUALAREAELLIPSOID") == 0) 
  { this->initialize = init_cylindrical_equal_area_ellipsoid;
    this->geo_to_map = cylindrical_equal_area_ellipsoid;
    this->map_to_geo = inverse_cylindrical_equal_area_ellipsoid;
  }
  
  else if (strcmp (projection, "LAMBERTCONICCONFORMALELLIPSOID") == 0) 
  { this->initialize =init_lambert_conic_conformal_ellipsoid;
    this->geo_to_map = lambert_conic_conformal_ellipsoid;
    this->map_to_geo = inverse_lambert_conic_conformal_ellipsoid;
  }
  else if (strcmp (projection, "INTERUPTEDHOMOLOSINEEQUALAREA") == 0) 
  { this->initialize =init_interupted_homolosine_equal_area;
    this->geo_to_map = interupted_homolosine_equal_area;
    this->map_to_geo = inverse_interupted_homolosine_equal_area;
  }
  else if (strcmp (projection, "ALBERSCONICEQUALAREA") == 0)     
  { this->initialize = init_albers_conic_equal_area; 
    this->geo_to_map = albers_conic_equal_area;
    this->map_to_geo = inverse_albers_conic_equal_area;
  }
  else
  { fprintf (stderr, "init_mapx: unknown projection %s\n", original_name);
    fprintf (stderr, "valid types are:\n");
    fprintf (stderr, " Albers Conic Equal-Area\n");
    fprintf (stderr, " Azimuthal Equal-Area\n");
    fprintf (stderr, " Azimuthal Equal-Area Ellipsoid\n");
    fprintf (stderr, " Cylindrical Equal-Area\n");
    fprintf (stderr, " Cylindrical Equal-Area Ellipsoid\n");
    fprintf (stderr, " Cylindrical Equidistant\n");
    fprintf (stderr, " Interupted Homolosine Equal-Area\n");
    fprintf (stderr, " Lambert Conic Conformal Ellipsoid\n");
    fprintf (stderr, " Mercator\n");
    fprintf (stderr, " Mollweide\n");
    fprintf (stderr, " Orthographic\n");
    fprintf (stderr, " Polar Stereographic\n");
    fprintf (stderr, " Polar Stereographic Ellipsoid\n");
    fprintf (stderr, " Sinusoidal\n");
    close_mapx (this);
    return NULL;
  }
  
  /*
   *	initialize map projection constants
   */
  if (0 != reinit_mapx(this))
  { close_mapx(this);
    return NULL;
  }
  
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
  if (this->projection_name != NULL) free(this->projection_name);
  if (this->mpp_file != NULL) fclose(this->mpp_file);
  if (this->mpp_filename != NULL) free(this->mpp_filename);
  free (this);
}

/*------------------------------------------------------------------------
 * reinit_mapx - re-initialize map projection constants
 *
 *	input : this - pointer to map data structure (returned by init_mapx)
 *
 *		The client may set user specified constants in the 
 *		mapx_class struct and this routine will re-calculate
 *		the appropriate private constants for the projection
 *
 *	result: 0 = success, -1 = error return
 *
 *------------------------------------------------------------------------*/
int reinit_mapx (mapx_class *this)
{ float theta, u, v;
  
  /*
   *	check map bounds
   */
  if (this->east < -180 || this->east > 360 
      || this->west < -180 || this->west > 360)
  { fprintf(stderr,"init_mapx: illegal bounds: west=%f, east=%f\n",
	    this->west, this->east);
    fprintf(stderr,"           should be >= -180 and <= 360\n");
    return -1;
  }
  
  if (fabs(this->east - this->west) > 360)
  { fprintf(stderr,"init_mapx: illegal bounds: west=%f, east=%f\n",
	    this->west, this->east);
    fprintf(stderr,"           bounds cannot span > 360 degrees.\n");
    return -1;
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
  
  /*
   *	set series expansion constants
   */
  this->e2 = (this->eccentricity) * (this->eccentricity);
  this->e4 = (this->e2) * (this->e2);
  this->e6 = (this->e4) * (this->e2); 
  this->e8 = (this->e4) * (this->e4);
  

  /*
   *	set scaled radius for spherical projections
   */
  this->Rg = this->equatorial_radius / this->scale;
  
  /*
   *	set projection constants
   */
  if ((*(this->initialize))(this)) return -1;

  /*
   *	create rotation matrix
   */
  theta = RADIANS(this->rotation);
  
  this->T00 =  cos(theta);
  this->T01 =  sin(theta);
  this->T10 = -sin(theta);
  this->T11 =  cos(theta);
  
  /*
   *	get the offset from the projection origin (lat0,lon0)
   *	to this map's origin (center_lat, center_lon)
   */
  this->u0 = this->v0 = 0.0;
  forward_mapx (this, this->center_lat, this->center_lon, &u, &v);
  this->u0 = u;
  this->v0 = v;
  
  return 0;
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
  status = (*(this->geo_to_map))(this, lat, lon, u, v);
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
  status = (*(this->map_to_geo))(this, u, v, lat, lon);
  if (errno != 0) 
    return -1; 
  else
    return status;
}

/*--------------------------------------------------------------------------
 * standard_name - standardize projection name
 *
 *	input : s - original projection name string
 *
 *	result: a valid projection name or ""
 *
 *-------------------------------------------------------------------------*/
static char *standard_name(char *s)
{
  static char new_name[80];
  char *p = new_name;
  
  for(; *s != '\n' && *s != '\0'; ++s)
  {
    if ((*s == '_') || (*s == ' ') || (*s == '-') 
	|| (*s == '(') || (*s == ')'))
      ;
    else 
      *p++ = toupper(*s);
  }
  
  *p = '\0';
  
  if (streq(new_name, "AZIMUTHALEQUALAREA") || 
      streq(new_name, "AZIMUTHALEQUALAREASPHERE") || 
      streq(new_name, "EQUALAREAAZIMUTHALSPHERE") || 
      streq(new_name, "SPHEREAZIMUTHALEQUALAREA") || 
      streq(new_name, "SPHEREEQUALAREAAZIMUTHAL") || 
      streq(new_name, "EQUALAREAAZIMUTHAL"))
  { strcpy(new_name,"AZIMUTHALEQUALAREA");
  }
  else if (streq(new_name, "EQUALAREACYLINDRICAL") || 
	   streq(new_name, "CYLINDRICALEQUALAREA") ) 
  { strcpy(new_name,"CYLINDRICALEQUALAREA");
  }
  else if (streq(new_name, "CYLINDRICALEQUIDISTANT") || 
	   streq(new_name, "EQUIDISTANTCYLINDRICAL"))
  { strcpy(new_name, "CYLINDRICALEQUIDISTANT");
  }
  else if (streq(new_name, "POLARSTEREOGRAPHIC") || 
	   streq(new_name, "STEREOGRAPHICPOLAR"))
  { strcpy(new_name, "POLARSTEREOGRAPHIC");
  }
  else if (streq(new_name, "POLARSTEREOGRAPHICELLIPSOID") || 
	   streq(new_name, "ELLIPSOIDPOLARSTEREOGRAPHIC") ||
	   streq(new_name, "STEREOGRAPHICPOLARELLIPSOID") ||
	   streq(new_name, "ELLIPSOIDSTEREOGRAPHICPOLAR"))
  { strcpy(new_name, "POLARSTEREOGRAPHICELLIPSOID");
  }
  else if (streq(new_name, "AZIMUTHALEQUALAREAELLIPSOID") || 
	   streq(new_name, "ELLIPSOIDAZIMUTHALEQUALAREA") || 
	   streq(new_name, "EQUALAREAAZIMUTHALELLIPSOID") || 
	   streq(new_name, "ELLIPSOIDEQUALAREAAZIMUTHAL"))
  { strcpy(new_name, "AZIMUTHALEQUALAREAELLIPSOID");
  }
  else if (streq(new_name, "CYLINDRICALEQUALAREAELLIPSOID") || 
	   streq(new_name, "ELLIPSOIDCYLINDRICALEQUALAREA") || 
	   streq(new_name, "EQUALAREACYLINDRICALELLIPSOID") || 
	   streq(new_name, "ELLIPSOIDEQUALAREACYLINDRICAL") )
  { strcpy(new_name,  "CYLINDRICALEQUALAREAELLIPSOID");
  }
  else if (streq(new_name, "LAMBERTCONICCONFORMALELLIPSOID") ||
	   streq(new_name, "LAMBERTCONFORMALCONICELLIPSOID") ||
	   streq(new_name, "ELLIPSOIDLAMBERTCONICCONFORMAL") ||
	   streq(new_name, "ELLIPSOIDLAMBERTCONFORMALCONIC"))
  { strcpy(new_name, "LAMBERTCONICCONFORMALELLIPSOID");
  }
  else if (streq(new_name, "INTERUPTEDHOMOLOSINEEQUALAREA") ||
	   streq(new_name, "GOODESINTERUPTEDHOMOLOSINE") ||
	   streq(new_name, "GOODEHOMOLOSINEEQUALAREA") ||
	   streq(new_name, "GOODESHOMOLOSINEEQUALAREA") ||
	   streq(new_name, "INTERUPTEDHOMOLOSINE") ||
	   streq(new_name, "GOODEINTERUPTEDHOMOLOSINE") ||
	   streq(new_name, "GOODEHOMOLOSINE") ||
	   streq(new_name, "GOODESHOMOLOSINE"))
  { strcpy(new_name, "INTERUPTEDHOMOLOSINEEQUALAREA");
  }
  else if (streq(new_name, "ALBERSCONICEQUALAREA") || 
      streq(new_name, "ALBERSCONICEQUALAREASPHERE") || 
      streq(new_name, "ALBERSEQUALAREACONIC") || 
      streq(new_name, "CONICEQUALAREA") || 
      streq(new_name, "EQUALAREACONIC") || 
      streq(new_name, "ALBERSCONIC") || 
      streq(new_name, "ALBERSEQUALAREA"))
  { strcpy(new_name,"ALBERSCONICEQUALAREA");
  }
  
  return new_name;
}

#ifdef MTEST
/*------------------------------------------------------------------------
 * mtest - interactive test for mapx routines
 *------------------------------------------------------------------------*/
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
    { printf("enter u v - ");
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
/*------------------------------------------------------------------------
 * macct - accuracy test mapx routines
 *------------------------------------------------------------------------*/
#define MPMON
static float dist_km(float lat1, float lon1, float lat2, float lon2)
{ register float phi1, lam1, phi2, lam2, beta;
  phi1 = radians(lat1);
  lam1 = radians(lon1);
  phi2 = radians(lat2);
  lam2 = radians(lon2);
  beta = acos(cos(phi1)*cos(phi2)*cos(lam1-lam2) + sin(phi1)*sin(phi2));
  return beta*mapx_Re_km;
}
#endif

#ifdef MPMON
#ifndef usage
#define usage "usage: mpmon mpp_file [num_its]"
#endif
/*------------------------------------------------------------------------
 * mpmon - performance test mapx routines
 *------------------------------------------------------------------------*/
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
    fprintf(stderr,"#\tTo run the test type:\n");
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
