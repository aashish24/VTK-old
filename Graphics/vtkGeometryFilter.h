/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME vlGeometryFilter - extract geometry from data
// .SECTION Description
// vlGeometryFilter is a filter to extract geometry (and associated data)
// from a dataset. Geometry is obtained as follows: all 0D, 1D, and 2D cells
// are extracted. All 2D faces that are used by only one 3D cell (i.e., 
// boundary faces) are extracted. It is also possible to specify conditions
// on point ids, cell ids, and on bounding box to control the extraction 
// process.

#ifndef __vlGeometryFilter_h
#define __vlGeometryFilter_h

#include "DS2PolyF.hh"

class vlGeometryFilter : public vlDataSetToPolyFilter
{
public:
  vlGeometryFilter();
  ~vlGeometryFilter() {};
  char *GetClassName() {return "vlGeometryFilter";};
  void PrintSelf(ostream& os, vlIndent indent);

  // Description:
  // Turn on/off selection of geometry by point id.
  vlSetMacro(PointClipping,int);
  vlGetMacro(PointClipping,int);
  vlBooleanMacro(PointClipping,int);

  // Description:
  // Turn on/off selection of geometry by cell id.
  vlSetMacro(CellClipping,int);
  vlGetMacro(CellClipping,int);
  vlBooleanMacro(CellClipping,int);

  // Description:
  // Turn on/off selection of geometry via bounding box.
  vlSetMacro(ExtentClipping,int);
  vlGetMacro(ExtentClipping,int);
  vlBooleanMacro(ExtentClipping,int);

  // Description:
  // Specify the minimum point id for point id selection.
  vlSetClampMacro(PointMinimum,int,0,LARGE_INTEGER);
  vlGetMacro(PointMinimum,int);

  // Description:
  // Specify the maximum point id for point id selection.
  vlSetClampMacro(PointMaximum,int,0,LARGE_INTEGER);
  vlGetMacro(PointMaximum,int);

  // Description:
  // Specify the minimum cell id for point id selection.
  vlSetClampMacro(CellMinimum,int,0,LARGE_INTEGER);
  vlGetMacro(CellMinimum,int);

  // Description:
  // Specify the maximum cell id for point id selection.
  vlSetClampMacro(CellMaximum,int,0,LARGE_INTEGER);
  vlGetMacro(CellMaximum,int);

  void SetExtent(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
  void SetExtent(float *extent);
  float *GetExtent() { return this->Extent;};

protected:
  void Execute();
  int PointMinimum;
  int PointMaximum;
  int CellMinimum;
  int CellMaximum;
  float Extent[6];
  int PointClipping;
  int CellClipping;
  int ExtentClipping;

};

#endif


