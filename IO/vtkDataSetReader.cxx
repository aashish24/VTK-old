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
#include "vtkDSRdr.hh"
#include "vtkPolyR.hh"
#include "vtkSPtsR.hh"
#include "vtkSGrdR.hh"
#include "vtkUGrdR.hh"


vtkDataSetReader::vtkDataSetReader()
{
}

vtkDataSetReader::~vtkDataSetReader()
{
  if ( this->DataSet ) this->DataSet->Delete();
}

// Description:
// Specify file name of vtk data file to read.
void vtkDataSetReader::SetFilename(char *name) 
{
  this->Reader.SetFilename(name);
}
char *vtkDataSetReader::GetFilename() 
{
  return this->Reader.GetFilename();
}

// Description:
// Get the type of file (ASCII or BINARY)
int vtkDataSetReader::GetFileType() 
{
  return this->Reader.GetFileType();
}

// Description:
// Set the name of the scalar data to extract. If not specified, first 
// scalar data encountered is extracted.
void vtkDataSetReader::SetScalarsName(char *name) 
{
  this->Reader.SetScalarsName(name);
}
char *vtkDataSetReader::GetScalarsName() 
{
  return this->Reader.GetScalarsName();
}

// Description:
// Set the name of the vector data to extract. If not specified, first 
// vector data encountered is extracted.
void vtkDataSetReader::SetVectorsName(char *name) 
{
  this->Reader.SetVectorsName(name);
}
char *vtkDataSetReader::GetVectorsName() 
{
  return this->Reader.GetVectorsName();
}

// Description:
// Set the name of the tensor data to extract. If not specified, first 
// tensor data encountered is extracted.
void vtkDataSetReader::SetTensorsName(char *name) 
{
  this->Reader.SetTensorsName(name);
}
char *vtkDataSetReader::GetTensorsName() 
{
  return this->Reader.GetTensorsName();
}

// Description:
// Set the name of the normal data to extract. If not specified, first 
// normal data encountered is extracted.
void vtkDataSetReader::SetNormalsName(char *name) 
{
  this->Reader.SetNormalsName(name);
}
char *vtkDataSetReader::GetNormalsName() 
{
  return this->Reader.GetNormalsName();
}

// Description:
// Set the name of the texture coordinate data to extract. If not specified,
// first texture coordinate data encountered is extracted.
void vtkDataSetReader::SetTCoordsName(char *name) 
{
  this->Reader.SetTCoordsName(name);
}
char *vtkDataSetReader::GetTCoordsName() 
{
  return this->Reader.GetTCoordsName();
}

// Description:
// Set the name of the lookup table data to extract. If not specified, uses 
// lookup table named by scalar. Otherwise, this specification supersedes.
void vtkDataSetReader::SetLookupTableName(char *name) 
{
  this->Reader.SetLookupTableName(name);
}
char *vtkDataSetReader::GetLookupTableName() 
{
  return this->Reader.GetLookupTableName();
}


void vtkDataSetReader::Execute()
{
  FILE *fp;
  int retStat;
  char line[257];
  vtkDataSet *reader;

  vtkDebugMacro(<<"Reading vtk dataset...");
  this->Initialize();
  if ( this->Debug ) this->Reader.DebugOn();
  else this->Reader.DebugOff();

  if ( !(fp=this->Reader.OpenVTKFile()) || !this->Reader.ReadHeader(fp) )
      return;
//
// Determine dataset type
//
  if ( (retStat=fscanf(fp,"%256s",line)) == EOF || retStat < 1 ) 
    {
    vtkErrorMacro(<< "Premature EOF reading dataset keyword");
    return;
    }

  if ( !strncmp(this->Reader.LowerCase(line),"dataset",(unsigned long)7) )
    {
//
// See if type is recognized.
//
    if ( (retStat=fscanf(fp,"%256s",line)) == EOF || retStat < 1 ) 
      {
      vtkErrorMacro(<< "Premature EOF reading type");
      return;
      }

    rewind(fp);
    if ( ! strncmp(this->Reader.LowerCase(line),"polydata",8) )
      {
      vtkPolyReader *preader = new vtkPolyReader;
      preader->SetFilename(this->Reader.GetFilename());
      preader->SetScalarsName(this->Reader.GetScalarsName());
      preader->SetVectorsName(this->Reader.GetVectorsName());
      preader->SetNormalsName(this->Reader.GetNormalsName());
      preader->SetTensorsName(this->Reader.GetTensorsName());
      preader->SetTCoordsName(this->Reader.GetTCoordsName());
      preader->SetLookupTableName(this->Reader.GetLookupTableName());
      preader->Update();
      reader = (vtkDataSet *)preader;
      }

    else if ( ! strncmp(line,"structured_points",17) )
      {
      vtkStructuredPointsReader *preader = new vtkStructuredPointsReader;
      preader->SetFilename(this->Reader.GetFilename());
      preader->SetScalarsName(this->Reader.GetScalarsName());
      preader->SetVectorsName(this->Reader.GetVectorsName());
      preader->SetNormalsName(this->Reader.GetNormalsName());
      preader->SetTensorsName(this->Reader.GetTensorsName());
      preader->SetTCoordsName(this->Reader.GetTCoordsName());
      preader->SetLookupTableName(this->Reader.GetLookupTableName());
      preader->Update();
      reader = (vtkDataSet *)preader;
      }

    else if ( ! strncmp(line,"structured_grid",15) )
      {
      vtkStructuredGridReader *preader = new vtkStructuredGridReader;
      preader->SetFilename(this->Reader.GetFilename());
      preader->SetScalarsName(this->Reader.GetScalarsName());
      preader->SetVectorsName(this->Reader.GetVectorsName());
      preader->SetNormalsName(this->Reader.GetNormalsName());
      preader->SetTensorsName(this->Reader.GetTensorsName());
      preader->SetTCoordsName(this->Reader.GetTCoordsName());
      preader->SetLookupTableName(this->Reader.GetLookupTableName());
      preader->Update();
      reader = (vtkDataSet *)preader;
      }

    else if ( ! strncmp(line,"unstructured_grid",17) )
      {
      vtkUnstructuredGridReader *preader = new vtkUnstructuredGridReader;
      preader->SetFilename(this->Reader.GetFilename());
      preader->SetScalarsName(this->Reader.GetScalarsName());
      preader->SetVectorsName(this->Reader.GetVectorsName());
      preader->SetNormalsName(this->Reader.GetNormalsName());
      preader->SetTensorsName(this->Reader.GetTensorsName());
      preader->SetTCoordsName(this->Reader.GetTCoordsName());
      preader->SetLookupTableName(this->Reader.GetLookupTableName());
      preader->Update();
      reader = (vtkDataSet *)preader;
      }

    else
      {
      vtkErrorMacro(<< "Cannot read dataset type: " << line);
      return;
      }
    }
//
// Create appropriate dataset
//
  if ( this->DataSet ) this->DataSet->Delete();
  this->DataSet = reader;
  this->PointData = *(reader->GetPointData());

  return;
}

void vtkDataSetReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetSource::PrintSelf(os,indent);
  this->Reader.PrintSelf(os,indent);
}
