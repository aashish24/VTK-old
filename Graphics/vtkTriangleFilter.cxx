/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.


=========================================================================*/
#include "TriF.hh"
#include "Polygon.hh"

void vtkTriangleFilter::Execute()
{
  vtkPolyData *input=(vtkPolyData *)this->Input;
  vtkCellArray *inPolys=input->GetPolys();
  vtkCellArray *inStrips=input->GetStrips();;
  int npts, *pts;
  vtkCellArray *newPolys;
  int numCells;
  int p1, p2, p3;
  vtkPolygon poly;
  int i, j;
  vtkIdList outVerts(3*MAX_CELL_SIZE);
  vtkPoints *inPoints=input->GetPoints();
  vtkPointData *pd;

  vtkDebugMacro(<<"Executing triangle filter");
  this->Initialize();

  newPolys = new vtkCellArray();
  // approximation
  numCells = input->GetNumberOfPolys() + input->GetNumberOfStrips();
  newPolys->Allocate(newPolys->EstimateSize(numCells,3),3*numCells);

  // pass through triangles; triangulate polygons if necessary
  for (inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); )
    {
    if ( npts == 3 )
      {
      newPolys->InsertNextCell(npts,pts);
      }
    else if ( npts > 3 ) // triangulate poly
      {
      poly.Initialize(npts,pts,inPoints);
      poly.Triangulate(outVerts);
      for (i=0; i<outVerts.GetNumberOfIds()/3; i++)
        {
        newPolys->InsertNextCell(3);
        for (j=0; j<3; j++)
          newPolys->InsertCellPoint(outVerts.GetId(3*i+j));
        }
      }
    }

  for (inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); )
    {
    p1 = pts[0];
    p2 = pts[1];
    p3 = pts[2];
    for (i=0; i<(npts-2); i++)
      {
      newPolys->InsertNextCell(3);
      if ( (i % 2) ) // flip ordering to preserve consistency
        {
        newPolys->InsertCellPoint(p2);
        newPolys->InsertCellPoint(p1);
        newPolys->InsertCellPoint(p3);
        }
      else
        {
        newPolys->InsertCellPoint(p1);
        newPolys->InsertCellPoint(p2);
        newPolys->InsertCellPoint(p3);
        }
      p1 = p2;
      p2 = p3;
      p3 = pts[3+i];
      }
    }
//
// Update ourselves
//
  newPolys->Squeeze();
  this->SetPolys(newPolys);
  newPolys->Delete();

  // pass through points and point data
  this->SetPoints(input->GetPoints());
  pd = input->GetPointData();
  this->PointData = *pd;

  // pass through other stuff if requested
  if ( this->PassVerts ) this->SetVerts(input->GetVerts());
  if ( this->PassLines ) this->SetLines(input->GetLines());

  vtkDebugMacro(<<"Converted " << inPolys->GetNumberOfCells() <<
               " polygons and " << inStrips->GetNumberOfCells() <<
               " strips to " << newPolys->GetNumberOfCells() <<
               " triangles");
}

void vtkTriangleFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyToPolyFilter::PrintSelf(os,indent);

  os << indent << "Pass Verts: " << (this->PassVerts ? "On\n" : "Off\n");
  os << indent << "Pass Lines: " << (this->PassLines ? "On\n" : "Off\n");

}

