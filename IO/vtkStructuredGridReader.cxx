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
#include "vtkSGrdR.hh"

vtkStructuredGridReader::vtkStructuredGridReader()
{
}

vtkStructuredGridReader::~vtkStructuredGridReader()
{
}

unsigned long int vtkStructuredGridReader::GetMTime()
{
  unsigned long dtime = this->vtkStructuredGridSource::GetMTime();
  unsigned long rtime = this->Reader.GetMTime();
  return (dtime > rtime ? dtime : rtime);
}

// Description:
// Specify file name of vtk polygonal data file to read.
void vtkStructuredGridReader::SetFilename(char *name) 
{
  this->Reader.SetFilename(name);
}
char *vtkStructuredGridReader::GetFilename() 
{
  return this->Reader.GetFilename();
}

// Description:
// Get the type of file (ASCII or BINARY)
int vtkStructuredGridReader::GetFileType() 
{
  return this->Reader.GetFileType();
}

// Description:
// Set the name of the scalar data to extract. If not specified, first 
// scalar data encountered is extracted.
void vtkStructuredGridReader::SetScalarsName(char *name) 
{
  this->Reader.SetScalarsName(name);
}
char *vtkStructuredGridReader::GetScalarsName() 
{
  return this->Reader.GetScalarsName();
}

// Description:
// Set the name of the vector data to extract. If not specified, first 
// vector data encountered is extracted.
void vtkStructuredGridReader::SetVectorsName(char *name) 
{
  this->Reader.SetVectorsName(name);
}
char *vtkStructuredGridReader::GetVectorsName() 
{
  return this->Reader.GetVectorsName();
}

// Description:
// Set the name of the tensor data to extract. If not specified, first 
// tensor data encountered is extracted.
void vtkStructuredGridReader::SetTensorsName(char *name) 
{
  this->Reader.SetTensorsName(name);
}
char *vtkStructuredGridReader::GetTensorsName() 
{
  return this->Reader.GetTensorsName();
}

// Description:
// Set the name of the normal data to extract. If not specified, first 
// normal data encountered is extracted.
void vtkStructuredGridReader::SetNormalsName(char *name) 
{
  this->Reader.SetNormalsName(name);
}
char *vtkStructuredGridReader::GetNormalsName() 
{
  return this->Reader.GetNormalsName();
}

// Description:
// Set the name of the texture coordinate data to extract. If not specified,
// first texture coordinate data encountered is extracted.
void vtkStructuredGridReader::SetTCoordsName(char *name) 
{
  this->Reader.SetTCoordsName(name);
}
char *vtkStructuredGridReader::GetTCoordsName() 
{
  return this->Reader.GetTCoordsName();
}

// Description:
// Set the name of the lookup table data to extract. If not specified, uses 
// lookup table named by scalar. Otherwise, this specification supersedes.
void vtkStructuredGridReader::SetLookupTableName(char *name) 
{
  this->Reader.SetLookupTableName(name);
}
char *vtkStructuredGridReader::GetLookupTableName() 
{
  return this->Reader.GetLookupTableName();
}

void vtkStructuredGridReader::Execute()
{
  FILE *fp;
  int numPts=0, npts;
  int retStat;
  char line[257];
  int dimsRead=0;

  vtkDebugMacro(<<"Reading vtk structured grid file...");
  this->Initialize();
  if ( this->Debug ) this->Reader.DebugOn();
  else this->Reader.DebugOff();

  if ( !(fp=this->Reader.OpenVLFile()) || !this->Reader.ReadHeader(fp) )
      return;
//
// Read structured grid specific stuff
//
  if ( (retStat=fscanf(fp,"%256s",line)) == EOF || retStat < 1 ) 
    {
    vtkErrorMacro(<<"Data file ends prematurely!");
    return;
    }

  if ( !strncmp(this->Reader.LowerCase(line),"dataset",(unsigned long)7) )
    {
//
// Make sure we're reading right type of geometry
//
    if ( (retStat=fscanf(fp,"%256s",line)) == EOF || retStat < 1 ) 
      {
      vtkErrorMacro(<<"Data file ends prematurely!");
      return;
      } 

    if ( strncmp(this->Reader.LowerCase(line),"structured_grid",17) )
      {
      vtkErrorMacro(<< "Cannot read dataset type: " << line);
      return;
      }
//
// Read keyword and number of points
//
    while (1)
      {
      if ( (retStat=fscanf(fp,"%256s",line)) == EOF || retStat < 1 ) break;

      if ( ! strncmp(this->Reader.LowerCase(line),"dimensions",10) )
        {
        int dim[3];
        if ( (retStat=fscanf(fp,"%d %d %d",dim, dim+1, dim+2)) == EOF
        || retStat < 3 ) 
          {
          vtkErrorMacro(<<"Error reading dimensions!");
          return;
          }

        numPts = dim[0] * dim[1] * dim[2];
        this->SetDimensions(dim);
        dimsRead = 1;
        }

      else if ( ! strncmp(line,"points",6) )
        {
        if ( (retStat=fscanf(fp,"%d", &npts)) == EOF || retStat < 1 ) 
          {
          vtkErrorMacro(<<"Error reading points!");
          return;
          }

        this->Reader.ReadPoints(fp, (vtkPointSet *)this, npts);
        }

      else if ( ! strncmp(line, "point_data", 10) )
        {
        if ( (retStat=fscanf(fp,"%d", &numPts)) == EOF || retStat < 1 ) 
          {
          vtkErrorMacro(<<"Cannot read point data!");
          return;
          }
        
        if ( npts != numPts )
          {
          vtkErrorMacro(<<"Number of points don't match!");
          return;
          }

        this->Reader.ReadPointData(fp, (vtkDataSet *)this, npts);
        break; //out of this loop
        }

      else
        {
        vtkErrorMacro(<< "Unrecognized keyord: " << line);
        return;
        }
      }

      if ( !dimsRead ) vtkWarningMacro(<<"No dimensions read.");
      if ( !this->GetPoints() ) vtkWarningMacro(<<"No points read.");
    }

  else if ( !strncmp(line, "point_data", 10) )
    {
    vtkWarningMacro(<<"No geometry defined in data file!");
    if ( (retStat=fscanf(fp,"%d", &npts)) == EOF || retStat < 1 ) 
      {
      vtkErrorMacro(<<"Cannot read point data!");
      return;
      }
    }

  else 
    {
    vtkErrorMacro(<< "Unrecognized keyord: " << line);
    }
}

void vtkStructuredGridReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkStructuredGridSource::PrintSelf(os,indent);
  this->Reader.PrintSelf(os,indent);
}
