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
// .NAME vtkStatisticsAlgorithm - Base class for statistics algorithms
//
// .SECTION Description
// All statistics algorithms can conceptually be operated in a variety of 
// execution modes:
// * Learn: given an input data set, calculate some type of statistics (e.g., 
//   descriptive, 5-point, correlations, PCA).
// * Assess: given an input data set, input statistics, and some form of 
//   threshold, assess a subset of the data set. 
// Therefore, a vtkStatisticsAlgorithm has the following vtkTable ports
// * 2 input ports:
//   * Data (mandatory)
//   * Statistics (optional) 
// * 1 output port (called Output):
//   * When in Learn mode, the output table contains summary statistics of 
//     the input inData.
//   * When in Assess mode, the output table is a list of input rows that don't 
//     fit (or fit) the model to within some amount specified by the filter input 
//     parameters.
//
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this class.

#ifndef __vtkStatisticsAlgorithm_h
#define __vtkStatisticsAlgorithm_h

#include "vtkTableAlgorithm.h"

class vtkVariantArray;

class VTK_INFOVIS_EXPORT vtkStatisticsAlgorithm : public vtkTableAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkStatisticsAlgorithm, vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // A convenience method for setting the statistics table input.
  // NB: This is mainly for the benefit of the VTK client/server 
  // layer, vanilla VTKcode should use, e.g:
  //
  // stats_algo2->SetInputConnection( 1, stats_algo1->output() );
  //
  virtual void SetInputStatisticsConnection( vtkAlgorithmOutput* );

  // Description:
  // Set the sample size.
  vtkSetMacro( SampleSize, vtkIdType );

  // Description:
  // Get the sample size.
  vtkGetMacro( SampleSize, vtkIdType );

  // Description:
  // Set the Learn option.
  vtkSetMacro( Learn, bool );

  // Description:
  // Get the Learn option.
  vtkGetMacro( Learn, bool );

  // Description:
  // Set the Validate option.
  vtkSetMacro( Validate, bool );

  // Description:
  // Get the Validate option.
  vtkGetMacro( Validate, bool );

  // Description:
  // Set the Assess option.
  vtkSetMacro( Assess, bool );

  // Description:
  // Get the Assess option.
  vtkGetMacro( Assess, bool );

  // Description:
  // Set the assessment name.
  vtkSetStringMacro( AssessmentName );

  // Description:
  // Get the assessment name.
  vtkGetStringMacro( AssessmentName );

  vtkVariantArray* AssessParameters;

protected:
  vtkStatisticsAlgorithm();
  ~vtkStatisticsAlgorithm();

  virtual int FillInputPortInformation( int port, vtkInformation* info );
  virtual int FillOutputPortInformation( int port, vtkInformation* info );

  virtual int RequestData(
    vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector* );

  // Description:
  // Execute the required calculations in the specified execution modes
  virtual void ExecuteLearn( vtkTable*,
                             vtkTable* ) = 0;
  virtual void ExecuteAssess( vtkTable*,
                              vtkTable*,
                              vtkTable*,
                              vtkTable* ) = 0; 

  vtkIdType SampleSize;
  bool Learn;
  bool Validate;
  bool Assess;
  char* AssessmentName;

private:
  vtkStatisticsAlgorithm(const vtkStatisticsAlgorithm&); // Not implemented
  void operator=(const vtkStatisticsAlgorithm&);   // Not implemented
};

#endif

