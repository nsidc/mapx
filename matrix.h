/*========================================================================
 * matrix - allocate 2-D matrix
 *
 * 13-Jan-1993 K.Knowles knowles@sastrugi.colorado.edu 303-492-0644
 *========================================================================*/
#ifndef matrix_h_
#define matrix_h_

static const char matrix_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/matrix.h,v 1.2 1993-11-03 16:43:08 knowles Exp $";

void **matrix(int rows, int cols, int bytes, int zero);

#endif
