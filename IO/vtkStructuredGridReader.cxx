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
#include "vtkStructuredGridReader.h"

vtkStructuredGridReader::vtkStructuredGridReader()
{
  this->Reader.SetSource(this);
}

unsigned long int vtkStructuredGridReader::GetMTime()
{
  unsigned long dtime = this->vtkSource::GetMTime();
  unsigned long rtime = this->Reader.GetMTime();
  return (dtime > rtime ? dtime : rtime);
}

// Description:
// Specify file name of vtk polygonal data file to read.
void vtkStructuredGridReader::SetFileName(char *name) 
{
  this->Reader.SetFileName(name);
}
char *vtkStructuredGridReader::GetFileName() 
{
  return this->Reader.GetFileName();
}

// Description:
// Get the type of file (VTK_ASCII or VTK_BINARY)
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

// Description:
// Set the name of the field data to extract. If not specified, uses 
// first field data encountered in file.
void vtkStructuredGridReader::SetFieldDataName(char *name) 
{
  this->Reader.SetFieldDataName(name);
}
char *vtkStructuredGridReader::GetFieldDataName() 
{
  return this->Reader.GetFieldDataName();
}

void vtkStructuredGridReader::Execute()
{
  int numPts=0, npts, numCells=0, ncells;
  char line[256];
  int dimsRead=0;
  vtkStructuredGrid *output=(vtkStructuredGrid *)this->Output;
  
  vtkDebugMacro(<<"Reading vtk structured grid file...");
  if ( this->Debug ) this->Reader.DebugOn();
  else this->Reader.DebugOff();

  if (!this->Reader.OpenVTKFile() || !this->Reader.ReadHeader())
      return;
//
// Read structured grid specific stuff
//
  if (!this->Reader.ReadString(line))
    {
    vtkErrorMacro(<<"Data file ends prematurely!");
    this->Reader.CloseVTKFile ();
    return;
    }

  if ( !strncmp(this->Reader.LowerCase(line),"dataset",(unsigned long)7) )
    {
//
// Make sure we're reading right type of geometry
//
    if (!this->Reader.ReadString(line))
      {
      vtkErrorMacro(<<"Data file ends prematurely!");
      this->Reader.CloseVTKFile ();
      return;
      } 

    if ( strncmp(this->Reader.LowerCase(line),"structured_grid",17) )
      {
      vtkErrorMacro(<< "Cannot read dataset type: " << line);
      this->Reader.CloseVTKFile ();
      return;
      }
//
// Read keyword and number of points
//
    while (1)
      {
      if (!this->Reader.ReadString(line)) break;

      if ( ! strncmp(this->Reader.LowerCase(line),"dimensions",10) )
        {
        int dim[3];
        if (!(this->Reader.Read(dim) && 
	      this->Reader.Read(dim+1) && 
	      this->Reader.Read(dim+2)))
          {
          vtkErrorMacro(<<"Error reading dimensions!");
          this->Reader.CloseVTKFile ();
          return;
          }

        numPts = dim[0] * dim[1] * dim[2];
        output->SetDimensions(dim);
	numCells = output->GetNumberOfCells();
        dimsRead = 1;
        }

      else if ( ! strncmp(line,"points",6) )
        {
        if (!this->Reader.Read(&npts))
          {
          vtkErrorMacro(<<"Error reading points!");
          this->Reader.CloseVTKFile ();
          return;
          }

        this->Reader.ReadPoints(output, npts);
        }

      else if ( ! strncmp(line, "cell_data", 9) )
        {
        if (!this->Reader.Read(&numCells))
          {
          vtkErrorMacro(<<"Cannot read cell data!");
          this->Reader.CloseVTKFile ();
          return;
          }
        
        if ( ncells != numCells )
          {
          vtkErrorMacro(<<"Number of cells don't match!");
          this->Reader.CloseVTKFile ();
          return;
          }

        this->Reader.ReadCellData(output, npts);
        break; //out of this loop
        }

      else if ( ! strncmp(line, "point_data", 10) )
        {
        if (!this->Reader.Read(&numPts))
          {
          vtkErrorMacro(<<"Cannot read point data!");
          this->Reader.CloseVTKFile ();
          return;
          }
        
        if ( npts != numPts )
          {
          vtkErrorMacro(<<"Number of points don't match!");
          this->Reader.CloseVTKFile ();
          return;
          }

        this->Reader.ReadPointData(output, npts);
        break; //out of this loop
        }

      else
        {
        vtkErrorMacro(<< "Unrecognized keyword: " << line);
        this->Reader.CloseVTKFile ();
        return;
        }
      }

      if ( !dimsRead ) vtkWarningMacro(<<"No dimensions read.");
      if ( !output->GetPoints() ) vtkWarningMacro(<<"No points read.");
    }

  else if ( !strncmp(line, "cell_data", 9) )
    {
    vtkWarningMacro(<<"No geometry defined in data file!");
    if (!this->Reader.Read(&ncells))
      {
      vtkErrorMacro(<<"Cannot read cell data!");
      this->Reader.CloseVTKFile ();
      return;
      }
    this->Reader.ReadCellData(output, ncells);
    }

  else if ( !strncmp(line, "point_data", 10) )
    {
    vtkWarningMacro(<<"No geometry defined in data file!");
    if (!this->Reader.Read(&npts))
      {
      vtkErrorMacro(<<"Cannot read point data!");
      this->Reader.CloseVTKFile ();
      return;
      }
    this->Reader.ReadPointData(output, npts);
    }

  else 
    {
    vtkErrorMacro(<< "Unrecognized keyword: " << line);
    }
    this->Reader.CloseVTKFile ();
}

void vtkStructuredGridReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkStructuredGridSource::PrintSelf(os,indent);
  this->Reader.PrintSelf(os,indent);
}
