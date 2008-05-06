/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile$

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
  -------------------------------------------------------------------------*/
// .NAME vtkStatisticsLinearCorrelation - A class for linear correlation
//
// .SECTION Description
// This class provides the following functionalities, depending on the
// execution mode it is executed in:
// * Learn: given two data vectors X and Y with the same number of entries, 
//   their means, unbiased variance and covariance estimators, and their
//   linear regressions and linear correlation coefficient. More precisely, 
//   ExecuteLearn calculates the raw moments; one then needs to call the
//   (static) function CalculateFromRawMoments to turn these moments into the 
//   estimators.
// * Validate: not implemented.
// * Evince: given two data vectors X and Y with the same number of entries as
//   input in port 0, and reference means, variances, and covariance, along
//   with an acceptable threshold t>1, evince all pairs of values of (X,Y) 
//   whose relative PDF (assuming a bivariate Gaussian model) is below t.
// NB: The input data set is passed as a vtkTable, of which 2 columns must thus
// be selected for X and Y -- by default, the 1st are 2nd columns, respectively.
//  
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this class.

#ifndef __vtkStatisticsLinearCorrelation_h
#define __vtkStatisticsLinearCorrelation_h

#include "vtkStatisticsAlgorithm.h"

class vtkTable;

class VTK_INFOVIS_EXPORT vtkStatisticsLinearCorrelation : public vtkStatisticsAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkStatisticsLinearCorrelation, vtkStatisticsAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkStatisticsLinearCorrelation* New();

  // Description:
  // Set the column index of variable X.
  vtkSetMacro( IdX, vtkIdType );

  // Description:
  // Get the column index of variable X.
  vtkGetMacro( IdX, vtkIdType );

  // Description:
  // Set the column index of variable Y.
  vtkSetMacro( IdY, vtkIdType );

  // Description:
  // Get the column index of variable Y.
  vtkGetMacro( IdY, vtkIdType );

  // Description:
  // Calculate the following unbiased estimators from the raw moments:
  // means, variances, and covariance estimators, as well as the linear regressions
  // of X in Y, and Y in X, along with the correlation coefficient of X and Y.
  // Input: the sample size and a vector of doubles of size 5, initialized as (in this
  //        order) sum X, sum Y, sum X^2, sum Y^2, and sum XY.
  // Output: -1 if meaningless input (sample size < 2),
  //          1 if one of the variances is 0,
  //          0 otherwise.
  // NB: this is a static function, so as to provide this functionality even when no
  // vtkStatistics are instantiated.
  static int CalculateFromRawMoments( int n, double* s, double* r );

protected:
  vtkStatisticsLinearCorrelation();
  ~vtkStatisticsLinearCorrelation();

  // Description:
  // Execute the required calculations in the specified execution modes
  virtual void ExecuteLearn( vtkTable* dataset,
                             vtkTable* output );
  virtual void ExecuteValidate( vtkTable* dataset,
                                vtkTable* params,
                                vtkTable* output); 
  virtual void ExecuteEvince( vtkTable* dataset,
                              vtkTable* params,
                              vtkTable* output ); 

  vtkIdType IdX;
  vtkIdType IdY;

private:
  vtkStatisticsLinearCorrelation(const vtkStatisticsLinearCorrelation&); // Not implemented
  void operator=(const vtkStatisticsLinearCorrelation&);   // Not implemented
};

#endif

