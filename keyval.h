/*======================================================================
 * keyval - "keyword: value" decoder
 *
 * 23-Oct-1996 K.Knowles knowles@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *======================================================================*/
#ifndef keyval_h_
#define keyval_h_

static const char keyval_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/keyval.h,v 1.1 1996-10-24 20:14:46 knowles Exp $";

#include "define.h"

char *get_label_keyval(const char *filename, FILE *fp);

char *get_field_keyval(const char *label, const char *key, const char *default_string);

bool get_value_keyval(const char *label, const char *key, const char *format, 
		      void *value, const char *default_string);

int boolean_decode(const char *field_ptr, bool *value);

#endif
