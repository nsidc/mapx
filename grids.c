/*========================================================================
 * grids - grid coordinate system definition and transformations
 *
 *	26-Dec-1991 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 *	7-July-1992 K.Knowles - changed grid_struct to grid_class
 *				updated references to mapx
 *	30-Dec-1992 K.Knowles - added interactive and performance tests
 *========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  register int status;
  float u,v;

  status = forward_mapx(this->mapx, lat, lon, &u, &v);
  if (status != 0) return FALSE;

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
  register int status;
  float u,v;

  u =  (r - this->map_origin_col) / this->cols_per_map_unit;
  v = -(s - this->map_origin_row) / this->rows_per_map_unit;
  status = inverse_mapx(this->mapx, u, v, lat, lon);
  if (status != 0) return FALSE;
  return within_mapx(this->mapx, *lat, *lon);
}

#ifdef GTEST
/*========================================================================
 * gtest - interactive test grid routines
 *========================================================================*/
main(int argc, char* argv[])
{
  float lat, lon, r, s;
  int status;
  char readln[80];
  grid_class *the_grid;

  for (;;)
  { printf("\nenter .gpd file name - ");
    gets(readln);
    if (feof(stdin)) { printf("\n"); exit(0);}
    if (*readln == '\0') break;
    the_grid = init_grid(readln);
    if (the_grid == NULL) continue;

    printf("\nforward_grid:\n");
    for (;;)
    { printf("enter lat lon - ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%f %f", &lat, &lon);
      status = forward_grid(the_grid, lat, lon, &r, &s);
      printf("col,row = %f %f    status = %d\n", r, s, status);
      status = inverse_grid(the_grid, r, s, &lat, &lon);
      printf("lat,lon = %f %f    status = %d\n", lat, lon, status);
    }

    printf("\ninverse_grid:\n");
    for (;;)
    { printf("enter r s - ");
      gets(readln);
      if (feof(stdin)) { printf("\n"); exit(0);}
      if (*readln == '\0') break;
      sscanf(readln, "%f %f", &r, &s);
      status = inverse_grid(the_grid, r, s, &lat, &lon);
      printf("lat,lon = %f %f    status = %d\n", lat, lon, status);
      status = forward_grid(the_grid, lat, lon, &r, &s);
      printf("col,row = %f %f    status = %d\n", r, s, status);
    }
  }
}
#endif

#ifdef GPMON
/*========================================================================
 * gpmon - performance test grid routines
 *========================================================================*/
#define usage "usage: gpmon gpd_file [num_its]"
main(int argc, char* argv[])
{
  register int ii, npts = 0, status;
  int its = 1;
  register float r, s;
  float lat, lon, rx, sx;
  grid_class *the_grid;

  if (argc < 2) 
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
  the_grid = init_grid(argv[1]);
  if (the_grid == NULL) error_exit(usage);
  if (argc < 3 || sscanf(argv[2],"%d",&its) != 1) its = 1;

  for (ii = 1; ii <= its; ii++)
  { for (r = 0; r < the_grid->cols; r++)
    { for (s = 0; s < the_grid->rows; s++)
      { ++npts;
	status = inverse_grid(the_grid, r, s, &lat, &lon);
	status = forward_grid(the_grid, lat, lon, &rx, &sx);
      }
    }
  }
  fprintf(stderr,"%d points\n", npts);
}
#endif
