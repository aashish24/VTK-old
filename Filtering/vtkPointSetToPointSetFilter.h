/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Description:
---------------------------------------------------------------------------
This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
//
// PointSetToPointSetFilter are filters that take PointSet in and 
// generate PointSet as output.
//
#ifndef __vlPointSetToPointSetFilter_h
#define __vlPointSetToPointSetFilter_h

#include "PtSetF.hh"
#include "PointSet.hh"

class vlPointSetToPointSetFilter : public vlPointSet, public vlPointSetFilter
{
public:
  vlPointSetToPointSetFilter();
  ~vlPointSetToPointSetFilter();
  char *GetClassName() {return "vlPointSetToPointSetFilter";};
  char *GetDataType() {return this->PointSet->GetDataType();};
  void PrintSelf(ostream& os, vlIndent indent);

  // dataset interface
  vlDataSet *MakeObject() {return this->PointSet->MakeObject();};
  int GetNumberOfCells() {return this->PointSet->GetNumberOfCells();}
  int GetNumberOfPoints() {return this->PointSet->GetNumberOfPoints();}
  float *GetPoint(int i) {return this->PointSet->GetPoint(i);}
  vlCell *GetCell(int cellId) {return this->PointSet->GetCell(cellId);}
  void Initialize();

  void ComputeBounds() {this->PointSet->ComputeBounds();};
  void Update();

protected:
  vlDataSet *PointSet;

};

#endif


