/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDataSetTriangleFilter.h"

#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkGenericCell.h"
#include "vtkIdList.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkOrderedTriangulator.h"
#include "vtkPointData.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRectilinearGrid.h"

vtkCxxRevisionMacro(vtkDataSetTriangleFilter, "$Revision$");
vtkStandardNewMacro(vtkDataSetTriangleFilter);

vtkDataSetTriangleFilter::vtkDataSetTriangleFilter()
{
  this->Triangulator = vtkOrderedTriangulator::New();
  this->Triangulator->PreSortedOff();
  this->Triangulator->UseTemplatesOn();
}

vtkDataSetTriangleFilter::~vtkDataSetTriangleFilter()
{
  this->Triangulator->Delete();
  this->Triangulator = NULL;
}

void vtkDataSetTriangleFilter::Execute()
{
  vtkDataSet *input = this->GetInput();
  if (!input)
    {
    return;
    }
  if (input->IsA("vtkStructuredPoints") ||
      input->IsA("vtkStructuredGrid") || 
      input->IsA("vtkImageData") ||
      input->IsA("vtkRectilinearGrid"))
    {
    this->StructuredExecute();
    }
  else
    {
    this->UnstructuredExecute();
    }

  vtkDebugMacro(<<"Produced " << this->GetOutput()->GetNumberOfCells() << " cells");
}

void vtkDataSetTriangleFilter::StructuredExecute()
{
  vtkDataSet *input = this->GetInput();
  vtkUnstructuredGrid *output = this->GetOutput();
  int dimensions[3], i, j, k, l, m;
  vtkIdType newCellId, inId;
  vtkGenericCell *cell = vtkGenericCell::New();
  vtkCellData *inCD = input->GetCellData();
  vtkCellData *outCD = output->GetCellData();
  vtkPoints *cellPts = vtkPoints::New();
  vtkPoints *newPoints = vtkPoints::New();
  vtkIdList *cellPtIds = vtkIdList::New();
  int numSimplices, numPts, dim, type;
  vtkIdType pts[4], num;
  
  // Create an array of points. This does an explicit creation
  // of each point.
  num = input->GetNumberOfPoints();
  newPoints->SetNumberOfPoints(num);
  for (i = 0; i < num; ++i)
    {
    newPoints->SetPoint(i,input->GetPoint(i));
    }

  outCD->CopyAllocate(inCD,input->GetNumberOfCells()*5);
  output->Allocate(input->GetNumberOfCells()*5);
  
  if (input->IsA("vtkStructuredPoints"))
    {
    static_cast<vtkStructuredPoints*>(input)->GetDimensions(dimensions);
    }
  else if (input->IsA("vtkStructuredGrid"))
    {
    static_cast<vtkStructuredGrid*>(input)->GetDimensions(dimensions);
    }
  else if (input->IsA("vtkImageData"))
    {
    static_cast<vtkImageData*>(input)->GetDimensions(dimensions);
    }
  else if (input->IsA("vtkRectilinearGrid"))
    {
    static_cast<vtkRectilinearGrid*>(input)->GetDimensions(dimensions);
    }
  
  dimensions[0] = dimensions[0] - 1;
  dimensions[1] = dimensions[1] - 1;
  dimensions[2] = dimensions[2] - 1;

  vtkIdType numSlices=dimensions[2];
  int abort=0;
  for (k = 0; k < dimensions[2] && !abort; k++)
    {
    this->UpdateProgress((float)k / numSlices);
    abort = this->GetAbortExecute();

    for (j = 0; j < dimensions[1]; j++)
      {
      for (i = 0; i < dimensions[0]; i++)
        {
        inId = i+(j+(k*dimensions[1]))*dimensions[0];
        input->GetCell(inId, cell);
        if ((i+j+k)%2 == 0)
          {
          cell->Triangulate(0, cellPtIds, cellPts);
          }
        else
          {
          cell->Triangulate(1, cellPtIds, cellPts);
          }
        
        dim = cell->GetCellDimension() + 1;
        
        numPts = cellPtIds->GetNumberOfIds();
        numSimplices = numPts / dim;
        type = 0;
        switch (dim)
          {
          case 1:
            type = VTK_VERTEX;    break;
          case 2:
            type = VTK_LINE;      break;
          case 3:
            type = VTK_TRIANGLE;  break;
          case 4:
            type = VTK_TETRA;     break;
          }
        for (l = 0; l < numSimplices; l++ )
          {
          for (m = 0; m < dim; m++)
            {
            pts[m] = cellPtIds->GetId(dim*l+m);
            }
          // copy cell data
          newCellId = output->InsertNextCell(type, dim, pts);
          outCD->CopyData(inCD, inId, newCellId);
          }//for all simplices
        }//i dimension
      }//j dimension
    }//k dimension
  
  // Update output
  output->SetPoints(newPoints);
  output->GetPointData()->PassData(input->GetPointData());
  output->Squeeze();
  
  cell->Delete();
  newPoints->Delete();
  cellPts->Delete();
  cellPtIds->Delete();
}

// 3D cells use the ordered triangulator. The ordered triangulator is used
// to create templates on the fly. Once the templates are created then they
// are used to produce the final triangulation.
//
void vtkDataSetTriangleFilter::UnstructuredExecute()
{
  vtkPointSet *input = (vtkPointSet*) this->GetInput(); //has to be
  vtkUnstructuredGrid *output = this->GetOutput();
  vtkIdType numCells = input->GetNumberOfCells();
  vtkGenericCell *cell;
  vtkIdType newCellId, j;
  int k;
  vtkCellData *inCD=input->GetCellData();
  vtkCellData *outCD=output->GetCellData();
  vtkPoints *cellPts;
  vtkIdList *cellPtIds;
  vtkIdType ptId, numTets, ncells;
  int numPts, type;
  int numSimplices, dim;
  vtkIdType pts[4];
  float x[3];

  if (numCells == 0)
    {
    return;
    }

  cell = vtkGenericCell::New();
  cellPts = vtkPoints::New();
  cellPtIds = vtkIdList::New();

  // Create an array of points
  outCD->CopyAllocate(inCD,input->GetNumberOfCells()*5);
  output->Allocate(input->GetNumberOfCells()*5);
  
  // Points are passed through
  output->SetPoints(input->GetPoints());
  output->GetPointData()->PassData(input->GetPointData());

  int abort=0;
  vtkIdType updateTime = numCells/20 + 1;  // update roughly every 5%
  for (vtkIdType cellId=0; cellId < numCells && !abort; cellId++)
    {
    if ( !(cellId % updateTime) )
      {
      this->UpdateProgress((float)cellId / numCells);
      abort = this->GetAbortExecute();
      }

    input->GetCell(cellId, cell);
    dim = cell->GetCellDimension();

    if ( dim == 3 ) //use ordered triangulation
      {
      numPts = cell->GetNumberOfPoints();
      float *p, *pPtr=cell->GetParametricCoords();
      this->Triangulator->InitTriangulation(0.0,1.0, 0.0,1.0, 0.0,1.0, numPts);
      for (p=pPtr, j=0; j<numPts; j++, p+=3)
        {
        ptId = cell->PointIds->GetId(j);
        cell->Points->GetPoint(j, x);
        this->Triangulator->InsertPoint(ptId, x, p, 0);
        }//for all cell points
      if ( cell->IsPrimaryCell() ) //use templates if topology is fixed
        {
        int numEdges=cell->GetNumberOfEdges();
        this->Triangulator->TemplateTriangulate(cell->GetCellType(),
                                                numPts,numEdges);
        }
      else //use ordered triangulator
        {
        this->Triangulator->Triangulate();
        }

      ncells = output->GetNumberOfCells();
      numTets = this->Triangulator->AddTetras(0,output);
        
      for (j=0; j < numTets; j++)
        {
        outCD->CopyData(inCD, cellId, ncells+j);
        }
      }

    else //2D or lower dimension
      {
      dim++;
      cell->Triangulate(0, cellPtIds, cellPts);
      numPts = cellPtIds->GetNumberOfIds();
    
      numSimplices = numPts / dim;
      type = 0;
      switch (dim)
        {
        case 1:
          type = VTK_VERTEX;    break;
        case 2:
          type = VTK_LINE;      break;
        case 3:
          type = VTK_TRIANGLE;  break;
        }

      for ( j=0; j < numSimplices; j++ )
        {
        for (k=0; k<dim; k++)
          {
          pts[k] = cellPtIds->GetId(dim*j+k);
          }
        // copy cell data
        newCellId = output->InsertNextCell(type, dim, pts);
        outCD->CopyData(inCD, cellId, newCellId);
        }
      } //if 2D or less cell
    } //for all cells
  
  // Update output
  output->Squeeze();
  
  cellPts->Delete();
  cellPtIds->Delete();
  cell->Delete();
}

void vtkDataSetTriangleFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

