/*========================================================================
 * matrix - allocate 2-D matrix
 *
 * 13-Jan-1993 K.Knowles knowlesk@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 * Copyright (C) 1993 University of Colorado
 *========================================================================*/
#ifndef matrix_h_
#define matrix_h_

#ifdef matrix_c_
const char matrix_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/matrix.h,v 1.7 2004-01-23 01:53:34 knowlesk Exp $";
#endif

#define matrix_ZERO 1

void **matrix(int rows, int cols, int bytes, int zero);

#endif
