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
#include "CellArr.hh"

vlCellArray::vlCellArray (const vlCellArray& ca)
{
  this->NumberOfCells = ca.NumberOfCells;
  this->Location = 0;
  this->Ia = ca.Ia;
}

// Description:
// Returns the size of the largest cell. The size is the number of points
// defining the cell.
int vlCellArray::GetMaxCellSize()
{
  int i, npts, maxSize=0;

  for (i=0; i<this->Ia.GetMaxId(); i+=npts+1)
    {
    if ( (npts=this->Ia.GetValue(i)) > maxSize )
      maxSize = npts;
    }
  return maxSize;
}
