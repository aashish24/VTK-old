/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkPointSet.h"
#include "vtkSource.h"

vtkPointSet::vtkPointSet ()
{
  this->Points = NULL;
  this->Locator = NULL;
}

vtkPointSet::vtkPointSet(const vtkPointSet& ps) :
vtkDataSet(ps)
{
  this->Points = ps.Points;
  if (this->Points)
    {
    this->Points->Register(this);
    }

  this->Locator = ps.Locator;
  if (this->Locator)
    {
    this->Locator->Register(this);
    }
}

vtkPointSet::~vtkPointSet ()
{
  this->Initialize();
  if ( this->Locator ) 
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
}

// Copy the geometric structure of an input point set object.
void vtkPointSet::CopyStructure(vtkDataSet *ds)
{
  vtkPointSet *ps=(vtkPointSet *)ds;
  this->Initialize();

  this->Points = ps->Points;
  if (this->Points)
    {
    this->Points->Register(this);
    }
}


void vtkPointSet::Initialize()
{
  vtkDataSet::Initialize();

  if ( this->Points ) 
    {
    this->Points->UnRegister(this);
    this->Points = NULL;
    }

  if ( this->Locator ) 
    {
    this->Locator->Initialize();
    }
}
void vtkPointSet::ComputeBounds()
{
  float *bounds;

  if ( this->Points )
    {
    bounds = this->Points->GetBounds();
    for (int i=0; i<6; i++)
      {
      this->Bounds[i] = bounds[i];
      }
    this->ComputeTime.Modified();
    }
}

unsigned long int vtkPointSet::GetMTime()
{
  unsigned long int dsTime = vtkDataSet::GetMTime();

  if ( this->Points ) 
    {
    if ( this->Points->GetMTime() > dsTime )
      {
      dsTime = this->Points->GetMTime();
      }
    }

  // don't get locator's mtime because its an internal object that cannot be 
  // modified directly from outside. Causes problems due to FindCell() 
  // SetPoints() method.

  return dsTime;
}

int vtkPointSet::FindPoint(float x[3])
{
  if ( !this->Points )
    {
    return -1;
    }

  if ( !this->Locator )
    {
    this->Locator = vtkPointLocator::New();
    this->Locator->SetDataSet(this);
    }

  if ( this->Points->GetMTime() > this->Locator->GetMTime() )
    {
    this->Locator->SetDataSet(this);
    }

  return this->Locator->FindClosestPoint(x);
}

//the furthest the walk can be - prevents aimless wandering
#define VTK_MAX_WALK 12

int vtkPointSet::FindCell(float x[3], vtkCell *cell, int cellId, float tol2, 
                          int& subId, float pcoords[3], float *weights)
{
  int ptId, walk;
  float closestPoint[3];
  float dist2;
  vtkIdList *cellIds, *ptIds;

  // make sure everything is up to snuff
  if ( !this->Points )
    {
    return -1;
    }

  cellIds = new vtkIdList(8,100);
  ptIds = new vtkIdList(8,100);

  if ( !this->Locator )
    {
    this->Locator = vtkPointLocator::New();
    this->Locator->SetDataSet(this);
    }

  if ( this->Points->GetMTime() > this->Locator->GetMTime() )
    {
    this->Locator->SetDataSet(this);
    }

  // If we don't have a starting cell, we'll have to find one. Find
  // the closest point to the input position, then get the cells that use
  // the point.  Then use one of the cells to begin the walking process.
  if ( ! cell )
    {
    ptId = this->Locator->FindClosestPoint(x);
    if ( ptId < 0 )
      {
      cellIds->Delete();
      ptIds->Delete();
      return (-1); //if point completely outside of data
      }

    this->GetPointCells(ptId, cellIds);
    if ( cellIds->GetNumberOfIds() > 0 )
      {
      cellId = cellIds->GetId(0); //arbitrarily use first cell in list
      cell = this->GetCell(cellId);
  
      // See whether this randomly choosen cell contains the point      
      if ( cell->EvaluatePosition(x,closestPoint,subId,pcoords,dist2,weights) == 1
      && dist2 <= tol2 )
        {
	cellIds->Delete();
	ptIds->Delete();  
        return cellId;
        }
      }
    }

  // If a cell is supplied, or we were able to find a starting cell (in the
  // previous chunk of code), then we use this to start our search. A
  // walking scheme is used, where we walk towards the point and eventually
  // locate the cell that contains the point.
  if ( cell ) //we have a starting cell
    {
    for ( walk=0; walk < VTK_MAX_WALK; walk++ )
      {
      cell->CellBoundary(subId, pcoords, ptIds);
      this->GetCellNeighbors(cellId, ptIds, cellIds);
      if ( cellIds->GetNumberOfIds() > 0 )
        {
        cellId = cellIds->GetId(0);
        cell = this->GetCell(cellId);
        }
      else
        {
        break; //outside of data
        }

      if ( cell->EvaluatePosition(x,closestPoint,subId,pcoords,dist2,weights) == 1
      && dist2 <= tol2 )
        {
	cellIds->Delete();
	ptIds->Delete();  
        return cellId;
        }

      }//for a walk
    }

  cellIds->Delete();
  ptIds->Delete();
  
  return -1;
}
#undef VTK_MAX_WALK


void vtkPointSet::Squeeze()
{
  if ( this->Points )
    {
    this->Points->Squeeze();
    }
  vtkDataSet::Squeeze();
}

void vtkPointSet::UnRegister(vtkObject *o)
{
  // detect the circular loop source <-> data
  // If we have two references and one of them is my data
  // and I am not being unregistered by my data, break the loop.
  if (this->ReferenceCount == 2 && this->Source != NULL &&
      this->Source->GetOutputAsDataObject() == this && 
      o != this->Source &&
      this->Source->GetReferenceCount() == 1)
    {
    this->SetSource(NULL);
    }
  // detect the circular loop PointSet <-> Locator
  // If we have two references and one of them is my locator
  // and I am not being unregistered by my locator, break the loop.
  if (this->ReferenceCount == 2 && this->Locator &&
      this->Locator->GetDataSet() == this && 
      this->Locator != o)
    {
    this->Locator->SetDataSet(NULL);
    }
  // catch the case when both of the above are true
  if (this->ReferenceCount == 3 && this->Source != NULL &&
      this->Source->GetOutputAsDataObject() == this && 
      o != this->Source &&
      this->Source->GetReferenceCount() == 1 &&
      this->Locator &&
      this->Locator->GetDataSet() == this && 
      this->Locator != o)
    {
    this->SetSource(NULL);
    // the locater will get freed by a recursive call
    }  
  
  this->vtkObject::UnRegister(o);
}



void vtkPointSet::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSet::PrintSelf(os,indent);

  os << indent << "Number Of Points: " << this->GetNumberOfPoints() << "\n";
  os << indent << "Point Coordinates: " << this->Points << "\n";
  os << indent << "Locator: " << this->Locator << "\n";
}

