/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "LinExtrd.hh"

// Description:
// Create object with normal extrusion type, capping on, scale factor=1.0,
// vector (0,0,1), and extrusion point (0,0,0).
vtkLinearExtrusionFilter::vtkLinearExtrusionFilter()
{
  this->ExtrusionType = NORMAL_EXTRUSION;
  this->Capping = 1;
  this->ScaleFactor = 1.0;
  this->Vector[0] = this->Vector[1] = 0.0; this->Vector[2] = 1.0;
  this->ExtrusionPoint[0] = this->ExtrusionPoint[1] = this->ExtrusionPoint[2] = 0.0;
}

float *vtkLinearExtrusionFilter::ViaNormal(float x[3], int id, vtkNormals *n)
{
  static float xNew[3], *normal;
  int i;

  normal = n->GetNormal(id);
  for (i=0; i<3; i++) 
    xNew[i] = x[i] + this->ScaleFactor*normal[i];

  return xNew;
}

float *vtkLinearExtrusionFilter::ViaVector(float x[3], int id, vtkNormals *n)
{
  static float xNew[3];
  int i;

  for (i=0; i<3; i++) 
    xNew[i] = x[i] + this->ScaleFactor*this->Vector[i];

  return xNew;
}

float *vtkLinearExtrusionFilter::ViaPoint(float x[3], int id, vtkNormals *n)
{
  static float xNew[3];
  int i;

  for (i=0; i<3; i++) 
    xNew[i] = x[i] + this->ScaleFactor*(x[i] - this->ExtrusionPoint[i]);

  return xNew;
}

void vtkLinearExtrusionFilter::Execute()
{
  int numPts, numCells;
  vtkPolyData *input=(vtkPolyData *)this->Input;
  vtkPointData *pd=input->GetPointData();
  vtkNormals *inNormals;
  vtkPolyData mesh;
  vtkPoints *inPts;
  vtkCellArray *inVerts, *inLines, *inPolys, *inStrips;
  int npts, *pts, numEdges, cellId, dim;
  int ptId, ncells, ptIds[MAX_CELL_SIZE], i, j, k, p1, p2;
  float *x;
  vtkFloatPoints *newPts;
  vtkCellArray *newLines=NULL, *newPolys=NULL, *newStrips=NULL;
  vtkCell *cell, *edge;
  vtkIdList cellIds(MAX_CELL_SIZE), *cellPts;
//
// Initialize / check input
//
  vtkDebugMacro(<<"Linearly extruding data");
  this->Initialize();

  if ( (numPts=input->GetNumberOfPoints()) < 1 || 
  (numCells=input->GetNumberOfCells()) < 1 )
    {
    vtkErrorMacro(<<"No data to extrude!");
    return;
    }
//
// Decide which vector to use for extrusion
//
  if ( this->ExtrusionType == POINT_EXTRUSION )
    {
    this->ExtrudePoint = &vtkLinearExtrusionFilter::ViaPoint;
    }
  else if ( this->ExtrusionType == NORMAL_EXTRUSION  &&
  (inNormals = pd->GetNormals()) != NULL )
    {
    this->ExtrudePoint = &vtkLinearExtrusionFilter::ViaNormal;
    inNormals = pd->GetNormals();
    }
  else // Vector_EXTRUSION
    {
    this->ExtrudePoint = &vtkLinearExtrusionFilter::ViaVector;
    }
//
// Build cell data structure.
//
  inPts = input->GetPoints();
  inVerts = input->GetVerts();
  inLines = input->GetLines();
  inPolys = input->GetPolys();
  inStrips = input->GetStrips();
  mesh.SetPoints(inPts);
  mesh.SetVerts(inVerts);
  mesh.SetLines(inLines);
  mesh.SetPolys(inPolys);
  mesh.SetStrips(inStrips);
  if ( inPolys || inStrips ) mesh.BuildLinks();
//
// Allocate memory for output. We don't copy normals because surface geometry
// is modified. Copy all points - this is the usual requirement and it makes
// creation of skirt much easier.
//
  this->PointData.CopyNormalsOff();
  this->PointData.CopyAllocate(pd,2*numPts);
  newPts = new vtkFloatPoints(2*numPts);
  if ( (ncells=inVerts->GetNumberOfCells()) > 0 ) 
    {
    newLines = new vtkCellArray;
    newLines->Allocate(newLines->EstimateSize(ncells,2));
    }
  // arbitrary initial allocation size
  ncells = inLines->GetNumberOfCells() + inPolys->GetNumberOfCells()/10 +
           inStrips->GetNumberOfCells()/10;
  ncells = (ncells < 100 ? 100 : ncells);
  newStrips = new vtkCellArray;
  newStrips->Allocate(newStrips->EstimateSize(ncells,4));

  // copy points
  for (ptId=0; ptId < numPts; ptId++)
    {
    x = inPts->GetPoint(ptId);
    newPts->SetPoint(ptId,x);
    newPts->SetPoint(ptId+numPts,(this->*(ExtrudePoint))(x,ptId,inNormals));
    this->PointData.CopyData(pd,ptId,ptId);
    this->PointData.CopyData(pd,ptId,ptId+numPts);
    }
//
// If capping is on, copy 2D cells to output (plus create cap)
//
  if ( this->Capping )
    {
    if ( inPolys->GetNumberOfCells() > 0 )
      {
      newPolys = new vtkCellArray(inPolys->GetSize());
      for ( inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); )
        {
        newPolys->InsertNextCell(npts,pts);
        newPolys->InsertNextCell(npts);
        for (i=0; i < npts; i++)
          newPolys->InsertCellPoint(pts[i] + numPts);
        }
      }
    
    if ( inStrips->GetNumberOfCells() > 0 )
      {
      for ( inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); )
        {
        newStrips->InsertNextCell(npts,pts);
        newStrips->InsertNextCell(npts);
        for (i=0; i < npts; i++)
          newStrips->InsertCellPoint(pts[i] + numPts);
        }
      }
    }
//
// Loop over all polygons and triangle strips searching for boundary edges. 
// If boundary edge found, extrude triangle strip.
//
  for ( cellId=0; cellId < numCells; cellId++)
    {
    cell = mesh.GetCell(cellId);
    cellPts = cell->GetPointIds();

    if ( (dim=cell->GetCellDimension()) == 0 ) //create lines from points
      {
      for (i=0; i<cellPts->GetNumberOfIds(); i++)
        {
        newLines->InsertNextCell(2);
        ptId = cellPts->GetId(i);
        newLines->InsertCellPoint(ptId);
        newLines->InsertCellPoint(ptId+numPts);
        }
      }

    else if ( dim == 1 ) // create strips from lines
      {
      for (i=0; i < (cellPts->GetNumberOfIds()-1); i++)
        {
        p1 = cellPts->GetId(i);
        p2 = cellPts->GetId(i+1);
        newStrips->InsertNextCell(4);
        newStrips->InsertCellPoint(p1);
        newStrips->InsertCellPoint(p2);
        newStrips->InsertCellPoint(p1+numPts);
        newStrips->InsertCellPoint(p2+numPts);
        }
      }

    else if ( dim == 2 ) // create strips from boundary edges
      {
      numEdges = cell->GetNumberOfEdges();
      for (i=0; i<numEdges; i++)
        {
        edge = cell->GetEdge(i);
        for (j=0; j<(edge->GetNumberOfPoints()-1); j++)
          {
          p1 = edge->PointIds.GetId(j);
          p2 = edge->PointIds.GetId(j+1);
          mesh.GetCellEdgeNeighbors(cellId, p1, p2, cellIds);

          if ( cellIds.GetNumberOfIds() < 1 ) //generate strip
            {
            newStrips->InsertNextCell(4);
            newStrips->InsertCellPoint(p1);
            newStrips->InsertCellPoint(p2);
            newStrips->InsertCellPoint(p1+numPts);
            newStrips->InsertCellPoint(p2+numPts);
            }
          } //for each sub-edge
        } //for each edge
      } //for each polygon or triangle strip
    } //for each cell
//
// Send data to output and release memory
//
  this->SetPoints(newPts);
  newPts->Delete();

  if ( newLines ) 
    {
    this->SetLines(newLines);
    newLines->Delete();
    }

  if ( newPolys ) 
    {
    this->SetPolys(newPolys);
    newPolys->Delete();
    }

  this->SetStrips(newStrips);
  newStrips->Delete();

  this->Squeeze();
}



void vtkLinearExtrusionFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyToPolyFilter::PrintSelf(os,indent);

  if ( this->ExtrusionType == VECTOR_EXTRUSION )
    {
    os << indent << "Extrusion Type: Extrude along vector\n";
    os << indent << "Vector: (" << this->Vector[0] << ", " 
       << this->Vector[1] << ", " << this->Vector[2] << ")\n";
    }
  else if ( this->ExtrusionType == NORMAL_EXTRUSION )
    {
    os << indent << "Extrusion Type: Extrude along vertex normals\n";
    }
  else //POINT_EXTRUSION
    {
    os << indent << "Extrusion Type: Extrude towards point\n";
    os << indent << "Extrusion Point: (" << this->ExtrusionPoint[0] << ", " 
       << this->ExtrusionPoint[1] << ", " << this->ExtrusionPoint[2] << ")\n";
    }

  os << indent << "Capping: " << (this->Capping ? "On\n" : "Off\n");
  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
}

