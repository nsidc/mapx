/*========================================================================
 * mapenum - enumerate map feature vectors
 *
 * 4-Mar-1993 K.Knowles knowles@sastrugi.colorado.edu 303-492-0644
 *========================================================================*/
static const char mapenum_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/mapenum.c,v 1.2 1993-11-08 17:20:47 knowles Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <define.h>
#include <maps.h>
#include <grids.h>
#include <cdb.h>

#define usage "\n"\
  "usage: mapenum [-d cdb_file -s map_style -g grat_style] gpd_file\n"\
  "\n"\
  " input : gpd_file - grid parameters definition\n"\
  "\n"\
  " output: stdout - list of map feature vectors of the form:\n"\
  "                  style x1 y1 x2 y2\n"\
  "\n"\
  " option: d cdb_filename - specify coastline database\n"\
  "                          default is global.cdb\n"\
  "         s map_style - specify style (default 0)\n"\
  "         g grat_style - specify graticule style (default 1)\n"

#define CDB_DEFAULT "global.cdb"
#define MAP_STYLE_DEFAULT 0
#define GRAT_STYLE_DEFAULT 1

static grid_class *grid;
static cdb_class *cdb;
static int pen_style = MAP_STYLE_DEFAULT;
static int move_pu(float,float);
static int draw_pd(float,float);

/*------------------------------------------------------------------------
 * mapenum [-d cdb_file -s map_style -g grat_style] gpd_file
 *
 *	input : gpd_file - grid parameters definition
 *
 *	output: stdout - list of map feature vectors of the form:
 *			 style x1 y1 x2 y2
 *
 *	option: d cdb_file - specify alternative coastline database
 *				 default is global.cdb
 *		s map_style - specify style (default 0)
 *		g grat_style - specify graticule style (default 1)
 *
 *------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{ int map_style = MAP_STYLE_DEFAULT, grat_style = GRAT_STYLE_DEFAULT;
  char *option, *gpd_filename, *cdb_filename = NULL;

/*
 *	get command line options
 */
  while (--argc > 0 && (*++argv)[0] == '-')
  { for (option = argv[0]+1; *option != '\0'; option++)
    { switch (*option)
      { case 'd':
	  ++argv; --argc;
	  cdb_filename = strdup(*argv);
	  break;
	case 's':
	  ++argv; --argc;
	  if (sscanf(*argv, "%d", &map_style) != 1)
	  { map_style = MAP_STYLE_DEFAULT;
	  }
	  break;
	case 'g':
	  ++argv; --argc;
	  if (sscanf(*argv, "%d", &grat_style) != 1)
	  { grat_style = GRAT_STYLE_DEFAULT;
	  }
	  break;
	default:
	  fprintf(stderr, "invalid option %c\n", *option);
	  error_exit(usage);
      }
    }
  }

/*
 *	process command line arguments
 */
  if (argc < 1) error_exit(usage);
  gpd_filename = strdup(*argv);
  if (gpd_filename == NULL) error_exit(usage);

  grid = init_grid(gpd_filename);
  if (grid == NULL) error_exit("mapenum: error initializing grid");

  if (cdb_filename == NULL) cdb_filename = strdup(CDB_DEFAULT);
  cdb = init_cdb(cdb_filename);
  if (cdb == NULL) error_exit("mapenum: error openning coastline database");

  pen_style = map_style;
  draw_cdb(cdb, grid->mapx->west, grid->mapx->east, 
	   CDB_INDEX_LON_MIN, move_pu, draw_pd);

  pen_style = grat_style;
  draw_graticule(grid->mapx, move_pu, draw_pd, NULL);

}

static float pen_x1, pen_y1, pen_x2, pen_y2;

/*------------------------------------------------------------------------
 * move_pu
 *
 *	input : lat, lon
 *
 *------------------------------------------------------------------------*/
static int move_pu(float lat, float lon)
{
  (void) forward_grid(grid, lat, lon, &pen_x1, &pen_y1);
  return 0;
}

/*------------------------------------------------------------------------
 * draw_pd
 *
 *	input : lat, lon
 *
 *------------------------------------------------------------------------*/
static int draw_pd(float lat, float lon)
{ int on_grid;

  on_grid = forward_grid(grid, lat, lon, &pen_x2, &pen_y2);
  if (on_grid) 
  { printf("%d %f %f %f %f\n", pen_style, pen_x1, pen_y1, pen_x2, pen_y2);
  }
  pen_x1 = pen_x2;
  pen_y1 = pen_y2;

  return 0;
}
