/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its 
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "PolyData.hh"
#include "Vertex.hh"
#include "PolyVert.hh"
#include "Line.hh"
#include "PolyLine.hh"
#include "Triangle.hh"
#include "TriStrip.hh"
#include "Quad.hh"
#include "Polygon.hh"

// Initialize static member.  This member is used to simplify traversal
// of verts, lines, polygons, and triangle strips lists.  It basically 
// "marks" empty lists so that the traveral method "GetNextCell" 
// works properly.
vlCellArray *vlPolyData::Dummy = NULL;

vlPolyData::vlPolyData ()
{
  this->Verts = NULL;
  this->Lines = NULL;
  this->Polys = NULL;
  this->Strips = NULL;

  // static variable, initialized only once.
  if (!this->Dummy) this->Dummy = new vlCellArray;

  this->Cells = NULL;
  this->Links = NULL;
}

// Description:
// Perform shallow construction of vlPolyData.
vlPolyData::vlPolyData(const vlPolyData& pd) :
vlPointSet(pd)
{
  this->Verts = pd.Verts;
  if (this->Verts) this->Verts->Register(this);

  this->Lines = pd.Lines;
  if (this->Lines) this->Lines->Register(this);

  this->Polys = pd.Polys;
  if (this->Polys) this->Polys->Register(this);

  this->Strips = pd.Strips;
  if (this->Strips) this->Strips->Register(this);
 
  this->Cells = pd.Cells;
  if (this->Cells) this->Cells->Register(this);

  this->Links = pd.Links;
  if (this->Links) this->Links->Register(this);
}

vlPolyData::~vlPolyData()
{
  vlPolyData::Initialize();
}

int vlPolyData::GetCellType(int cellId)
{
  if ( !this->Cells ) this->BuildCells();
  return this->Cells->GetCellType(cellId);
}

vlCell *vlPolyData::GetCell(int cellId)
{
  static vlVertex vertex;
  static vlPolyVertex pvertex;
  static vlLine line;
  static vlPolyLine pline;
  static vlTriangle triangle;
  static vlTriangleStrip strip;
  static vlPolygon poly;
  static vlQuad quad;
  int i, loc, numPts, *pts;
  vlCell *cell;
  unsigned char type;

  if ( !this->Cells ) this->BuildCells();

  type = this->Cells->GetCellType(cellId);
  loc = this->Cells->GetCellLocation(cellId);

  switch (type)
    {
    case vlVERTEX:
     cell = &vertex;
     this->Verts->GetCell(loc,numPts,pts);
     break;

    case vlPOLY_VERTEX:
     cell = &pvertex;
     this->Verts->GetCell(loc,numPts,pts);
     break;

    case vlLINE: 
      cell = &line;
      this->Lines->GetCell(loc,numPts,pts);
      break;

    case vlPOLY_LINE:
      cell = &pline;
      this->Lines->GetCell(loc,numPts,pts);
      break;

    case vlTRIANGLE:
      cell = &triangle;
      this->Polys->GetCell(loc,numPts,pts);
      break;

    case vlQUAD:
      cell = &quad;
      this->Polys->GetCell(loc,numPts,pts);
      break;

    case vlPOLYGON:
      cell = &poly;
      this->Polys->GetCell(loc,numPts,pts);
      break;

    case vlTRIANGLE_STRIP:
      cell = &strip;
      this->Strips->GetCell(loc,numPts,pts);
      break;
    }
  for (i=0; i<numPts; i++)
    {
    cell->PointIds.SetId(i,pts[i]);
    cell->Points.SetPoint(i,this->Points->GetPoint(pts[i]));
    }

  return cell;

}

// Description:
// Set the cell array defining vertices.
void vlPolyData::SetVerts (vlCellArray* v) 
{
  if ( v != this->Verts && v != this->Dummy )
    {
    if (this->Verts) this->Verts->UnRegister(this);
    this->Verts = v;
    if (this->Verts) this->Verts->Register(this);
    this->Modified();
    }
}

// Description:
// Get the cell array defining vertices. If there are no vertices, an
// empty array will be returned (convenience to simplify traversal).
vlCellArray* vlPolyData::GetVerts()
{
  if ( !this->Verts ) return this->Dummy;
  else return this->Verts;
}

// Description:
// Set the cell array defining lines.
void vlPolyData::SetLines (vlCellArray* l) 
{
  if ( l != this->Lines && l != this->Dummy )
    {
    if (this->Lines) this->Lines->UnRegister(this);
    this->Lines = l;
    if (this->Lines) this->Lines->Register(this);
    this->Modified();
    }
}

// Description:
// Get the cell array defining lines. If there are no lines, an
// empty array will be returned (convenience to simplify traversal).
vlCellArray* vlPolyData::GetLines()
{
  if ( !this->Lines ) return this->Dummy;
  else return this->Lines;
}

// Description:
// Set the cell array defining polygons.
void vlPolyData::SetPolys (vlCellArray* p) 
{
  if ( p != this->Polys && p != this->Dummy )
    {
    if (this->Polys) this->Polys->UnRegister(this);
    this->Polys = p;
    if (this->Polys) this->Polys->Register(this);
    this->Modified();
    }
}

// Description:
// Get the cell array defining polygons. If there are no polygons, an
// empty array will be returned (convenience to simplify traversal).
vlCellArray* vlPolyData::GetPolys()
{
  if ( !this->Polys ) return this->Dummy;
  else return this->Polys;
}

// Description:
// Set the cell array defining triangle strips.
void vlPolyData::SetStrips (vlCellArray* s) 
{
  if ( s != this->Strips && s != this->Dummy )
    {
    if (this->Strips) this->Strips->UnRegister(this);
    this->Strips = s;
    if (this->Strips) this->Strips->Register(this);
    this->Modified();
    }
}

// Description:
// Get the cell array defining triangle strips. If there are no
// triangle strips, an empty array will be returned (convenience to 
// simplify traversal).
vlCellArray* vlPolyData::GetStrips()
{
  if ( !this->Strips ) return this->Dummy;
  else return this->Strips;
}

// Description:
// Restore object to initial state. Release memory back to system.
void vlPolyData::Initialize()
{
  vlPointSet::Initialize();

  if ( this->Verts ) 
  {
    this->Verts->UnRegister(this);
    this->Verts = NULL;
  }

  if ( this->Lines ) 
  {
    this->Lines->UnRegister(this);
    this->Lines = NULL;
  }

  if ( this->Polys ) 
  {
    this->Polys->UnRegister(this);
    this->Polys = NULL;
  }

  if ( this->Strips ) 
  {
    this->Strips->UnRegister(this);
    this->Strips = NULL;
  }

  if ( this->Cells )
  {
    this->Cells->UnRegister(this);
    this->Cells = NULL;
  }

  if ( this->Links )
  {
    this->Links->UnRegister(this);
    this->Links = NULL;
  }

};

int vlPolyData::GetNumberOfCells() 
{
  return GetNumberOfVerts() + GetNumberOfLines() + 
         GetNumberOfPolys() + GetNumberOfStrips();
}

int vlPolyData::GetNumberOfVerts() 
{
  return (this->Verts ? this->Verts->GetNumberOfCells() : 0);
}

int vlPolyData::GetNumberOfLines() 
{
  return (this->Lines ? this->Lines->GetNumberOfCells() : 0);
}

int vlPolyData::GetNumberOfPolys() 
{
  return (this->Polys ? this->Polys->GetNumberOfCells() : 0);
}

int vlPolyData::GetNumberOfStrips() 
{
  return (this->Strips ? this->Strips->GetNumberOfCells() : 0);
}

// Description:
// Create data structure that allows random access of cells.
void vlPolyData::BuildCells()
{
  int numCells=0;
  vlCellArray *inVerts=this->GetVerts();
  vlCellArray *inLines=this->GetLines();
  vlCellArray *inPolys=this->GetPolys();
  vlCellArray *inStrips=this->GetStrips();
  int npts, *pts;
  vlCellList *cells;
  vlPoints *inPoints=this->GetPoints();

  vlDebugMacro (<< "Building PolyData cells.");

  numCells = this->GetNumberOfCells();

  if ( inPoints == NULL || numCells < 1 ) 
    {
    vlErrorMacro (<< "No data to build");
    return;
    }
  else
    {
    this->Cells = cells = new vlCellList(numCells,3*numCells);
    this->Cells->Register(this);
    }
//
// Traverse various lists to create cell array
//
  for (inVerts->InitTraversal(); inVerts->GetNextCell(npts,pts); )
    {
    if ( npts > 1 )
      cells->InsertNextCell(vlPOLY_VERTEX,inVerts->GetLocation(npts));
    else
      cells->InsertNextCell(vlVERTEX,inVerts->GetLocation(npts));
    }

  for (inLines->InitTraversal(); inLines->GetNextCell(npts,pts); )
    {
    if ( npts > 2 )
      cells->InsertNextCell(vlPOLY_LINE,inLines->GetLocation(npts));
    else
      cells->InsertNextCell(vlLINE,inLines->GetLocation(npts));
    }

  for (inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); )
    {
    if ( npts == 3 )
      cells->InsertNextCell(vlTRIANGLE,inPolys->GetLocation(npts));
    else if ( npts == 4 )
      cells->InsertNextCell(vlQUAD,inPolys->GetLocation(npts));
    else
      cells->InsertNextCell(vlPOLYGON,inPolys->GetLocation(npts));
    }

  for (inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); )
    {
    cells->InsertNextCell(vlTRIANGLE_STRIP,inStrips->GetLocation(npts));
    }
}

// Description:
// Create upward links from points to cells that use each point. Enables
// topologically complex queries.
void vlPolyData::BuildLinks()
{
  if ( this->Cells == NULL ) this->BuildCells();
  this->Links = new vlLinkList(this->GetNumberOfPoints());
  this->Links->Register(this);

  this->Links->BuildLinks(this);
}

// Description:
// Copy a cells point ids into list provided. (Less efficient).
void vlPolyData::GetCellPoints(int cellId, vlIdList& ptIds)
{
  int i, npts, *pts;

  ptIds.Reset();
  if ( this->Cells == NULL ) this->BuildCells();

  this->vlPolyData::GetCellPoints(cellId, npts, pts);
  for (i=0; i<npts; i++) ptIds.SetId(i,pts[i]);
}

// Description:
// Return a pointer to a list of point ids defining cell. (More efficient).
// Assumes that cells have been built (with BuildCells()).
void vlPolyData::GetCellPoints(int cellId, int& npts, int* &pts)
{
  int loc;
  unsigned char type;

  type = this->Cells->GetCellType(cellId);
  loc = this->Cells->GetCellLocation(cellId);

  switch (type)
    {
    case vlVERTEX: case vlPOLY_VERTEX:
     this->Verts->GetCell(loc,npts,pts);
     break;

    case vlLINE: case vlPOLY_LINE:
      this->Lines->GetCell(loc,npts,pts);
      break;

    case vlTRIANGLE: case vlQUAD: case vlPOLYGON:
      this->Polys->GetCell(loc,npts,pts);
      break;

    case vlTRIANGLE_STRIP:
      this->Strips->GetCell(loc,npts,pts);
      break;
    }
}

void vlPolyData::GetPointCells(int ptId, vlIdList& cellIds)
{
  int *cells;
  int numCells;
  int i;

  if ( ! this->Links ) this->BuildLinks();
  cellIds.Reset();

  numCells = this->Links->GetNcells(ptId);
  cells = this->Links->GetCells(ptId);

  for (i=0; i < numCells; i++)
    {
    cellIds.InsertId(i,cells[i]);
    }
}

// Description:
// Method allocates initial storage for vertex, line, polygon, and 
// triangle strips arrays. Use this method before the method 
// PolyData::InsertNextCell(). (Or, provide vertex, line, polygon, and
// triangle strip cell arrays.)
void vlPolyData::Allocate(int numCells, int extSize)
{
  this->SetVerts(new vlCellArray(numCells,extSize));
  this->SetLines(new vlCellArray(numCells,extSize));
  this->SetPolys(new vlCellArray(numCells,extSize));
  this->SetStrips(new vlCellArray(numCells,extSize));
}

// Description:
// Insert a cell of type vlVERTEX, vlPOLY_VERTEX, vlLINE, vlPOLY_LINE,
// vlTRIANGLE, vlQUAD, vlPOLYGON, or vlTRIANGLE_STRIP.  Make sure that
// the PolyData::Allocate() function has been called first or that vertex,
// line, polygon, and triangle strip arrays have been supplied.
// Note: will also insert vlPIXEL, but converts it to vlQUAD.
int vlPolyData::InsertNextCell(int type, int npts, int pts[MAX_CELL_SIZE])
{
  int id = 0;

  switch (type)
    {
    case vlVERTEX: case vlPOLY_VERTEX:
      id = this->Verts->InsertNextCell(npts,pts);
      break;

    case vlLINE: case vlPOLY_LINE:
      id = this->Lines->InsertNextCell(npts,pts);
      break;

    case vlTRIANGLE: case vlQUAD: case vlPOLYGON:
      id = this->Polys->InsertNextCell(npts,pts);
      break;

    case vlPIXEL: //need to rearrange vertices
      {
      static int pixPts[4];
      pixPts[0] = pts[0];
      pixPts[1] = pts[1];
      pixPts[2] = pts[3];
      pixPts[3] = pts[2];
      id = this->Polys->InsertNextCell(npts,pixPts);
      break;
      }

    case vlTRIANGLE_STRIP:
      id = this->Strips->InsertNextCell(npts,pts);
      break;

    default:
      vlErrorMacro(<<"Bad cell type! Can't insert!");
    }
  return id;
}

// Description:
// Recover extra allocated memory when creating data whose initial size
// is unknown. Examples include using the InsertNextCell() method, or
// when using the CellArray::EstimateSize() method to create vertices,
// lines, polygons, or triangle strips.

void vlPolyData::Squeeze()
{
  if ( this->Verts != NULL ) this->Verts->Squeeze();
  if ( this->Lines != NULL ) this->Lines->Squeeze();
  if ( this->Polys != NULL ) this->Polys->Squeeze();
  if ( this->Strips != NULL ) this->Strips->Squeeze();

  vlPointSet::Squeeze();
}

// Description:
// Reverse the order of point ids defining the cell.
void vlPolyData::ReverseCell(int cellId)
{
  int loc, type;

  if ( this->Cells == NULL ) this->BuildCells();
  loc = this->Cells->GetCellLocation(cellId);
  type = this->Cells->GetCellType(cellId);

  switch (type)
    {
    case vlVERTEX: case vlPOLY_VERTEX:
     this->Verts->ReverseCell(loc);
     break;

    case vlLINE: case vlPOLY_LINE:
      this->Lines->ReverseCell(loc);
      break;

    case vlTRIANGLE: case vlQUAD: case vlPOLYGON:
      this->Polys->ReverseCell(loc);
      break;

    case vlTRIANGLE_STRIP:
      this->Strips->ReverseCell(loc);
      break;
    }
}

// Description:
// Replace the points defining cell "cellId" with a new set of points.
void vlPolyData::ReplaceCell(int cellId, int npts, int *pts)
{
  int loc, type;

  if ( this->Cells == NULL ) this->BuildCells();
  loc = this->Cells->GetCellLocation(cellId);
  type = this->Cells->GetCellType(cellId);

  switch (type)
    {
    case vlVERTEX: case vlPOLY_VERTEX:
     this->Verts->ReplaceCell(loc,npts,pts);
     break;

    case vlLINE: case vlPOLY_LINE:
      this->Lines->ReplaceCell(loc,npts,pts);
      break;

    case vlTRIANGLE: case vlQUAD: case vlPOLYGON:
      this->Polys->ReplaceCell(loc,npts,pts);
      break;

    case vlTRIANGLE_STRIP:
      this->Strips->ReplaceCell(loc,npts,pts);
      break;
    }
}

void vlPolyData::ReplaceLinkedCell(int cellId, int npts, int *pts)
{
  int loc = this->Cells->GetCellLocation(cellId);
  int type = this->Cells->GetCellType(cellId);

  switch (type)
    {
    case vlVERTEX: case vlPOLY_VERTEX:
     this->Verts->ReplaceCell(loc,npts,pts);
     break;

    case vlLINE: case vlPOLY_LINE:
      this->Lines->ReplaceCell(loc,npts,pts);
      break;

    case vlTRIANGLE: case vlQUAD: case vlPOLYGON:
      this->Polys->ReplaceCell(loc,npts,pts);
      break;

    case vlTRIANGLE_STRIP:
      this->Strips->ReplaceCell(loc,npts,pts);
      break;
    }

  for (int i=0; i < npts; i++)
    {
    this->Links->InsertNextCellReference(pts[i],cellId);
    }
}

// Description:
// Get the neighbors at an edge. More efficient than the general 
// GetCellNeighbors(). Assumes links have been built (with BuildLinks()), 
// and looks specifically for edge neighbors.
void vlPolyData::GetCellEdgeNeighbors(int cellId, int p1, int p2,
                                      vlIdList& cellIds)
{
  int *cells;
  int numCells;
  int i,j;
  int npts, *pts;

  cellIds.Reset();

  numCells = this->Links->GetNcells(p1);
  cells = this->Links->GetCells(p1);

  for (i=0; i < numCells; i++)
    {
    if ( cells[i] != cellId )
      {
      this->GetCellPoints(cells[i],npts,pts);
      for (j=0; j < npts; j++) if ( pts[j] == p2 ) break;
      if ( j < npts ) cellIds.InsertNextId(cells[i]);
      }
    }
}

void vlPolyData::PrintSelf(ostream& os, vlIndent indent)
{
  vlPointSet::PrintSelf(os,indent);

  os << indent << "Number Of Vertices: " << this->GetNumberOfVerts() << "\n";
  os << indent << "Number Of Lines: " << this->GetNumberOfLines() << "\n";
  os << indent << "Number Of Polygons: " << this->GetNumberOfPolys() << "\n";
  os << indent << "Number Of Triangle Strips: " << this->GetNumberOfStrips() << "\n";
}

