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
#include "SPt2Poly.hh"

void vlStructuredPointsToPolyDataFilter::Modified()
{
  this->vlPolyData::Modified();
  this->vlStructuredPointsFilter::_Modified();
}

unsigned long int vlStructuredPointsToPolyDataFilter::GetMTime()
{
  unsigned long dtime = this->vlPolyData::GetMTime();
  unsigned long ftime = this->vlStructuredPointsFilter::_GetMTime();
  return (dtime > ftime ? dtime : ftime);
}

void vlStructuredPointsToPolyDataFilter::DebugOn()
{
  vlPolyData::DebugOn();
  vlStructuredPointsFilter::_DebugOn();
}

void vlStructuredPointsToPolyDataFilter::DebugOff()
{
  vlPolyData::DebugOff();
  vlStructuredPointsFilter::_DebugOff();
}

void vlStructuredPointsToPolyDataFilter::Update()
{
  this->UpdateFilter();
}

int vlStructuredPointsToPolyDataFilter::GetDataReleased()
{
  return this->DataReleased;
}

void vlStructuredPointsToPolyDataFilter::SetDataReleased(int flag)
{
  this->DataReleased = flag;
}

void vlStructuredPointsToPolyDataFilter::PrintSelf(ostream& os, vlIndent indent)
{
  vlPolyData::PrintSelf(os,indent);
  vlStructuredPointsFilter::_PrintSelf(os,indent);
}
