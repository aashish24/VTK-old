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
#include "Scalars.hh"
#include "FScalars.hh"
#include "IdList.hh"

vlScalars::vlScalars()
{
  this->Range[0] = 0.0;
  this->Range[1] = 1.0;
}

// Description:
// Given a list of pt ids, return an array of scalar values.
void vlScalars::GetScalars(vlIdList& ptId, vlFloatScalars& fs)
{
  for (int i=0; i<ptId.GetNumberOfIds(); i++)
    {
    fs.InsertScalar(i,this->GetScalar(ptId.GetId(i)));
    }
}

// Description:
// Determine (rmin,rmax) range of scalar values.
void vlScalars::ComputeRange()
{
  int i;
  float s;

  if ( this->GetMTime() > this->ComputeTime )
    {
    this->Range[0] =  LARGE_FLOAT;
    this->Range[1] =  -LARGE_FLOAT;
    for (i=0; i<this->GetNumberOfScalars(); i++)
      {
      s = this->GetScalar(i);
      if ( s < this->Range[0] ) this->Range[0] = s;
      if ( s > this->Range[1] ) this->Range[1] = s;
      }

    this->ComputeTime.Modified();
    }
}

// Description:
// Return the range of scalar values.
float *vlScalars::GetRange()
{
  this->ComputeRange();
  return this->Range;
}

void vlScalars::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlScalars::GetClassName()))
    {
    float *range;

    vlRefCount::PrintSelf(os,indent);

    os << indent << "Number Of Scalars: " << this->GetNumberOfScalars() << "\n";
    range = this->GetRange();
    os << indent << "Range: (" << range[0] << ", " << range[1] << ")\n";
    }
}
