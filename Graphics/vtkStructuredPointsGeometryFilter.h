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
// .NAME vlStructuredPointsGeometryFilter - extract geometry for structured points
// .SECTION Description
// vlStructuredPointsGeometryFilter is a filter that extracts geometry from a
// structured points. By specifying appropriate i-j-k indices, it is possible
// to extract a point, a curve, a surface, or a "volume". Depending upon the
// type of data, the curve and surface may be curved or planar. The volume
// is actually a (n x m x o) region of points.

#ifndef __vlStructuredPointsGeometryFilter_h
#define __vlStructuredPointsGeometryFilter_h

#include "SPt2Poly.hh"

class vlStructuredPointsGeometryFilter : public vlStructuredPointsToPolyDataFilter
{
public:
  vlStructuredPointsGeometryFilter();
  ~vlStructuredPointsGeometryFilter() {};
  char *GetClassName() {return "vlStructuredPointsGeometryFilter";};
  void PrintSelf(ostream& os, vlIndent indent);

  void SetExtent(int iMin, int iMax, int jMin, int jMax, int kMin, int kMax);
  void SetExtent(int *extent);
  int *GetExtent() { return this->Extent;};

protected:
  void Execute();
  int Extent[6];
};

#endif


