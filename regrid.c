/*========================================================================
 * regrid - resample one grid to another 
 *
 * 27-Apr-1994 K.Knowles knowles@sastrugi.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *========================================================================*/
static const char regrid_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/regrid.c,v 1.4 1995-09-20 20:11:21 knowles Exp $";

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "matrix.h"
#include "mapx.h"
#include "grids.h"

#define usage \
"usage: regrid [-n -p power -bsl -v -f fill -k kernel] from.gpd to.gpd from_data to_data\n"\
"\n"\
" input : from.gpd  - original grid parameters definition file\n"\
"         to.gpd    - new grid parameters definition file\n"\
"         from_data - original gridded data file (flat file by rows)\n"\
"\n"\
" output: to_data - new gridded data file (flat file by rows)\n"\
"\n"\
" option: n - do nearest neighbor resampling\n"\
"         p power - 0=smooth, 6=sharp, 2=default\n"\
"         b - unsigned byte data (default)\n"\
"         s - unsigned short (2 bytes per sample)\n"\
"         l - unsigned long (4 bytes)\n"\
"         v - verbose (can be repeated)\n"\
"         f fill - set fill value for empty cells (default 0) \n"\
"         k kernel - force kernel size (rowsxcols)\n"\
"\n"

#define VV_INTERVAL 30

main (int argc, char *argv[])
{
  register int i, j, k, row, col;
  int fill, k_cols, k_rows, nparams;
  int data_bytes, row_bytes, status, total_bytes;
  int do_nearest_neighbor, verbose, very_verbose;
  float km_per_col, km_per_row, lat, lon, r, s;
  double weight, power, d2, dw, dr, ds;
  byte1 *iobuf, *bufp;
  float **from_data, **to_data, **to_beta;
  char *option;
  char *from_filename, *to_filename;
  FILE *from_file, *to_file;
  grid_class *from_grid, *to_grid;

/*
 *	set defaults
 */
  do_nearest_neighbor = FALSE;
  data_bytes = 1;
  k_rows = k_cols = 0;
  power = 2;
  fill = 0;
  verbose = FALSE;
  very_verbose = FALSE;

/* 
 *	get command line options
 */
  while (--argc > 0 && (*++argv)[0] == '-')
  { for (option = argv[0]+1; *option != '\0'; option++)
    { switch (*option)
      { case 'n':
	  do_nearest_neighbor = TRUE;
	  break;
	case 'k':
	  ++argv; --argc;
	  if (argc <= 0) error_exit(usage);
	  nparams = sscanf(*argv, "%dx%d", &k_rows, &k_cols);
	  if (0 == nparams) error_exit(usage);
	  if (1 == nparams) k_cols = k_rows;
	  break;
	case 'p':
	  ++argv; --argc;
	  if (sscanf(*argv, "%f", &r) != 1) error_exit(usage);
	  power = r;
	  break;
	case 'b':
	  data_bytes = 1;
	  break;
	case 's':
	  data_bytes = 2;
	  break;
	case 'l':
	  data_bytes = 4;
	  break;
	case 'f':
	  ++argv; --argc;
	  if (sscanf(*argv, "%d", &fill) != 1) error_exit(usage);
	  break;
	case 'v':
	  if (verbose) very_verbose = TRUE;
	  verbose = TRUE;
	  break;
	default:
	  fprintf(stderr,"invalid option %c\n", *option);
	  error_exit(usage);
      }
    }
  }

/*
 *	get command line arguments
 */
  if (argc != 4) error_exit(usage);
  
  from_grid = init_grid(*argv);
  if (NULL == from_grid) exit(ABORT);
  if (verbose) fprintf(stderr,"> from .gpd file %s...\n", 
		       from_grid->gpd_filename);
  ++argv; --argc;
  
  to_grid = init_grid(*argv);
  if (NULL == to_grid) exit(ABORT);
  if (verbose) fprintf(stderr,"> to .gpd file %s...\n",
		       to_grid->gpd_filename);
  ++argv; --argc;
  
  from_filename = strdup(*argv);
  from_file = fopen(from_filename, "r");
  if (NULL == from_file) { perror(from_filename); exit(ABORT); }
  if (verbose) fprintf(stderr,"> from data file %s...\n", from_filename);
  ++argv; --argc;
  
  to_filename = strdup(*argv);
  to_file = fopen(to_filename, "w");
  if (NULL == to_file) { perror(to_filename); exit(ABORT); }
  if (verbose) fprintf(stderr,"> to data file %s...\n", to_filename);
  ++argv; --argc;
  
/*
 *	allocate storage for data grids
 */
  if (verbose) fprintf(stderr,"> allocating data matrices...\n");

  from_data = (float **)matrix(from_grid->rows, from_grid->cols,
				sizeof(float), TRUE);
  if (NULL == from_data) { exit(ABORT); }

  to_data = (float **)matrix(to_grid->rows, to_grid->cols,
			      sizeof(float), TRUE);
  if (NULL == to_data) { exit(ABORT); }

  to_beta = (float **)matrix(to_grid->rows, to_grid->cols,
			      sizeof(float), TRUE);
  if (NULL == to_beta) { exit(ABORT); }

/*
 *	read original data
 */
  if (verbose) fprintf(stderr,"> reading %d bytes per sample...\n",data_bytes);

  total_bytes = 0;
  row_bytes = data_bytes * from_grid->cols;

  iobuf = calloc(from_grid->cols, data_bytes);
  if (NULL == iobuf) { perror("calloc"); exit(ABORT); }
  for (i = 0; i < from_grid->rows; i++)
  { status = fread(iobuf, 1, row_bytes, from_file);
    if (status != row_bytes) { perror (from_filename); exit(ABORT); }
    total_bytes += status;
    for (j = 0, bufp = iobuf; j < from_grid->cols; j++, bufp += data_bytes)
    { switch (data_bytes)
      { case 1: from_data[i][j] = (float) *((byte1 *)bufp); break;
	case 2: from_data[i][j] = (float) *((byte2 *)bufp); break;
	case 4: from_data[i][j] = (float) *((byte4 *)bufp); break;
      }
    }
  }
  if (verbose) fprintf(stderr,"> read %d bytes total\n", total_bytes);

/*  
 *	resample to the new grid
 */
  if (do_nearest_neighbor)
  { if (verbose) fprintf(stderr,"> inverse resampling nearest neighbor...\n");

    for (i = 0; i < to_grid->rows; i++) 
    { for (j = 0; j < to_grid->cols; j++)
      {
	status = inverse_grid(to_grid, (float)j, (float)i, &lat, &lon);
#ifdef DEBUG
	if (!status) to_data[i][j] = ~fill;
#endif
	if (!status) continue;

	status = forward_grid(from_grid, lat, lon, &r, &s);
#ifdef DEBUG
	if (!status) to_data[i][j] = ~fill;
#endif
	if (!status) continue;

	if (very_verbose && 0 == i % VV_INTERVAL && 0 == j % VV_INTERVAL)
	  fprintf(stderr,">> %4d %4d --> %7.2f %7.2f --> %4d %4d\n",
		  j, i, lat, lon, (int)(r + 0.5), (int)(s + 0.5));

	row = (int)(s + 0.5);
	col = (int)(r + 0.5);

	to_data[i][j] = from_data[row][col];
	to_beta[i][j] = 1;
      }
    }
  }
  else
  { 
/*
 *	determine extent of kernel
 */
    km_per_col = to_grid->mapx->scale/to_grid->cols_per_map_unit;
    km_per_row = to_grid->mapx->scale/to_grid->rows_per_map_unit;

    if (!k_rows || !k_cols)
    { k_cols = nint(from_grid->mapx->scale/from_grid->cols_per_map_unit
		    / km_per_col);
      if (k_cols < 1) k_cols = 1;
      k_rows = nint(from_grid->mapx->scale/from_grid->rows_per_map_unit
		    / km_per_row);
      if (k_rows < 1) k_rows = 1;
    }

    if (verbose) fprintf(stderr,"> forward resampling %dx%d kernel...\n", 
			 k_rows, k_cols);

    for (i = 0; i < from_grid->rows; i++) 
    { for (j = 0; j < from_grid->cols; j++)
      {
	status = inverse_grid(from_grid, (float)j, (float)i, &lat, &lon);
	if (!status) continue;

	status = forward_grid(to_grid, lat, lon, &r, &s);
	if (!status) continue;

	if (very_verbose && 0 == i % VV_INTERVAL && 0 == j % VV_INTERVAL)
	  fprintf(stderr,">> %4d %4d --> %7.2f %7.2f --> %4d %4d\n",
		  j, i, lat, lon, (int)(r + 0.5), (int)(s + 0.5));

	for (row=(int)(s-k_rows/2+.5); row <= (int)(s+k_rows/2+.5); row++)
	{ if (row < 0 || row >= to_grid->rows) continue;
	  ds = (s - row) * km_per_row;
	  for (col=(int)(r-k_cols/2+.5); col <= (int)(r+k_cols/2+.5); col++)
	  { if (col < 0 || col >= to_grid->cols) continue;
	    dr = (r - col) * km_per_col;
	    d2 = dr*dr + ds*ds;
	    dw = pow(d2, power/2);
	    weight = dw > 0 ? 1/dw : 9e9;
	    to_data[row][col] += from_data[i][j]*weight;
	    to_beta[row][col] += weight;
	  }
	}
      }
    }
  }

/*
 *	normalize result
 */
  if (verbose) fprintf(stderr,"> normalizing result...\n");
  for (i = 0; i < to_grid->rows; i++)
  { for (j = 0; j < to_grid->cols; j++)
    { 
      if (to_beta[i][j] != 0) 
	to_data[i][j] /= to_beta[i][j];
      else
	to_data[i][j] = fill;
    }
  }

/*
 *	write out result
 */
  if (verbose) fprintf(stderr,"> writing new data...\n");
  total_bytes = 0;
  row_bytes = data_bytes * to_grid->cols;

  for (i = 0; i < to_grid->rows; i++)
  { for (j = 0, bufp = iobuf; j < to_grid->cols; j++, bufp += data_bytes)
    { switch (data_bytes)
      { case 1: *((byte1 *)bufp) = nint(to_data[i][j]); break;
	case 2: *((byte2 *)bufp) = nint(to_data[i][j]); break;
	case 4: *((byte4 *)bufp) = nint(to_data[i][j]); break;
      }
    }
    status = fwrite(iobuf, 1, row_bytes, to_file);
    if (status != row_bytes) { perror (from_filename); exit(ABORT); }
    total_bytes += status;
  }
  if (verbose) fprintf(stderr,"> wrote %d bytes\n", total_bytes);

}
