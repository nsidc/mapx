/*========================================================================
 * ungrid - extract point data from a grid
 *
 * 20-Feb-2004 K.Knowles knowlesk@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 * Copyright (C) 2004 University of Colorado
 *========================================================================*/
static const char ungrid_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/ungrid.c,v 1.2 2004-07-20 21:19:39 knowlesk Exp $";

#include "define.h"
#include "matrix.h"
#include "grids.h"

#define usage									\
"usage: ungrid [-v] [-b] [-i fill] [-n min_value] [-x max_value]\n"		\
"              [-c method] [-r radius] [-p power]\n"				\
"              from_gpd from_data\n"						\
"\n"										\
" input : from.gpd  - source grid parameters definition file\n"			\
"         from_data - source gridded data file (4 byte floats)\n"		\
"         < stdin - list of locations one lat/lon pair per line\n"		\
"\n"										\
" output: > stdout - list of '[lat lon] value' for each input point\n"		\
"\n"										\
" option: v - verbose\n"							\
"         b - binary (float) stdin and stdout (default is ASCII) \n"		\
"             Note: the input grid is always binary (float).\n"			\
"             If binary is set, the location is not echoed to\n"		\
"             the output but the data values are written in the\n"		\
"             same order as the input points.\n"				\
"         i fill - fill value for missing data (default = 0)\n"			\
"         n min_value - ignore data values less than min_value\n"		\
"         x max_value - ignore data values greater than max_value\n"		\
"         c method - choose interpolation method\n"				\
"                    N = nearest neighbor (default)\n"				\
"                    D = drop-in-the-bucket\n"					\
"                    B = bilinear\n"						\
"                    C = cubic convolution\n"					\
"                    I = inverse distance\n"					\
"         r radius - circle to average over (-c D or I only) \n"		\
"         p power - inverse distance exponent (default = 2, -c I only) \n"	\
"\n"

static int verbose = 0;

struct interp_control {
  grid_class *grid;
  int min_set;
  float min_value;
  int max_set;
  float max_value;
  float fill_value;
  float shell_radius;
  float power;
};

static int cubic(float *value, float **from_data, double r, double s, 
		 struct interp_control *control);
static int nearest(float *value, float **from_data, double r, double s,
		   struct interp_control *control);
static int average(float *value, float **from_data, double r, double s, 
		   struct interp_control *control);
static int bilinear(float *value, float **from_data, double r, double s, 
		    struct interp_control *control);
static int distance(float *value, float **from_data, double r, double s, 
			struct interp_control *control);

static char possible_methods[] = "NDBCI";

static int (*method_function[])()  = { nearest,
				       average,
				       bilinear,
				       cubic,
				       distance };

static char *method_string[] = { "nearest-neighbor",
				 "drop-in-the-bucket",
				 "bilinear",
				 "cubic convolution",
				 "inverse distance" };


int main(int argc, char *argv[]) { 
  int io_err, status, method_number, line_num, row;
  bool do_binary;
  double to_lat, to_lon;
  double from_r, from_s;
  float **from_data;
  float value;
  char *option, *position;
  char method;
  char readln[MAX_STRING];
  char from_filename[FILENAME_MAX];
  FILE *from_file;
  int (*interpolate)();
  struct interp_control control;

/*
 * set defaults
 */
  control.min_set = FALSE;
  control.max_set = FALSE;
  control.fill_value = 0;
  control.shell_radius = 0.5;
  control.power = 2;
  do_binary = FALSE;
  verbose = 0;
  method = 'N';

/* 
 * get command line options
 */
  while (--argc > 0 && (*++argv)[0] == '-') {
    for (option = argv[0]+1; *option != '\0'; option++) {
      switch (*option) {
	case 'v':
	  ++verbose;
	  break;
	case 'b':
	  do_binary = TRUE;
	  break;
	case 'c':
	  ++argv; --argc;
	  method = *argv[0];
	  break;
	case 'r':
	  ++argv; --argc;
	  if (sscanf(*argv, "%f", &(control.shell_radius)) != 1) error_exit(usage);
	  break;
	case 'n':
	  ++argv; --argc;
	  if (sscanf(*argv, "%f", &(control.min_value)) != 1) error_exit(usage);
	  control.min_set = TRUE;
	  break;
	case 'x':
	  ++argv; --argc;
	  if (sscanf(*argv, "%f", &(control.max_value)) != 1) error_exit(usage);
	  control.max_set = TRUE;
	  break;
	case 'i':
	  ++argv; --argc;
	  if (sscanf(*argv, "%f", &(control.fill_value)) != 1) error_exit(usage);
	  break;
	case 'p':
	  ++argv; --argc;
	  if (sscanf(*argv, "%f", &(control.power)) != 1) error_exit(usage);
	  break;
	case 'V':
	  fprintf(stderr,"%s\n", ungrid_c_rcsid);
	  break;
	default:
	  fprintf(stderr,"invalid option %c\n", *option);
	  error_exit(usage);
      }
    }
  }

/*
 * validate method option
 */
  position = strchr(possible_methods, method);
  if (!position) {
    fprintf(stderr,"resamp: method %c not in [%s]\n",
	    method, possible_methods);
    error_exit(usage);
  }
  method_number = position - possible_methods;
  interpolate = method_function[method_number];

/*
 * get command line arguments
 */
  if (argc != 2) error_exit(usage);

  control.grid = init_grid(*argv);
  if (!control.grid) error_exit("ungrid: ABORTING");
  ++argv; --argc;

  strcpy(from_filename, *argv);
  from_file = fopen(from_filename, "r");
  if (!from_file) { perror(from_filename); error_exit("ungrid: ABORTING"); }
  ++argv; --argc;
  
/*
 * echo defaults and settings
 */
  if (verbose) {
    fprintf(stderr,"> Data grid:\t%s\n", control.grid->gpd_filename);
    fprintf(stderr,"> Data file:\t%s\n", from_filename);
    fprintf(stderr,"> Method:\t%c = %s\n", method, method_string[method_number]);
    fprintf(stderr,"> Fill value:\t%g\n", control.fill_value);
    if (control.min_set) fprintf(stderr,"> Valid min:\t%g\n", control.min_value);
    if (control.max_set) fprintf(stderr,"> Valid max:\t%g\n", control.max_value);
    fprintf(stderr,"> Shell radius:\t%g\n", control.shell_radius);
    if (method == 'I') fprintf(stderr,"> Power:\t%g\n", control.power);
    fprintf(stderr,"> Format:\t%s\n", do_binary ? "binary" : "ascii");
  }

/*
 * read in grid of input data values
 */
  from_data = (float **)matrix(control.grid->rows, control.grid->cols,
			       sizeof(float), TRUE);
  if (!from_data) { error_exit("ungrid: ABORTING"); }

  for (row = 0; row < control.grid->rows; row++) {
    status = fread(from_data[row], sizeof(float), control.grid->cols, from_file);
    if (status != control.grid->cols) {
      perror(from_filename);
      free(from_data);
      error_exit("ungrid: ABORTING");
    }
  }
/*
 * loop through input points
 */
  for (line_num = 1; !feof(stdin); line_num++) {

/*
 * read a point
 */
    if (do_binary) {
      fread(&to_lat, sizeof(to_lat), 1, from_file);
      fread(&to_lon, sizeof(to_lon), 1, from_file);
      io_err = ferror(from_file);
    } else {
      fgets(readln, MAX_STRING, stdin);
      io_err = (sscanf(readln, "%lf %lf", &to_lat, &to_lon) != 2);
    }

    if (io_err != 0) { 
      fprintf(stderr, "ungrid: error reading lat/lon at line %i\n", line_num);
      if (do_binary) error_exit("ungrid: ABORTING");
      continue;
    }

    if (feof(stdin)) break;

/*
 * extract data from grid
 */
    value = control.fill_value;

    status = forward_grid(control.grid, to_lat, to_lon, &from_r, &from_s);
    if (!status) {
      if (verbose >= 2) 
	fprintf(stderr,">> line %d lat/lon %f %f is off the grid\n",
		line_num, to_lat, to_lon);

    } else {

      status = interpolate(&value, from_data, from_r, from_s, &control);
      if (status < 0) {
	if (verbose >= 2) fprintf(stderr,">> can't interpolate to %f %f at line %d\n",
				  to_lat, to_lon, line_num);
      }
    }

/*
 * write the point
 */
    if (do_binary) {
      fwrite(&value, sizeof(value), 1, stdout);
      io_err = ferror(stdout);
    } else {
      printf("%f %f %f\n", to_lat, to_lon, value);
      io_err = ferror(stdout);
    }

    if (io_err != 0) {
      perror("writing to stdout");
      fprintf(stderr, "ungrid: line %d\n", line_num);
    }
  }

  if (verbose) fprintf(stderr,"> %d points processed\n", line_num - 1);

  exit(EXIT_SUCCESS);
}

/*------------------------------------------------------------------------
 * cubic - cubic convolution
 *
 *	input : from_data - pointer to input data array
 *              r, s - column, row coordinates within input grid
 *              control - control parameter structure
 *
 *	output: value - data value interpolated to r,s
 *
 *	result: number of valid points sampled
 *
 *------------------------------------------------------------------------*/
static int cubic(float *value, float **from_data, double r, double s, 
		 struct interp_control *control) {
  int col, row, npts;
  double dr, ds, ccr[4], ccs[4], ccr_col, ccs_row;
  double weight, value_sum, weight_sum;
 
/*
 * get cubic coefficients
 */
  dr = r - (int)r;
  ds = s - (int)s;

  ccr[0] = -dr*(1-dr)*(1-dr);
  ccr[1] = (1 - 2*dr*dr + dr*dr*dr);
  ccr[2] = dr*(1 + dr - dr*dr);
  ccr[3] = -dr*dr*(1-dr);

  ccs[0] = -ds*(1-ds)*(1-ds);
  ccs[1] = (1 - 2*ds*ds + ds*ds*ds);
  ccs[2] = ds*(1 + ds - ds*ds);
  ccs[3] = -ds*ds*(1-ds);

  value_sum = 0.;
  weight_sum = 0.;
  npts = 0;

/*
 * interpolated value is weighted sum of sixteen surrounding samples
 */
  for (row = (int)s - 1; row <= (int)s + 2; row++) {
    if (row < 0 || row >= control->grid->rows ) continue;

    ccs_row = ccs[row - ((int)s - 1)];

    for (col = (int)r - 1; col <= (int)r + 2; col++) {
      if (col < 0 || col >= control->grid->cols) continue;

      if (control->max_set && control->max_value < from_data[row][col]) continue;
      if (control->min_set && control->min_value > from_data[row][col]) continue;

      ccr_col = ccr[col - ((int)r - 1)];

      weight = ccs_row*ccr_col;

      value_sum += weight*from_data[row][col];
      weight_sum += weight;
      ++npts;
    }
  }

  if (npts > 0) {
    *value = value_sum;
    if (weight_sum != 0) *value /= weight_sum;
  } else {
    *value = control->fill_value;
  }

  return npts;
}

/*------------------------------------------------------------------------
 * average - weighted average within a radius
 *
 *	input : from_data - pointer to input data array
 *              r, s - column, row coordinates within input grid
 *              control - control parameter structure
 *
 *	output: value - data value interpolated to r,s
 *
 *	result: number of valid points sampled
 *
 *------------------------------------------------------------------------*/
static int average(float *value, float **from_data, double r, double s, 
		   struct interp_control *control) {
  int col, row, half_width;
  double dr, ds, dr2, ds2, r2, value_sum;
  int npts;

  r2 = control->shell_radius*control->shell_radius;

  value_sum = 0;
  npts = 0;

  half_width = ceil((double)control->shell_radius);
  if (half_width < 1) half_width = 1;

  for (row=(int)s - (half_width-1); row <= (int)s + half_width; row++) {
    if (row < 0 || row >= control->grid->rows) continue;

    ds = row - s;
    ds2 = ds*ds;

    for (col=(int)r - (half_width-1); col <= (int)r + half_width; col++) {
      if (col < 0 || col >= control->grid->cols) continue;
      if (control->max_set && control->max_value < from_data[row][col]) continue;
      if (control->min_set && control->min_value > from_data[row][col]) continue;

      dr = col - r;
      dr2 = dr*dr;

      if ((dr2 + ds2) <= r2) {
	value_sum += from_data[row][col];
	++npts;
      }
    }
  }

  if (npts > 0) {
    *value = value_sum/npts;
  } else {
    *value = control->fill_value;
  }

  return npts;
}

/*------------------------------------------------------------------------
 * bilinear - bilinear interpolation
 *
 *	input : from_data - pointer to input data array
 *              r, s - column, row coordinates within input grid
 *              control - control parameter structure
 *
 *	output: value - data value interpolated to r,s
 *
 *	result: number of valid points sampled
 *
 *------------------------------------------------------------------------*/
static int bilinear(float *value, float **from_data, double r, double s, 
		    struct interp_control *control) {
  int col, row;
  double dr, ds, weight, value_sum, weight_sum;
  int npts;

  value_sum = 0.;
  weight_sum = 0.;
  npts = 0;

  for (row=(int)s; row <= (int)s + 1; row++) {
    if (row < 0 || row >= control->grid->rows) continue;

    ds = fabs(s - row);

    for (col=(int)r; col <= (int)r + 1; col++) {
      if (col < 0 || col >= control->grid->cols) continue;
      if (control->max_set && control->max_value < from_data[row][col]) continue;
      if (control->min_set && control->min_value > from_data[row][col]) continue;

      dr = fabs(r - col);

      weight = (1 - ds)*(1 - dr);
      value_sum += weight*from_data[row][col];
      weight_sum += weight;
      ++npts;
    }
  }

  if (weight_sum > 0) {
    *value = value_sum/weight_sum;
  } else {
    *value = control->fill_value;
  }

  return npts;
}

/*------------------------------------------------------------------------
 * nearest - nearest-neighbor
 *
 *	input : from_data - pointer to input data array
 *              r, s - column, row coordinates within input grid
 *              control - control parameter structure
 *
 *	output: value - data value interpolated to r,s
 *
 *	result: number of valid points sampled
 *
 *------------------------------------------------------------------------*/
static int nearest(float *value, float **from_data, double r, double s, 
		   struct interp_control *control) {
  int col, row;
  int npts=0;

  row = nint(s);
  col = nint(r);

  if (row < 0 || row >= control->grid->rows
      || col < 0 || col >= control->grid->cols) {
    *value = control->fill_value;
  } else {
    *value = from_data[row][col];
    npts = 1;
  }

  return npts;
}

/*------------------------------------------------------------------------
 * distance - normalized distance interpolation
 *
 *	input : from_data - pointer to input data array
 *              r, s - column, row coordinates within input grid
 *              control - control parameter structure
 *
 *	output: value - data value interpolated to r,s
 *
 *	result: number of valid points sampled
 *
 *------------------------------------------------------------------------*/
static int distance(float *value, float **from_data, double r, double s, 
		    struct interp_control *control) {
  int col, row, half_width;
  double dr, ds, dr2, ds2, dd, weight, value_sum, weight_sum;
  int npts;

  value_sum = 0;
  weight_sum = 0;
  npts = 0;

  half_width = ceil((double)control->shell_radius);
  if (half_width < 1) half_width = 1;

  for (row=(int)s - (half_width-1); row <= (int)s + half_width; row++) {
    if (row < 0 || row >= control->grid->rows) continue;

    ds = row - s;
    ds2 = ds*ds;

    for (col=(int)r - (half_width-1); col <= (int)r + half_width; col++) {
      if (col < 0 || col >= control->grid->cols) continue;
      if (control->max_set && control->max_value < from_data[row][col]) continue;
      if (control->min_set && control->min_value > from_data[row][col]) continue;

      dr = col - r;
      dr2 = dr*dr;

      dd = sqrt(dr2 + ds2);

      weight = dd <= control->shell_radius ? pow(dd,-control->power) : 0.0;

      value_sum += weight*from_data[row][col];
      weight_sum += weight;
      ++npts;
    }
  }

  if (npts > 0) {
    *value = value_sum;
    if (weight_sum != 0) *value /= weight_sum;
  } else {
    *value = control->fill_value;
  }

  return npts;
}
