/*========================================================================
 * cdb_edit - decimate .cdb files sensibly
 *
 * assumes machine byte order is most significant byte first
 * for least significant byte first machines compile with -DLSB1ST
 *
 * 7-Apr-1993 R.Swick swick@krusty.colorado.edu  303-492-6069
 *========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include "define.h"
#include "cdb.h"
#include "mapx.h"
#include "cdb_byteswap.h"


/*------------------------------------------------------------------------
 * globals
 *------------------------------------------------------------------------*/

static char *new_filename;
static FILE *new_file;
static mapx_class *map;
static cdb_file_header header;
static cdb_index_entry *seg_index = NULL;
static cdb_seg_data *data = NULL;
static int seg_count = 0;
static int npoints = 0;
static int max_seg_size = 0;
static int max_segment_rank = 0;
static int ilat_extent = 0, ilon_extent = 0;
static int max_index_entries = 0;
static int max_data_points = 0;
static int verbose = FALSE;
static int very_verbose = FALSE;
static int very_very_verbose = FALSE;
static float thin = 0.01;
static char mpp_filename[] = "cdb_edit.mpp";

/*------------------------------------------------------------------------
 * function prototypes
 *------------------------------------------------------------------------*/
static int move_pu(float lat, float lon);
static int draw_pd(float lat, float lon);
static void write_segment_data(int);
static int parallels_min(cdb_index_entry *, cdb_index_entry *);
static int meridians_min(cdb_index_entry *, cdb_index_entry *);
static int parallels_max(cdb_index_entry *, cdb_index_entry *);
static int meridians_max(cdb_index_entry *, cdb_index_entry *);
static void thin_segment(cdb_class *source);
static void copy_segment(cdb_class *source);
static void decimate_map(char *cdb_filename,  
		 float lat_min, float lat_max, 
		 float lon_min, float lon_max);

/*------------------------------------------------------------------------
 * usage
 *------------------------------------------------------------------------*/
#define usage "\n"\
 "usage: cdb_edit [-tnsewpqlmhv] new_cdb_file source_cdb_file ...\n"\
 "\n"\
 " input : source_cdb_file - file(s) to edit (may be more than one)\n"\
 "\n"\
 " output: new_cdb_file - edit applied to source(s)\n"\
 "\n"\
 " option: t thin - thin strokes to a maximum error of t kilometers.\n"\
 "                (default = 0.01 kilometer = no thinning)\n"\
 "         n north - northern bound (default 90)\n"\
 "         s south - southern bound (default -90)\n"\
 "         e east - eastern bound (default 180)\n"\
 "         w west - western bound (default -180)\n"\
 "         p parallels_min - sort index by lat_min (cancels -m, -q, -l)\n"\
 "         q parallels_max - sort index by lat_max (cancels -m, -l, -p)\n"\
 "         l meridians_min - sort index by lon_min (cancels -p, -q, -m)\n"\
 "         m meridians_max - sort index by lon_max (cancels -p, -q, -l)\n"\
 "         h label - specify header label text (31 chars max)\n"\
 "         v - verbose diagnostic messages (may be repeated)\n"\
 "\n"


/*------------------------------------------------------------------------
 * cdb_edit - decimate .cdb files sensibly
 *
 *	input : "cdb" formatted files
 *
 *	output: decimated and concatenated "cdb" formatted file
 *
 *----------------------------------------------------------------------*/
main(int argc, char *argv[])
{ 
  register int i, ios, extent;
  char *option, label[32] = "cdb_edit";
  float north = 90.0, south = -90.0, east = 180.0, west = -180.0;
  int do_sort = FALSE;
  int (*compare)();
  
/*
 *	get command line options
 */
  while (--argc > 0 && (*++argv)[0] == '-')
  { 
    for (option = argv[0]+1; *option != '\0'; option++)
    { 
      switch (*option)
      { 
      case 't':
	argc--; argv++;
	if (argc <= 0 || sscanf(*argv,"%f", &thin) != 1) error_exit(usage);
	break;
      case 'n':
	argc--; argv++;
	if (argc <= 0 || sscanf(*argv,"%f", &north) != 1) error_exit(usage);
	break;
      case 's':
	argc--; argv++;
	if (argc <= 0 || sscanf(*argv,"%f", &south) != 1) error_exit(usage);
	break;
      case 'e':
	argc--; argv++;
	if (argc <= 0 || sscanf(*argv,"%f", &east) != 1) error_exit(usage);
	break;
      case 'w':
	argc--; argv++;
	if (argc <= 0 || sscanf(*argv,"%f", &west) != 1) error_exit(usage);
	break;
      case 'v':
	if (very_verbose) very_very_verbose = TRUE;
	if (verbose) very_verbose = TRUE;
	verbose = TRUE;
	break;
      case 'p':
	do_sort = TRUE;
	compare = parallels_min;
	break; 
      case 'q':
	do_sort = TRUE;
	compare = parallels_max;
	break;
      case 'l':
	do_sort = TRUE;
	compare = meridians_min;
	break;
      case 'm':
	do_sort = TRUE;
	compare = meridians_max;
	break;
      case 'h':
	argc--; argv++;
	strncpy(label, *argv, 31);
	break;
      default:
	fprintf(stderr, "invalid option %c\n", *option);
	error_exit(usage);
      }
    }
  }

/*
 *	get command line arguments
 */
  if (argc < 2) error_exit(usage);
  new_filename = strdup(*argv);
  argc--; argv++;
  if (verbose) 
    fprintf(stderr,">creating: %s\n>thin: %f km\n",
	    new_filename, thin);

/*
 *	open output file and reserve space for header
 */
  new_file = fopen(new_filename, "w");
  if (new_file == NULL) { perror(new_filename); error_exit(usage); }
  ios = fwrite(&header, 1, CDB_FILE_HEADER_SIZE, new_file);
  if (very_verbose) fprintf(stderr,">>wrote %d bytes for header.\n",ios);
  if (ios != CDB_FILE_HEADER_SIZE)
  { fprintf(stderr,"cdb_edit: error writing header.\n");
    perror(new_filename);
    exit(ABORT);
  }

/*
 *      initialize .mpp file
 */
  map = init_mapx(mpp_filename);
  if (NULL == map) 
  { fprintf(stderr,"cdb_edit: get a copy of %s, or set the environment\n"\
	    "          variable PATHMPP to the appropriate directory\n",
	    mpp_filename);
    exit(ABORT);
  }
  map->scale = thin/3;
  reinit_mapx(map);
  if(very_verbose) 
    fprintf(stderr, ">>initialized map\n"); 

/*
 *	record segment data and create index
 */

  for (; argc > 0; argc--, argv++)
  { if (verbose) fprintf(stderr,">processing %s...\n", *argv);
    decimate_map(*argv, south, north, west, east);
  }

/*
 *	flush segment data buffer
 */
  if (npoints > 0) write_segment_data(seg_count-1);

/*
 *	sort index
 */
  if (do_sort)
  { if (verbose) fprintf(stderr,">sorting %d index entries...\n", seg_count);
    qsort(seg_index, seg_count, sizeof(cdb_index_entry), compare);
  }

/*
 *	get maximum lat,lon extent 
 */

  for (i=0; i < seg_count; i++)
  { extent = seg_index[i].ilat_max - seg_index[i].ilat_min;
    if (ilat_extent < extent) ilat_extent = extent;
    extent = seg_index[i].ilon_max - seg_index[i].ilon_min;
    if (ilon_extent < extent) ilon_extent = extent;
  }

/*
 *	update header information
 */
  header.code_number = CDB_MAGIC_NUMBER;
  header.max_seg_size = max_seg_size;
  header.segment_rank = max_segment_rank;
  strcpy(header.text, label);
  header.index_addr = ftell(new_file);
  header.index_size = seg_count*sizeof(cdb_index_entry);
  header.index_order = (compare == parallels_min ? CDB_INDEX_LAT_MIN :
			compare == meridians_min ? CDB_INDEX_LON_MIN :
			compare == parallels_max ? CDB_INDEX_LAT_MAX :
			compare == meridians_max ? CDB_INDEX_LON_MAX :
			CDB_INDEX_SEG_ID);
  header.ilat_max = nint(north/CDB_LAT_SCALE);
  header.ilon_max = nint(east/CDB_LON_SCALE);
  header.ilat_min = nint(south/CDB_LAT_SCALE);
  header.ilon_min = nint(west/CDB_LON_SCALE);
  header.ilat_extent = ilat_extent;
  header.ilon_extent = ilon_extent;
  if (verbose) fprintf(stderr,">max segment size %d bytes.\n", max_seg_size);

/*
 *	output index
 */
  cdb_byteswap_index(seg_index, seg_count);
  ios = fwrite(seg_index, sizeof(cdb_index_entry), seg_count, new_file);
  if (verbose) fprintf(stderr,">wrote %d index entries.\n",ios);
  if (ios != seg_count)
  { fprintf(stderr,"cdb_edit: error writing index.\n");
    perror(new_filename);
    exit(ABORT);
  }

/*
 *	output header
 */
  cdb_byteswap_header(&header);
  fseek(new_file, 0L, SEEK_SET);
  ios = fwrite(&header, 1, CDB_FILE_HEADER_SIZE, new_file);
  if (verbose) fprintf(stderr,">wrote %d bytes of header.\n",ios);
  if (ios != CDB_FILE_HEADER_SIZE)
  { fprintf(stderr,"cdb_edit: error writing header.\n");
    perror(new_filename);
    exit(ABORT);
  }

  fclose(new_file);

}

/*------------------------------------------------------------------------
 * move_pu - move pen up function for decimate_map
 *------------------------------------------------------------------------*/
static int move_pu(float lat, float lon)
{
  float nlon;

/*
 *	write current segment
 */
  if (npoints > 0) write_segment_data(seg_count-1);

/*
 *	make sure index is big enough
 */
  if (seg_count >= max_index_entries)
  { max_index_entries += 1000;
    seg_index = (cdb_index_entry *)
      realloc(seg_index,sizeof(cdb_index_entry)*max_index_entries);
    assert(seg_index != NULL);
    if (verbose) fprintf(stderr,">allocating %d index entries.\n",
			 max_index_entries);
  }


/*
 *	start a new segment
 */
  seg_index[seg_count].ID = seg_count;
  seg_index[seg_count].ilat0 = nint(lat/CDB_LAT_SCALE);
  nlon = lon;
  NORMALIZE(nlon);
  seg_index[seg_count].ilon0 = nint(nlon/CDB_LON_SCALE);
  seg_index[seg_count].ilat_max = seg_index[seg_count].ilat0;
  seg_index[seg_count].ilon_max = seg_index[seg_count].ilon0;
  seg_index[seg_count].ilat_min = seg_index[seg_count].ilat0;
  seg_index[seg_count].ilon_min = seg_index[seg_count].ilon0;

/*
 * recenter map
 */

  map->center_lat = seg_index[seg_count].ilat0 * CDB_LAT_SCALE;
  map->center_lon = seg_index[seg_count].ilon0 * CDB_LON_SCALE;
  map->lat0 = seg_index[seg_count].ilat0 * CDB_LAT_SCALE;
  map->lon0 = seg_index[seg_count].ilon0 * CDB_LON_SCALE;
  reinit_mapx(map);
  ++seg_count;
  npoints = 0;

  return 0;
}

/*------------------------------------------------------------------------
 * draw_pd - draw pen down function for decimate_map
 *------------------------------------------------------------------------*/
static int draw_pd(float lat, float lon)
{
  register int ilat, ilon;
  static float lat1,lon1;
  auto float lat3,lon3;

/*
 *	make sure segment is big enough
 */
  if (npoints >= max_data_points)
  { max_data_points += 1000;
    data = (cdb_seg_data *)
      realloc(data,sizeof(cdb_seg_data)*max_data_points);
    assert(data != NULL);
    if (verbose) fprintf(stderr,">allocating %d data points.\n",
			 max_data_points);
  }

/*
 *	check for start of new segment
 */
  if (npoints == 0)
  {
    lat1 = seg_index[seg_count-1].ilat0 * CDB_LAT_SCALE;
    lon1 = seg_index[seg_count-1].ilon0 * CDB_LON_SCALE;
    if (very_verbose)fprintf(stderr,">>new segment: %f %f.\n",lat1,lon1);
    if (very_very_verbose)fprintf(stderr,">>> recentered map to %f %f.\n",
				  lat1, lon1);
  }

/*
 *	add point lat3,lon3 to segment
 */

  lat3 = lat;
  lon3 = lon;
  if(lon3>180.0) lon3 + 180.0;
  if(lon3<-180.0) lon3 = -180.0;
  lat1 = lat3-lat1;
  lon1 = lon3-lon1;
  lat1 = lat1/CDB_LAT_SCALE;
  lon1 = lon1/CDB_LON_SCALE;

  data[npoints].dlat = nint(lat1);
  data[npoints].dlon = nint(lon1);
  
  ilat = nint(lat3/CDB_LAT_SCALE);
  ilon = nint(lon3/CDB_LON_SCALE);
   
  lat1 = lat3;
  lon1 = lon3;
  ++npoints;
  
  if (max_seg_size < npoints*sizeof(cdb_seg_data))
    max_seg_size = npoints*sizeof(cdb_seg_data);
  if (very_very_verbose)fprintf(stderr,">>>add point %f %f.\n",lat3,lon3);
  
  /*
   *	update index entry
   */
  if (seg_index[seg_count-1].ilat_max < ilat)
    seg_index[seg_count-1].ilat_max = ilat;
  if (seg_index[seg_count-1].ilon_max < ilon)
    seg_index[seg_count-1].ilon_max = ilon;
  if (seg_index[seg_count-1].ilat_min > ilat)
    seg_index[seg_count-1].ilat_min = ilat;
  if (seg_index[seg_count-1].ilon_min > ilon)
    seg_index[seg_count-1].ilon_min = ilon;

/*
 * recenter map
 */
  map->center_lat = lat1;
  map->center_lon = lon1;
  map->lat0 = lat1;
  map->lon0 = lon1;
  reinit_mapx(map);
  if (very_very_verbose)fprintf(stderr,">>> recentered map to %f %f.\n",
				lat1, lon1);

  return 0;
}

/*------------------------------------------------------------------------
 * write_segment_data - and get segment address and size
 *------------------------------------------------------------------------*/
static void write_segment_data(int seg)
{
  register int ios;

  seg_index[seg].addr = ftell(new_file);
  seg_index[seg].size = npoints*sizeof(cdb_seg_data);
  cdb_byteswap_data_buffer(data, npoints);
  ios = fwrite(data, sizeof(cdb_seg_data), npoints, new_file);
  if (very_verbose) fprintf(stderr,">>wrote %d points of segment %d.\n",
	 ios+1, seg_index[seg].ID);
  if (ios != npoints)
  { fprintf(stderr,"cdb_edit: error writing data segment %d.\n",
     seg_index[seg].ID);
    perror(new_filename);
    exit(ABORT);
  }
}

/*------------------------------------------------------------------------
 * parallels - compare functions for qsort.
 *             parallels_min -> increasing lat_min
 *             parallels_max -> decreasing lat_max
 *------------------------------------------------------------------------*/
static int parallels_min(cdb_index_entry *seg1, cdb_index_entry *seg2)
{
  return (seg1->ilat_min - seg2->ilat_min);
}

static int parallels_max(cdb_index_entry *seg1, cdb_index_entry *seg2)
{
  return (seg2->ilat_max - seg1->ilat_max);
}


/*------------------------------------------------------------------------
 * meridians - compare functions for qsort. 
 *             meridians_min -> increasing lon_min
 *             meridians_max -> decreasing lon_max
 *------------------------------------------------------------------------*/

static int meridians_min(cdb_index_entry *seg1, cdb_index_entry *seg2)
{
  return (seg1->ilon_min - seg2->ilon_min);
}

static int meridians_max(cdb_index_entry *seg1, cdb_index_entry *seg2)
{
  return (seg2->ilon_max - seg1->ilon_max);
}


/*----------------------------------------------------------------------
 *    decimate_map  map decimation routine
 *
 *	input : cdb_file - cdb map file to be thinned
 *              lat_min...lon_max - furthest bounds of map, should be
 *		  outside of any clipping window you have defined
 *		  (note: no clipping is done by this routine, it just
 *		  ignores any segments outside of these bounds)
 *
 *	result:  0 = normal completion
 *		-1 = some file error
 *		-2 = some memory allocation error
 *
 *	you must define the following routines:
 *
 *	move_pu (float *lat, float *lon)
 *	draw_pd (float *lat, float *lon)
 *
 *	each takes as input lat, lon in decimal degrees
 *	move_pu is called once at the start of each segment, it can be 
 *	used to move the current plotter position (pen up) to the plotter 
 *	coordinates corresponding to lat,lon, it could also be used to 
 *	initialize a data structure or start a new record in a file 
 *	draw_pd is called for each stroke in the segment, it can be used
 *	to draw (pen down) from the current position to lat,lon, or add 
 *	the point to a data structure or file
 *	
 *----------------------------------------------------------------------*/
static void decimate_map(char *cdb_filename,  
		     float lat_min, float lat_max, 
		     float lon_min, float lon_max)
{
  cdb_class *source;
  int map_stradles_180;
  int iseg;
  
/*
 *	check for special case where map stradles 180 degrees
 */
  if (lon_max < lon_min)
  { lon_max += 360;
    map_stradles_180 = TRUE;
  }
  else if (lon_min < -180)
  { lon_min += 360;
    lon_max += 360;
    map_stradles_180 = TRUE;
  }
  else if (lon_max > 180)
  { map_stradles_180 = TRUE;
  }
  else
  { map_stradles_180 = FALSE;
  }
    
/*
 *	open map file
 */
    
  source = init_cdb(cdb_filename);
  if(source == NULL) { perror(cdb_filename); error_exit(usage); }
 
  if (source->header->segment_rank > max_segment_rank)
    max_segment_rank = source->header->segment_rank;
/*
 *	step thru segment dictionary entries
 */

  for (iseg = 0, reset_current_seg_cdb(source); 
       iseg < source->seg_count; iseg++, next_segment_cdb(source))
  {
    
/*
 *	  look at each segment and decide if we're
 *	  going to keep it or skip to the next one
 */ 
    if (source->segment->ilat_min*CDB_LAT_SCALE > lat_max) continue;
    if (source->segment->ilat_max*CDB_LAT_SCALE < lat_min) continue;
    if (map_stradles_180)
    { if (source->segment->ilon_min < 0)  
	source->segment->ilon_min += 360/CDB_LON_SCALE;
      if (source->segment->ilon_max < 0)  
	source->segment->ilon_max += 360/CDB_LON_SCALE;
    }
    if (source->segment->ilon_min*CDB_LON_SCALE > lon_max) continue;
    if (source->segment->ilon_max*CDB_LON_SCALE < lon_min) continue;
      
/*
 *    read and draw the segment
 */
    load_current_seg_data_cdb(source);
    if(thin < 0.09)
      copy_segment(source);
    else
      thin_segment(source);
  }
 list_cdb(source, very_very_verbose); 
}
  
/*------------------------------------------------------------------------
 * thin_segment - check each stroke of the current segment
 *------------------------------------------------------------------------*/ 

static void thin_segment(cdb_class *source)
{
  int idata, ipoints;
  double lat = 0.0, lon = 0.0;
  float x1, x2, x3, y1, y2, y3;
  int next_point_ok = FALSE, inside = TRUE;
  lat = (double)source->segment->ilat0*CDB_LAT_SCALE;
  lon = (double)source->segment->ilon0*CDB_LON_SCALE;
  
  move_pu(lat, lon);
  forward_mapx(map, lat, lon, &x1, &y1);
  x1 = nint(x1);
  y1 = nint(y1);
  x2 = x1;
  y2 = y1;
 
  for (idata = 0, ipoints = 1; idata < source->npoints; 
       idata++, source->data_ptr++)
  {
    lat += (double)source->data_ptr->dlat*CDB_LAT_SCALE;
    lon += (double)source->data_ptr->dlon*CDB_LON_SCALE;
    forward_mapx(map, lat, lon, &x3, &y3);
    x3 = nint(x3);
    y3 = nint(y3);
    next_point_ok = FALSE;
    
/*
 *    see if current point is too far away
 */    
    if(fabs(x1-x3) >= 2 || fabs(y1-y3) >= 2)
    {
      if(y1 != y2 && y1 != y3 && (x1-x3)/(y1-y3) == (x1-x2)/(y1-y2))
	next_point_ok = TRUE;
      if(x1 != x2 && x1 != x3 && (y1-y3)/(x1-x3) == (y1-y2)/(x1-x2))
	next_point_ok = TRUE;
      
      if (inside)
      {
	inside = FALSE;
	next_point_ok = TRUE;
      }
    }
    else { next_point_ok = TRUE; }
    
    if(next_point_ok)
    {
      x2 = x3;
      y2 = y3;
    }
    
    else
    { lat -= (double)source->data_ptr->dlat*CDB_LAT_SCALE;
      lon -= (double)source->data_ptr->dlon*CDB_LON_SCALE;
      draw_pd(lat, lon);
      x1 = x2;
      y1 = y2;
      source->data_ptr--;
      idata--;
      ipoints++;
      inside = TRUE;
      
    }  /* end else    */
    
  }  /* end for.... */

  draw_pd(lat, lon);
  next_point_ok = FALSE;
  ipoints++;
  if (very_verbose) 
    { fprintf(stderr,">>segment was %d points.\n",
	      source->npoints); 
      fprintf(stderr,">>segment is now %d points.\n",
	      ipoints); 
    }
  
 
}
  
/*------------------------------------------------------------------------
 * copy_segment - copy each stroke of the current segment
 *------------------------------------------------------------------------*/ 


static void copy_segment(cdb_class *source)
{
  int idata;
  double lat = 0.0, lon = 0.0;
  
  lat = (double)source->segment->ilat0*CDB_LAT_SCALE;
  lon = (double)source->segment->ilon0*CDB_LON_SCALE;
  
  move_pu(lat, lon);
   
  for (idata = 0; idata <= source->npoints; 
       idata++, source->data_ptr++)
  {
    lat += (double)source->data_ptr->dlat*CDB_LAT_SCALE;
    lon += (double)source->data_ptr->dlon*CDB_LON_SCALE;
    draw_pd(lat, lon);
  }
}
