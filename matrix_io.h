/*======================================================================
 * matrix_io.h - header file for matrix_io.c:
 *   programs to read and write matrix data
 * 
 * 03/7/1997 brodzik brodzik@zamboni.colorado.edu 303-492-8263
 *
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *
 *$Log: not supported by cvs2svn $
 * Revision 1.2  1997/03/07  19:42:09  brodzik
 * Took _data off names.  Reran unit test.
 *
 * Revision 1.1  1997/02/27  21:55:27  brodzik
 * Initial revision
 *
 *
 *======================================================================*/
#ifndef matrix_io_h_
#define matrix_io_h_

#include <define.h>

static const char matrix_io_h_RCSID[]="$Header: /tmp_mnt/FILES/mapx/matrix_io.h,v 1.3 1997-03-22 22:54:20 brodzik Exp $";

size_t read_matrix (void **data, char *file_name, 
		    int rows, int cols, size_t size);
size_t write_matrix (char *file_name, void **data, 
		     int rows, int cols, size_t size);

#endif
