/*======================================================================
 * keyval - "keyword: value" decoder
 *
 * 23-Oct-1996 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *======================================================================*/
#ifndef keyval_h_
#define keyval_h_

static const char keyval_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/keyval.h,v 1.3 1996-10-25 21:46:57 knowles Exp $";

#include "define.h"

static const char *keyval_LATITUDE = "NSns";
static const char *keyval_LONGITUDE = "EWew";

char *get_label_keyval(const char *filename, FILE *fp, int label_length);

char *get_field_keyval(const char *label, const char *keyword, 
		       const char *default_string);

bool get_value_keyval(const char *label, const char *keyword, 
		      const char *format, void *value, 
		      const char *default_string);

int boolean_keyval(const char *field_ptr, bool *value);

int lat_lon_keyval(const char *field_ptr, const char *designators, 
		   float *value);

#endif
