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
#include "vtkPolyData.h"
#include "vtkVertex.h"
#include "vtkPolyVertex.h"
#include "vtkLine.h"
#include "vtkPolyLine.h"
#include "vtkTriangle.h"
#include "vtkTriangleStrip.h"
#include "vtkQuad.h"
#include "vtkPolygon.h"
#include "vtkEmptyCell.h"

// Initialize static member.  This member is used to simplify traversal
// of verts, lines, polygons, and triangle strips lists.  It basically 
// "marks" empty lists so that the traveral method "GetNextCell" 
// works properly.
vtkCellArray *vtkPolyData::Dummy = NULL;

vtkPolyData::vtkPolyData ()
{
  this->Vertex = vtkVertex::New();
  this->PolyVertex = vtkPolyVertex::New();
  this->Line = vtkLine::New();
  this->PolyLine = vtkPolyLine::New();
  this->Triangle = vtkTriangle::New();
  this->Quad = vtkQuad::New();
  this->Polygon = vtkPolygon::New();
  this->TriangleStrip = vtkTriangleStrip::New();
  this->EmptyCell = vtkEmptyCell::New();
  
  this->Verts = NULL;
  this->Lines = NULL;
  this->Polys = NULL;
  this->Strips = NULL;

  // static variable, initialized only once.
  if (this->Dummy == NULL) 
    {
    this->Dummy = vtkCellArray::New();
    this->Dummy->Register(this);
    this->Dummy->Delete();
    }
  else
    {
    this->Dummy->Register(this);
    }

  this->Cells = NULL;
  this->Links = NULL;
}

// Perform shallow construction of vtkPolyData.
vtkPolyData::vtkPolyData(const vtkPolyData& pd) :
vtkPointSet(pd)
{
  this->Verts = pd.Verts;
  if (this->Verts)
    {
    this->Verts->Register(this);
    }

  this->Lines = pd.Lines;
  if (this->Lines)
    {
    this->Lines->Register(this);
    }

  this->Polys = pd.Polys;
  if (this->Polys)
    {
    this->Polys->Register(this);
    }

  this->Strips = pd.Strips;
  if (this->Strips)
    {
    this->Strips->Register(this);
    }
 
  this->Cells = pd.Cells;
  if (this->Cells)
    {
    this->Cells->Register(this);
    }

  this->Links = pd.Links;
  if (this->Links)
    {
    this->Links->Register(this);
    }
}

vtkPolyData::~vtkPolyData()
{
  vtkPolyData::Initialize();
  // Reference to static dummy persists. 
  // Keep destructed dummy from being used again.
  if (this->Dummy->GetReferenceCount() == 1)
    {
    this->Dummy->UnRegister(this);
    this->Dummy = NULL;
    }
  else
    {
    this->Dummy->UnRegister(this);
    }

  this->Vertex->Delete();
  this->PolyVertex->Delete();
  this->Line->Delete();
  this->PolyLine->Delete();
  this->Triangle->Delete();
  this->Quad->Delete();
  this->Polygon->Delete();
  this->TriangleStrip->Delete();
  this->EmptyCell->Delete();
  
}

// Copy the geometric and topological structure of an input poly data object.
void vtkPolyData::CopyStructure(vtkDataSet *ds)
{
  vtkPolyData *pd=(vtkPolyData *)ds;
  vtkPointSet::CopyStructure(ds);

  this->Verts = pd->Verts;
  if (this->Verts)
    {
    this->Verts->Register(this);
    }

  this->Lines = pd->Lines;
  if (this->Lines)
    {
    this->Lines->Register(this);
    }

  this->Polys = pd->Polys;
  if (this->Polys)
    {
    this->Polys->Register(this);
    }

  this->Strips = pd->Strips;
  if (this->Strips)
    {
    this->Strips->Register(this);
    }
}

int vtkPolyData::GetCellType(int cellId)
{
  if ( !this->Cells )
    {
    this->BuildCells();
    }
  return this->Cells->GetCellType(cellId);
}

vtkCell *vtkPolyData::GetCell(int cellId)
{
  int i, loc, numPts, *pts;
  vtkCell *cell = NULL;
  unsigned char type;

  if ( !this->Cells )
    {
    this->BuildCells();
    }

  type = this->Cells->GetCellType(cellId);
  loc = this->Cells->GetCellLocation(cellId);

  switch (type)
    {
    case VTK_VERTEX:
      cell = this->Vertex;
      this->Verts->GetCell(loc,numPts,pts);
      break;

    case VTK_POLY_VERTEX:
      cell = this->PolyVertex;
      this->Verts->GetCell(loc,numPts,pts);
      cell->PointIds->SetNumberOfIds(numPts); //reset number of points
      cell->Points->SetNumberOfPoints(numPts);
      break;

    case VTK_LINE: 
      cell = this->Line;
      this->Lines->GetCell(loc,numPts,pts);
      break;

    case VTK_POLY_LINE:
      cell = this->PolyLine;
      this->Lines->GetCell(loc,numPts,pts);
      cell->PointIds->SetNumberOfIds(numPts); //reset number of points
      cell->Points->SetNumberOfPoints(numPts);
      break;

    case VTK_TRIANGLE:
      cell = this->Triangle;
      this->Polys->GetCell(loc,numPts,pts);
      break;

    case VTK_QUAD:
      cell = this->Quad;
      this->Polys->GetCell(loc,numPts,pts);
      break;

    case VTK_POLYGON:
      cell = this->Polygon;
      this->Polys->GetCell(loc,numPts,pts);
      cell->PointIds->SetNumberOfIds(numPts); //reset number of points
      cell->Points->SetNumberOfPoints(numPts);
      break;

    case VTK_TRIANGLE_STRIP:
      cell = this->TriangleStrip;
      this->Strips->GetCell(loc,numPts,pts);
      cell->PointIds->SetNumberOfIds(numPts); //reset number of points
      cell->Points->SetNumberOfPoints(numPts);
      break;

    default:
      cell = this->EmptyCell;
      this->Cell = cell;
      cell->Register(this);
      cell->Delete();
      return this->Cell;
    }

  for (i=0; i < numPts; i++)
    {
    cell->PointIds->SetId(i,pts[i]);
    cell->Points->SetPoint(i,this->Points->GetPoint(pts[i]));
    }

  return cell;
}

// Set the cell array defining vertices.
void vtkPolyData::SetVerts (vtkCellArray* v) 
{
  if ( v != this->Verts && v != this->Dummy)
    {
    if (this->Verts)
      {
      this->Verts->UnRegister(this);
      }
    this->Verts = v;
    if (this->Verts)
      {
      this->Verts->Register(this);
      }
    this->Modified();
    }
}

// Get the cell array defining vertices. If there are no vertices, an
// empty array will be returned (convenience to simplify traversal).
vtkCellArray* vtkPolyData::GetVerts()
{
  if ( !this->Verts )
    {
    return this->Dummy;
    }
  else
    {
    return this->Verts;
    }
}

// Set the cell array defining lines.
void vtkPolyData::SetLines (vtkCellArray* l) 
{
  if ( l != this->Lines && l != this->Dummy )
    {
    if (this->Lines)
      {
      this->Lines->UnRegister(this);
      }
    this->Lines = l;
    if (this->Lines)
      {
      this->Lines->Register(this);
      }
    this->Modified();
    }
}

// Get the cell array defining lines. If there are no lines, an
// empty array will be returned (convenience to simplify traversal).
vtkCellArray* vtkPolyData::GetLines()
{
  if ( !this->Lines )
    {
    return this->Dummy;
    }
  else
    {
    return this->Lines;
    }
}

// Set the cell array defining polygons.
void vtkPolyData::SetPolys (vtkCellArray* p) 
{
  if ( p != this->Polys && p != this->Dummy )
    {
    if (this->Polys)
      {
      this->Polys->UnRegister(this);
      }
    this->Polys = p;
    if (this->Polys)
      {
      this->Polys->Register(this);
      }
    this->Modified();
    }
}

// Get the cell array defining polygons. If there are no polygons, an
// empty array will be returned (convenience to simplify traversal).
vtkCellArray* vtkPolyData::GetPolys()
{
  if ( !this->Polys )
    {
    return this->Dummy;
    }
  else
    {
    return this->Polys;
    }
}

// Set the cell array defining triangle strips.
void vtkPolyData::SetStrips (vtkCellArray* s) 
{
  if ( s != this->Strips && s != this->Dummy )
    {
    if (this->Strips)
      {
      this->Strips->UnRegister(this);
      }
    this->Strips = s;
    if (this->Strips)
      {
      this->Strips->Register(this);
      }
    this->Modified();
    }
}

// Get the cell array defining triangle strips. If there are no
// triangle strips, an empty array will be returned (convenience to 
// simplify traversal).
vtkCellArray* vtkPolyData::GetStrips()
{
  if ( !this->Strips )
    {
    return this->Dummy;
    }
  else
    {
    return this->Strips;
    }
}

// Restore object to initial state. Release memory back to system.
void vtkPolyData::Initialize()
{
  vtkPointSet::Initialize();

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
}

int vtkPolyData::GetMaxCellSize() 
{
  int maxCellSize=0, cellSize;

  if ( this->Verts ) 
    {
    cellSize = this->Verts->GetMaxCellSize();
    if ( cellSize > maxCellSize )
      {
      maxCellSize = cellSize;
      }
    }

  if ( this->Lines ) 
    {
    cellSize = this->Lines->GetMaxCellSize();
    if ( cellSize > maxCellSize )
      {
      maxCellSize = cellSize;
      }
    }

  if ( this->Polys ) 
    {
    cellSize = this->Polys->GetMaxCellSize();
    if ( cellSize > maxCellSize )
      {
      maxCellSize = cellSize;
      }
    }

  if ( this->Strips ) 
    {
    cellSize = this->Strips->GetMaxCellSize();
    if ( cellSize > maxCellSize )
      {
      maxCellSize = cellSize;
      }
    }

  return maxCellSize;
}

int vtkPolyData::GetNumberOfCells() 
{
  return this->GetNumberOfVerts() + this->GetNumberOfLines() + 
         this->GetNumberOfPolys() + this->GetNumberOfStrips();
}

int vtkPolyData::GetNumberOfVerts() 
{
  return (this->Verts ? this->Verts->GetNumberOfCells() : 0);
}

int vtkPolyData::GetNumberOfLines() 
{
  return (this->Lines ? this->Lines->GetNumberOfCells() : 0);
}

int vtkPolyData::GetNumberOfPolys() 
{
  return (this->Polys ? this->Polys->GetNumberOfCells() : 0);
}

int vtkPolyData::GetNumberOfStrips() 
{
  return (this->Strips ? this->Strips->GetNumberOfCells() : 0);
}

// Create data structure that allows random access of cells.
void vtkPolyData::BuildCells()
{
  int numCells;
  vtkCellArray *inVerts=this->GetVerts();
  vtkCellArray *inLines=this->GetLines();
  vtkCellArray *inPolys=this->GetPolys();
  vtkCellArray *inStrips=this->GetStrips();
  int npts, *pts;
  vtkCellTypes *cells;

  vtkDebugMacro (<< "Building PolyData cells.");

  if ( (numCells = this->GetNumberOfCells()) < 1 )
    {
    numCells = 1000; //may be allocating empty list to begin with
    }

  this->Cells = cells = vtkCellTypes::New();
  this->Cells->Allocate(numCells,3*numCells);
  this->Cells->Register(this);
  cells->Delete();
//
// Traverse various lists to create cell array
//
  for (inVerts->InitTraversal(); inVerts->GetNextCell(npts,pts); )
    {
    if ( npts > 1 )
      {
      cells->InsertNextCell(VTK_POLY_VERTEX,
			    inVerts->GetTraversalLocation(npts));
      }
    else
      {
      cells->InsertNextCell(VTK_VERTEX,inVerts->GetTraversalLocation(npts));
      }
    }

  for (inLines->InitTraversal(); inLines->GetNextCell(npts,pts); )
    {
    if ( npts > 2 )
      {
      cells->InsertNextCell(VTK_POLY_LINE,inLines->GetTraversalLocation(npts));
      }
    else
      {
      cells->InsertNextCell(VTK_LINE,inLines->GetTraversalLocation(npts));
      } 
    }

  for (inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); )
    {
    if ( npts == 3 )
      {
      cells->InsertNextCell(VTK_TRIANGLE,inPolys->GetTraversalLocation(npts));
      }
    else if ( npts == 4 )
      {
      cells->InsertNextCell(VTK_QUAD,inPolys->GetTraversalLocation(npts));
      }
    else
      {
      cells->InsertNextCell(VTK_POLYGON,inPolys->GetTraversalLocation(npts));
      }
    }

  for (inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); )
    {
    cells->InsertNextCell(VTK_TRIANGLE_STRIP,inStrips->GetTraversalLocation(npts));
    }
}

// Create upward links from points to cells that use each point. Enables
// topologically complex queries.
void vtkPolyData::BuildLinks()
{
  if ( this->Cells == NULL )
    {
    this->BuildCells();
    }
  this->Links = new vtkCellLinks(this->GetNumberOfPoints());
  this->Links->Register(this);
  this->Links->Delete();

  this->Links->BuildLinks(this);
}

// Copy a cells point ids into list provided. (Less efficient.)
void vtkPolyData::GetCellPoints(int cellId, vtkIdList *ptIds)
{
  int i, npts, *pts;

  ptIds->Reset();
  if ( this->Cells == NULL )
    {
    this->BuildCells();
    }

  this->vtkPolyData::GetCellPoints(cellId, npts, pts);
  ptIds->InsertId (npts-1,pts[npts-1]);
  for (i=0; i<npts-1; i++)
    {
    ptIds->SetId(i,pts[i]);
    }
}

// Return a pointer to a list of point ids defining cell. (More efficient.)
// Assumes that cells have been built (with BuildCells()).
void vtkPolyData::GetCellPoints(int cellId, int& npts, int* &pts)
{
  int loc;
  unsigned char type;

  type = this->Cells->GetCellType(cellId);
  loc = this->Cells->GetCellLocation(cellId);

  switch (type)
    {
    case VTK_VERTEX: case VTK_POLY_VERTEX:
      this->Verts->GetCell(loc,npts,pts);
      break;

    case VTK_LINE: case VTK_POLY_LINE:
      this->Lines->GetCell(loc,npts,pts);
      break;

    case VTK_TRIANGLE: case VTK_QUAD: case VTK_POLYGON:
      this->Polys->GetCell(loc,npts,pts);
      break;

    case VTK_TRIANGLE_STRIP:
      this->Strips->GetCell(loc,npts,pts);
      break;

    default:
      npts = 0;
      pts = NULL;
    }
}

void vtkPolyData::GetPointCells(int ptId, vtkIdList *cellIds)
{
  int *cells;
  int numCells;
  int i;

  if ( ! this->Links )
    {
    this->BuildLinks();
    }
  cellIds->Reset();

  numCells = this->Links->GetNcells(ptId);
  cells = this->Links->GetCells(ptId);

  for (i=0; i < numCells; i++)
    {
    cellIds->InsertId(i,cells[i]);
    }
}

// Method allocates initial storage for vertex, line, polygon, and 
// triangle strip arrays. Use this method before the method 
// PolyData::InsertNextCell(). (Or, provide vertex, line, polygon, and
// triangle strip cell arrays.)
void vtkPolyData::Allocate(int numCells, int extSize)
{
  vtkCellArray *cells;

  cells = vtkCellArray::New();
  cells->Allocate(numCells,extSize);
  this->SetVerts(cells);
  cells->Delete();

  cells = vtkCellArray::New();
  cells->Allocate(numCells,extSize);
  this->SetLines(cells);
  cells->Delete();

  cells = vtkCellArray::New();
  cells->Allocate(numCells,extSize);
  this->SetPolys(cells);
  cells->Delete();

  cells = vtkCellArray::New();
  cells->Allocate(numCells,extSize);
  this->SetStrips(cells);
  cells->Delete();
}

// Insert a cell of type vtkVERTEX, vtkPOLY_VERTEX, vtkLINE, vtkPOLY_LINE,
// vtkTRIANGLE, vtkQUAD, vtkPOLYGON, or vtkTRIANGLE_STRIP.  Make sure that
// the PolyData::Allocate() function has been called first or that vertex,
// line, polygon, and triangle strip arrays have been supplied.
// Note: will also insert vtkPIXEL, but converts it to vtkQUAD.
int vtkPolyData::InsertNextCell(int type, int npts, int *pts)
{
  int id;

  if ( !this->Cells ) 
    {
    this->Cells = new vtkCellTypes(5000,10000);
    }

  switch (type)
    {
    case VTK_VERTEX: case VTK_POLY_VERTEX:
      this->Verts->InsertNextCell(npts,pts);
      id = this->Cells->InsertNextCell(type, this->Verts->GetInsertLocation(npts));
      break;

    case VTK_LINE: case VTK_POLY_LINE:
      this->Lines->InsertNextCell(npts,pts);
      id = this->Cells->InsertNextCell(type, this->Lines->GetInsertLocation(npts));
      break;

    case VTK_TRIANGLE: case VTK_QUAD: case VTK_POLYGON:
      this->Polys->InsertNextCell(npts,pts);
      id = this->Cells->InsertNextCell(type, this->Polys->GetInsertLocation(npts));
      break;

    case VTK_PIXEL: //need to rearrange vertices
      {
      static int pixPts[4];
      pixPts[0] = pts[0];
      pixPts[1] = pts[1];
      pixPts[2] = pts[3];
      pixPts[3] = pts[2];
      this->Polys->InsertNextCell(npts,pixPts);
      id = this->Cells->InsertNextCell(VTK_QUAD, this->Polys->GetInsertLocation(npts));
      break;
      }

    case VTK_TRIANGLE_STRIP:
      this->Strips->InsertNextCell(npts,pts);
      id = this->Cells->InsertNextCell(type, this->Strips->GetInsertLocation(npts));
      break;

    default:
      id = -1;
      vtkErrorMacro(<<"Bad cell type! Can't insert!");
    }
  return id;
}

// Insert a cell of type VTK_VERTEX, VTK_POLY_VERTEX, VTK_LINE, VTK_POLY_LINE,
// VTK_TRIANGLE, VTK_QUAD, VTK_POLYGON, or VTK_TRIANGLE_STRIP.  Make sure that
// the PolyData::Allocate() function has been called first or that vertex,
// line, polygon, and triangle strip arrays have been supplied.
// Note: will also insert VTK_PIXEL, but converts it to VTK_QUAD.
int vtkPolyData::InsertNextCell(int type, vtkIdList &pts)
{
  int id;
  int npts=pts.GetNumberOfIds();

  if ( !this->Cells ) 
    {
    this->Cells = new vtkCellTypes(5000,10000);
    }

  switch (type)
    {
    case VTK_VERTEX: case VTK_POLY_VERTEX:
      this->Verts->InsertNextCell(pts);
      id = this->Cells->InsertNextCell(type, this->Verts->GetInsertLocation(npts));
      break;

    case VTK_LINE: case VTK_POLY_LINE:
      this->Lines->InsertNextCell(pts);
      id = this->Cells->InsertNextCell(type, this->Lines->GetInsertLocation(npts));
      break;

    case VTK_TRIANGLE: case VTK_QUAD: case VTK_POLYGON:
      this->Polys->InsertNextCell(pts);
      id = this->Cells->InsertNextCell(type, this->Polys->GetInsertLocation(npts));
      break;

    case VTK_PIXEL: //need to rearrange vertices
      {
      static int pixPts[4];
      pixPts[0] = pts.GetId(0);
      pixPts[1] = pts.GetId(1);
      pixPts[2] = pts.GetId(3);
      pixPts[3] = pts.GetId(2);
      this->Polys->InsertNextCell(4,pixPts);
      id = this->Cells->InsertNextCell(VTK_QUAD, this->Polys->GetInsertLocation(npts));
      break;
      }

    case VTK_TRIANGLE_STRIP:
      this->Strips->InsertNextCell(pts);
      id = this->Cells->InsertNextCell(type, this->Strips->GetInsertLocation(npts));
      break;

    default:
      id = -1;
      vtkErrorMacro(<<"Bad cell type! Can't insert!");
    }

  return id;
}

// Recover extra allocated memory when creating data whose initial size
// is unknown. Examples include using the InsertNextCell() method, or
// when using the CellArray::EstimateSize() method to create vertices,
// lines, polygons, or triangle strips.
void vtkPolyData::Squeeze()
{
  if ( this->Verts != NULL )
    {
    this->Verts->Squeeze();
    }
  if ( this->Lines != NULL )
    {
    this->Lines->Squeeze();
    }
  if ( this->Polys != NULL )
    {
    this->Polys->Squeeze();
    }
  if ( this->Strips != NULL )
    {
    this->Strips->Squeeze();
    }

  vtkPointSet::Squeeze();
}

// Begin inserting data all over again. Memory is not freed but otherwise
// objects are returned to their initial state.
void vtkPolyData::Reset()
{
  if ( this->Verts != NULL )
    {
    this->Verts->Reset();
    }
  if ( this->Lines != NULL )
    {
    this->Lines->Reset();
    }
  if ( this->Polys != NULL )
    {
    this->Polys->Reset();
    }
  if ( this->Strips != NULL )
    {
    this->Strips->Reset();
    }
}

// Reverse the order of point ids defining the cell.
void vtkPolyData::ReverseCell(int cellId)
{
  int loc, type;

  if ( this->Cells == NULL )
    {
    this->BuildCells();
    }
  loc = this->Cells->GetCellLocation(cellId);
  type = this->Cells->GetCellType(cellId);

  switch (type)
    {
    case VTK_VERTEX: case VTK_POLY_VERTEX:
     this->Verts->ReverseCell(loc);
     break;

    case VTK_LINE: case VTK_POLY_LINE:
      this->Lines->ReverseCell(loc);
      break;

    case VTK_TRIANGLE: case VTK_QUAD: case VTK_POLYGON:
      this->Polys->ReverseCell(loc);
      break;

    case VTK_TRIANGLE_STRIP:
      this->Strips->ReverseCell(loc);
      break;

    default:
      break;
    }
}

// Add a point to the cell data structure (after cell pointers have been
// built). This method adds the point and then allocates memory for the
// links to the cells.  (To use this method, make sure points are available
// and BuildLinks() has been invoked.)
int vtkPolyData::InsertNextLinkedPoint(float x[3], int numLinks)
{
  this->Links->InsertNextPoint(numLinks);
  return this->Points->InsertNextPoint(x);
}

// Add a new cell to the cell data structure (after cell pointers have been
// built). This method adds the cell and then updates the links from the points
// to the cells. (Memory is allocated as necessary.)
int vtkPolyData::InsertNextLinkedCell(int type, int npts, int *pts)
{
  int i, id;

  id = this->InsertNextCell(type,npts,pts);

  for (i=0; i<npts; i++)
    {
    this->Links->ResizeCellList(pts[i],1);
    this->Links->AddCellReference(id,pts[i]);  
    }

  return id;
}

// Remove a reference to a cell in a particular point's link list. You may also
// consider using RemoveCellReference() to remove the references from all the 
// cell's points to the cell. This operator does not reallocate memory; use the
// operator ResizeCellList() to do this if necessary.
void vtkPolyData::RemoveReferenceToCell(int ptId, int cellId)
{
  this->Links->RemoveCellReference(cellId, ptId);  
}

// Add a reference to a cell in a particular point's link list. (You may also
// consider using AddCellReference() to add the references from all the 
// cell's points to the cell.) This operator does not realloc memory; use the
// operator ResizeCellList() to do this if necessary.
void vtkPolyData::AddReferenceToCell(int ptId, int cellId)
{
  this->Links->AddCellReference(cellId, ptId);  
}

// Replace the points defining cell "cellId" with a new set of points. This
// operator is (typically) used when links from points to cells have not been 
// built (i.e., BuildLinks() has not been executed). Use the operator 
// ReplaceLinkedCell() to replace a cell when cell structure has been built.
void vtkPolyData::ReplaceCell(int cellId, int npts, int *pts)
{
  int loc, type;

  if ( this->Cells == NULL )
    {
    this->BuildCells();
    }
  loc = this->Cells->GetCellLocation(cellId);
  type = this->Cells->GetCellType(cellId);

  switch (type)
    {
    case VTK_VERTEX: case VTK_POLY_VERTEX:
     this->Verts->ReplaceCell(loc,npts,pts);
     break;

    case VTK_LINE: case VTK_POLY_LINE:
      this->Lines->ReplaceCell(loc,npts,pts);
      break;

    case VTK_TRIANGLE: case VTK_QUAD: case VTK_POLYGON:
      this->Polys->ReplaceCell(loc,npts,pts);
      break;

    case VTK_TRIANGLE_STRIP:
      this->Strips->ReplaceCell(loc,npts,pts);
      break;

    default:
      break;
    }
}

// Replace one cell with another in cell structure. This operator updates the
// connectivity list and the point's link list. It does not delete references
// to the old cell in the point's link list. Use the operator 
// RemoveCellReference() to delete all references from points to (old) cell.
// You may also want to consider using the operator ResizeCellList() if the 
// link list is changing size.
void vtkPolyData::ReplaceLinkedCell(int cellId, int npts, int *pts)
{
  int loc = this->Cells->GetCellLocation(cellId);
  int type = this->Cells->GetCellType(cellId);

  switch (type)
    {
    case VTK_VERTEX: case VTK_POLY_VERTEX:
     this->Verts->ReplaceCell(loc,npts,pts);
     break;

    case VTK_LINE: case VTK_POLY_LINE:
      this->Lines->ReplaceCell(loc,npts,pts);
      break;

    case VTK_TRIANGLE: case VTK_QUAD: case VTK_POLYGON:
      this->Polys->ReplaceCell(loc,npts,pts);
      break;

    case VTK_TRIANGLE_STRIP:
      this->Strips->ReplaceCell(loc,npts,pts);
      break;

    default:
      npts = 0;
    }

  for (int i=0; i < npts; i++)
    {
    this->Links->InsertNextCellReference(pts[i],cellId);
    }
}

// Get the neighbors at an edge. More efficient than the general 
// GetCellNeighbors(). Assumes links have been built (with BuildLinks()), 
// and looks specifically for edge neighbors.
void vtkPolyData::GetCellEdgeNeighbors(int cellId, int p1, int p2,
                                      vtkIdList& cellIds)
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
      for (j=0; j < npts; j++)
	{
	if ( pts[j] == p2 )
	  {
	  break;
	  }
	}
      if ( j < npts )
	{
	cellIds.InsertNextId(cells[i]);
	}
      }
    }
}

void vtkPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPointSet::PrintSelf(os,indent);

  os << indent << "Number Of Vertices: " << this->GetNumberOfVerts() << "\n";
  os << indent << "Number Of Lines: " << this->GetNumberOfLines() << "\n";
  os << indent << "Number Of Polygons: " << this->GetNumberOfPolys() << "\n";
  os << indent << "Number Of Triangle Strips: " << this->GetNumberOfStrips() << "\n";
}

