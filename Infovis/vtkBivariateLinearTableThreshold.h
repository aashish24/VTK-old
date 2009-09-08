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
  Copyright 2009 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
#ifndef __vtkBivariateLinearTableThreshold__h
#define __vtkBivariateLinearTableThreshold__h

#include "vtkTableAlgorithm.h"
#include "vtkSmartPointer.h"  //Required for smart pointer internal ivars

class vtkDataArrayCollection;
class vtkDoubleArray;
class vtkIdTypeArray;
class vtkTable;

class VTK_INFOVIS_EXPORT vtkBivariateLinearTableThreshold : public vtkTableAlgorithm
{
public:
  static vtkBivariateLinearTableThreshold* New();
  vtkTypeRevisionMacro(vtkBivariateLinearTableThreshold, vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(Inclusive,int);
  vtkGetMacro(Inclusive,int);

  void AddColumnToThreshold(vtkIdType column, vtkIdType component);
  int GetNumberOfColumnsToThreshold();
  void GetColumnToThreshold(vtkIdType idx, vtkIdType& column, vtkIdType& component);
  void ClearColumnsToThreshold();

  vtkIdTypeArray* GetSelectedRowIds(int selection=0);

  //BTX
  enum OutputPorts
  {
    OUTPUT_ROW_IDS=0,
    OUTPUT_ROW_DATA
  };
  enum LinearThresholdType
  {
    BLT_ABOVE=0,
    BLT_BELOW,
    BLT_NEAR,
    BLT_BETWEEN
  };
  //ETX
  
  void Initialize();

  void AddLineEquation(double* p1, double* p2);
  void AddLineEquation(double* p, double slope);
  void AddLineEquation(double a, double b, double c);
  void ClearLineEquations();

  vtkGetMacro(LinearThresholdType,int);
  vtkSetMacro(LinearThresholdType,int);
  void SetLinearThresholdTypeToAbove() { this->SetLinearThresholdType(vtkBivariateLinearTableThreshold::BLT_ABOVE); }
  void SetLinearThresholdTypeToBelow() { this->SetLinearThresholdType(vtkBivariateLinearTableThreshold::BLT_BELOW); }
  void SetLinearThresholdTypeToNear() { this->SetLinearThresholdType(vtkBivariateLinearTableThreshold::BLT_NEAR); }
  void SetLinearThresholdTypeToBetween() { this->SetLinearThresholdType(vtkBivariateLinearTableThreshold::BLT_BETWEEN); }

  vtkSetVector2Macro(ColumnRanges,double);
  vtkGetVector2Macro(ColumnRanges,double);

  vtkSetMacro(DistanceThreshold,double);
  vtkGetMacro(DistanceThreshold,double);

  vtkSetMacro(UseNormalizedDistance,int);
  vtkGetMacro(UseNormalizedDistance,int);
  vtkBooleanMacro(UseNormalizedDistance,int);

  static void ComputeImplicitLineFunction(double* p1, double* p2, double* abc);
  static void ComputeImplicitLineFunction(double* p, double slope, double* abc);
  
protected:
  vtkBivariateLinearTableThreshold();
  virtual ~vtkBivariateLinearTableThreshold();

  double ColumnRanges[2];
  double DistanceThreshold;
  int Inclusive;
  int LinearThresholdType;
  int NumberOfLineEquations;
  int UseNormalizedDistance;

  //BTX
  vtkSmartPointer<vtkDoubleArray> LineEquations;
  class Internals;
  Internals* Implementation;
  //ETX

  virtual int RequestData(
    vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector*);

  virtual int FillInputPortInformation( int port, vtkInformation* info );
  virtual int FillOutputPortInformation( int port, vtkInformation* info );

  virtual int ApplyThreshold(vtkTable* tableToThreshold, vtkIdTypeArray* acceptedIds);

  int ThresholdAbove(double x, double y);
  int ThresholdBelow(double x, double y);
  int ThresholdNear(double x, double y);
  int ThresholdBetween(double x, double y);
private:
  vtkBivariateLinearTableThreshold(const vtkBivariateLinearTableThreshold&); // Not implemented
  void operator=(const vtkBivariateLinearTableThreshold&); // Not implemented
};

#endif
