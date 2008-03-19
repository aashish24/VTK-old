/*
 * Copyright 2007 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */
#include "vtkPolynomialSolvers.h"
#include "vtkTimerLog.h"

//=============================================================================
void PrintPolynomial( double* P, unsigned int degP )
{
  cout << "\n";

  unsigned int degPm1 = degP - 1;
  for ( unsigned int i = 0; i < degPm1; ++ i ) 
    {
    if ( P[i] > 0 ) 
      {
      if ( i ) cout << "+" << P[i] << "*x**" << degP - i;
      else cout << P[i] << "*x**" << degP - i;
      }
    else if ( P[i] < 0 ) cout << P[i] << "*x**" << degP - i;
    }   

  if ( degP > 0 )
    {
    if ( P[degPm1] > 0 ) cout << "+" << P[degPm1] << "*x";
    else if ( P[degPm1] < 0 ) cout << P[degPm1] << "*x";
    }

  if ( P[degP] > 0 ) cout << "+" << P[degP];
  else if ( P[degP] < 0 ) cout << P[degP];

  cout << "\n";
}

//=============================================================================
int TestPolynomialSolvers( int, char *[] )
{
  int testIntValue;
  double tolLinBairstow = 1.e-12;
  double tolSturm = 1.e-6;
  double tolRoots = 1.e-15;
  double tolDirectSolvers = VTK_DBL_EPSILON;
  double roots[5];
  int mult[4];
  double rootInt[] = { -4., 4. };
  double upperBnds[22];
  vtkTimerLog *timer = vtkTimerLog::New();

  // 1. find the roots of a degree 4 polynomial with a 1 double root (1) and 2
  // simple roots (2 and 3) using:
  // 1.a FerrariSolve
  // 1.b SturmBissectionSolve
  double P4[] = { 1., -7., 17., -17., 6. } ;
  PrintPolynomial( P4, 4 );

  // 1.a FerrariSolve
  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::FerrariSolve( P4 + 1, roots, mult, tolDirectSolvers );
  timer->StopTimer();

  if ( testIntValue != 3 )
    {
    vtkGenericWarningMacro("FerrariSolve(x^4 -7x^3 +17x^2 -17 x +6 ) = "<<testIntValue<<" != 3");
    return 1;
    }
  cout << "Ferrari tol=" << tolDirectSolvers
               << ", " << testIntValue << " " 
               << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) 
    {
    cout << roots[i]; 
    if ( mult[i] > 1 ) cout << "(" << mult[i] << ")";
    cout << "\n";
    }
  double actualRoots[] = { 1., 2., 3. };
  int actualMult[] = { 2, 1, 1 };
  for ( int i = 0; i < testIntValue ; ++ i ) 
    {
    if ( fabs ( roots[i] - actualRoots[i] ) > tolRoots )
      {
      vtkGenericWarningMacro("FerrariSolve(x^4 -7x^3 +17x^2 -17 x +6, ]-4;4] ): root "<<roots[i]<<" != "<<actualRoots[i]);
      return 1;  
      }
    if ( mult[i] != actualMult[i] )
      {
      vtkGenericWarningMacro("FerrariSolve(x^4 -7x^3 +17x^2 -17 x +6, ]-4;4] ): multiplicity "<<mult[i]<<" != "<<actualMult[i]);
      return 1;  
      }
    }

  // 1.b SturmBisectionSolve
  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::SturmBisectionSolve( P4, 4, rootInt, upperBnds, tolSturm );
  timer->StopTimer();

  if ( testIntValue != 3 )
    {
    vtkGenericWarningMacro("SturmBisectionSolve(x^4 -7x^3 +17x^2 -17 x +6, ]-4;4] ): "<<testIntValue<<" root(s) instead of 3.");
    return 1;  
    }
  cout << "SturmBisection +/-" << tolSturm << " ]" 
       << rootInt[0] << ";"
       << rootInt[1] << "], "
       << testIntValue << " "
       << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) cout << upperBnds[i] - tolSturm * .5 << "\n";

  // 2. find the roots of a degree 5 polynomial with LinBairstowSolve
  double P5[] = { 1., -10., 35., -50., 24., 0. } ;
  PrintPolynomial( P5, 5 );

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::LinBairstowSolve( P5, 5, roots, tolLinBairstow );
  timer->StopTimer();

  if ( testIntValue != 5 )
    {
    vtkGenericWarningMacro("LinBairstowSolve(x^5 -10x^4 +35x^3 -50x^2 +24x ) = "<<testIntValue<<" != 5");
    return 1;
    }
  cout << "LinBairstow tol=" << tolLinBairstow << ", " 
       << testIntValue << " " 
       << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) cout << roots[i] << "\n";

  // 3. find the roots of a quadratic trinomial with SturmBisectionSolve
  double P2[] = { 1., -2., 1. };
  PrintPolynomial( P2, 2 );

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::SturmBisectionSolve( P2, 2, rootInt, upperBnds, tolSturm );
  timer->StopTimer();

  if ( testIntValue != 1 )
    {
    vtkGenericWarningMacro("SturmBisectionSolve(x^2 -  2x + 1, ]-4;4] ): "<<testIntValue<<" root(s) instead of 1.");
    return 1;
    }
  if ( fabs( upperBnds[0] - 1. ) > tolSturm )
    {
    vtkGenericWarningMacro("SturmBisectionSolve(x^2 -  2x + 1, ]-4;4] ): root "<<upperBnds[0]<<" instead of 1 (within tolSturmerance of "<<tolSturm<<").");
    return 1;
    }
  cout << "SturmBisection +/-" << tolSturm << " ]" 
       << rootInt[0] << ";"
       << rootInt[1] << "], "
       << testIntValue << " "
       << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) cout << upperBnds[i] - tolSturm * .5 << "\n";

  // 4. find the roots of a biquadratic trinomial with SturmBisectionSolve,
  // whose 2 double roots (-4 and 4) are also the bounds of the interval, thus
  // being a limiting case of Sturm's theorem, using:
  // 4.a FerrariSolve
  // 4.b SturmBisectionSolve

  double P4_2[] = { 1., 0., -32., 0., 256. };
  PrintPolynomial( P4_2, 4 );

  // 4.a FerrariSolve
  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::FerrariSolve( P4_2 + 1, roots, mult, tolDirectSolvers );
  timer->StopTimer();

  if ( testIntValue != 2 )
    {
    vtkGenericWarningMacro("FerrariSolve(x^4 -32x^2 +256 ) = "<<testIntValue<<" != 2");
    return 1;
    }
  cout << "Ferrari tol=" << tolDirectSolvers
               << ", " << testIntValue << " " 
               << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) 
    {
    cout << roots[i]; 
    if ( mult[i] > 1 ) cout << "(" << mult[i] << ")";
    cout << "\n";
    }
  for ( int i = 0; i < testIntValue ; ++ i ) 
    {
    if ( fabs ( roots[i] ) - 4. > tolRoots )
      {
      vtkGenericWarningMacro("FerrariSolve(1*x**4-32*x**2+256, ]-4;4] ): root "<<roots[i]<<" != +/-4");
      return 1;  
      }
    if ( mult[i] != 2 )
      {
      vtkGenericWarningMacro("FerrariSolve(1*x**4-32*x**2+256, ]-4;4] ): multiplicity "<<mult[i]<<" != 2");
      return 1;  
      }
    }

  // 4.b SturmBisectionSolve
  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::SturmBisectionSolve( P4_2, 4, rootInt, upperBnds, tolSturm );
  timer->StopTimer();

  if ( testIntValue != 2 )
    {
    vtkGenericWarningMacro("SturmBisectionSolve(x^2 -  2x + 1, ]-4;4] ): "<<testIntValue<<" root(s) instead of 2.");
    return 1;
    }
  if ( fabs( upperBnds[0] - 4. ) > tolSturm )
    {
    vtkGenericWarningMacro("SturmBisectionSolve(x^2 -  2x + 1, ]-4;4] ): root "<<upperBnds[0]<<" instead of 1 (within tolSturmerance of "<<tolSturm<<").");
    return 1;
    }
  cout << "SturmBisection +/-" << tolSturm << " ]" 
       << rootInt[0] << ";"
       << rootInt[1] << "], "
       << testIntValue << " "
       << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) cout << upperBnds[i] - tolSturm * .5 << "\n";
  

  // 5. find the quadruple roots of the degree 12 polynomial (x-1)^4 (x-2)^4 (x-3)^4
  // All roots are quadruple roots, making it challenging for solvers using floating
  // point arithmetic.
  rootInt[0] = 0.;
  rootInt[1] = 20.;
  double P12[] = {
    1,
    -24,
    260,
    -1680,
    7206,
    -21600,
    46364,
    -71760,
    79441,
    -61320,
    31320,
    -9504,
    1296
  };
  PrintPolynomial( P12, 12 );

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::SturmBisectionSolve( P12, 12, rootInt, upperBnds, tolSturm );
  timer->StopTimer();

  if ( testIntValue != 3 )
    {
    vtkGenericWarningMacro("SturmBisectionSolve( (x-1)^4 (x-2)^4 (x-3)^4, ]0;20] ): "<<testIntValue<<" root(s) instead of 3");
    return 1;
    }
  cout << "SturmBisection +/-" << tolSturm << " ]" 
       << rootInt[0] << ";"
       << rootInt[1] << "], "
       << testIntValue << " "
       << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) cout << upperBnds[i] - tolSturm * .5 << "\n";

  // 6. Find the roots of a degree 22 polynomial with SturmBisectionSolve
  rootInt[0] = -10.;
  rootInt[1] = 10.;
  double P22[] = {
    -0.0005, -0.001, 0.05, 0.1, -0.2,
    1., 0., -5.1, 0., 4., 
    -1., .2, 3., 2.2, 2.,
    -7., -.3, 3.8, 14., -16.,
    80., -97.9, 5. };
  PrintPolynomial( P22, 22 );

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::SturmBisectionSolve( P22, 22, rootInt, upperBnds, tolSturm );
  timer->StopTimer();

  if ( testIntValue != 8 )
    {
    vtkGenericWarningMacro("SturmBisectionSolve( -0.0005x^22 -0.001x^21 +0.05x^20 +0.1x^19 -0.2x^18 +1x^17 -5.1x^15 +4x^13 -1x^12 +0.2x^11 +3x^10 +2.2x^9 +2x^8 -7x^7 -0.3x^6 +3.8x^5 +14x^4 -16x^3 +80x^2 -97.9x +5, ]-10;10] ): "<<testIntValue<<" root(s) instead of 8");
    return 1;
    }
  cout << "SturmBisection +/-" << tolSturm << " ]" 
       << rootInt[0] << ";"
       << rootInt[1] << "], "
       << testIntValue << " "
       << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) cout << upperBnds[i] - tolSturm * .5 << "\n";

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::LinBairstowSolve( P22, 22, roots, tolLinBairstow );
  timer->StopTimer();

  if ( testIntValue != 8 )
    {
    vtkGenericWarningMacro("LinBairstowSolve( -0.0005x^22 -0.001x^21 +0.05x^20 +0.1x^19 -0.2x^18 +1x^17 -5.1x^15 +4x^13 -1x^12 +0.2x^11 +3x^10 +2.2x^9 +2x^8 -7x^7 -0.3x^6 +3.8x^5 +14x^4 -16x^3 +80x^2 -97.9x +5, ]-10;10] ): "<<testIntValue<<" root(s) instead of 8");
    return 1;
    }
  cout << "LinBairstow tol=" << tolLinBairstow << ", " 
       << testIntValue << " " 
       << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) cout << roots[i] << "\n";

  // 7. Solving x^4 + 3x^3 - 4x + 1e-18 = 0 illustrates how the Ferrari solver
  // filters some numerical noise by noticing there is a double root.
  // This also exercises a case not otherwise tested.
  double P4_3[] = { 1., 3., -4., 0., 1.e-18 };
  PrintPolynomial( P4_3, 4 );

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::FerrariSolve( P4_3 + 1, roots, mult, tolDirectSolvers );
  timer->StopTimer();

  if ( testIntValue != 3 )
    {
    vtkGenericWarningMacro("FerrariSolve(x^4 +3x^3 -3x^2 +1e-18 ) = "<<testIntValue<<" != 3");
    return 1;
    }
  cout << "Ferrari tol=" << tolDirectSolvers
               << ", " << testIntValue << " " 
               << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) 
    {
    cout << roots[i]; 
    if ( mult[i] > 1 ) cout << "(" << mult[i] << ")";
    cout << "\n";
    }

  // 8. Solving x(x - 10^-4)^2 = 0 illustrates how the Tartaglia-Cardan solver
  // filters some numerical noise by noticing there is a double root (that
  // SolveCubic does not notice).
  double P3[] = { 1., -2.e-4, 1.e-8, 0.};
  PrintPolynomial( P3, 3 );

#if 0
  double r1, r2, r3;
  int nr;
  testIntValue = vtkMath::SolveCubic( P3[0], P3[1], P3[2], P3[3], &r1, &r2, &r3, &nr );
  if ( testIntValue != 2 )
    {
    vtkGenericWarningMacro("SolveCubic returned "<<testIntValue<<" != 3");
    return 1;
    }
#endif // 0  

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::TartagliaCardanSolve( P3 + 1, roots, mult, tolDirectSolvers );
  timer->StopTimer();

  if ( testIntValue != 2 )
    {
    vtkGenericWarningMacro("TartagliaCardanSolve returned "<<testIntValue<<" != 2");
    return 1;
    }
  cout << "TartagliaCardan tol=" << tolDirectSolvers
               << ", " << testIntValue << " " 
               << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) 
    {
    cout << roots[i]; 
    if ( mult[i] > 1 ) cout << "(" << mult[i] << ")";
    cout << "\n";
    }

  // 9. Solving x^3+x^2+x+1 = 0 to exercise a case not otherwise tested.
  double P3_2[] = { 1., 1., 1., 1.};
  PrintPolynomial( P3_2, 3 );

#if 0
  double r1, r2, r3;
  int nr;
  testIntValue = vtkMath::SolveCubic( P3_2[0], P3_2[1], P3_2[2], P3_2[3], &r1, &r2, &r3, &nr );
  if ( testIntValue != 1 )
    {
    vtkGenericWarningMacro("SolveCubic returned "<<testIntValue<<" != 1");
    return 1;
    }
#endif // 0  

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::TartagliaCardanSolve( P3_2 + 1, roots, mult, tolDirectSolvers );
  timer->StopTimer();

  if ( testIntValue != 1 )
    {
    vtkGenericWarningMacro("TartagliaCardanSolve returned "<<testIntValue<<" != 1");
    return 1;
    }
  cout << "TartagliaCardan tol=" << tolDirectSolvers
               << ", " << testIntValue << " " 
               << timer->GetElapsedTime() << "s\n";
  for ( int i = 0; i < testIntValue ; ++ i ) 
    {
    cout << roots[i]; 
    if ( mult[i] > 1 ) cout << "(" << mult[i] << ")";
    cout << "\n";
    }

  // 10. Solving x^3 - 2e-6 x^2 + 0.999999999999999e-12 x = 0 to test a nearly degenerate case.
  double P3_3[] = { 1., -2.e-6 , .999999999999999e-12, 0.};
  PrintPolynomial( P3_3, 3 );

#if 0
  double r1, r2, r3;
  int nr;
  testIntValue = vtkMath::SolveCubic( P3_3[0], P3_3[1], P3_3[2], P3_3[3], &r1, &r2, &r3, &nr );
  if ( testIntValue != 3 )
    {
    vtkGenericWarningMacro("SolveCubic returned "<<testIntValue<<" != 3");
    return 1;
    }
#endif // 0  

  timer->StartTimer();
  testIntValue = vtkPolynomialSolvers::TartagliaCardanSolve( P3_3 + 1, roots, mult, tolDirectSolvers );
  timer->StopTimer();

  if ( testIntValue != 3 )
    {
    vtkGenericWarningMacro("TartagliaCardanSolve returned "<<testIntValue<<" != 3");
    return 1;
    }
  cout << "TartagliaCardan tol=" << tolDirectSolvers
               << ", " << testIntValue << " " 
               << timer->GetElapsedTime() << "s\n";
  cout.precision( 9 );
  for ( int i = 0; i < testIntValue ; ++ i ) 
    {
    cout << roots[i]; 
    if ( mult[i] > 1 ) cout << "(" << mult[i] << ")";
    cout << "\n";
    }

  timer->Delete();
  return 0;
}
