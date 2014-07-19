/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/

/*
 $Log: not supported by cvs2svn $
 Revision 1.3  2002/08/14 16:48:28  soliday
 Added Open License

 Revision 1.2  2001/04/30 21:58:34  borland
 It is now acceptable for the sigma-y array to be NULL.

 Revision 1.1  2000/04/11 16:22:40  soliday
 Moved here from mdbmth.

 Revision 1.3  1999/05/28 14:57:01  soliday
 Removed compiler warnings under Linux.

 Revision 1.2  1995/09/05 21:20:21  saunders
 First test release of the SDDS1.5 package.

*/

/* routine: lsfg()
 * purpose: compute generalized least squares fits using a function
 *          passed by the caller
 * 
 * Michael Borland, 1986.
 */
#include "matlib.h"
#include "mdb.h"
int p_merror(char *message);

long lsfg(
    double *xd, double *yd, double *sy,         /* data */
    long n_pts,                                 /* number of data points */
    long n_terms,                               /* number of terms of the form An.x^n */
    int32_t *order,                                /* order for each term */
    double *coef,                               /* place to put co-efficients */
    double *s_coef,                             /* and their sigmas */
    double *chi,                                /* place to put reduced chi-squared */
    double *diff,                               /* place to put difference table    */
    double (*fn)(double x, long ord)            /* basis functions */
    )
{
    long i, j, unweighted;
    double xp, *x_i, x0;
    static MATRIX *X, *Y, *Yp, *C, *C_1, *Xt, *A, *Ca, 
                  *XtC, *XtCX, *T, *Tt, *TC;

    if (n_pts<n_terms) {
        printf("error: insufficient data for requested order of fit\n");
        printf("(%ld data points, %ld terms in fit)\n", n_pts, n_terms);
        exit(1);
        }

    unweighted = 1;
    if (sy)
      for (i=1; i<n_pts; i++)
        if (sy[i]!=sy[0]) {
          unweighted = 0;
          break;
        }
    
    /* allocate matrices */
    m_alloc(&X, n_pts, n_terms);        
    m_alloc(&Y, n_pts, 1);
    m_alloc(&Yp, n_pts, 1);
    m_alloc(&Xt, n_terms, n_pts);
    if (!unweighted) {
        m_alloc(&C, n_pts, n_pts);
        m_alloc(&C_1, n_pts, n_pts);
        m_zero(C);
        m_zero(C_1);
        }
    m_alloc(&A, n_terms, 1);
    m_alloc(&Ca, n_terms, n_terms);
    m_alloc(&XtC, n_terms, n_pts);
    m_alloc(&XtCX, n_terms, n_terms);
    m_alloc(&T, n_terms, n_pts);
    m_alloc(&Tt, n_pts, n_terms);
    m_alloc(&TC, n_terms, n_pts);

    /* Compute X, Y, C, C_1.  X[i][j] = F(xd[i]), order[j]). Y[i][0] = yd[i].  
     * C   = delta(i,j)*sy[i]^2  (covariance matrix of yd)            
     * C_1 = INV(C)                                                        
     */
    for (i=0; i<n_pts; i++) {
        x_i = X->a[i];
        Y->a[i][0] = yd[i];
        if (!unweighted) {
            C->a[i][i] = sqr(sy[i]);
            C_1->a[i][i] = 1/C->a[i][i];
            }
        x0 = xd[i];
        for (j=0; j<n_terms; j++)
            x_i[j] = (*fn)(x0, order[j]);
        }

    /* Compute A, the matrix of coefficients.
     * Weighted least-squares solution is A = INV(Xt.INV(C).X).Xt.INV(C).y 
     * Unweighted solution is A = INV(Xt.X).Xt.y 
     */
    if (unweighted) {
        /* eliminating 2 matrix operations makes this much faster than a weighted fit
         * if there are many data points.
         */
        if (!m_trans(Xt, X))
            return(p_merror("transposing X"));
        if (!m_mult(XtCX, Xt, X)) 
            return(p_merror("multiplying Xt.X"));
        if (!m_invert(XtCX, XtCX)) 
            return(p_merror("inverting XtCX"));
        if (!m_mult(T, XtCX, Xt)) 
            return(p_merror("multiplying XtX.Xt"));
        if (!m_mult(A, T, Y))
            return(p_merror("multiplying T.Y"));

        /* Compute covariance matrix of A, Ca = (T.Tt)*C[0][0] */
        if (!m_trans(Tt, T))
            return(p_merror("computing transpose of T"));
        if (!m_mult(Ca, T, Tt))
            return(p_merror("multiplying T.Tt"));
        if (!m_scmul(Ca, Ca, sy?sqr(sy[0]):1))
            return(p_merror("multiplying T.Tt by scalar"));
        }
    else {
        if (!m_trans(Xt, X))
            return(p_merror("transposing X"));
        if (!m_mult(XtC, Xt, C_1)) 
            return(p_merror("multiplying Xt.C_1"));
        if (!m_mult(XtCX, XtC, X)) 
            return(p_merror("multiplying XtC.X"));
        if (!m_invert(XtCX, XtCX)) 
            return(p_merror("inverting XtCX"));
        if (!m_mult(T, XtCX, XtC)) 
            return(p_merror("multiplying XtCX.XtC"));
        if (!m_mult(A, T, Y))
            return(p_merror("multiplying T.Y"));

        /* Compute covariance matrix of A, Ca = T.C.Tt */
        if (!m_mult(TC, T, C))
            return(p_merror("multiplying T.C"));
        if (!m_trans(Tt, T))
            return(p_merror("computing transpose of T"));
        if (!m_mult(Ca, TC, Tt))
            return(p_merror("multiplying TC.Tt"));
        }

  
    for (i=0; i<n_terms; i++) {
        coef[i]   = A->a[i][0];
        s_coef[i] = sqrt(Ca->a[i][i]);
        }
  
    /* Compute Yp = X.A, use to compute chi-squared */
    if (!m_mult(Yp, X, A))
        return(p_merror("multiplying X.A"));
    *chi = 0;
    for (i=0; i<n_pts; i++) {
        xp = (Yp->a[i][0] - yd[i]);
        if (diff!=NULL) 
            diff[i] = xp;
        xp /= sy?sy[i]:1;
        *chi += xp*xp;
        }
    if (n_pts!=n_terms)
        *chi /= (n_pts-n_terms);

    /* de-allocate matrices */
    m_free(&X);        
    m_free(&Y);
    m_free(&Yp);
    m_free(&Xt);
    if (!unweighted) {
        m_free(&C);
        m_free(&C_1);
        }
    m_free(&A);
    m_free(&Ca);
    m_free(&XtC);
    m_free(&XtCX);
    m_free(&T);
    m_free(&Tt);
    m_free(&TC);
  
    return(1);
    }
