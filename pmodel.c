/*======================================================================
 * pmodel - approximation by polynomial model
 *
 *	1 dimensional model : t = [b](r)
 *
 *	solve P([r_data])*[b] = [t_data] for [b] by least squares
 *
 *	2 dimensional model : t = [b](r,s) 
 *
 *	solve P([r_data],[s_data])*[b] = [t_data] for [b] by least squares
 *	  
 * 2-Aug-1990 K.Knowles knowles@sastrugi.colorado.edu 303-492-0644
 *======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <define.h>
#include <pmodel.h>

static const char pmodel_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/pmodel.c,v 1.2 1993-08-02 10:30:58 knowles Exp $";

static void design();
static void factor();
static void solve();
static void svdcmp();
static void svbksb();
static void nrerror();
static double **dmatrix();
static void free_dmatrix();
static void free_dvector();

/*----------------------------------------------------------------------
 * peval - evaluate polynomial
 *
 *	input : P - polynomial model
 *		r,s - point to evaluate at
 *
 *	result: polynomial evaluated at r,s
 *
 *	dim = 1:
 *
 *	t = b0 + b1*r + b2*r^2 + ... + bk*r^k,  k = order
 *
 *	dim = 2:
 *
 *	tcode = 1 means triangular coefficient matrix
 *	tcode = 0 means full coefficient matrix
 *
 *	For example order=2, tcode=1          tcode=0
 *		             1    s   s^2      1    s     s^2
 *	                     r    rs           r    rs    rs^2
 *	                     r^2               r^2  r^2s  r^2s^2
 *
 *	Coefficients are numbered sequentially in either case,
 *	starting at upper left increasing down each column.
 *
 *	For the above examples
 *	                    b0   b3   b5       b0   b3   b6
 *                          b1   b4            b1   b4   b7
 *	                    b2                 b2   b5   b8
 *
 *----------------------------------------------------------------------*/
double peval (Polynomial *P, double r, double s)
{
	register int i,j,k,m,n;
	register double sum_i, sum_j;

/*
 *	the polynomials are factored for significantly
 *	faster evaluation and to avoid over/underflow
 */
	if (P->dim == 1)
	{ sum_i = P->coef[P->order];
	  for (i = P->order - 1; i >= 0; i--)
	    sum_i = r*sum_i + P->coef[i];
	  return (sum_i);
	}
	else if (P->dim == 2)
	{ n = P->order + 1;
	  if (P->tcode)
	  { m = 1;
	    k = n*(n+1)/2 -1;
	  }
	  else
	  { m = n;
	    k = m*n -1;
	  }
	  sum_j = 0.0;
	  for (j=0; j<n; j++)
	  { sum_i = 0.0;
	    for (i=0; i<m; i++)
	    { sum_i = r*sum_i + P->coef[k];
	      --k;
	    }
	    sum_j = s*sum_j + sum_i;
	    if (P->tcode)  m++;
	  }
	  return (sum_j);
	}
}

/*----------------------------------------------------------------------
 * pmatrix - get design matrix for polynomial model
 *
 *	input : P - polynomial model (dim, order, tcode)
 *		npts - number of data points
 *		r_data, s_data - coords. of the data points
 *		
 *	output: M - design matrix
 *
 *	number of data points must be greater than number of variables
 *	for example, with a 2 dimensional model,
 *		if order=2 and tcode=1
 *		then (b0, b1, ... b5) = 6 variables
 *		so, you must have at least 7 data points
 *
 *----------------------------------------------------------------------*/
void pmatrix (Polynomial *P, int npts, double M[MAXPOINTS][MAXVARS],
	      double r_data[MAXPOINTS], double s_data[MAXPOINTS])
{
	register int i, ipt, j, k, m, n;

	if (P->dim == 1)
	{ for (ipt=0; ipt < npts; ipt++)
	  { for (k=0; k <= P->order; k++)
	      M[ipt][k] = ipow(r_data[ipt],k);
	  }
	}
	else if (P->dim == 2)
	{ m = P->order + 1;
	  for (ipt=0; ipt < npts; ipt++)
	  { n = m;
	    k = 0;
	    for (j=0; j < m; j++)
	    { for (i=0; i < n; i++)
	      { M[ipt][k] = ipow(r_data[ipt],i) * ipow(s_data[ipt],j);
	        k++;
	      }
	      n -= P->tcode;
	    }
	  }
	}

	return;
}

static double sval[MAXVARS];

#define TOL 1.0e-12

/*----------------------------------------------------------------------
 * pmodel - solve for polynomial coefficients
 *
 *	input : P - polynomial model (dim, order, tcode)
 *		npts - number of data points
 *		M - design matrix
 *		t_data - values of the data points
 *		method - "SVD" = Singular Value Decomposition
 *			 "LUD" = Lower Upper Diagonal Factoring
 *
 *	output: P - polynomial model (coefficients)
 *
 *----------------------------------------------------------------------*/
void pmodel (Polynomial *P, int npts, double M[MAXPOINTS][MAXVARS],
	     double t_data[MAXPOINTS], char *method)
{
	int i, j, nvars;
	double **u, **v;
	double A[MAXVARS][MAXVARS];
	double max_sval, thresh;
	extern double **dmatrix();
	extern void design(), factor(), solve();
	extern void free_dmatrix(), svbksb(), svdcmp();

	if (P->dim == 1)
	  nvars = P->order + 1;
	else if (P->dim == 2)
	  nvars = P->tcode == 0 ? (P->order + 1) * (P->order + 1)
				: (P->order + 1) * (P->order + 2) / 2;

	if (strcmp (method, "SVD") == 0)
	{ u = dmatrix (1, npts, 1, nvars);
	  v = dmatrix (1, nvars, 1, nvars);
/*
 *	  note: svd routines expect subscripts to start at 1 rather than 0
 */
	  for (i=0; i < npts; i++)
	    for (j=0; j < nvars; j++)
	      u[i+1][j+1] = M[i][j];

	  svdcmp (u, npts, nvars, sval-1, v);

	  max_sval = 0.0;
	  for (j=0; j < nvars; j++)
	    if (sval[j] > max_sval) max_sval = sval[j];
	  thresh = TOL * max_sval;
	  for (j=0; j < nvars; j++)
	    if (sval[j] < thresh) sval[j] = 0.0;

	  svbksb (u, sval-1, v, npts, nvars, t_data-1, P->coef-1);

	  free_dmatrix (u, 1, npts, 1, nvars);
	  free_dmatrix (v, 1, nvars, 1, nvars);
	}

	else if (strcmp (method, "LUD") == 0)
	{ design (M, t_data, A, P->coef, npts, nvars);
	  factor (A, nvars);
	  solve (A, P->coef, nvars);
	}

	else
	{ fprintf (stderr, "pmodel: unknown method >%s<\n", method);
	  exit(ABORT);
	}

	return;
}

/*----------------------------------------------------------------------
 * pstats - test fit of polynomial model
 *
 *	input : P - polynomial model
 *		npts - number of data points
 *		t_data - values of the data points
 *		r_data, s_data - coords. of the data points
 *
 *	output: SSE - Sum Squared Error, the value to be minimized in
 *		      the least squares method, the smaller the better
 *		R2 - a measure of how well the model accounts for the
 *		     variance in the data values, should be 1.000000
 *----------------------------------------------------------------------*/
void pstats (double *SSE, double *R2, Polynomial *P, int npts, 
	     double t_data[MAXPOINTS], double r_data[MAXPOINTS], 
	     double s_data[MAXPOINTS])
{
	int ipt;
	double e, t, sum_t, sum_t2, TSS;
	double r, s, peval();

	*SSE = 0.0;
	sum_t = sum_t2 = 0.0;
	for (ipt=0; ipt < npts; ipt++)
	{ r = r_data[ipt];
	  s = P->dim == 2 ? s_data[ipt] : 0.0;
	  t = peval (P, r,s);
	  sum_t += t;
	  sum_t2 += t*t;
	  e = t_data[ipt] - t;
	  *SSE += e*e;
	}
	TSS = sum_t2 - sum_t*sum_t / npts;
	*R2 = 1 - *SSE/TSS;
}

/*======================================================================
 *	to solve x*b = y, where x is m x n
 *	first calculate xT*x*b = xT*y, xT*x is n x n, pos.def.symmetric
 *	then, factor xT*x into triangular form
 *	and solve for b by back substitution
 *======================================================================*/

/*----------------------------------------------------------------------
 * design - calculate xT*x and xT*y
 *
 *	input : x - design matrix (m x n)
 *		y - data
 *		m - number of data points
 *		n - number of variables
 *
 *	output: A - xT*x (n x n)
 *		z - xT*y
 *
 *----------------------------------------------------------------------*/
static void design (x, y, A, z, m, n)
double x[MAXPOINTS][MAXVARS];
double y[MAXPOINTS];
double A[MAXVARS][MAXVARS];
double z[MAXVARS];
int m, n;
{
	int i,j,k;
	double sum;

/*
 *	A = xT*x, z = xT*y
 */
	for (i=0; i<n; i++)
	{ for (j=0; j<n; j++)
	  { sum = 0.0;
	    for (k=0; k<m; k++)
	      sum += x[k][i]*x[k][j];
	    A[i][j] = sum;
	  }
	  sum = 0.0;
	  for (k=0; k<m; k++)
	    sum += x[k][i]*y[k];
	  z[i] = sum;
	}
}

/*----------------------------------------------------------------------
 * factor - factor positive definite matrix A into triangular form
 *
 *	input : A - pos. def. sym matrix (n x n)
 *		n - dimension of A
 *
 *	output: A - factored matrix
 *
 *----------------------------------------------------------------------*/
static void factor (A, n)
double A[MAXVARS][MAXVARS];
int n;
{
	double nf;
	int i,j,k;
/*
 * triangularization
 */
	nf = sqrt(A[0][0]);
	for (i=0; i<n; i++)
	  A[0][i] /= nf;
	for (i=1; i<n; i++)
	{ for (j=i; j<n; j++)
	    for (k=0; k<i; k++)
	      A[i][j] -= A[k][i]*A[k][j];
	  nf = sqrt(A[i][i]);
	  for (j=i; j<n; j++)
	    A[i][j] /= nf;
	}
}

/*----------------------------------------------------------------------
 * solve - substitute z into A to solve A*b = z
 *
 *	input : A - triangular matrix
 *		z - data vector
 *		n - dimension of A (and z)
 *
 *	output: z - solution vector
 *
 *----------------------------------------------------------------------*/
static void solve (A, z, n)
double A[MAXVARS][MAXVARS], z[MAXVARS];
int n;
{
	int i,k;
	double sum;
/*
 * forward elimination
 */
	z[0] /= A[0][0];
	for (i=1; i<n; i++)
	{ sum = 0.0;
	  for (k=0; k<i; k++)
	    sum += A[k][i]*z[k];
	  z[i] = (z[i] - sum) / A[i][i];
	}
/*
 * back substitution
 */
	z[n-1] /= A[n-1][n-1];
	for (i=n-1; i>=1; i--)
	{ for (k=i; k<n; k++)
	    z[i-1] -= A[i-1][k]*z[k];
	  z[i-1] /= A[i-1][i-1];
	}
}

/*======================================================================
 * svd - singular value decomposition
 *
 *	adapted from Numerical Recipes
 *
 *======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static double at,bt,ct;
#define PYTHAG(a,b) ((at=fabs(a)) > (bt=fabs(b)) ? \
(ct=bt/at,at*sqrt(1.0+ct*ct)) : (bt ? (ct=at/bt,bt*sqrt(1.0+ct*ct)): 0.0))

static double maxarg1,maxarg2;
#define MAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
	(maxarg1) : (maxarg2))
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

/*----------------------------------------------------------------------
 * svdcmp - singular value decomp
 *
 *	input : a - matrix to factor (m x n)
 *		m - row dimension
 *		n - column dimension
 *
 *	output: a - first factor of input a (m x n) (called u in book)
 *		w - diagonal of second factor (singular values)
 *		v - third factor (n x n)
 *
 *----------------------------------------------------------------------*/
static void svdcmp(a,m,n,w,v)
double **a,*w,**v;
int m,n;
{
	int flag,i,its,j,jj,k,l,nm;
	double c,f,h,s,x,y,z;
	double anorm=0.0,g=0.0,scale=0.0;
	double *rv1,*dvector();
	void nrerror(),free_dvector();

	if (m < n) nrerror("SVDCMP: You must augment A with extra zero rows");
	rv1=dvector(1,n);
	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += fabs(a[k][i]);
			if (scale) {
				for (k=i;k<=m;k++) {
					a[k][i] /= scale;
					s += a[k][i]*a[k][i];
				}
				f=a[i][i];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][i]=f-g;
				if (i != n) {
					for (j=l;j<=n;j++) {
						for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
						f=s/h;
						for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
					}
				}
				for (k=i;k<=m;k++) a[k][i] *= scale;
			}
		}
		w[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += fabs(a[i][k]);
			if (scale) {
				for (k=l;k<=n;k++) {
					a[i][k] /= scale;
					s += a[i][k]*a[i][k];
				}
				f=a[i][l];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][l]=f-g;
				for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
				if (i != m) {
					for (j=l;j<=m;j++) {
						for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
						for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
					}
				}
				for (k=l;k<=n;k++) a[i][k] *= scale;
			}
		}
		anorm=MAX(anorm,(fabs(w[i])+fabs(rv1[i])));
	}
	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++)
					v[j][i]=(a[i][j]/a[i][l])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
					for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
				}
			}
			for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
		}
		v[i][i]=1.0;
		g=rv1[i];
		l=i;
	}
	for (i=n;i>=1;i--) {
		l=i+1;
		g=w[i];
		if (i < n)
			for (j=l;j<=n;j++) a[i][j]=0.0;
		if (g) {
			g=1.0/g;
			if (i != n) {
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
					f=(s/a[i][i])*g;
					for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
				}
			}
			for (j=i;j<=m;j++) a[j][i] *= g;
		} else {
			for (j=i;j<=m;j++) a[j][i]=0.0;
		}
		++a[i][i];
	}
	for (k=n;k>=1;k--) {
		for (its=1;its<=30;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if (fabs(rv1[l])+anorm == anorm) {
					flag=0;
					break;
				}
				if (fabs(w[nm])+anorm == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					if (fabs(f)+anorm != anorm) {
						g=w[i];
						h=PYTHAG(f,g);
						w[i]=h;
						h=1.0/h;
						c=g*h;
						s=(-f*h);
						for (j=1;j<=m;j++) {
							y=a[j][nm];
							z=a[j][i];
							a[j][nm]=y*c+z*s;
							a[j][i]=z*c-y*s;
						}
					}
				}
			}
			z=w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = -z;
					for (j=1;j<=n;j++) v[j][k]=(-v[j][k]);
				}
				break;
			}
			if (its == 30) nrerror("No convergence in 30 SVDCMP iterations");
			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=PYTHAG(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=PYTHAG(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g=g*c-x*s;
				h=y*s;
				y=y*c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj][j];
					z=v[jj][i];
					v[jj][j]=x*c+z*s;
					v[jj][i]=z*c-x*s;
				}
				z=PYTHAG(f,h);
				w[j]=z;
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=(c*g)+(s*y);
				x=(c*y)-(s*g);
				for (jj=1;jj<=m;jj++) {
					y=a[jj][j];
					z=a[jj][i];
					a[jj][j]=y*c+z*s;
					a[jj][i]=z*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
	free_dvector(rv1,1,n);
}

#undef SIGN
#undef MAX
#undef PYTHAG

/*----------------------------------------------------------------------
 * svbksb - back substitution (solve a*x = b)
 *
 *	input : u,w,v - factors of a (from svdcmp)
 *		m - row dimension
 *		n - column dimension
 *		b - data vector
 *
 *	output: x - solution vector
 *
 *----------------------------------------------------------------------*/
static void svbksb(u,w,v,m,n,b,x)
double **u,w[],**v,b[],x[];
int m,n;
{
	int jj,j,i;
	double s,*tmp,*dvector();
	void free_dvector();

	tmp=dvector(1,n);
	for (j=1;j<=n;j++) {
		s=0.0;
		if (w[j]) {
			for (i=1;i<=m;i++) s += u[i][j]*b[i];
			s /= w[j];
		}
		tmp[j]=s;
	}
	for (j=1;j<=n;j++) {
		s=0.0;
		for (jj=1;jj<=n;jj++) s += v[j][jj]*tmp[jj];
		x[j]=s;
	}
	free_dvector(tmp,1,n);
}

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
static void nrerror(error_text)
char error_text[];
/* Numerical Recipes standard error handler */
{
	void _exit();

	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	_exit(1);
}

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
double *dvector(nl,nh)
int nl,nh;
/* allocate a double vector with subscript range v[nl..nh] */
{
	double *v;

	v=(double *)malloc((unsigned) (nh-nl+1)*sizeof(double))-nl;
	if (!v) nrerror("allocation failure in dvector()");
	return v;
}

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
static double **dmatrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
/* allocate a double matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	int i;
	double **m;

	/* allocate pointers to rows */
	m=(double **) malloc((unsigned) (nrh-nrl+1)*sizeof(double*))-nrl;
	if (!m) nrerror("allocation failure 1 in dmatrix()");

	/* allocate rows and set pointers to them */
	for(i=nrl;i<=nrh;i++) {
		m[i]=(double *) malloc((unsigned) (nch-ncl+1)*sizeof(double))-ncl;
		if (!m[i]) nrerror("allocation failure 2 in dmatrix()");
	}
	/* return pointer to array of pointers to rows */
	return m;
}

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
static void free_dvector(v,nl,nh)
double *v;
int nl,nh;
/* free a double vector allocated with dvector() */
{
	free((char*) (v+nl));
}

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
static void free_dmatrix(m,nrl,nrh,ncl,nch)
double **m;
int nrl,nrh,ncl,nch;
/* free a double matrix allocated by dmatrix() */
{
	int i;

	for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
	free((char*) (m+nrl));
}

/*------------------------------------------------------------------------
 * chebyshev - determine ith Chebyshev point on an interval
 *------------------------------------------------------------------------*/
double chebyshev(int i, int n, double a, double b)
{
  return .5*(a+b + (a-b)*cos(PI*(2.*i+1)/(2.*n+2)));
}

