/*======================================================================
 * svd - singular value decomposition
 *
 * 2-Aug-1990 K.Knowles knowlesk@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 * Copyright (C) 1990 University of Colorado
 *======================================================================*/
#ifndef svd_h_
#define svd_h_

const char svd_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/svd.h,v 1.4 2004-01-21 18:01:32 knowlesk Exp $";

int svdecomp(double **u, int m, int n, double *w, double **v);
int svdsolve(double **u, double *w, double **v, int m, int n, 
	     double *b, double *x);

#endif
