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
#include "TenGlyph.hh"
#include "Trans.hh"
#include "FVectors.hh"
#include "FNormals.hh"
#include "vtkMath.hh"

// Description
// Construct object with scaling on and scale factor 1.0. Eigenvalues are 
// extracted, glyphs are colored with input scalar data, and logarithmic
// scaling is turned off.
vtkTensorGlyph::vtkTensorGlyph()
{
  this->Source = NULL;
  this->Scaling = 1;
  this->ScaleFactor = 1.0;
  this->ExtractEigenvalues = 1;
  this->ColorGlyphs = 1;
  this->LogScaling = 1;
}

vtkTensorGlyph::~vtkTensorGlyph()
{
}

void vtkTensorGlyph::Execute()
{
  vtkPointData *pd;
  vtkTensors *inTensors;
  vtkTensor *tensor;
  vtkScalars *inScalars;
  int numPts, numSourcePts, numSourceCells;
  int inPtId, i, j;
  vtkPoints *sourcePts;
  vtkNormals *sourceNormals;
  vtkCellArray *sourceCells;  
  vtkFloatPoints *newPts;
  vtkFloatScalars *newScalars=NULL;
  vtkFloatNormals *newNormals=NULL;
  float *x, s;
  vtkTransform trans;
  vtkCell *cell;
  vtkIdList *cellPts;
  int npts, pts[MAX_CELL_SIZE];
  int ptIncr, cellId;
  vtkMath math;
  vtkMatrix4x4 matrix;
  double *m[3], w[3], *e[3];
  double m0[3], m1[3], m2[3];
  double e0[3], e1[3], e2[3];
  float xv[3], yv[3], zv[3];

  // set up working matrices
  m[0] = m0; m[1] = m1; m[2] = m2; 
  e[0] = e0; e[1] = e1; e[2] = e2; 

  vtkDebugMacro(<<"Generating tensor glyphs");
  this->Initialize();

  pd = this->Input->GetPointData();
  inTensors = pd->GetTensors();
  inScalars = pd->GetScalars();
  numPts = this->Input->GetNumberOfPoints();

  if ( !inTensors || numPts < 1 )
    {
    vtkErrorMacro(<<"No data to glyph!");
    return;
    }
//
// Allocate storage for output PolyData
//
  sourcePts = this->Source->GetPoints();
  numSourcePts = sourcePts->GetNumberOfPoints();
  numSourceCells = this->Source->GetNumberOfCells();

  newPts = new vtkFloatPoints(numPts*numSourcePts);

  // Setting up for calls to PolyData::InsertNextCell()
  if ( (sourceCells=this->Source->GetVerts())->GetNumberOfCells() > 0 )
    {
    this->SetVerts(new vtkCellArray(numPts*sourceCells->GetSize()));
    }
  if ( (sourceCells=this->Source->GetLines())->GetNumberOfCells() > 0 )
    {
    this->SetLines(new vtkCellArray(numPts*sourceCells->GetSize()));
    }
  if ( (sourceCells=this->Source->GetPolys())->GetNumberOfCells() > 0 )
    {
    this->SetPolys(new vtkCellArray(numPts*sourceCells->GetSize()));
    }
  if ( (sourceCells=this->Source->GetStrips())->GetNumberOfCells() > 0 )
    {
    this->SetStrips(new vtkCellArray(numPts*sourceCells->GetSize()));
    }

  // only copy scalar data through
  pd = this->Source->GetPointData();
  if ( inScalars &&  this->ColorGlyphs ) 
    newScalars = new vtkFloatScalars(numPts*numSourcePts);
  else
    {
    this->PointData.CopyAllOff();
    this->PointData.CopyScalarsOn();
    this->PointData.CopyAllocate(pd,numPts*numSourcePts);
    }
  if ( sourceNormals = pd->GetNormals() )
    newNormals = new vtkFloatNormals(numPts*numSourcePts);
//
// First copy all topology (transformation independent)
//
  for (inPtId=0; inPtId < numPts; inPtId++)
    {
    ptIncr = inPtId * numSourcePts;
    for (cellId=0; cellId < numSourceCells; cellId++)
      {
      cell = this->Source->GetCell(cellId);
      cellPts = cell->GetPointIds();
      npts = cellPts->GetNumberOfIds();
      for (i=0; i < npts; i++) pts[i] = cellPts->GetId(i) + ptIncr;
      this->InsertNextCell(cell->GetCellType(),npts,pts);
      }
    }
//
// Traverse all Input points, transforming glyph at Source points
//
  trans.PreMultiply();

  for (inPtId=0; inPtId < numPts; inPtId++)
    {
    ptIncr = inPtId * numSourcePts;
    
    trans.Identity();

    // translate Source to Input point
    x = this->Input->GetPoint(inPtId);
    trans.Translate(x[0], x[1], x[2]);

    tensor = inTensors->GetTensor(inPtId);

    // compute orientation vectors and scale factors from tensor
    if ( this->ExtractEigenvalues ) // extract appropriate eigenfunctions
      {
      for (j=0; j<3; j++)
        for (i=0; i<3; i++)
          m[i][j] = tensor->GetComponent(i,j);

      math.SingularValueDecomposition(m,3,3,w,e);

      //copy eigenvectors
      xv[0] = e[0][0]; xv[1] = e[1][0]; xv[2] = e[2][0];
      yv[0] = e[0][1]; yv[1] = e[1][1]; yv[2] = e[2][1];
      zv[0] = e[0][2]; zv[1] = e[1][2]; zv[2] = e[2][2];
      }
    else //use tensor columns as eigenvectors
      {
      for (i=0; i<3; i++)
        {
        w[i] = 1.0;
        xv[i] = tensor->GetComponent(i,0);
        yv[i] = tensor->GetComponent(i,1);
        zv[i] = tensor->GetComponent(i,2);
        }
      }

    // normalize eigenvectors / compute scale factors
    w[0] = fabs(w[0] * math.Normalize(xv) * this->ScaleFactor);
    w[1] = fabs(w[1] * math.Normalize(yv) * this->ScaleFactor);
    w[2] = fabs(w[2] * math.Normalize(zv) * this->ScaleFactor);
    
    if ( this->LogScaling )
      {
      for (i=0; i<3; i++)
        if ( w[i] != 0.0 ) 
          if ( (w[i]=log10(w[i])) < 0.0 ) w[i] = -1.0/w[i];
      }

    // normalized eigenvectors rotate object
    matrix.Element[0][0] = xv[0];
    matrix.Element[0][1] = yv[0];
    matrix.Element[0][2] = zv[0];
    matrix.Element[1][0] = xv[1];
    matrix.Element[1][1] = yv[1];
    matrix.Element[1][2] = zv[1];
    matrix.Element[2][0] = xv[2];
    matrix.Element[2][1] = yv[2];
    matrix.Element[2][2] = zv[2];
    trans.Concatenate(matrix);
    trans.Scale(w[0], w[1], w[2]);

    // multiply points (and normals if available) by resulting matrix
    trans.MultiplyPoints(sourcePts,newPts);
    if ( newNormals )
      trans.MultiplyNormals(sourceNormals,newNormals);

    // Copy point data from source
    if ( inScalars && this->ColorGlyphs ) 
      {
      s = inScalars->GetScalar(inPtId);
      for (i=0; i < numSourcePts; i++) 
        newScalars->InsertScalar(ptIncr+i, s);
      }
    else
      {
      for (i=0; i < numSourcePts; i++) 
        this->PointData.CopyData(pd,i,ptIncr+i);
      }
    }
//
// Update ourselves
//
  this->SetPoints(newPts);
  if ( newScalars ) this->PointData.SetScalars(newScalars);
  if ( newNormals ) this->PointData.SetNormals(newNormals);
  this->Squeeze();
}

// Description:
// Override update method because execution can branch two ways (Input 
// and Source)
void vtkTensorGlyph::Update()
{
  // make sure input is available
  if ( this->Input == NULL || this->Source == NULL )
    {
    vtkErrorMacro(<< "No input!");
    return;
    }

  // prevent chasing our tail
  if (this->Updating) return;

  this->Updating = 1;
  this->Input->Update();
  this->Source->Update();
  this->Updating = 0;

  if (this->Input->GetMTime() > this->GetMTime() || 
  this->Source->GetMTime() > this->GetMTime() || 
  this->GetMTime() > this->ExecuteTime || this->GetDataReleased() )
    {
    if ( this->StartMethod ) (*this->StartMethod)(this->StartMethodArg);
    this->Execute();
    this->ExecuteTime.Modified();
    this->SetDataReleased(0);
    if ( this->EndMethod ) (*this->EndMethod)(this->EndMethodArg);
    }

  if ( this->Input->ShouldIReleaseData() ) this->Input->ReleaseData();
  if ( this->Source->ShouldIReleaseData() ) this->Source->ReleaseData();
}

void vtkTensorGlyph::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToPolyFilter::PrintSelf(os,indent);

  os << indent << "Source: " << this->Source << "\n";
  os << indent << "Scaling: " << (this->Scaling ? "On\n" : "Off\n");
  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
  os << indent << "Extract Eigenvalues: " << (this->ExtractEigenvalues ? "On\n" : "Off\n");
  os << indent << "Color Glyphs: " << (this->ColorGlyphs ? "On\n" : "Off\n");
  os << indent << "Log Scaling: " << (this->LogScaling ? "On\n" : "Off\n");
}

