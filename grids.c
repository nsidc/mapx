/*========================================================================
 * grids - grid coordinate system definition and transformations
 *
 * 26-Dec-1991 K.Knowles knowlesk@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 * Copyright (C) 1991 University of Colorado
 *========================================================================*/
static const char grids_c_rcsid[] = "$Id$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "keyval.h"
#include "mapx.h"
#include "maps.h"
#define grids_c_
#include "grids.h"

static bool decode_gpd(grid_class *this, char *label);
static bool old_fixed_format_decode_gpd(grid_class *this, char *label);

const char *id_grids(void)
{
  return grids_c_rcsid;
}

/*----------------------------------------------------------------------
 * init_grid - initialize grid coordinate system from file
 *
 *	input : filename - grid parameter definitions file name
 *		file must have following fields:
 *		 Grid Width: number_of_columns
 *		 Grid Height: number_of_rows
 *		 Grid Map Origin Column: col0
 *		 Grid Map Origin Row: row0
 *		and either one of:
 *		 Grid Cells per Map Unit: columns_and_rows
 *		 Grid Columns per Map Unit: columns
 *		 Grid Rows per Map Unit: rows
 *		or:
 *		 Grid Map Units per Cell: map_units
 *		 Grid Map Units per Column: map_units
 *		 Grid Map Units per Row: map_units
 *		plus:
 *		 Grid MPP File: mpp_filename
 *		or map projection parameters embedded in the same label
 *
 *		old fixed format was as follows:
 *		  mpp_filename
 *		  number_of_columns number_of_rows 
 *		  columns_per_map_unit rows_per_map_unit 
 *		  map_origin_column map_origin_row 
 *
 *	result: pointer to new grid_class instance
 *		or NULL if an error occurs during initialization
 *
 *	note:	for some parameters, if no value is specified in 
 *		the .gpd file the parameter is set to a default 
 *		value without warning
 *		also, if init_grid fails to open the gpd_file on its
 *		first attempt it will then search the colon separated
 *		list of directories in the map parameters search path 
 *		envornment variable
 *
 *----------------------------------------------------------------------*/
grid_class *init_grid(char *filename)
{
  char *gpd_filename, *label=NULL;
  FILE *gpd_file=NULL;
  grid_class *this=NULL;

/*
 *	open .gpd file and read label
 */
  gpd_filename = (char *)malloc(FILENAME_MAX);
  if (gpd_filename == NULL) { perror("init_grid"); return NULL; }

  strncpy(gpd_filename, filename, FILENAME_MAX);
  gpd_file = search_path_fopen(gpd_filename, mapx_PATH, "r");
  if (!gpd_file)
  { fprintf(stderr,"init_grid: error opening parameters file.\n");
    perror(filename);
    return NULL;
  }

  label = get_label_keyval(gpd_filename, gpd_file, 0);
  if (NULL == label) return NULL;

  /*
   *	initialize projection parameters
   */
  this = new_grid(label);
  if (NULL == this) goto error_return;
  free(label); label = NULL;

  /*
   *	fill in file and filename fields
   */
  this->gpd_filename = gpd_filename;
  this->gpd_file = gpd_file;

  if (!this->mapx->mpp_filename) 
    this->mapx->mpp_filename = strdup(gpd_filename);

  return this;

error_return:
  fprintf(stderr,"init_grid: error reading grid parameters definition file\n");
  if (label) free(label);
  if (gpd_filename) free(gpd_filename);
  if (gpd_file) fclose(gpd_file);
  close_grid(this);
  return NULL;
}

/*----------------------------------------------------------------------
 * new_grid - initialize grid coordinate system from label
 *
 *	input : label - char buffer with initialization information
 *		        see init_grid for format
 *
 *	result: pointer to new grid_class instance
 *		or NULL if an error occurs during initialization
 *
 *----------------------------------------------------------------------*/
grid_class *new_grid(char *label)
{
  grid_class *this;

/*
 *	allocate storage for grid parameters
 */
  this = (grid_class *)calloc(1, sizeof(grid_class));
  if (this == NULL) { perror("new_grid"); return NULL; }

/*
 *	decode grid parameters definitions
 */
  if (!decode_gpd(this, label)) { close_grid(this); return NULL; }

  return this;
}

/*------------------------------------------------------------------------
 * decode_gpd - parse information in grid parameters definition label
 *
 *	input : this - pointer to grid data structure (returned by new_grid)
 *		label - grid parameters definition label
 *
 *	result: TRUE iff success
 *
 *	effect: fills grid data structure with values read from label
 *
 *------------------------------------------------------------------------*/
static bool decode_gpd(grid_class *this, char *label)
{
  double f1, f2;
  char filename[FILENAME_MAX] = "";

  /*
   *	initialize map projection and determine file format
   *	first check for Grid MPP File tag
   */
  if (get_value_keyval(label, "Grid MPP File", "%s", filename, keyval_FALL_THRU_STRING) &&
      !streq(filename, keyval_FALL_THRU_STRING)) {

    this->mapx = init_mapx(filename);
    if (NULL == this->mapx) return FALSE;

  } else {

    /*
     *	look for embedded MPP parameters, but don't complain
     *  about unknown projection
     */
    this->mapx = new_mapx(label, TRUE);

    if (NULL == this->mapx) {

      /*
       * try old fixed format
       */
      if (grid_verbose) fprintf(stderr,"> assuming old style fixed format file\n");
      return old_fixed_format_decode_gpd(this, label);
    }
  }

  /*
   * go with keyword: value format
   */
  if (!get_value_keyval(label, "Grid Width", "%d", &(this->cols), NULL)) {
    fprintf(stderr,"grids: Grid Width is a required field\n");
    return FALSE;
  }

  if (!get_value_keyval(label, "Grid Height", "%d", &(this->rows), NULL)) {
    fprintf(stderr,"grids: Grid Height is a required field\n");
    return FALSE;
  }

  /*
   * map origin defaults to (0,0)
   */
  get_value_keyval(label, "Grid Map Origin Column", "%lf", &(this->map_origin_col), "0");
  get_value_keyval(label, "Grid Map Origin Row", "%lf", &(this->map_origin_row), "0");

  /*
   * there are many ways to specify the column/row to map unit scale, default is 1
   */
  get_value_keyval(label, "Grid Cells per Map Unit", "%lf", &f1, "0");
  f2 = f1;
  if (0 == f1) {
    get_value_keyval(label, "Grid Map Units per Cell", "%lf", &f1, "0");
    f1 = f1 ? 1/f1 : 0;
    f2 = f1;
  }

  if ( 0 == f1) {
    get_value_keyval(label, "Grid Columns per Map Unit", "%lf", &f1, "0");
    if (0 == f1) {
      get_value_keyval(label, "Grid Map Units per Column", "%lf", &f1, "1");
      f1 = 1/f1;
    }
  }

  if (0 == f2) {
    get_value_keyval(label, "Grid Rows per Map Unit", "%lf", &f2, "0");
    if (0 == f2) {
      get_value_keyval(label, "Grid Map Units per Row", "%lf", &(f2), "1");
      f2 = 1/f2;
    }
  }

  this->cols_per_map_unit = f1;
  this->rows_per_map_unit = f2;

  return TRUE;
}

/*------------------------------------------------------------------------
 * old_fixed_format_decode_gpd
 *
 *	input : this - pointer to grid data structure (returned by new_grid)
 *		label - contents of grid parameters definition file
 *
 *	result: TRUE iff success
 *
 *	effect: fills grid data structure with values read from label
 *
 *------------------------------------------------------------------------*/
static bool old_fixed_format_decode_gpd(grid_class *this, char *label)
{
  int ios;
  double f1, f2;
  char filename[FILENAME_MAX], readln[FILENAME_MAX];

/*
 *	initialize map transformation
 */
  
  if ((label = next_line_from_buffer(label, readln)) == NULL) return FALSE;
  sscanf(readln, "%s", filename);
  this->mapx = init_mapx(filename);
  if (this->mapx == NULL) return FALSE;

/*
 *	read in remaining parameters
 */
  if ((label = next_line_from_buffer(label, readln)) == NULL) return FALSE;
  ios = sscanf(readln, "%lf %lf", &f1, &f2);
  this->cols = (ios >= 1) ? f1 : 512;
  this->rows = (ios >= 2) ? f2 : 512;

  if ((label = next_line_from_buffer(label, readln)) == NULL) return FALSE;
  ios = sscanf(readln, "%lf %lf", &f1, &f2);
  this->cols_per_map_unit = (ios >= 1) ? f1 : 64;
  this->rows_per_map_unit = (ios >= 2) ? f2 : this->cols_per_map_unit;

  if ((label = next_line_from_buffer(label, readln)) == NULL) return FALSE;
  ios = sscanf(readln, "%lf %lf", &f1, &f2);
  this->map_origin_col = (ios >= 1) ? f1 : this->cols/2.;
  this->map_origin_row = (ios >= 2) ? f2 : this->rows/2.;

  return TRUE;
}

/*------------------------------------------------------------------------
 * close_grid - free storage and close files associated with grid
 *
 *	input : this - pointer to grid data structure (returned by init_grid)
 *
 *------------------------------------------------------------------------*/
void close_grid(grid_class *this)
{
  if (this == NULL) return;
  if (this->gpd_file != NULL) fclose(this->gpd_file);
  if (this->gpd_filename != NULL) free(this->gpd_filename);
  close_mapx(this->mapx);
  free(this);
}

/*------------------------------------------------------------------------
 * forward_grid - forward grid transformation
 *
 *	input : this - pointer to grid data structure (returned by init_grid)
 *		lat,lon - geographic coordinates in decimal degrees
 *
 *	output: r,s - grid coordinates 
 *
 *	result: TRUE iff r,s are on the grid
 *
 *	Grid coordinates (r,s) start at (0,0) in the upper left corner 
 *	with r increasing to the right and s increasing downward. Note 
 *	that the r coordinate corresponds to the grid column number j
 *	and the s coordinate corresponds to the grid row number i.
 *	Also, grid r is in the same direction as map u, and grid s is
 *	in the opposite direction of map v.
 *
 *------------------------------------------------------------------------*/
int forward_grid(grid_class *this,
		 double lat, double lon, double *r, double *s)
{
  register int status;
  double u,v;

  status = forward_mapx(this->mapx, lat, lon, &u, &v);
  if (status != 0) return FALSE;

  *r = this->map_origin_col + u * this->cols_per_map_unit;
  *s = this->map_origin_row - v * this->rows_per_map_unit;

  if (*r < -0.5 || *r >= this->cols - 0.5 
      || *s < -0.5 || *s >= this->rows - 0.5)
    return FALSE;
  else
    return TRUE;
}

/*------------------------------------------------------------------------
 * inverse_grid - inverse grid transformation
 *
 *	input : this - pointer to grid data structure (returned by init_grid)
 *		r,s - grid coordinates
 *
 *	output: lat,lon - geographic coordinates in decimal degrees
 *
 *	result: TRUE iff lat, lon are within map boundaries
 *
 *------------------------------------------------------------------------*/
int inverse_grid (grid_class *this,
		  double r, double s, double *lat, double *lon)
{
  register int status;
  double u,v;

  u =  (r - this->map_origin_col) / this->cols_per_map_unit;
  v = -(s - this->map_origin_row) / this->rows_per_map_unit;

  status = inverse_mapx(this->mapx, u, v, lat, lon);
  if (status != 0) return FALSE;
  return within_mapx(this->mapx, *lat, *lon);
}

#ifdef GTEST
/*------------------------------------------------------------------------
 * gtest - interactive test grid routines
 *------------------------------------------------------------------------*/
main(int argc, char* argv[])
{
  double lat, lon, r, s;
  int status;
  char readln[FILENAME_MAX];
  grid_class *the_grid = NULL;

  grid_verbose = 1;

  for (;;)
  { 
    if (argc > 1) {
      --argc; ++argv;
      strcpy(readln, *argv);
    }
    else {
      printf("\nenter .gpd file name: ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
    }

    close_grid(the_grid);
    the_grid = init_grid(readln);
    if (the_grid == NULL) continue;

    printf("\ngpd: %s\n", the_grid->gpd_filename);
    printf("mpp:%s\n", the_grid->mapx->mpp_filename);

    printf("\nforward_grid:\n");
    for (;;)
    { printf("enter lat lon: ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%lf %lf", &lat, &lon);
      status = forward_grid(the_grid, lat, lon, &r, &s);
      printf("col,row = %lf %lf    status = %d\n", r, s, status);
      status = inverse_grid(the_grid, r, s, &lat, &lon);
      printf("lat,lon = %lf %lf    status = %d\n", lat, lon, status);
    }

    printf("\ninverse_grid:\n");
    for (;;)
    { printf("enter col row: ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%lf %lf", &r, &s);
      status = inverse_grid(the_grid, r, s, &lat, &lon);
      printf("lat,lon = %lf %lf    status = %d\n", lat, lon, status);
      status = forward_grid(the_grid, lat, lon, &r, &s);
      printf("col,row = %lf %lf    status = %d\n", r, s, status);
    }
  }
}
#endif

#ifdef CRTEST
/*------------------------------------------------------------------------
 * crtest - interactive test for grid routines
 *------------------------------------------------------------------------*/
main(int argc, char *argv[])
{
  double lat, lon, col, row;
  int status;
  char readln[FILENAME_MAX];
  grid_class *the_grid = NULL;

  grid_verbose = 1;

  for (;;)
  { 
    if (argc > 1) {
      --argc; ++argv;
      strcpy(readln, *argv);
    }
    else {
      printf("\nenter .gpd file name: ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
    }

    close_grid(the_grid);
    the_grid = init_grid(readln);
    if (the_grid == NULL) continue;
    
    printf("\ngpd: %s\n", the_grid->gpd_filename);
    printf("mpp:%s\n", the_grid->mapx->mpp_filename);

   printf("\nforward_grid:\n");
    for (;;)
    { printf("enter lat lon: ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%lf %lf", &lat, &lon);
      status = forward_grid(the_grid, lat, lon, &col, &row);
      printf("col,row = %17.7lf %17.7lf     %s\n", col, row, 
	     status == 1 ? "valid" : "invalid");
      status = inverse_grid(the_grid, col, row, &lat, &lon);
      printf("lat,lon = %11.7lf %12.7lf     %s\n", lat, lon, 
	     status == 1 ? "valid" : "invalid");
    }
    
    printf("\ninverse_grid:\n");
    for (;;)
    { printf("enter col row: ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%lf %lf", &col, &row);
      status = inverse_grid(the_grid, col, row, &lat, &lon);
      printf("lat,lon = %11.7lf %#12.7lf    %s\n", lat, lon, 
	     status == 1 ? "valid" : "invalid");
      status = forward_grid(the_grid, lat, lon, &col, &row);
      printf("col,row = %17.7lf %#17.7lf    %s\n", col, row, 
	     status == 1 ? "valid" : "invalid");
    }
  }
}
#endif

#ifdef GACCT
#define usage "usage: gacct mpp_file"
/*------------------------------------------------------------------------
 * gacct - accuracy test grid routines
 *------------------------------------------------------------------------*/
#define GPMON
static double dist_pixels(double r1, double s1, double r2, double s2)
{
  double dr, ds;

  dr = r1 - r2;
  ds = s1 - s2;
  return(sqrt(dr * dr + ds * ds));
}
#endif

#ifdef GPMON
/*------------------------------------------------------------------------
 * gpmon - performance test grid routines
 *------------------------------------------------------------------------*/
#ifndef usage
#define usage "usage: gpmon gpd_file [num_its]"
#endif
main(int argc, char* argv[])
{
  register int ii, npts = 0, status1, status2;
  int its = 1;
  register double r, s;
  double lat, lon, rx, sx;
  grid_class *the_grid;
#ifdef GACCT
  int bad_pts=0;
  double err=0, sum=0, sum2=0, stdv=0, max_err=-1, lat_max=0, lon_max=0;
  int r_max=0, s_max=0;
#endif

  if (argc < 2) 
#ifdef GACCT
  { fprintf(stderr,"#\tgacct can be used to test the accuracy\n");
    fprintf(stderr,"#\tof the grid routines. It runs the forward and\n");
    fprintf(stderr,"#\tinverse transforms over the whole grid.\n");
    fprintf(stderr,"#\tError statistics are accumulated in kilometers.\n");
    fprintf(stderr,"#\tTo run the test type:\n");
    fprintf(stderr,"#\t\tgacct test.gpd\n");
    fprintf(stderr,"\n");
    error_exit(usage);
  }
#else
  { fprintf(stderr,"#\tgpmon can be used to monitor the performance\n");
    fprintf(stderr,"#\tof the grid routines. It runs the forward and\n");
    fprintf(stderr,"#\tinverse transforms on each point in the grid.\n");
    fprintf(stderr,"#\tThe optional parameter num_its specifies how\n");
    fprintf(stderr,"#\tmany times to run through the entire grid, (the\n");
    fprintf(stderr,"#\tdefault is 1). To run the test type:\n");
    fprintf(stderr,"#\t\tgpmon test.gpd\n");
    fprintf(stderr,"#\t\tprof gpmon\n");
    fprintf(stderr,"\n");
    error_exit(usage);
  }
#endif

  the_grid = init_grid(argv[1]);
  if (the_grid == NULL) error_exit(usage);
  if (argc < 3 || sscanf(argv[2],"%d",&its) != 1) its = 1;

  for (ii = 1; ii <= its; ii++)
  { for (r = 0; r < the_grid->cols; r++)
    { for (s = 0; s < the_grid->rows; s++)
      { status1 = inverse_grid(the_grid, r, s, &lat, &lon);
	status2 = forward_grid(the_grid, lat, lon, &rx, &sx);
	if (!status1 || !status2)
	  ++bad_pts;
	++npts;
#ifdef GACCT
	if (status1 && status2)
	{
	  err = dist_pixels(r, s, rx, sx);
	  if (err > 0) {
	    sum += err;
	    sum2 += err*err;
	  }
	  if (err > max_err) {
	    max_err=err;
	    r_max = r;
	    s_max = s;
	    lat_max = lat;
	    lon_max = lon;
	  }
	}
#endif
	  
      }
    }
  }
  fprintf(stderr,"%d points,  %d bad points\n", npts, bad_pts);
#ifdef GACCT
  npts -= bad_pts;
  if (npts > 0)
  { err = sum/npts;
    stdv = sqrt((sum2 - npts*err*err)/(npts-1));
  }
  fprintf(stderr,"average error = %10.4le pixels\n", err);
  fprintf(stderr,"std dev error = %10.4le pixels\n", stdv);
  fprintf(stderr,"maximum error = %10.4le pixels\n", max_err);
  fprintf(stderr,"max error was at col: %d  row: %d   lat: %lf  lon: %lf\n",
	  r_max, s_max, lat_max, lon_max);
#endif

}
#endif
