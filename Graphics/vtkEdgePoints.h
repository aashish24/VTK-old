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
// .NAME vlEdgePoints - generate points on iso-surface
// .SECTION Description
// vlEdgePoints is a filter that takes as input any dataset and 
// generates on output points that lie on an iso-surface. The points are
// created by interpolation along cells whose end-points are below and 
// above the contour value.
//    vlEdgePoints can be considered a "poor man's" dividing cubes algorithm.
// Points are generated only on the edges of cells, not in the interior.

#ifndef __vlEdgePoints_h
#define __vlEdgePoints_h

#include "DS2PolyF.hh"

#define MAX_CONTOURS 256

class vlEdgePoints : public vlDataSetToPolyFilter
{
public:
  vlEdgePoints();
  ~vlEdgePoints();
  char *GetClassName() {return "vlEdgePoints";};
  void PrintSelf(ostream& os, vlIndent indent);

  // Description:
  // Set/get the contour value.
  vlSetMacro(Value,float);
  vlGetMacro(Value,float);

protected:
  void Execute();

  float Value;
};

#endif


