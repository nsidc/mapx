/*========================================================================
 * coastlines data base decimation - decimate .cdb files sensibly
 *
 *      7-Apr-1993 R.Swick swick@krusty.colorado.edu  303-
 *========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <define.h>
#include <cdb.h>
#include <assert.h>
#include <mapx.h>
#include <cdb_edit.h>
#include <errno.h>
#include <ctype.h>


/*------------------------------------------------------------------------
 * globals
 *------------------------------------------------------------------------*/

static char *cdb_filename;
static FILE *cdb_file;
static mapx_class *map;
static cdb_file_header header;
static cdb_index_entry *seg_index = NULL;
static cdb_seg_data *data = NULL;
static int seg_count = 0;
static int npoints = 0;
static int max_seg_size = 0;
static int ilat_extent = 0, ilon_extent = 0;
static int max_index_entries = 0;
static int max_data_points = 0;
static int verbose = FALSE;
static int very_verbose = FALSE;
static int very_very_verbose = FALSE;
static float thin = 0.01;

/*------------------------------------------------------------------------
 * function prototypes
 *------------------------------------------------------------------------*/
static void move_pu_dec(float lat, float lon);
static void draw_pd_dec(float lat, float lon);
static void write_segment_data_dec(int);
static int parallels_min_dec(cdb_index_entry *, cdb_index_entry *);
static int meridians_min_dec(cdb_index_entry *, cdb_index_entry *);
static int parallels_max_dec(cdb_index_entry *, cdb_index_entry *);
static int meridians_max_dec(cdb_index_entry *, cdb_index_entry *);
static void thin_segment_dec(cdb_class *this);
static void copy_segment_dec(cdb_class *this);
static void decimate_map_dec(char *cdb_filename,  
		 float lat_min, float lat_max, 
		 float lon_min, float lon_max);

/*------------------------------------------------------------------------
 * coastlines data base decimation - decimate .cdb files sensibly
 *
 *	input : "cdb" formatted (byte swapped) file created by cdbx
 *                   cil, riv, and bdy files in decimate.h
 *
 *	output: decimated "cdb" formatted file
 *
 *	option: t thin - thin strokes to a maximum error of t kilometers.
 *                  (default = 0.01 kilometer = no thinning)
 *		d detail - only use segments with rank <= detail (default 1)
 *                  (least detail = 1,  most detail = 5)
 *		n north - northern bound (default 90)
 *		s south - southern bound (default -90)
 *		e east - eastern bound (default 180)
 *		w west - western bound (default -180)
 *		p parallels_min - sort index by lat_min (cancels -m, -q, -l)
 *		q parallels_max - sort index by lat_max (cancels -m, -l, -p)
 *		l meridians_min - sort index by lon_min (cancels -p, -q, -m)
 *		m meridians_max - sort index by lon_max (cancels -p, -q, -l)
 *                  (default is meridians_min)
 *		c - coastlines, islands, and lakes (default)
 *		b - boundaries
 *		r - rivers      (c, b, and r are NOT mutually exclusive)
 *              v - verbose diagnostic messages
 *	        vv - very verbose
 *		vvv - very very verbose
 *
 *
 *----------------------------------------------------------------------*/

#define usage \
 "usage: cdb_edit [-tdnsewpqlmcbrv] new_cdb_filename\n"\
 " option: t thin - thin strokes to a maximum error of t kilometers.\n"\
 "                (default = 0.01 kilometer = no thinning)\n"\
 "         d detail - only use segments with rank <= detail\n"\
 "                 (least detail = 1,  most detail = 5, default = 1)\n"\
 "         n north - northern bound (default 90)\n"\
 "         s south - southern bound (default -90)\n"\
 "         e east - eastern bound (default 180)\n"\
 "         w west - western bound (default -180)\n"\
 "         p parallels_min - sort index by lat_min (cancels -m, -q, -l)\n"\
 "         q parallels_max - sort index by lat_max (cancels -m, -l, -p)\n"\
 "         l meridians_min - sort index by lon_min (cancels -p, -q, -m)\n"\
 "         m meridians_max - sort index by lon_max (cancels -p, -q, -l)\n"\
 "                 (default is meridians_min)\n"\
 "         c - coastlines, islands, and lakes (default)\n"\
 "         b - boundaries\n"\
 "         r - rivers      (c, b, and r are NOT mutually exclusive)\n"\
 "         v - verbose diagnostic messages\n"\
 "         vv - very verbose\n"\
 "         vvv - very very verbose\n"


main(int argc, char *argv[])
{ 
  register int i, ios, extent;
  char *option;
  int detail = 1;
  float north = 90.0, south = -90.0, east = 180.0, west = -180.0;
  int sort_by_lat = FALSE, sort_by_lon = FALSE;
  int do_cil = FALSE, do_bdy = FALSE, do_riv = FALSE;
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
      case 'd':
	argc--; argv++;
	if (argc <= 0 || sscanf(*argv,"%d", &detail) != 1) error_exit(usage);
	break;
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
	sort_by_lat = TRUE;
	compare = parallels_min_dec;
	sort_by_lon = FALSE;
	break; 
      case 'q':
	sort_by_lat = TRUE;
	compare = parallels_max_dec;
	sort_by_lon = FALSE;
	break;
      case 'l':
	sort_by_lon = TRUE;
	compare = meridians_min_dec;
	sort_by_lat = FALSE;
	break;
      case 'm':
	sort_by_lon = TRUE;
	compare = meridians_max_dec;
	sort_by_lat = FALSE;
	break;
      case 'c':
	do_cil = TRUE;
	break;
      case 'b':
	do_bdy = TRUE;
	break;
      case 'r':
	do_riv = TRUE;
	break;
      default:
	fprintf(stderr, "invalid option %c\n", *option);
	error_exit(usage);
      }
    }
  }

  if (!do_cil && !do_bdy && !do_riv) do_cil = TRUE;

/*
 *	get command line arguments
 */
  if (argc != 1) error_exit(usage);
  cdb_filename = *argv;
  if (verbose && thin < 0.09) 
    fprintf(stderr, ">filename: %s\n>thin: none\n>detail: %d\n",
	    cdb_filename, detail);
  else if (verbose) 
    fprintf(stderr,">filename: %s\n>thin: %f Kilometer(s)\n>detail: %d\n",
	    cdb_filename, thin, detail);

/*
 *	open output file and reserve space for header
 */
  cdb_file = fopen(cdb_filename, "w");
  if (cdb_file == NULL) { perror(cdb_filename); error_exit(usage); }
  ios = fwrite(&header, 1, CDB_FILE_HEADER_SIZE, cdb_file);
  if (very_verbose) fprintf(stderr,">>wrote %d bytes for header.\n",ios);
  if (ios != CDB_FILE_HEADER_SIZE)
  { fprintf(stderr,"cdb_edit: error writing header.\n");
    perror(cdb_filename);
    exit(ABORT);
  }

/*
 *      initialize .mpp file
 */
  map = init_mapx(MPP_FILENAME);
  if (NULL == map) 
  { fprintf(stderr,"cdb_edit: get a copy of %s, or set the environment\n"\
	    "          variable PATHMPP to the appropriate directory\n",
	    MPP_FILENAME);
    exit(ABORT);
  }
  map->scale = thin/3;
  reinit_mapx(map);
  if(very_verbose) 
    fprintf(stderr, ">>initialized map\n"); 

/*
 *	record segment data and create index
 */

  if (do_cil)
    { if (verbose) fprintf(stderr,">processing %s...\n", 
			   cil_filename[detail-1]);
      decimate_map_dec(cil_filename[detail-1], south, north, west, east);
    }
  
  if (do_bdy)
    { if (verbose) fprintf(stderr,">processing %s...\n", 
			   bdy_filename[detail-1]);
      decimate_map_dec(bdy_filename[detail-1], south, north, west, east);
    }
  
  if (do_riv)
    { if (verbose) fprintf(stderr,">processing %s...\n", 
			   riv_filename[detail-1]);
      decimate_map_dec(riv_filename[detail-1], south, north, west, east);
    }
 

/*
 *	flush segment data buffer
 */
  if (npoints > 0) write_segment_data_dec(seg_count-1);

/*
 *	sort index
 */
  if (sort_by_lat || sort_by_lon)
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
  header.code_number = (CDB_MAGIC_NUMBER);
  header.max_seg_size = (max_seg_size);
  header.segment_rank = (detail);
  sprintf(header.text, "WDBII (%d:1)", thin);
  if (do_cil) strcat(header.text," CIL");
  if (do_bdy) strcat(header.text," BDY");
  if (do_riv) strcat(header.text," RIV");
  header.index_addr = (ftell(cdb_file));
  header.index_size = (seg_count*sizeof(cdb_index_entry));
  header.index_order = (sort_by_lat ? CDB_INDEX_LAT_MIN 
			     : sort_by_lon ? CDB_INDEX_LON_MIN 
			     : CDB_INDEX_SEG_ID);
  header.ilat_max = (nint(north/CDB_LAT_SCALE));
  header.ilon_max = (nint(east/CDB_LON_SCALE));
  header.ilat_min = (nint(south/CDB_LAT_SCALE));
  header.ilon_min = (nint(west/CDB_LON_SCALE));
  header.ilat_extent = (ilat_extent);
  header.ilon_extent = (ilon_extent);
  if (verbose) fprintf(stderr,">max segment size %d bytes.\n", max_seg_size);



/*
 *	output index
 */
  ios = fwrite(seg_index, sizeof(cdb_index_entry), seg_count, cdb_file);
  if (verbose) fprintf(stderr,">wrote %d index entries.\n",ios);
  if (ios != seg_count)
  { fprintf(stderr,"cdb_edit: error writing index.\n");
    perror(cdb_filename);
    exit(ABORT);
  }

/*
 *	output header
 */
  fseek(cdb_file, 0L, SEEK_SET);
  ios = fwrite(&header, 1, CDB_FILE_HEADER_SIZE, cdb_file);
  if (verbose) fprintf(stderr,">wrote %d bytes of header.\n",ios);
  if (ios != CDB_FILE_HEADER_SIZE)
  { fprintf(stderr,"cdb_edit: error writing header.\n");
    perror(cdb_filename);
    exit(ABORT);
  }

  fclose(cdb_file);

}  /* END OF MAIN */



/*------------------------------------------------------------------------
 * move_pu_dec - move pen up function for decimate_map
 *------------------------------------------------------------------------*/
static void move_pu_dec(float lat, float lon)
{
  float nlon;

/*
 *	write current segment
 */
  if (npoints > 0) write_segment_data_dec(seg_count-1);

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

}




/*------------------------------------------------------------------------
 * draw_pd_dec - draw pen down function for decimate_map
 *------------------------------------------------------------------------*/
static void draw_pd_dec(float lat, float lon)
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
}


/*------------------------------------------------------------------------
 * write_segment_data_dec - and get segment address and size
 *------------------------------------------------------------------------*/
static void write_segment_data_dec(int seg)
{
  register int ios;

  seg_index[seg].addr = ftell(cdb_file);
  seg_index[seg].size = npoints*sizeof(cdb_seg_data);
  ios = fwrite(data, sizeof(cdb_seg_data), npoints, cdb_file);
  if (very_verbose) fprintf(stderr,">>wrote %d points of segment %d.\n",
	 ios+1, seg_index[seg].ID);
  if (ios != npoints)
  { fprintf(stderr,"cdb_edit: error writing data segment %d.\n",
     seg_index[seg].ID);
    perror(cdb_filename);
    exit(ABORT);
  }
}

/*------------------------------------------------------------------------
 * parallels - compare functions for qsort.
 *             parallels_min_dec -> increasing lat_min
 *             parallels_max_dec -> decreasing lat_max
 *------------------------------------------------------------------------*/
static int parallels_min_dec(cdb_index_entry *seg1, cdb_index_entry *seg2)
{
  return (seg1->ilat_min - seg2->ilat_min);
}

static int parallels_max_dec(cdb_index_entry *seg1, cdb_index_entry *seg2)
{
  return (seg2->ilat_max - seg1->ilat_max);
}


/*------------------------------------------------------------------------
 * meridians - compare functions for qsort. 
 *             meridians_min_dec -> increasing lon_min
 *             meridians_max_dec -> decreasing lon_max
 *------------------------------------------------------------------------*/

static int meridians_min_dec(cdb_index_entry *seg1, cdb_index_entry *seg2)
{
  return (seg1->ilon_min - seg2->ilon_min);
}

static int meridians_max_dec(cdb_index_entry *seg1, cdb_index_entry *seg2)
{
  return (seg2->ilon_max - seg1->ilon_max);
}


/*----------------------------------------------------------------------
 *    decimate_map_dec  map decimation routine
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
static void decimate_map_dec(char *cdb_filename,  
		     float lat_min, float lat_max, 
		     float lon_min, float lon_max)
{
  cdb_class *this;
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
    
  this = init_cdb(cdb_filename);
  if(this == NULL) { perror(cdb_filename); error_exit(usage); }
    
/*
 *	step thru segment dictionary entries
 */

  for (iseg = 0, reset_current_seg_cdb(this); 
       iseg < this->seg_count; iseg++, next_segment_cdb(this))
  {
    
/*
 *	  look at each segment and decide if we're
 *	  going to keep it or skip to the next one
 */ 
    if (this->segment->ilat_min*CDB_LAT_SCALE > lat_max) continue;
    if (this->segment->ilat_max*CDB_LAT_SCALE < lat_min) continue;
    if (map_stradles_180)
    { if (this->segment->ilon_min < 0)  
	this->segment->ilon_min += 360/CDB_LON_SCALE;
      if (this->segment->ilon_max < 0)  
	this->segment->ilon_max += 360/CDB_LON_SCALE;
    }
    if (this->segment->ilon_min*CDB_LON_SCALE > lon_max) continue;
    if (this->segment->ilon_max*CDB_LON_SCALE < lon_min) continue;
      
/*
 *    read and draw the segment
 */
    load_current_seg_data_cdb(this);
    if(thin < 0.09)
      copy_segment_dec(this);
    else
      thin_segment_dec(this);
  }
 list_cdb(this, very_very_verbose); 
}
  
/*------------------------------------------------------------------------
 * thin_segment_dec - check each stroke of the current segment
 *------------------------------------------------------------------------*/ 

static void thin_segment_dec(cdb_class *this)
{
  int idata, ipoints;
  double lat = 0.0, lon = 0.0;
  float x1, x2, x3, y1, y2, y3;
  int next_point_ok = FALSE, inside = TRUE;
  lat = (double)this->segment->ilat0*CDB_LAT_SCALE;
  lon = (double)this->segment->ilon0*CDB_LON_SCALE;
  
  move_pu_dec(lat, lon);
  forward_mapx(map, lat, lon, &x1, &y1);
  x1 = nint(x1);
  y1 = nint(y1);
  x2 = x1;
  y2 = y1;
 
  for (idata = 0, ipoints = 1; idata < this->npoints; 
       idata++, this->data_ptr++)
  {
    lat += (double)this->data_ptr->dlat*CDB_LAT_SCALE;
    lon += (double)this->data_ptr->dlon*CDB_LON_SCALE;
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
    { lat -= (double)this->data_ptr->dlat*CDB_LAT_SCALE;
      lon -= (double)this->data_ptr->dlon*CDB_LON_SCALE;
      draw_pd_dec(lat, lon);
      x1 = x2;
      y1 = y2;
      this->data_ptr--;
      idata--;
      ipoints++;
      inside = TRUE;
      
    }  /* end else    */
    
  }  /* end for.... */

  draw_pd_dec(lat, lon);
  next_point_ok = FALSE;
  ipoints++;
  if (very_verbose) 
    { fprintf(stderr,">>segment was %d points.\n",
	      this->npoints); 
      fprintf(stderr,">>segment is now %d points.\n",
	      ipoints); 
    }
  
 
}
  
/*------------------------------------------------------------------------
 * copy_segment_dec - copy each stroke of the current segment
 *------------------------------------------------------------------------*/ 


static void copy_segment_dec(cdb_class *this)
{
  int idata;
  double lat = 0.0, lon = 0.0;
  
  lat = (double)this->segment->ilat0*CDB_LAT_SCALE;
  lon = (double)this->segment->ilon0*CDB_LON_SCALE;
  
  move_pu_dec(lat, lon);
   
  for (idata = 0; idata <= this->npoints; 
       idata++, this->data_ptr++)
  {
    lat += (double)this->data_ptr->dlat*CDB_LAT_SCALE;
    lon += (double)this->data_ptr->dlon*CDB_LON_SCALE;
    draw_pd_dec(lat, lon);
  }
}
