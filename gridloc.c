/*========================================================================
 * gridloc - create grids of lat, lon locations
 *
 * 20-Sep-1995 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *========================================================================*/
static const char gridloc_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/gridloc.c,v 1.3 1996-05-28 17:43:41 knowles Exp $";

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "mapx.h"
#include "grids.h"
#include "byteswap.h"

#define usage \
"usage: gridloc [-s -o output_name -p -m -q] file.gpd\n"\
"\n"\
" input : file.gpd  - grid parameters definition file\n"\
"\n"\
" output: grid of signed decimal latitudes and/or longitudes\n"\
"         4 byte integers * 100000 by row\n"\
"\n"\
" option: s - swap bytes\n"\
"	  o - write data to file output_name.WIDTHxHEIGHTxNBANDS.int4\n"\
"             otherwise output goes to stdout\n"\
"         p - do latitudes only\n"\
"         m - do longitudes only\n"\
"         pm - do latitudes followed by longitudes\n"\
"         mp - do longitudes followed by latitudes (default)\n"\
"         q - quiet\n"\
"\n"

#define SCALE_FACTOR 100000
#define UNDEFINED 'U'

main (int argc, char *argv[])
{
  register int i, j, k;
  int band[2], nbands;
  int nbytes, row_bytes, status, total_bytes;
  bool swap_bytes, verbose;
  float datum[2];
  int4 *data;
  char *option, *output_name, output_filename[MAX_STRING];
  static char *datum_name[2] = {"latitude", "longitude"};
  FILE *output_file;
  grid_class *grid_def;

/*
 *	set defaults
 */
  swap_bytes = FALSE;
  nbands = 0;
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
	case 's':
	  swap_bytes = TRUE;
	  break;
	case 'm':
	  if (nbands < 2)
	  { band[nbands] = 1;
	    nbands++;
	  }
	  break;
	case 'p':
	  if (nbands < 2)
	  { band[nbands] = 0;
	    nbands++;
	  }
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

  if (0 == nbands)
  { band[0] = 1;
    band[1] = 0;
    nbands = 2;
  }

/*
 *	get command line arguments
 */
  if (argc != 1) error_exit(usage);
  
  grid_def = init_grid(*argv);
  if (NULL == grid_def) exit(ABORT);
  if (verbose) fprintf(stderr,"> using %s...\n", grid_def->gpd_filename);

  if (NULL != output_name)
  { sprintf(output_filename, "%s.%dx%dx%d.int4", 
	    output_name, grid_def->cols, grid_def->rows, nbands);
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
  data = (int4 *)calloc(grid_def->cols, sizeof(int4));
  if (NULL == data) { perror("data"); exit(ABORT); }
  row_bytes = grid_def->cols * sizeof(int4);
  total_bytes = 0;

/*
 *	write data 
 */
  for (k = 0; k < nbands; k++)
  { if (verbose) fprintf(stderr,"> writing %s data...\n", datum_name[band[k]]);
    for (i = 0; i < grid_def->rows; i++) 
    { memset(data, UNDEFINED, row_bytes);
      for (j = 0; j < grid_def->cols; j++)
      { status = inverse_grid(grid_def, (float)j, (float)i, 
			      &(datum[0]), &(datum[1]));
	if (!status) continue;
	data[j] = (int4)(datum[band[k]] * SCALE_FACTOR);
	if (swap_bytes) data[j] = SWAP4(data[j]);
      }
      nbytes = fwrite(data, 1, row_bytes, output_file);
      if (nbytes != row_bytes) { perror (output_filename); exit(ABORT); }
      total_bytes += nbytes;
    }
  }
  if (verbose) fprintf(stderr,"> wrote %d bytes to %s\n", 
		       total_bytes, output_filename);

}
