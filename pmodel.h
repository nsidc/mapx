/*======================================================================
 * pmodel - approximation by polynomial model
 *
 * 2-Aug-1990 K.Knowles knowles@sastrugi.colorado.edu 303-492-0644
 *======================================================================*/
#ifndef pmodel_h_
#define pmodel_h_

static const char pmodel_h_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/pmodel.h,v 1.2 1993-08-02 10:30:26 knowles Exp $";

#define MAXVARS 64
#define MAXPOINTS 450

#define ipow(x,i) ((i) == 0 ? 1.0 : (x) == 0.0 ? 0.0 : pow((x), (double) (i)))

typedef struct {
	int dim;		/* dimension	*/
	int order;		/* size		*/
	int tcode;		/* shape	*/
	double coef[MAXVARS];	/* coefficients */
	} Polynomial;

double peval (Polynomial *P, double r, double s);

void pmatrix (Polynomial *P, int npts, double M[MAXPOINTS][MAXVARS],
	      double r_data[MAXPOINTS], double s_data[MAXPOINTS]);

void pmodel (Polynomial *P, int npts, double M[MAXPOINTS][MAXVARS],
	     double t_data[MAXPOINTS], char *method);

void pstats (double *SSE, double *R2, Polynomial *P, int npts, 
	     double t_data[MAXPOINTS], double r_data[MAXPOINTS], 
	     double s_data[MAXPOINTS]);

double chebyshev(int i, int n, double a, double b);

#endif
