/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its 
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
//
// Contours arbitrary input
//
#ifndef __vlContourFilter_h
#define __vlContourFilter_h

#include "DS2PolyF.hh"

class vlContourFilter : public vlDataSetToPolyFilter
{
public:
  vlContourFilter(float value=0.0) {this->Value = value;};
  ~vlContourFilter() {};
  char *GetClassName() {return "vlContourFilter";};

  vlSetMacro(Value,float);
  vlGetMacro(Value,float);

protected:
  void Execute();
  float Value;
};

#endif


