/*======================================================================
 * matrix_io.h - header file for matrix_io.c:
 *   programs to read and write matrix data
 * 
 *$Log: not supported by cvs2svn $
 *
 *======================================================================*/
#include <stdio.h>
#include <define.h>

static const char matrix_io_h_RCSID[]="$Header: /tmp_mnt/FILES/mapx/matrix_io.h,v 1.1 1997-02-27 21:55:27 brodzik Exp $";

size_t read_matrix_data (void **data, char *file_name, 
			 int rows, int cols, size_t size);
size_t write_matrix_data (char *file_name, void **at, 
			  int rows, int cols, size_t size);
