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


=========================================================================*/
#include "vtkBYUReader.hh"
#include "vtkFloatPoints.hh"
#include "vtkFloatVectors.hh"
#include "vtkFloatScalars.hh"
#include "vtkFloatTCoords.hh"

vtkBYUReader::vtkBYUReader()
{
  this->GeometryFilename = NULL;
  this->DisplacementFilename = NULL;
  this->ScalarFilename = NULL;
  this->TextureFilename = NULL;

  this->ReadDisplacement = 1;
  this->ReadScalar = 1;
  this->ReadTexture = 1;

  this->PartNumber = 0;
}

vtkBYUReader::~vtkBYUReader()
{
  if ( this->GeometryFilename ) delete [] this->GeometryFilename;
  if ( this->DisplacementFilename ) delete [] this->DisplacementFilename;
  if ( this->ScalarFilename ) delete [] this->ScalarFilename;
  if ( this->TextureFilename ) delete [] this->TextureFilename;
}

void vtkBYUReader::Execute()
{
  FILE *geomFp;
  int numPts;

  if ((geomFp = fopen(this->GeometryFilename, "r")) == NULL)
    {
    vtkErrorMacro(<< "Geometry file: " << this->GeometryFilename << " not found");
    return;
    }
  else
    {
    this->ReadGeometryFile(geomFp,numPts);
    }

  this->ReadDisplacementFile(numPts);
  this->ReadScalarFile(numPts);
  this->ReadTextureFile(numPts);
}

void vtkBYUReader::ReadGeometryFile(FILE *geomFile, int &numPts)
{
  int numParts, numPolys, numEdges;
  int partStart, partEnd;
  int i;
  vtkFloatPoints *newPts;
  vtkCellArray *newPolys;
  float x[3];
  vtkIdList pts(VTK_CELL_SIZE);
  int polyId, pt;
  vtkPolyData *output = this->GetOutput();
  
  //
  // Read header (not using fixed format! - potential problem in some files.)
  //
  fscanf (geomFile, "%d %d %d %d", &numParts, &numPts, &numPolys, &numEdges);

  if ( this->PartNumber > numParts )
    {
    vtkWarningMacro(<<"Specified part number > number of parts");
    this->PartNumber = 0;
    }

  if ( this->PartNumber > 0 ) // read just part specified
    {
    vtkDebugMacro(<<"Reading part number: " << this->PartNumber);
    for (i=0; i < (this->PartNumber-1); i++) fscanf (geomFile, "%*d %*d");
    fscanf (geomFile, "%d %d", &partStart, &partEnd);
    for (i=this->PartNumber; i < numParts; i++) fscanf (geomFile, "%*d %*d");
    }
  else // read all parts
    {
    vtkDebugMacro(<<"Reading all parts.");
    for (i=0; i < numParts; i++) fscanf (geomFile, "%*d %*d");
    partStart = 1;
    partEnd = VTK_LARGE_INTEGER;
    }

  if ( numParts < 1 || numPts < 1 || numPolys < 1 )
    {
    vtkErrorMacro(<<"Bad MOVIE.BYU file");
    return;
    }
//
// Allocate data objects
//
  newPts = new vtkFloatPoints(numPts);
  newPolys = new vtkCellArray(numPolys+numEdges);
//
// Read data
//
  // read point coordinates
  for (i=0; i<numPts; i++)
    {
    fscanf(geomFile, "%e %e %e", x, x+1, x+2);
    newPts->InsertPoint(i,x);
    }

  // read poly data. Have to fix 1-offset. Only reading part number specified.
  for ( polyId=1; polyId <= numPolys; polyId++ )
    {
    // read this polygon
    for ( pts.Reset(); fscanf(geomFile, "%d", &pt) && pt > 0; )
      {
      pts.InsertNextId(pt-1);//convert to vtk 0-offset
      }
    pts.InsertNextId(-(pt+1));

    // Insert polygon (if in selected part)
    if ( partStart <= polyId && polyId <= partEnd )
      {
      newPolys->InsertNextCell(pts);
      }
    }

  vtkDebugMacro(<<"Reading:" << numPts << " points, "
                 << numPolys << " polygons.");

  output->SetPoints(newPts);
  newPts->Delete();

  output->SetPolys(newPolys);
  newPolys->Delete();
}

void vtkBYUReader::ReadDisplacementFile(int numPts)
{
  FILE *dispFp;
  int i;
  float v[3];
  vtkFloatVectors *newVectors;
  vtkPolyData *output = this->GetOutput();
  
  if ( this->ReadDisplacement && this->DisplacementFilename )
    {
    if ( !(dispFp = fopen(this->DisplacementFilename, "r")) )
      {
      vtkErrorMacro (<<"Couldn't open displacement file");
      return;
      }
    }
  else return;
//
// Allocate and read data
//
  newVectors = new vtkFloatVectors(numPts);

  for (i=0; i<numPts; i++)
    {
    fscanf(dispFp, "%e %e %e", v, v+1, v+2);
    newVectors->SetVector(i,v);
    }

  vtkDebugMacro(<<"Read " << numPts << " displacements");

  output->GetPointData()->SetVectors(newVectors);
  newVectors->Delete();
}

void vtkBYUReader::ReadScalarFile(int numPts)
{
  FILE *scalarFp;
  int i;
  float s;
  vtkFloatScalars *newScalars;
  vtkPolyData *output = this->GetOutput();
  
  if ( this->ReadScalar && this->ScalarFilename )
    {
    if ( !(scalarFp = fopen(this->ScalarFilename, "r")) )
      {
      vtkErrorMacro (<<"Couldn't open scalar file");
      return;
      }
    }
  else return;
//
// Allocate and read data
//
  newScalars = new vtkFloatScalars(numPts);

  for (i=0; i<numPts; i++)
    {
    fscanf(scalarFp, "%e", &s);
    newScalars->SetScalar(i,s);
    }

  vtkDebugMacro(<<"Read " << numPts << " scalars");

  output->GetPointData()->SetScalars(newScalars);
  newScalars->Delete();
}

void vtkBYUReader::ReadTextureFile(int numPts)
{
  FILE *textureFp;
  int i;
  float t[2];
  vtkFloatTCoords *newTCoords;
  vtkPolyData *output = this->GetOutput();

  if ( this->ReadTexture && this->TextureFilename )
    {
    if ( !(textureFp = fopen(this->TextureFilename, "r")) )
      {
      vtkErrorMacro (<<"Couldn't open texture file");
      return;
      }
    }
  else return;
//
// Allocate and read data
//
  newTCoords = new vtkFloatTCoords(numPts,2);

  for (i=0; i<numPts; i++)
    {
    fscanf(textureFp, "%e %e", t, t+1);
    newTCoords->SetTCoord(i,t);
    }

  vtkDebugMacro(<<"Read " << numPts << " texture coordinates");

  output->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();
}

void vtkBYUReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolySource::PrintSelf(os,indent);

  os << indent << "Geometry Filename: " 
     << (this->GeometryFilename ? this->GeometryFilename : "(none)") << "\n";
  os << indent << "Read Displacement: " << (this->ReadDisplacement ? "On\n" : "Off\n");
  os << indent << "Displacement Filename: " 
     << (this->DisplacementFilename ? this->DisplacementFilename : "(none)") << "\n";
  os << indent << "Read Scalar: " << (this->ReadScalar ? "On\n" : "Off\n");
  os << indent << "Scalar Filename: " 
     << (this->ScalarFilename ? this->ScalarFilename : "(none)") << "\n";
  os << indent << "Read Texture: " << (this->ReadTexture ? "On\n" : "Off\n");
  os << indent << "Texture Filename: " 
     << (this->TextureFilename ? this->TextureFilename : "(none)") << "\n";
}

