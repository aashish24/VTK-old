/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Scott Hill for implementing this class


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkDepthSortPolyData.h"
#include "vtkMath.h"
#include "vtkUnsignedIntArray.h"
#include "vtkObjectFactory.h"

//---------------------------------------------------------------------------
vtkDepthSortPolyData* vtkDepthSortPolyData::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDepthSortPolyData");
  if(ret)
    {
    return (vtkDepthSortPolyData*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkDepthSortPolyData;
}

vtkDepthSortPolyData::vtkDepthSortPolyData()
{
  this->Camera = NULL;
  this->Prop3D = NULL;
  this->Direction = VTK_DIRECTION_BACK_TO_FRONT;
  this->Vector[0] = this->Vector[1] = 0.0;
  this->Vector[2] = 0,0;
  this->Origin[0] = this->Origin[1] = this->Origin[2] = 0.0;
  this->Transform = vtkTransform::New();
  this->SortScalars = 0;
}

vtkDepthSortPolyData::~vtkDepthSortPolyData()
{
  if ( this->Camera )
    {
    this->Camera->Delete();
    }
  
  if ( this->Prop3D )
    {
    this->Prop3D->Delete();
    }
  this->Transform->Delete();
}

typedef struct _vtkSortValues {
  float z;
  int   cellId;
} vtkSortValues;

static int CompareBackToFront(const void *val1, const void *val2)
{
  if (((vtkSortValues *)val1)->z > ((vtkSortValues *)val2)->z)
    {
    return (-1);
    }
  else if (((vtkSortValues *)val1)->z < ((vtkSortValues *)val2)->z)
    {
    return (1);
    }
  else
    {
    return (0);
    }
}

static int CompareFrontToBack(const void *val1, const void *val2)
{
  if (((vtkSortValues *)val1)->z < ((vtkSortValues *)val2)->z)
    {
    return (-1);
    }
  else if (((vtkSortValues *)val1)->z > ((vtkSortValues *)val2)->z)
    {
    return (1);
    }
  else
    {
    return (0);
    }
}

void vtkDepthSortPolyData::Execute()
{
  vtkSortValues *depth;
  int cellId, id;
  vtkPolyData *input=this->GetInput();
  vtkPolyData *output=this->GetOutput();
  vtkGenericCell *cell=vtkGenericCell::New();
  int numCells=input->GetNumberOfCells();
  int numPts=input->GetNumberOfPoints();
  vtkCellData *inCD=input->GetCellData();
  vtkCellData *outCD=output->GetCellData();
  vtkScalars *sortScalars;
  unsigned int *scalars;
  double x[3];
  double vector[3];
  double origin[3];
  int type, npts, *pts, newId;

  // Initialize
  //
  vtkDebugMacro(<<"Sorting polygonal data");

  // Compute the sort vector
  if ( this->Direction == VTK_DIRECTION_SPECIFIED_VECTOR )
    {
    for (int i=0; i<3; i++)
      {
      vector[i] = this->Vector[i];
      origin[i] = this->Origin[i];
      }
    }
  else //compute view vector
    {
    if ( this->Camera == NULL)
      {
      vtkErrorMacro(<<"Need a camera to sort");
      return;
      }
  
    this->ComputeProjectionVector(vector, origin);
    }

  // Compute the depth value
  depth = new vtkSortValues [numCells];
  for ( cellId=0; cellId < numCells; cellId++ )
    {
    input->GetCell(cellId, cell);
    cell->Points->GetPoint(0,x);
    x[0] -= origin[0];
    x[1] -= origin[1];
    x[2] -= origin[2];
    depth[cellId].z = vtkMath::Dot(x,vector);
    depth[cellId].cellId = cellId;
    }

  // Sort the depths
  if ( this->Direction == VTK_DIRECTION_FRONT_TO_BACK )
    {
    qsort((void *)depth, numCells, sizeof(vtkSortValues), CompareFrontToBack);
    }
  else
    {
    qsort((void *)depth, numCells, sizeof(vtkSortValues), CompareBackToFront);
    }

  // Generate sorted output
  if ( this->SortScalars )
    {
    sortScalars = vtkScalars::New(VTK_UNSIGNED_INT);
    sortScalars->SetNumberOfScalars(numCells);
    scalars = ((vtkUnsignedIntArray *)sortScalars->GetData())->GetPointer(0);
    }
  output->Allocate(numCells);
  for ( cellId=0; cellId < numCells; cellId++ )
    {
    id = depth[cellId].cellId;
    input->GetCell(id, cell);
    type = cell->GetCellType();
    npts = cell->GetNumberOfPoints();
    pts = cell->GetPointIds()->GetPointer(0);

    // copy cell data
    newId = output->InsertNextCell(type, npts, pts);
    outCD->CopyData(inCD, id, newId);
    if ( this->SortScalars )
      {
      scalars[newId] = newId;
      }
    }

  // Points are left alone
  output->SetPoints(input->GetPoints());
  output->GetPointData()->PassData(input->GetPointData());
  if ( this->SortScalars )
    {
    output->GetCellData()->SetScalars(sortScalars);
    sortScalars->Delete();
    }

  // Clean up and get out    
  cell->Delete();
  output->Squeeze();
}

void vtkDepthSortPolyData::ComputeProjectionVector(double vector[3], 
                                                   double origin[3])
{
  double *focalPoint = this->Camera->GetFocalPoint();
  double *position = this->Camera->GetPosition();
 
  // If a camera is present, use it
  if ( !this->Prop3D )
    {
    for(int i=0; i<3; i++)
      { 
      vector[i] = focalPoint[i] - position[i];
      origin[i] = position[i];
      }
    }

  else  //Otherwise, use Prop3D
    {
    float focalPt[4], pos[4];
    int i;

    this->Transform->SetMatrix(*(this->Prop3D->GetMatrixPointer()));
    this->Transform->Push();
    this->Transform->Inverse();

    for(i=0; i<4; i++)
      {
      focalPt[i] = focalPoint[i];
      pos[i] = position[i];
      }

    this->Transform->TransformPoint(focalPt,focalPt);
    this->Transform->TransformPoint(pos,pos);

    for (i=0; i<3; i++) 
      {
      vector[i] = focalPt[i] - pos[i];
      origin[i] = pos[i];
      }
    this->Transform->Pop();
  }
}

unsigned long int vtkDepthSortPolyData::GetMTime()
{
  unsigned long mTime=this->vtkPolyDataToPolyDataFilter::GetMTime();
 
  if ( this->Direction != VTK_DIRECTION_SPECIFIED_VECTOR )
    {
    unsigned long time;
    if ( this->Camera != NULL )
      {
      time = this->Camera->GetMTime();
      mTime = ( time > mTime ? time : mTime );
      }

    if ( this->Prop3D != NULL )
      {
      time = this->Prop3D->GetMTime();
      mTime = ( time > mTime ? time : mTime );
      }
    }

  return mTime;
}

void vtkDepthSortPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);

  if ( this->Camera )
    {
    os << indent << "Camera:\n";
    this->Camera->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Camera: (none)\n";
    }

  if ( this->Prop3D )
    {
    os << indent << "Prop3D:\n";
    this->Prop3D->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Prop3D: (none)\n";
    }

  os << indent << "Direction: ";
  if ( this->Direction == VTK_DIRECTION_BACK_TO_FRONT )
    {
    os << "Back To Front" << endl;
    }
  else if ( this->Direction == VTK_DIRECTION_FRONT_TO_BACK )
    {
    os << "Front To Back";
    }
  else 
    {
    os << "Specified Direction: ";
    os << "(" << this->Vector[0] << ", " << this->Vector[1] << ", " 
       << this->Vector[2] << ")\n";
    os << "Specified Origin: ";
    os << "(" << this->Origin[0] << ", " << this->Origin[1] << ", " 
       << this->Origin[2] << ")\n";
    }
  
  os << indent << "Sort Scalars: " << (this->SortScalars ? "On\n" : "Off\n");
}
