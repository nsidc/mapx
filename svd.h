/*======================================================================
 * svd - singular value decomposition
 *
 * 2-Aug-1990 K.Knowles knowlesk@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *======================================================================*/
#ifndef svd_h_
#define svd_h_

static const char svd_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/svd.h,v 1.3 2004-01-19 00:57:22 knowlesk Exp $";

int svdecomp(double **u, int m, int n, double *w, double **v);
int svdsolve(double **u, double *w, double **v, int m, int n, 
	     double *b, double *x);

#endif
