/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include <math.h>
#include <stdlib.h>
#include <iostream.h>
#include "vtkMath.h"

long vtkMath::Seed = 1177; // One authors home address

//
// some constants we need
//
#define VTK_K_A 16807
#define VTK_K_M 2147483647			/* Mersenne prime 2^31 -1 */
#define VTK_K_Q 127773			/* VTK_K_M div VTK_K_A */
#define VTK_K_R 2836			/* VTK_K_M mod VTK_K_A */
//
// Some useful macros
//
#define VTK_SIGN(x)              (( (x) < 0 )?( -1 ):( 1 ))
// avoid dll boundary problems

// Generate random numbers between 0.0 and 1.0.
// This is used to provide portability across different systems.
float vtkMath::Random()
{
  long hi, lo;
    
  // Based on code in "Random Number Generators: Good Ones are Hard to Find,"
  // by Stephen K. Park and Keith W. Miller in Communications of the ACM,
  // 31, 10 (Oct. 1988) pp. 1192-1201.
  // Borrowed from: Fuat C. Baran, Columbia University, 1988.
  hi = vtkMath::Seed / VTK_K_Q;
  lo = vtkMath::Seed % VTK_K_Q;
  if ((vtkMath::Seed = VTK_K_A * lo - VTK_K_R * hi) <= 0)
    {
    Seed += VTK_K_M;
    }
  return ((float) vtkMath::Seed / VTK_K_M);
}

// Initialize seed value. NOTE: Random() has the bad property that 
// the first random number returned after RandomSeed() is called 
// is proportional to the seed value! To help solve this, call 
// RandomSeed() a few times inside seed. This doesn't ruin the 
// repeatability of Random().
//
void vtkMath::RandomSeed(long s)
{
  vtkMath::Seed = s;

  vtkMath::Random();
  vtkMath::Random();
  vtkMath::Random();
}

// Cross product of two 3-vectors. Result vector in z[3].
void vtkMath::Cross(float x[3], float y[3], float z[3])
{
  float Zx = x[1]*y[2] - x[2]*y[1]; 
  float Zy = x[2]*y[0] - x[0]*y[2];
  float Zz = x[0]*y[1] - x[1]*y[0];
  z[0] = Zx; z[1] = Zy; z[2] = Zz; 
}

#define VTK_SMALL_NUMBER 1.0e-12

// Solve linear equations Ax = b using Crout's method. Input is square matrix A
// and load vector x. Solution x is written over load vector. The dimension of
// the matrix is specified in size. If error is found, method returns a 0.
int vtkMath::SolveLinearSystem(double **A, double *x, int size)
{
  static int *index = NULL, maxSize=0;

  // if we solving something simple, just solve it
  //
  if (size == 2)
    {
    double det, y[2];

    det = vtkMath::Determinant2x2(A[0][0], A[0][1], A[1][0], A[1][1]);

    if (det == 0.0)
      {
      return 0;
      }

    y[0] = (A[1][1]*x[0] - A[0][1]*x[1]) / det;
    y[1] = (-A[1][0]*x[0] + A[0][0]*x[1]) / det;

    x[0] = y[0];
    x[1] = y[1];
    return 1;
    }
  else if (size == 1)
    {
    if (A[0][0] == 0.0)
      {
      return 0;
      }
    
    x[0] /= A[0][0];
    return 1;
    }

  //
  // System of equations is not trivial, use Crout's method
  //
  
  
  //
  // Check on allocation of working vectors
  //
  if ( index == NULL ) 
    {
    index = new int[size];
    maxSize = size;
    } 
  else if ( size > maxSize ) 
    {
    delete [] index;
    index = new int[size];
    maxSize = size;
    }
  //
  // Factor and solve matrix
  //
  if ( vtkMath::LUFactorLinearSystem(A, index, size) == 0 )
    {
    return 0;
    }
  vtkMath::LUSolveLinearSystem(A,index,x,size);

  return 1;
}

// Invert input square matrix A into matrix AI. Note that A is modified during
// the inversion. The size variable is the dimension of the matrix. Returns 0
// if inverse not computed.
int vtkMath::InvertMatrix(double **A, double **AI, int size)
{
  static int *index = NULL, maxSize=0;
  static double *column = NULL;
  int i, j;
  //
  // Check on allocation of working vectors
  //
  if ( index == NULL ) 
    {
    index = new int[size];
    column = new double[size];
    maxSize = size;
    } 
  else if ( size > maxSize ) 
    {
    delete [] index; delete [] column;
    index = new int[size];
    column = new double[size];
    maxSize = size;
    }
  //
  // Factor matrix; then begin solving for inverse one column at a time.
  //
  if ( vtkMath::LUFactorLinearSystem(A, index, size) == 0 )
    {
    return 0;
    }
  
  for ( i=0; i < size; i++ )
    {
    for ( j=0; j < size; j++ )
      {
      column[j] = 0.0;
      }
    column[i] = 1.0;

    vtkMath::LUSolveLinearSystem(A,index,column,size);

    for ( j=0; j < size; j++ )
      {
      AI[i][j] = column[j];
      }
    }

  return 1;
}

// Factor linear equations Ax = b using LU decompostion A = LU where L is
// lower triangular matrix and U is upper triangular matrix. Input is 
// square matrix A, integer array of pivot indices index[0->n-1], and size
// of square matrix n. Output factorization LU is in matrix A. If error is 
// found, method returns 0. 
int vtkMath::LUFactorLinearSystem(double **A, int *index, int size)
{
  static double *scale = NULL;
  static int maxSize=0;
  int i, j, k;
  int maxI = 0;
  double largest, temp1, temp2, sum;
  //
  // Check on allocation of working vectors
  //
  if ( scale == NULL ) 
    {
    scale = new double[size];
    maxSize = size;
    } 
  else if ( size > maxSize ) 
    {
    delete [] scale; 
    scale = new double[size];
    maxSize = size;
    }
  //
  // Loop over rows to get implicit scaling information
  //
  for ( i = 0; i < size; i++ ) 
    {
    for ( largest = 0.0, j = 0; j < size; j++ ) 
      {
      if ( (temp2 = fabs(A[i][j])) > largest )
	{
	largest = temp2;
	}
      }

    if ( largest == 0.0 )
      {
      return 0;
      }
      scale[i] = 1.0 / largest;
    }
  //
  // Loop over all columns using Crout's method
  //
  for ( j = 0; j < size; j++ ) 
    {
    for (i = 0; i < j; i++) 
      {
      sum = A[i][j];
      for ( k = 0; k < i; k++ )
	{
	sum -= A[i][k] * A[k][j];
	}
      A[i][j] = sum;
      }
    //
    // Begin search for largest pivot element
    //
    for ( largest = 0.0, i = j; i < size; i++ ) 
      {
      sum = A[i][j];
      for ( k = 0; k < j; k++ )
	{
	sum -= A[i][k] * A[k][j];
	}
      A[i][j] = sum;

      if ( (temp1 = scale[i]*fabs(sum)) >= largest ) 
        {
        largest = temp1;
        maxI = i;
        }
      }
    //
    // Check for row interchange
    //
    if ( j != maxI ) 
      {
      for ( k = 0; k < size; k++ ) 
        {
        temp1 = A[maxI][k];
        A[maxI][k] = A[j][k];
        A[j][k] = temp1;
        }
      scale[maxI] = scale[j];
      }
    //
    // Divide by pivot element and perform elimination
    //
    index[j] = maxI;

    if ( fabs(A[j][j]) <= VTK_SMALL_NUMBER )
      {
      return 0;
      }

    if ( j != (size-1) ) 
      {
      temp1 = 1.0 / A[j][j];
      for ( i = j + 1; i < size; i++ )
	{
	A[i][j] *= temp1;
	}
      }
    }

  return 1;
}


// Solve linear equations Ax = b using LU decompostion A = LU where L is
// lower triangular matrix and U is upper triangular matrix. Input is 
// factored matrix A=LU, integer array of pivot indices index[0->n-1],
// load vector x[0->n-1], and size of square matrix n. Note that A=LU and
// index[] are generated from method LUFactorLinearSystem). Also, solution
// vector is written directly over input load vector.
void vtkMath::LUSolveLinearSystem(double **A, int *index, double *x, int size)
{
  int i, j, ii, idx;
  double sum;
//
// Proceed with forward and backsubstitution for L and U
// matrices.  First, forward substitution.
//
  for ( ii = -1, i = 0; i < size; i++ ) 
    {
    idx = index[i];
    sum = x[idx];
    x[idx] = x[i];

    if ( ii >= 0 )
      {
      for ( j = ii; j <= (i-1); j++ )
	{
	sum -= A[i][j]*x[j];
	}
      }
    else if (sum)
      {
      ii = i;
      }

    x[i] = sum;
  }
//
// Now, back substitution
//
  for ( i = size-1; i >= 0; i-- ) 
    {
    sum = x[i];
    for ( j = i + 1; j < size; j++ )
      {
      sum -= A[i][j]*x[j];
      }
    x[i] = sum / A[i][i];
    }
}

#undef VTK_SMALL_NUMBER

#define VTK_ROTATE(a,i,j,k,l) g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau);\
        a[k][l]=h+s*(g-h*tau);

#define VTK_MAX_ROTATIONS 20

// Jacobi iteration for the solution of eigenvectors/eigenvalues of a 3x3
// real symmetric matrix. Square 3x3 matrix a; output eigenvalues in w;
// and output eigenvectors in v. Resulting eigenvalues/vectors are sorted
// in decreasing order; eigenvectors are normalized.
int vtkMath::Jacobi(float **a, float *w, float **v)
{
  return vtkMath::JacobiN(a, 3, w, v);
}


//#undef VTK_MAX_ROTATIONS

//#define VTK_MAX_ROTATIONS 50

// Jacobi iteration for the solution of eigenvectors/eigenvalues of a nxn
// real symmetric matrix. Square nxn matrix a; size of matrix in n;
// output eigenvalues in w; and output eigenvectors in v. Resulting
// eigenvalues/vectors are sorted in decreasing order; eigenvectors are
// normalized.
int vtkMath::JacobiN(float **a, int n, float *w, float **v)
{
  int i, j, k, iq, ip, numPos;
  float tresh, theta, tau, t, sm, s, h, g, c;
  float *b, *z, tmp;

  b = new float[n];
  z = new float[n];
  
  // initialize
  for (ip=0; ip<n; ip++) 
    {
    for (iq=0; iq<n; iq++)
      {
      v[ip][iq] = 0.0;
      }
    v[ip][ip] = 1.0;
    }
  for (ip=0; ip<n; ip++) 
    {
    b[ip] = w[ip] = a[ip][ip];
    z[ip] = 0.0;
    }

  // begin rotation sequence
  for (i=0; i<VTK_MAX_ROTATIONS; i++) 
    {
    sm = 0.0;
    for (ip=0; ip<n-1; ip++) 
      {
      for (iq=ip+1; iq<n; iq++)
	{
	sm += fabs(a[ip][iq]);
	}
      }
    if (sm == 0.0)
      {
      break;
      }

    if (i < 3)                                // first 3 sweeps
      {
      tresh = 0.2*sm/(n*n);
      }
    else
      {
      tresh = 0.0;
      }

    for (ip=0; ip<n-1; ip++) 
      {
      for (iq=ip+1; iq<n; iq++) 
        {
        g = 100.0*fabs(a[ip][iq]);

        // after 4 sweeps
        if (i > 3 && (fabs(w[ip])+g) == fabs(w[ip])
        && (fabs(w[iq])+g) == fabs(w[iq]))
          {
          a[ip][iq] = 0.0;
          }
        else if (fabs(a[ip][iq]) > tresh) 
          {
          h = w[iq] - w[ip];
          if ( (fabs(h)+g) == fabs(h))
	    {
	    t = (a[ip][iq]) / h;
	    }
          else 
            {
            theta = 0.5*h / (a[ip][iq]);
            t = 1.0 / (fabs(theta)+sqrt(1.0+theta*theta));
            if (theta < 0.0)
	      {
	      t = -t;
	      }
            }
          c = 1.0 / sqrt(1+t*t);
          s = t*c;
          tau = s/(1.0+c);
          h = t*a[ip][iq];
          z[ip] -= h;
          z[iq] += h;
          w[ip] -= h;
          w[iq] += h;
          a[ip][iq]=0.0;

          // ip already shifted left by 1 unit
          for (j = 0;j <= ip-1;j++) 
            {
            VTK_ROTATE(a,j,ip,j,iq)
              }
          // ip and iq already shifted left by 1 unit
          for (j = ip+1;j <= iq-1;j++) 
            {
            VTK_ROTATE(a,ip,j,j,iq)
              }
          // iq already shifted left by 1 unit
          for (j=iq+1; j<n; j++) 
            {
            VTK_ROTATE(a,ip,j,iq,j)
              }
          for (j=0; j<n; j++) 
            {
            VTK_ROTATE(v,j,ip,j,iq)
            }
          }
        }
      }

    for (ip=0; ip<n; ip++) 
      {
      b[ip] += z[ip];
      w[ip] = b[ip];
      z[ip] = 0.0;
      }
    }

  //// this is NEVER called
  if ( i >= VTK_MAX_ROTATIONS )
    {
    vtkGenericWarningMacro(
       "vtkMath::Jacobi: Error extracting eigenfunctions");
    return 0;
    }

  // sort eigenfunctions                 these changes do not affect accuracy 
  for (j=0; j<n-1; j++)                  // boundary incorrect
    {
    k = j;
    tmp = w[k];
    for (i=j+1; i<n; i++)                // boundary incorrect, shifted already
      {
      if (w[i] >= tmp)                   // why exchage if same?
        {
        k = i;
        tmp = w[k];
        }
      }
    if (k != j) 
      {
      w[k] = w[j];
      w[j] = tmp;
      for (i=0; i<n; i++) 
        {
        tmp = v[i][j];
        v[i][j] = v[i][k];
        v[i][k] = tmp;
        }
      }
    }
  // insure eigenvector consistency (i.e., Jacobi can compute vectors that
  // are negative of one another (.707,.707,0) and (-.707,-.707,0). This can
  // reek havoc in hyperstreamline/other stuff. We will select the most
  // positive eigenvector.
  for (j=0; j<n; j++)
    {
    for (numPos=0, i=0; i<n; i++)
      {
      if ( v[i][j] >= 0.0 )
	{
	numPos++;
	}
      }
    if ( numPos < ceil(double(n)/double(2.0)) )
      {
      for(i=0; i<n; i++)
	{
	v[i][j] *= -1.0;
	}
      }
    }

  delete [] b;
  delete [] z;
  return 1;
}

#undef VTK_ROTATE
#undef VTK_MAX_ROTATIONS

// Estimate the condition number of a LU factored matrix. Used to judge the
// accuracy of the solution. The matrix A must have been previously factored
// using the method LUFactorLinearSystem. The condition number is the ratio
// of the infinity matrix norm (i.e., maximum value of matrix component)
// divided by the minimum diagonal value. (This works for triangular matrices
// only: see Conte and de Boor, Elementary Numerical Analysis.)
double vtkMath::EstimateMatrixCondition(double **A, int size)
{
  int i;
  int j = 0;
  double min=VTK_LARGE_FLOAT, max=(-VTK_LARGE_FLOAT);

  // find the maximum value
  for (i=0; i < size; i++)
    {
    for (j=i; j < size; j++)
      {
      if ( fabs(A[i][j]) > max )
	{
	max = fabs(A[i][j]);
	}
      }
    }

  // find the minimum diagonal value
  for (i=0; i < size; i++)
    {
    if ( fabs(A[i][i]) < min )
      {
      min = fabs(A[i][j]);
      }
    }

  if ( min == 0.0 )
    {
    return VTK_LARGE_FLOAT;
    }
  else
    {
    return (max/min);
    }
}

// Solves a cubic equation c0*t^3  + c1*t^2  + c2*t + c3 = 0 when
// c0, c1, c2, and c3 are REAL.
// Solution is motivated by Numerical Recipes In C 2nd Ed.
// Return array contains number of (real) roots (counting multiple roots as one)
// followed by roots themselves. The value in roots[4] is a integer giving
// further information about the roots (see return codes for int SolveCubic()).
double* vtkMath::SolveCubic( double c0, double c1, double c2, double c3) 
{
  static double roots[5];
  roots[1] = 0.0;
  roots[2] = 0.0;
  roots[3] = 0.0;
  int num_roots;

  roots[4] = vtkMath::SolveCubic(c0, c1, c2, c3, 
				 &roots[1], &roots[2], &roots[3], &num_roots );
  roots[0] = num_roots;
  return roots;
}

// Solves a cubic equation when c0, c1, c2, And c3 Are REAL.  Solution
// is motivated by Numerical Recipes In C 2nd Ed.  Roots and number of
// real roots are stored in user provided variables r1, r2, r3, and
// num_roots. Note that the function can return the following integer
// values describing the roots: (0)-no solution; (-1)-infinite number
// of solutions; (1)-one distinct real root of multiplicity 3 (stored
// in r1); (2)-two distinct real roots, one of multiplicity 2 (stored
// in r1 & r2); (3)-three distinct real roots; (-2)-quadratic equation
// with complex conjugate solution (real part of root returned in r1,
// imaginary in r2); (-3)-one real root and a complex conjugate pair
// (real root in r1 and real part of pair in r2 and imaginary in r3).
int vtkMath::SolveCubic( double c0, double c1, double c2, double c3, 
			 double *r1, double *r2, double *r3, int *num_roots )
{
  double	Q, R;
  double	R_squared;	/* R*R */
  double	Q_cubed;	/* Q*Q*Q */
  double	theta;
  double	A, B;

  // Cubic equation: c0*t^3  + c1*t^2  + c2*t + c3 = 0 
  //                                               
  //   r1, r2, r3 are roots and num_roots is the number
  //   of real roots                               

  // Make Sure This Is A Bonafide Cubic Equation 
  if( c0 != 0.0 )
    {
    //Put Coefficients In Right Form 
    c1 = c1/c0;
    c2 = c2/c0;
    c3 = c3/c0;

    Q = ((c1*c1) - 3*c2)/9.0;

    R = (2.0*(c1*c1*c1) - 9.0*(c1*c2) + 27.0*c3)/54.0;

    R_squared = R*R;
    Q_cubed   = Q*Q*Q;

    if( R_squared <= Q_cubed )
      {
      if( Q_cubed == 0.0 )
	{
	*r1 = -c1/3.0;
	*r2 = *r1;
	*r3 = *r1;
	*num_roots = 1;
	return 1;
	} 
      else
	{
	theta = acos( R / (sqrt(Q_cubed) ) );

	*r1 = -2.0*sqrt(Q)*cos( theta/3.0 ) - c1/3.0;
	*r2 = -2.0*sqrt(Q)*cos( (theta + 2.0*3.141592653589)/3.0) - c1/3.0;
	*r3 = -2.0*sqrt(Q)*cos( (theta - 2.0*3.141592653589)/3.0) - c1/3.0;

	*num_roots = 3;

	// Reduce Number Of Roots To Two 
	if( *r1 == *r2 )
	  {
	  *num_roots = 2;
	  *r2 = *r3;
	  }
	else if( *r1 == *r3 )
	  {
	  *num_roots = 2;
	  }

	if( (*r2 == *r3) && (*num_roots == 3) )
	  {
	  *num_roots = 2;
	  }

	// Reduce Number Of Roots To One 
	if( (*r1 == *r2) )
	  {
	  *num_roots = 1;
	  }
 	}
      return *num_roots;
      }
    else //single real and complex conjugate pair
      {
      A = -VTK_SIGN(R) * pow(fabs(R) + sqrt(R_squared - Q_cubed),0.33333333);

      if( A == 0.0 )
	{
	B = 0.0;
	}
      else
	{
	B = Q/A;
	}

      *r1 =  (A + B) - c1/3.0;
      *r2 = -0.5*(A + B) - c1/3.0;
      *r3 = sqrt(3.0)/2.0*(A - B);

      *num_roots = 1;
      return (-3);
      }
    } //if cubic equation

  else // Quadratic Equation: c1*t  + c2*t + c3 = 0 
    {
    // Okay this was not a cubic - lets try quadratic
    return vtkMath::SolveQuadratic( c1, c2, c3, r1, r2, num_roots );
    }
}

// Solves a quadratic equation c1*t^2 + c2*t + c3 = 0 when c1, c2, and
// c3 are REAL.  Solution is motivated by Numerical Recipes In C 2nd
// Ed.  Return array contains number of (real) roots (counting
// multiple roots as one) followed by roots themselves. Note that 
// roots[3] contains a return code further describing solution - see
// documentation for SolveCubic() for meaining of return codes.
double* vtkMath::SolveQuadratic( double c1, double c2, double c3) 
{
  static double roots[4];
  roots[0] = 0.0;
  roots[1] = 0.0;
  roots[2] = 0.0;
  int num_roots;

  roots[3] = vtkMath::SolveQuadratic( c1, c2, c3, &roots[1], &roots[2], 
				      &num_roots );
  roots[0] = num_roots;
  return roots;
}

// Solves A Quadratic Equation c1*t^2  + c2*t  + c3 = 0 when 
// c1, c2, and c3 are REAL.
// Solution is motivated by Numerical Recipes In C 2nd Ed.
// Roots and number of roots are stored in user provided variables
// r1, r2, num_roots
int vtkMath::SolveQuadratic( double c1, double c2, double c3, 
			     double *r1, double *r2, int *num_roots )
{
  double	Q;
  double	determinant;

  // Quadratic equation: c1*t^2 + c2*t + c3 = 0 

  // Make sure this is a quadratic equation
  if( c1 != 0.0 )
    {
    determinant = c2*c2 - 4*c1*c3;

    if( determinant >= 0.0 )
      {
      Q = -0.5 * (c2 + VTK_SIGN(c2)*sqrt(determinant));

      *r1 = Q / c1;

      if( Q == 0.0 )
	{
	*r2 = 0.0;
	}
      else
	{
	*r2 = c3 / Q;
	}

      *num_roots = 2;

      // Reduce Number Of Roots To One 
      if( *r1 == *r2 )
	{
	*num_roots = 1;
	}
      return *num_roots;
      }
    else	// Equation Does Not Have Real Roots 
      {
      *num_roots = 0;
      return (-2);
      }
    }

  else // Linear Equation: c2*t + c3 = 0 
    {
    // Okay this was not quadratic - lets try linear
    return vtkMath::SolveLinear( c2, c3, r1, num_roots );
    }
}

// Solves a linear equation c2*t  + c3 = 0 when c2 and c3 are REAL.
// Solution is motivated by Numerical Recipes In C 2nd Ed.
// Return array contains number of roots followed by roots themselves.
double* vtkMath::SolveLinear( double c2, double c3) 
{
  static double roots[3];
  int num_roots;
  roots[1] = 0.0;
  roots[2] = vtkMath::SolveLinear( c2, c3, &roots[1], &num_roots );
  roots[0] = num_roots;
  return roots;
}

// Solves a linear equation c2*t + c3 = 0 when c2 and c3 are REAL.
// Solution is motivated by Numerical Recipes In C 2nd Ed.
// Root and number of (real) roots are stored in user provided variables
// r2 and num_roots.
int vtkMath::SolveLinear( double c2, double c3, double *r1, int *num_roots )
{
  // Linear equation: c2*t + c3 = 0 
  // Now this had better be linear 
  if( c2 != 0.0 )
    {
    *r1 = -c3 / c2;
    *num_roots = 1;
    return *num_roots;
    }
  else
    {
    *num_roots = 0;
    if ( c3 == 0.0 )
      {
      return (-1);
      }
    }

  return *num_roots;
}
