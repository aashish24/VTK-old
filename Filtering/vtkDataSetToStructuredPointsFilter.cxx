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
#include "DS2SPtsF.hh"

void vlDataSetToStructuredPointsFilter::Modified()
{
  this->vlStructuredPoints::Modified();
  this->vlDataSetFilter::_Modified();
}

unsigned long int vlDataSetToStructuredPointsFilter::GetMTime()
{
  unsigned long dtime = this->vlStructuredPoints::GetMTime();
  unsigned long ftime = this->vlDataSetFilter::_GetMTime();
  return (dtime > ftime ? dtime : ftime);
}

void  vlDataSetToStructuredPointsFilter::Update()
{
  this->UpdateFilter();
}

void vlDataSetToStructuredPointsFilter::PrintSelf(ostream& os, vlIndent indent)
{
  vlStructuredPoints::PrintSelf(os,indent);
  vlDataSetFilter::_PrintSelf(os,indent);
}
