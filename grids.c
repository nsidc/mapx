/*========================================================================
 * grids - grid coordinate system definition and transformations
 *
 *	26-Dec-1991 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 *	7-July-1992 K.Knowles - changed grid_struct to grid_class
 *				updated references to mapx
 *========================================================================*/
#include <stdio.h>
#include <define.h>
#include <mapx.h>
#include <grids.h>

/*----------------------------------------------------------------------
 * init_grid - initialize grid coordinate system
 *
 *	input : grid_filename - grid parameter definitions file name
 *			format as follows:
 *			 mpp_filename
 *			 number_of_columns number_of_rows 
 *			 columns_per_map_unit rows_per_map_unit 
 *			 map_origin_column map_origin_row 
 *
 *	result: pointer to new grid_class instance
 *		or NULL if an error occurs during initialization
 *
 *	note:	for some parameters, if no value is specified in 
 *		the .gpd file the parameter is set to a default 
 *		value without warning
 *
 *----------------------------------------------------------------------*/
grid_class *init_grid(char *grid_filename)
{
  register int ios;
  float f1, f2;
  char map_filename[80], readln[80];
  grid_class *this;

  this = (grid_class *) malloc(sizeof(grid_class));
  if (this == NULL)
  { perror("init_grid");
    return NULL;
  }
  this->gpd_file = NULL;
  this->gpd_filename = NULL;
  this->mapx = NULL;

  this->gpd_file = fopen(grid_filename, "r");
  if (this->gpd_file == NULL)
  { fprintf(stderr,"init_grid: error opening parameters file.\n");
    perror(grid_filename);
    close_grid(this);
    return NULL;
  }

  this->gpd_filename = (char *) malloc(strlen(grid_filename)+1);
  if (this->gpd_filename != NULL) strcpy(this->gpd_filename, grid_filename);

/*
 *	initialize map transformation
 */
  fgets(readln, sizeof(readln), this->gpd_file);
  sscanf(readln, "%s", map_filename);

  this->mapx = init_mapx(map_filename);
  if (this->mapx == NULL)
  { close_grid(this);
    return NULL;
  }

/*
 *	read in remaining parameters
 */
  fgets(readln, sizeof(readln), this->gpd_file);
  ios = sscanf(readln, "%f %f", &f1, &f2);
  this->cols = (ios >= 1) ? f1 : 512;
  this->rows = (ios >= 2) ? f2 : 512;

  fgets(readln, sizeof(readln), this->gpd_file);
  ios = sscanf(readln, "%f %f", &f1, &f2);
  this->cols_per_map_unit = (ios >= 1) ? f1 : 64;
  this->rows_per_map_unit = (ios >= 2) ? f2 : this->cols_per_map_unit;

  fgets(readln, sizeof(readln), this->gpd_file);
  ios = sscanf(readln, "%f %f", &f1, &f2);
  this->map_origin_col = (ios >= 1) ? f1 : this->cols/2.;
  this->map_origin_row = (ios >= 2) ? f2 : this->rows/2.;

  if (ferror(this->gpd_file) || feof(this->gpd_file))
  { fprintf(stderr,"init_grid: error reading parameters file.\n");
    if (feof(this->gpd_file))
      fprintf(stderr,"%s: unexpected end of file.\n", grid_filename);
    else
      perror(grid_filename);
    close_grid(this);
    return NULL;
  }

  return this;
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
int forward_grid(grid_class *this, float lat, float lon, float *r, float *s)
{
  float u,v;

  forward_mapx(this->mapx, lat, lon, &u, &v);
  *r = this->map_origin_col + u * this->cols_per_map_unit;
  *s = this->map_origin_row - v * this->rows_per_map_unit;

  if (*r <= -0.5 || *r >= this->cols - 0.5 
      || *s <= -0.5 || *s >= this->rows - 0.5)
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
int inverse_grid (grid_class *this, float r, float s, float *lat, float *lon)
{
  float u,v;

  u =  (r - this->map_origin_col) / this->cols_per_map_unit;
  v = -(s - this->map_origin_row) / this->rows_per_map_unit;
  inverse_mapx(this->mapx, u, v, lat, lon);
  return within_mapx(this->mapx, *lat, *lon);
}
