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
#include "SPtsSrc.hh"

void vlStructuredPointsSource::Modified()
{
  this->vlStructuredPoints::Modified();
  this->vlSource::_Modified();
}

unsigned long int vlStructuredPointsSource::GetMTime()
{
  unsigned long dtime = this->vlStructuredPoints::GetMTime();
  unsigned long ftime = this->vlSource::_GetMTime();
  return (dtime > ftime ? dtime : ftime);
}

void vlStructuredPointsSource::Update()
{
  this->UpdateFilter();
}

void vlStructuredPointsSource::PrintSelf(ostream& os, vlIndent indent)
{
  vlStructuredPoints::PrintSelf(os,indent);
  vlSource::_PrintSelf(os,indent);
}
