/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


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
#include "vtkFeatureEdges.h"
#include "vtkMath.h"
#include "vtkPolygon.h"
#include "vtkNormals.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkFeatureEdges* vtkFeatureEdges::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFeatureEdges");
  if(ret)
    {
    return (vtkFeatureEdges*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkFeatureEdges;
}




// Construct object with feature angle = 30; all types of edges extracted
// and colored.
vtkFeatureEdges::vtkFeatureEdges()
{
  this->FeatureAngle = 30.0;
  this->BoundaryEdges = 1;
  this->FeatureEdges = 1;
  this->NonManifoldEdges = 1;
  this->ManifoldEdges = 0;
  this->Coloring = 1;
  this->Locator = NULL;
}

vtkFeatureEdges::~vtkFeatureEdges()
{
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
}

// Generate feature edges for mesh
void vtkFeatureEdges::Execute()
{
  vtkPolyData *input= this->GetInput();
  vtkPoints *inPts;
  vtkPoints *newPts;
  vtkScalars *newScalars;
  vtkCellArray *newLines;
  vtkPolyData *Mesh;
  int i, j, numNei, cellId;
  int numBEdges, numNonManifoldEdges, numFedges, numManifoldEdges;
  float scalar, n[3], x1[3], x2[3];
  float cosAngle = 0;
  int lineIds[2];
  int npts, *pts;
  vtkCellArray *inPolys;
  vtkNormals *polyNormals = NULL;
  int numPts, numCells, nei;
  vtkIdList *neighbors;
  int p1, p2, newId;
  vtkPolyData *output = this->GetOutput();
  vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outCD=output->GetCellData();
  
  vtkDebugMacro(<<"Executing feature edges");

  //  Check input
  //
  inPts=input->GetPoints();
  inPolys=input->GetPolys();
  if ( (numPts=input->GetNumberOfPoints()) < 1 || inPts == NULL ||
       inPolys == NULL )
    {
    vtkErrorMacro(<<"No input data!");
    return;
    }
  numCells = input->GetNumberOfCells();

  if ( !this->BoundaryEdges && !this->NonManifoldEdges && 
  !this->FeatureEdges && !this->ManifoldEdges )
    {
    vtkWarningMacro(<<"All edge types turned off!");
    return;
    }

  // build cell structure.  Only operate with polygons.
  Mesh = vtkPolyData::New();
  Mesh->SetPoints(inPts);
  Mesh->SetPolys(inPolys);
  Mesh->BuildLinks();

  // Allocate storage for lines/points (arbitrary allocations size)
  //
  newPts = vtkPoints::New();
  newPts->Allocate(numPts/10,numPts); 
  newLines = vtkCellArray::New();
  newLines->Allocate(numPts/10);
  if ( this->Coloring )
    {
    newScalars = vtkScalars::New();
    newScalars->Allocate(numCells/10,numCells);
    outCD->CopyScalarsOff();
    }

  outPD->CopyAllocate(pd, numPts);
  outCD->CopyAllocate(cd, numCells);

  // Get our locator for merging points
  //
  if ( this->Locator == NULL )
    {
    this->CreateDefaultLocator();
    }
  this->Locator->InitPointInsertion (newPts, input->GetBounds());

  // Loop over all polygons generating boundary, non-manifold, 
  // and feature edges
  //
  if ( this->FeatureEdges ) 
    {    
    polyNormals = vtkNormals::New();
    polyNormals->Allocate(inPolys->GetNumberOfCells());

    for (cellId=0, inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); 
    cellId++)
      {
      vtkPolygon::ComputeNormal(inPts,npts,pts,n);
      polyNormals->InsertNormal(cellId,n);
      }

    cosAngle = cos ((double) vtkMath::DegreesToRadians() * this->FeatureAngle);
    }

  neighbors = vtkIdList::New();
  neighbors->Allocate(VTK_CELL_SIZE);

  numBEdges = numNonManifoldEdges = numFedges = numManifoldEdges = 0;
  for (cellId=0, inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); 
  cellId++)
    {
    if ( ! (cellId % 10000) ) //manage progress reports / early abort
      {
      this->UpdateProgress ((float)cellId / numCells);
      if ( this->GetAbortExecute() ) 
        {
        break;
        }
      }

    for (i=0; i < npts; i++) 
      {
      p1 = pts[i];
      p2 = pts[(i+1)%npts];

      Mesh->GetCellEdgeNeighbors(cellId,p1,p2, neighbors);
      numNei = neighbors->GetNumberOfIds();

      if ( this->BoundaryEdges && numNei < 1 )
        {
        numBEdges++;
        scalar = 0.0;
        }

      else if ( this->NonManifoldEdges && numNei > 1 )
        {
        // check to make sure that this edge hasn't been created before
        for (j=0; j < numNei; j++)
	  {
          if ( neighbors->GetId(j) < cellId )
	    {
            break;
	    }
	  }
        if ( j >= numNei )
          {
          numNonManifoldEdges++;
          scalar = 0.222222;
          }
        else
	  {
	  continue;
	  }
        }
      else if ( this->FeatureEdges && 
                numNei == 1 && (nei=neighbors->GetId(0)) > cellId ) 
        {
        if ( vtkMath::Dot(polyNormals->GetNormal(nei),
                          polyNormals->GetNormal(cellId)) <= cosAngle ) 
          {
          numFedges++;
          scalar = 0.444444;
          }
        else
	  {
	  continue;
	  }
        }
      else if ( this->ManifoldEdges )
        {
        numManifoldEdges++;
        scalar = 0.666667;
        }
      else
	{
	continue;
	}

      // Add edge to output
      Mesh->GetPoint(p1, x1);
      Mesh->GetPoint(p2, x2);

      if ( this->Locator->InsertUniquePoint(x1, lineIds[0]) )
        {
        outPD->CopyData (pd,p1,lineIds[0]);
        }
      
      if ( this->Locator->InsertUniquePoint(x2, lineIds[1]) )
        {
        outPD->CopyData (pd,p2,lineIds[1]);
        }

      newId = newLines->InsertNextCell(2,lineIds);
      outCD->CopyData (cd,cellId,newId);
      if ( this->Coloring )
	{
	newScalars->InsertScalar(newId, scalar);
	}
      }
    }

  vtkDebugMacro(<<"Created " << numBEdges << " boundary edges, "
                << numNonManifoldEdges << " non-manifold edges, "
                << numFedges << " feature edges, "
                << numManifoldEdges << " manifold edges");

  //  Update ourselves.
  //
  if ( this->FeatureEdges )
    {
    polyNormals->Delete();
    }

  Mesh->Delete();
  
  output->SetPoints(newPts);
  newPts->Delete();
  neighbors->Delete();

  output->SetLines(newLines);
  newLines->Delete();

  if ( this->Coloring )
    {
    outCD->SetScalars(newScalars);
    newScalars->Delete();
    }
}

void vtkFeatureEdges::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    }
}

// Specify a spatial locator for merging points. By
// default an instance of vtkMergePoints is used.
void vtkFeatureEdges::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator == locator ) 
    {
    return;
    }
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  if ( locator )
    {
    locator->Register(this);
    }
  this->Locator = locator;
  this->Modified();
}

unsigned long int vtkFeatureEdges::GetMTime()
{
  unsigned long mTime=this-> vtkPolyDataToPolyDataFilter::GetMTime();
  unsigned long time;

  if ( this->Locator != NULL )
    {
    time = this->Locator->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }
  return mTime;
}

void vtkFeatureEdges::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);

  os << indent << "Feature Angle: " << this->FeatureAngle << "\n";
  os << indent << "Boundary Edges: " << (this->BoundaryEdges ? "On\n" : "Off\n");
  os << indent << "Feature Edges: " << (this->FeatureEdges ? "On\n" : "Off\n"); 
  os << indent << "Non-Manifold Edges: " << (this->NonManifoldEdges ? "On\n" : "Off\n");
  os << indent << "Manifold Edges: " << (this->ManifoldEdges ? "On\n" : "Off\n");
  os << indent << "Coloring: " << (this->Coloring ? "On\n" : "Off\n");

  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }
}

