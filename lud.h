/*======================================================================
 * lud - lower, upper, diagonal matrix factoring
 *
 * 2-Aug-1990 K.Knowles knowles@sastrugi.colorado.edu 303-492-0644
 *======================================================================*/
#ifndef lud_h_
#define lud_h_

static const char lud_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/lud.h,v 1.1 1993-11-03 16:27:25 knowles Exp $";

int design(double **x, double *y, double **A, double *z, int m, int n);

int factor(double **A, int n);

int solve(double **A, double *z, int n);

#endif
