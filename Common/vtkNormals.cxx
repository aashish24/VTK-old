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
#include "Normals.hh"
#include "IdList.hh"
#include "FNormals.hh"

// Description:
// Given a list of pt ids, return an array of corresponding normals.
void vlNormals::GetNormals(vlIdList& ptId, vlFloatNormals& fp)
{
  for (int i=0; i<ptId.GetNumberOfIds(); i++)
    {
    fp.InsertNormal(i,this->GetNormal(ptId.GetId(i)));
    }
}

void vlNormals::PrintSelf(ostream& os, vlIndent indent)
{
  vlRefCount::PrintSelf(os,indent);

  os << indent << "Number Of Normals: " << this->GetNumberOfNormals() << "\n";
}
