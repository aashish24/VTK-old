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
#include "SGridSrc.hh"

void vlStructuredGridSource::Update()
{
  vlSource::Update();
}

void vlStructuredGridSource::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlStructuredGridSource::GetClassName()))
    {
    this->PrintWatchOn(); // watch for multiple inheritance
    
    vlStructuredGrid::PrintSelf(os,indent);
    vlSource::PrintSelf(os,indent);
    
    this->PrintWatchOff(); // stop worrying about it now
    }
}
