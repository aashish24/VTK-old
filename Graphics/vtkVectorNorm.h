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
// Generate scalars from euclidean norm of vectors
//
#ifndef __vlVectorNorm_h
#define __vlVectorNorm_h

#include "DS2DSF.hh"

class vlVectorNorm : public vlDataSetToDataSetFilter 
{
public:
  vlVectorNorm();
  char *GetClassName() {return "vlVectorNorm";};
  void PrintSelf(ostream& os, vlIndent indent);

  vlSetMacro(Normalize,int);
  vlGetMacro(Normalize,int);
  vlBooleanMacro(Normalize,int);

protected:
  void Execute();
  int Normalize;  // normalize 0<=n<=1 if true.
};

#endif


