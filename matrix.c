/*========================================================================
 * matrix - allocate 2-D matrix
 *
 * 13-Jan-1993 K.Knowles knowles@sastrugi.colorado.edu 303-492-0644
 *========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

static const char matrix_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/matrix.c,v 1.2 1993-10-26 11:27:57 knowles Exp $";

/*------------------------------------------------------------------------
 * matrix - allocate 2-D matrix
 *
 *	input : rows - number of rows
 *		cols - number of columns
 *		bytes - number of bytes per entry
 *		zero - if set then zero fill entries
 *
 *	result: pointer to column of pointers to rows
 *
 *	note: assumes all pointers are the same size, sizeof(void *)
 *
 *----------------------------------------------------------------------*/
void **matrix(int rows, int cols, int bytes, int zero)
{
  void **matrix_ptr, **row_ptr;
  char *block;
  size_t row_size, row_ptr_size;
  register int irow, free_byte;

  row_size = cols*bytes;
  row_ptr_size = rows * sizeof(void *);
  block = malloc(row_ptr_size + rows*row_size);
  if (block == NULL) {perror("matrix"); return(NULL);}
  matrix_ptr = (void **) block;
  row_ptr = matrix_ptr;
  free_byte = row_ptr_size;
  for (irow=0; irow < rows; irow++)
  {
    *row_ptr = &(block[free_byte]);
    free_byte += row_size;
    if (zero) memset(*row_ptr,0,row_size);
    ++row_ptr;
  }
  return(matrix_ptr);
}
