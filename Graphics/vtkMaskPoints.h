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
// .NAME vlMaskPoints - selectively filter points
// .SECTION Description
// vlMaskPoints is a filter that passes through points and point attributes 
// from input dataset. (Other geometry is not passed through). It is 
// possible to mask every nth point, and to specify an initial offset
// to begin masking from.

#ifndef __vlMaskPoints_h
#define __vlMaskPoints_h

#include "DS2PolyF.hh"

class vlMaskPoints : public vlDataSetToPolyFilter
{
public:
  vlMaskPoints():OnRatio(2),Offset(0),RandomMode(0) {};
  ~vlMaskPoints() {};
  char *GetClassName() {return "vlMaskPoints";};
  void PrintSelf(ostream& os, vlIndent indent);

  // Description:
  // Turn on every nth point.
  vlSetClampMacro(OnRatio,int,1,LARGE_INTEGER);
  vlGetMacro(OnRatio,int);

  // Description:
  // Start with this point.
  vlSetClampMacro(Offset,int,0,LARGE_INTEGER);
  vlGetMacro(Offset,int);

  // Description:
  // Special flag causes randomization of point selection. If this mode is on,
  // statitically every nth point (i.e., OnRatio) will be displayed.
  vlSetMacro(RandomMode,int);
  vlGetMacro(RandomMode,int);
  vlBooleanMacro(RandomMode,int);

protected:
  void Execute();

  int OnRatio;     // every OnRatio point is on; all others are off.
  int Offset;      // offset (or starting point id)
  int RandomMode;  // turn on/off randomization
};

#endif


