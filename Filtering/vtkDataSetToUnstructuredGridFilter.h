/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME vtkDataSetToUnstructuredGridFilter - abstract filter class
// .SECTION Description
// vtkDataSetToUnstructuredGridFilter is an abstract filter class whose 
// subclasses take as input any dataset and generate an unstructured
// grid on output.

#ifndef __vtkDataSetToUnstructuredGridFilter_h
#define __vtkDataSetToUnstructuredGridFilter_h

#include "DataSetF.hh"
#include "UGrid.hh"

class vtkDataSetToUnstructuredGridFilter : public vtkUnstructuredGrid, public vtkDataSetFilter
{
public:
  char *GetClassName() {return "vtkDataSetToUnstructuredGridFilter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Object interface
  void Modified();
  unsigned long int GetMTime();
  unsigned long int _GetMTime() {return this->GetMTime();};
  void DebugOn();
  void DebugOff();

  //DataSet interface
  void Update();

protected:
  //Filter interface
  int GetDataReleased();
  void SetDataReleased(int flag);
};

#endif


