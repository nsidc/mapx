/*========================================================================
 * gridloc - create grids of lat, lon locations
 *
 * 20-Sep-1995 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *========================================================================*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "mapx.h"
#include "grids.h"

#define usage \
"usage: gridloc [-q -o output_name] file.gpd\n"\
"\n"\
" input : file.gpd  - grid parameters definition file\n"\
"\n"\
" output: grid of signed decimal latitudes by row\n"\
"         followed by grid of longitudes \n"\
"         4 byte integers scaled by 100000\n"\
"\n"\
" option: o - write data to file output_name.WIDTHxHEIGHTx2.int4\n"\
"             otherwise output goes to stdout\n"\
"         q - quiet (default = verbose)\n"\
"\n"

#define LAT_SCALE 100000
#define LON_SCALE 100000
#define UNDEFINED 'U'

main (int argc, char *argv[])
{
  register int i, j;
  int nbytes, row_bytes, status, total_bytes, verbose;
  float lat, lon;
  int4 *data;
  char *option, *output_name, output_filename[MAX_STRING];
  FILE *output_file;
  grid_class *grid_def;

/*
 *	set defaults
 */
  verbose = TRUE;
  output_name = NULL;

/* 
 *	get command line options
 */
  while (--argc > 0 && (*++argv)[0] == '-')
  { for (option = argv[0]+1; *option != '\0'; option++)
    { switch (*option)
      { case 'q':
	  verbose = FALSE;
	  break;
	case 'o':
	  ++argv; --argc;
	  if (argc <= 0) error_exit(usage);
	  output_name = strdup(*argv);
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
  if (argc != 1) error_exit(usage);
  
  grid_def = init_grid(*argv);
  if (NULL == grid_def) exit(ABORT);
  if (verbose) fprintf(stderr,"> using %s...\n", grid_def->gpd_filename);

  if (NULL != output_name)
  { sprintf(output_filename, "%s.%dx%dx2.int4", 
	    output_name, grid_def->cols, grid_def->rows);
    output_file = fopen(output_filename, "w");
    if (NULL == output_file) { perror(output_filename); error_exit(usage); }
  }
  else
  { strcpy(output_filename, "stdout");
    output_file = stdout;
  }

/*
 *	allocate storage for data grids
 */
  if (verbose) fprintf(stderr,"> allocating data matrices...\n");

  data = (int4 *)calloc(grid_def->cols, sizeof(int4));
  if (NULL == data) { perror("data"); exit(ABORT); }
  row_bytes = grid_def->cols * sizeof(int4);
  total_bytes = 0;

/*
 *	latitudes
 */
  if (verbose) fprintf(stderr,"> writing latitudes...\n");
  for (i = 0; i < grid_def->rows; i++) 
  { memset(data, UNDEFINED, row_bytes);
    for (j = 0; j < grid_def->cols; j++)
    { status = inverse_grid(grid_def, (float)j, (float)i, &lat, &lon);
      if (!status) continue;
      data[j] = (int4)(lat * LAT_SCALE);
    }
    nbytes = fwrite(data, 1, row_bytes, output_file);
    if (nbytes != row_bytes) { perror (output_filename); exit(ABORT); }
    total_bytes += nbytes;
  }

/*
 *	longitudes
 */
  if (verbose) fprintf(stderr,"> writing longitudes...\n");
  for (i = 0; i < grid_def->rows; i++) 
  { memset(data, UNDEFINED, row_bytes);
    for (j = 0; j < grid_def->cols; j++)
    { status = inverse_grid(grid_def, (float)j, (float)i, &lat, &lon);
      if (!status) continue;
      data[j] = (int4)(lon * LON_SCALE);
    }
    nbytes = fwrite(data, 1, row_bytes, output_file);
    if (nbytes != row_bytes) { perror (output_filename); exit(ABORT); }
    total_bytes += nbytes;
  }

  if (verbose) fprintf(stderr,"> wrote %d bytes to %s\n", 
		       total_bytes, output_filename);

}
