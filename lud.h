/*======================================================================
 * lud - lower, upper, diagonal matrix factoring
 *
 * 2-Aug-1990 K.Knowles knowlesk@kryos.colorado.edu 303-492-0644
 * National Snow & Ice Data Center, University of Colorado, Boulder
 * Copyright (C) 1990 University of Colorado
 *======================================================================*/
#ifndef lud_h_
#define lud_h_

static const char lud_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/lud.h,v 1.3 2004-01-19 00:57:22 knowlesk Exp $";

int design(double **x, double *y, double **A, double *z, int m, int n);

int factor(double **A, int n);

int solve(double **A, double *z, int n);

#endif
