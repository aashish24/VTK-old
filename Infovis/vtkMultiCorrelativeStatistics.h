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
// .NAME vtkMultiCorrelativeStatistics - A class for linear correlation
//
// .SECTION Description
// Given a selection of sets of columns of interest, this class provides the
// following functionalities, depending on the execution mode it is executed in:
// * Learn: calculates means, unbiased variance and covariance estimators of
//   column pairs coefficient.
//   More precisely, ExecuteLearn calculates the averages and centered
//   variance/covariance sums; if \p finalize is set to true (default),
//   the final statistics are calculated.
//   The output metadata on port 1 is a multiblock dataset containing at a minimum
//   one vtkTable holding the raw sums in a sparse matrix style. If \a finalize is
//   true, then one additional vtkTable will be present for each requested set of
//   column correlations. These additional tables contain column averages, the
//   upper triangular portion of the covariance matrix (in the upper right hand
//   portion of the table) and the Cholesky decomposition of the covariance matrix
//   (in the lower portion of the table beneath the covariance triangle).
//   The leftmost column will be a vector of column averages.
//   The last entry in the column averages vector is the number of samples.
//   As an example, consider a request for a 3-column correlation with columns
//   named ColA, ColB, and ColC.
//   The resulting table will look like this:
//   <pre>
//      Column  |Column Averages|ColA     |ColB     |ColC
//      --------+---------------+---------+---------+---------
//      ColA    |avg(A)         |cov(A,A) |cov(A,B) |cov(A,C)
//      ColB    |avg(B)         |chol(1,1)|cov(B,B) |cov(B,C)
//      ColC    |avg(C)         |chol(2,1)|chol(2,2)|cov(C,C)
//      Cholesky|length(A)      |chol(3,1)|chol(3,2)|chol(3,3)
//   </pre>
// * Assess: given a set of results matrices as specified above in input port 1 and
//   tabular data on input port 0 that contains column names matching those
//   of the tables on input port 1, the assess mode computes the relative
//   deviation of each observation in port 0's table according to the linear
//   correlations implied by each table in port 1.
//  
// .SECTION Thanks
// Thanks to Philippe Pebay, Jackson Mayo, and David Thompson of
// Sandia National Laboratories for implementing this class.

#ifndef __vtkMultiCorrelativeStatistics_h
#define __vtkMultiCorrelativeStatistics_h

#include "vtkStatisticsAlgorithm.h"

class VTK_INFOVIS_EXPORT vtkMultiCorrelativeStatistics : public vtkStatisticsAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkMultiCorrelativeStatistics, vtkStatisticsAlgorithm);
  virtual void PrintSelf( ostream& os, vtkIndent indent );
  static vtkMultiCorrelativeStatistics* New();

protected:
  vtkMultiCorrelativeStatistics();
  ~vtkMultiCorrelativeStatistics();

  // Description:
  // This algorithm accepts and returns a multiblock dataset containing several tables for
  // its Learn input/output (port 1) instead of a single vtkTable.
  // We override FillInputPortInformation/FillOutputPortInformation to indicate this.
  virtual int FillInputPortInformation( int port, vtkInformation* info );
  virtual int FillOutputPortInformation( int port, vtkInformation* info );

  // Description:
  // Execute the calculations required by the Learn option.
  virtual void ExecuteLearn( vtkTable* inData,
                             vtkDataObject* outMeta );

  // Description:
  // Execute the calculations required by the Derive option.
  virtual void ExecuteDerive( vtkDataObject* );

  // Description:
  // Execute the calculations required by the Derive option.
  virtual void ExecuteAssess( vtkTable*, vtkDataObject*, vtkTable*, vtkDataObject* );

  //BTX  
  // Description:
  // Provide the appropriate assessment functor.
  virtual void SelectAssessFunctor( vtkTable* inData, 
                                    vtkDataObject* inMeta,
                                    vtkStringArray* rowNames,
                                    vtkStringArray* columnNames,
                                    AssessFunctor*& dfunc );
  //ETX

private:
  vtkMultiCorrelativeStatistics( const vtkMultiCorrelativeStatistics& ); // Not implemented
  void operator = ( const vtkMultiCorrelativeStatistics& );  // Not implemented
};

#endif


