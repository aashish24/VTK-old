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
#include "vtkMergeFilter.hh"
#include "vtkPolyData.hh"

vtkMergeFilter::vtkMergeFilter()
{
  // prevents dangling reference to DataSet
  this->Geometry = new vtkPolyData;

  this->Scalars = NULL;
  this->Vectors = NULL;
  this->Normals = NULL;
  this->TCoords = NULL;
  this->Tensors = NULL;
  this->UserDefined = NULL;
}

vtkMergeFilter::~vtkMergeFilter()
{
}

void vtkMergeFilter::Update()
{
  unsigned long int mtime=0, dsMtime;

  // prevent chasing our tail
  if (this->Updating) return;

  this->Updating = 1;
  this->Geometry->Update();
  if ( this->Scalars ) 
    {
    this->Scalars->Update();
    dsMtime = this->Scalars->GetMTime();
    if ( dsMtime > mtime ) mtime = dsMtime;
    }
  if ( this->Vectors )
    {
    this->Vectors->Update();
    dsMtime = this->Vectors->GetMTime();
    if ( dsMtime > mtime ) mtime = dsMtime;
    }
  if ( this->Normals )
    {
    this->Normals->Update();
    dsMtime = this->Normals->GetMTime();
    if ( dsMtime > mtime ) mtime = dsMtime;
    }
  if ( this->TCoords )
    {
    this->TCoords->Update();
    dsMtime = this->TCoords->GetMTime();
    if ( dsMtime > mtime ) mtime = dsMtime;
    }
  if ( this->Tensors )
    {
    this->Tensors->Update();
    dsMtime = this->Tensors->GetMTime();
    if ( dsMtime > mtime ) mtime = dsMtime;
    }
  if ( this->UserDefined )
    {
    this->UserDefined->Update();
    dsMtime = this->UserDefined->GetMTime();
    if ( dsMtime > mtime ) mtime = dsMtime;
    }
  this->Updating = 0;

  if (mtime > this->GetMTime() || this->GetMTime() > this->ExecuteTime ||
  this->GetDataReleased() )
    {
    if ( this->StartMethod ) (*this->StartMethod)(this->StartMethodArg);
    this->Execute();
    this->ExecuteTime.Modified();
    this->SetDataReleased(0);
    if ( this->EndMethod ) (*this->EndMethod)(this->EndMethodArg);
    }
  
  if ( this->Geometry->ShouldIReleaseData() ) this->Geometry->ReleaseData();

  if ( this->Scalars && this->Scalars->ShouldIReleaseData() ) 
    this->Scalars->ReleaseData();

  if ( this->Vectors && this->Vectors->ShouldIReleaseData() ) 
    this->Vectors->ReleaseData();

  if ( this->Normals && this->Normals->ShouldIReleaseData() ) 
    this->Normals->ReleaseData();

  if ( this->TCoords && this->TCoords->ShouldIReleaseData() ) 
    this->TCoords->ReleaseData();

  if ( this->Tensors && this->Tensors->ShouldIReleaseData() ) 
    this->Tensors->ReleaseData();

  if ( this->UserDefined && this->UserDefined->ShouldIReleaseData() ) 
    this->UserDefined->ReleaseData();
}

void vtkMergeFilter::Initialize()
{
  if ( this->Geometry )
    {
    // copies input geometry to internal data set
    this->Geometry->Delete();
    this->Geometry = this->Geometry->MakeObject(); 
    }
  else
    {
    return;
    }
}

int vtkMergeFilter::GetDataReleased()
{
  return this->DataReleased;
}

void vtkMergeFilter::SetDataReleased(int flag)
{
  this->DataReleased = flag;
}

// Merge it all together
void vtkMergeFilter::Execute()
{
  int numPts, numScalars=0, numVectors=0, numNormals=0, numTCoords=0;
  int numTensors=0, numUserDefined=0;
  vtkPointData *pd;
  vtkScalars *scalars;
  vtkVectors *vectors;
  vtkNormals *normals;
  vtkTCoords *tcoords;
  vtkTensors *tensors;
  vtkUserDefined *ud;

  vtkDebugMacro(<<"Merging data!");

  // geometry is created
  this->Initialize();

  if ( (numPts = this->Geometry->GetNumberOfPoints()) < 1 )
    {
    vtkErrorMacro(<<"Nothing to merge!");
    return;
    }
  
  if ( this->Scalars ) 
    {
    pd = this->Scalars->GetPointData();
    scalars = pd->GetScalars();
    if ( scalars != NULL ) numScalars= scalars->GetNumberOfScalars();
    }

  if ( this->Vectors ) 
    {
    pd = this->Vectors->GetPointData();
    vectors = pd->GetVectors();
    if ( vectors != NULL ) numVectors= vectors->GetNumberOfVectors();
    }

  if ( this->Normals ) 
    {
    pd = this->Normals->GetPointData();
    normals = pd->GetNormals();
    if ( normals != NULL ) numNormals= normals->GetNumberOfNormals();
    }

  if ( this->TCoords ) 
    {
    pd = this->TCoords->GetPointData();
    tcoords = pd->GetTCoords();
    if ( tcoords != NULL ) numTCoords= tcoords->GetNumberOfTCoords();
    }

  if ( this->Tensors ) 
    {
    pd = this->Tensors->GetPointData();
    tensors = pd->GetTensors();
    if ( tensors != NULL ) numTensors = tensors->GetNumberOfTensors();
    }

  if ( this->UserDefined ) 
    {
    pd = this->UserDefined->GetPointData();
    ud = pd->GetUserDefined();
    if ( ud != NULL ) numUserDefined = ud->GetNumberOfUserDefined();
    }

  // merge data only if it is consistent
  if ( numPts == numScalars )
    this->PointData.SetScalars(scalars);

  if ( numPts == numVectors )
    this->PointData.SetVectors(vectors);
    
  if ( numPts == numNormals )
    this->PointData.SetNormals(normals);

  if ( numPts == numTCoords )
    this->PointData.SetTCoords(tcoords);

  if ( numPts == numTensors )
    this->PointData.SetTensors(tensors);

  if ( numPts == numUserDefined )
    this->PointData.SetUserDefined(ud);
}

void vtkMergeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkFilter::_PrintSelf(os,indent);
  vtkDataSet::PrintSelf(os,indent);

  os << indent << "Geometry: (" << this->Geometry << ")\n";
  os << indent << "Geometry type: " << this->Geometry->GetClassName() << "\n";

  if ( this->Scalars )
    os << indent << "Scalars: (" << this->Scalars << ")\n";
  else
    os << indent << "Scalars: (none)\n";

  if ( this->Vectors )
    os << indent << "Vectors: (" << this->Vectors << ")\n";
  else
    os << indent << "Vectors: (none)\n";

  if ( this->Normals )
    os << indent << "Normals: (" << this->Normals << ")\n";
  else
    os << indent << "Normals: (none)\n";

  if ( this->TCoords )
    os << indent << "TCoords: (" << this->TCoords << ")\n";
  else
    os << indent << "TCoords: (none)\n";

  if ( this->Tensors )
    os << indent << "Tensors: (" << this->Tensors << ")\n";
  else
    os << indent << "Tensors: (none)\n";

  if ( this->UserDefined )
    os << indent << "UserDefined: (" << this->UserDefined << ")\n";
  else
    os << indent << "UserDefined: (none)\n";
}

