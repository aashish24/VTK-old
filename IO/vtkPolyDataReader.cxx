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
#include "vtkPolyDataReader.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkPolyDataReader* vtkPolyDataReader::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPolyDataReader");
  if(ret)
    {
    return (vtkPolyDataReader*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPolyDataReader;
}




vtkPolyDataReader::vtkPolyDataReader()
{
  this->Reader = vtkDataReader::New();
  this->Reader->SetSource(this);
}

vtkPolyDataReader::~vtkPolyDataReader()
{
  this->Reader->Delete();
  this->Reader = NULL;
}

unsigned long int vtkPolyDataReader::GetMTime()
{
  unsigned long dtime = this->vtkSource::GetMTime();
  unsigned long rtime = this->Reader->GetMTime();
  return (dtime > rtime ? dtime : rtime);
}

// Specify file name of vtk polygonal data file to read.
void vtkPolyDataReader::SetFileName(char *name) 
{
  this->Reader->SetFileName(name);
}
char *vtkPolyDataReader::GetFileName() 
{
  return this->Reader->GetFileName();
}

// Get the type of file (ASCII or BINARY)
int vtkPolyDataReader::GetFileType() 
{
  return this->Reader->GetFileType();
}

// Set the name of the scalar data to extract. If not specified, first 
// scalar data encountered is extracted.
void vtkPolyDataReader::SetScalarsName(char *name) 
{
  this->Reader->SetScalarsName(name);
}
char *vtkPolyDataReader::GetScalarsName() 
{
  return this->Reader->GetScalarsName();
}

// Set the name of the vector data to extract. If not specified, first 
// vector data encountered is extracted.
void vtkPolyDataReader::SetVectorsName(char *name) 
{
  this->Reader->SetVectorsName(name);
}
char *vtkPolyDataReader::GetVectorsName() 
{
  return this->Reader->GetVectorsName();
}

// Set the name of the tensor data to extract. If not specified, first 
// tensor data encountered is extracted.
void vtkPolyDataReader::SetTensorsName(char *name) 
{
  this->Reader->SetTensorsName(name);
}
char *vtkPolyDataReader::GetTensorsName() 
{
  return this->Reader->GetTensorsName();
}

// Set the name of the normal data to extract. If not specified, first 
// normal data encountered is extracted.
void vtkPolyDataReader::SetNormalsName(char *name) 
{
  this->Reader->SetNormalsName(name);
}
char *vtkPolyDataReader::GetNormalsName() 
{
  return this->Reader->GetNormalsName();
}

// Set the name of the texture coordinate data to extract. If not specified,
// first texture coordinate data encountered is extracted.
void vtkPolyDataReader::SetTCoordsName(char *name) 
{
  this->Reader->SetTCoordsName(name);
}
char *vtkPolyDataReader::GetTCoordsName() 
{
  return this->Reader->GetTCoordsName();
}

// Set the name of the lookup table data to extract. If not specified, uses 
// lookup table named by scalar. Otherwise, this specification supersedes.
void vtkPolyDataReader::SetLookupTableName(char *name) 
{
  this->Reader->SetLookupTableName(name);
}
char *vtkPolyDataReader::GetLookupTableName() 
{
  return this->Reader->GetLookupTableName();
}

// Set the name of the field data to extract. If not specified, uses 
// first field data encountered in file.
void vtkPolyDataReader::SetFieldDataName(char *name) 
{
  this->Reader->SetFieldDataName(name);
}
char *vtkPolyDataReader::GetFieldDataName() 
{
  return this->Reader->GetFieldDataName();
}

void vtkPolyDataReader::Execute()
{
  int numPts=0;
  char line[256];
  int npts, size, ncells;
  int done=0;
  vtkPolyData *output = this->GetOutput();

  vtkDebugMacro(<<"Reading vtk polygonal data...");

  if ( this->Debug )
    {
    this->Reader->DebugOn();
    }
  else
    {
    this->Reader->DebugOff();
    }

  if ( !(this->Reader->OpenVTKFile()) || !this->Reader->ReadHeader())
    {
    return;
    }
//
// Read polygonal data specific stuff
//
  if (!this->Reader->ReadString(line))
    {
    vtkErrorMacro(<<"Data file ends prematurely!");
    this->Reader->CloseVTKFile ();
    return;
    }

  if ( !strncmp(this->Reader->LowerCase(line),"dataset",(unsigned long)7) )
    {
//
// Make sure we're reading right type of geometry
//
    if (!this->Reader->ReadString(line))
      {
      vtkErrorMacro(<<"Data file ends prematurely!");
      this->Reader->CloseVTKFile ();
      return;
      } 

    if ( strncmp(this->Reader->LowerCase(line),"polydata",8) )
      {
      vtkErrorMacro(<< "Cannot read dataset type: " << line);
      this->Reader->CloseVTKFile ();
      return;
      }
//
// Might find points, vertices, lines, polygons, or triangle strips
//
    while (!done)
      {
      if (!this->Reader->ReadString(line))
	{
	break;
	}

      if ( ! strncmp(this->Reader->LowerCase(line),"points",6) )
        {
        if (!this->Reader->Read(&numPts))
          {
          vtkErrorMacro(<<"Cannot read number of points!");
          this->Reader->CloseVTKFile ();
          return;
          }

        this->Reader->ReadPoints(output, numPts);
        }

      else if ( ! strncmp(line,"vertices",8) )
        {
        vtkCellArray *verts = vtkCellArray::New();
        if (!(this->Reader->Read(&ncells) && this->Reader->Read(&size)))
          {
          vtkErrorMacro(<<"Cannot read vertices!");
          this->Reader->CloseVTKFile ();
          return;
          }

        this->Reader->ReadCells(size, verts->WritePointer(ncells,size));
        output->SetVerts(verts);
        verts->Delete();
        vtkDebugMacro(<<"Read " << ncells << " vertices");
        }

      else if ( ! strncmp(line,"lines",5) )
        {
        vtkCellArray *lines = vtkCellArray::New();
        if (!(this->Reader->Read(&ncells) && this->Reader->Read(&size)))
          {
          vtkErrorMacro(<<"Cannot read lines!");
          this->Reader->CloseVTKFile ();
          return;
          }

        this->Reader->ReadCells(size, lines->WritePointer(ncells,size));
        output->SetLines(lines);
        lines->Delete();
        vtkDebugMacro(<<"Read " << ncells << " lines");
        }

      else if ( ! strncmp(line,"polygons",8) )
        {
        vtkCellArray *polys = vtkCellArray::New();
        if (!(this->Reader->Read(&ncells) && this->Reader->Read(&size)))
          {
          vtkErrorMacro(<<"Cannot read polygons!");
          this->Reader->CloseVTKFile ();
          return;
          }

        this->Reader->ReadCells(size, polys->WritePointer(ncells,size));
        output->SetPolys(polys);
        polys->Delete();
        vtkDebugMacro(<<"Read " << ncells << " polygons");
        }

      else if ( ! strncmp(line,"triangle_strips",15) )
        {
        vtkCellArray *tris = vtkCellArray::New();
        if (!(this->Reader->Read(&ncells) && this->Reader->Read(&size)))
          {
          vtkErrorMacro(<<"Cannot read triangle strips!");
          this->Reader->CloseVTKFile ();
          return;
          }

        this->Reader->ReadCells(size, tris->WritePointer(ncells,size));
        output->SetStrips(tris);
        tris->Delete();
        vtkDebugMacro(<<"Read " << ncells << " triangle strips");
        }

      else if ( ! strncmp(line, "cell_data", 9) )
        {
        if (!this->Reader->Read(&ncells))
          {
          vtkErrorMacro(<<"Cannot read cell data!");
          this->Reader->CloseVTKFile ();
          return;
          }
        
        if ( ncells != output->GetNumberOfCells() )
          {
          vtkErrorMacro(<<"Number of cells don't match number data values!");
          return;
          }

        this->Reader->ReadCellData(output, ncells);
        break; //out of this loop
        }

      else if ( ! strncmp(line, "point_data", 10) )
        {
        if (!this->Reader->Read(&npts))
          {
          vtkErrorMacro(<<"Cannot read point data!");
          this->Reader->CloseVTKFile ();
          return;
          }
        
        if ( npts != numPts )
          {
          vtkErrorMacro(<<"Number of points don't match number data values!");
          return;
          }

        this->Reader->ReadPointData(output, npts);
        break; //out of this loop
        }

      else
        {
        vtkErrorMacro(<< "Unrecognized keyword: " << line);
        this->Reader->CloseVTKFile ();
        return;
        }
      }

      if ( ! output->GetPoints() ) vtkWarningMacro(<<"No points read!");
      if ( !(output->GetVerts() || output->GetLines() || 
      output->GetPolys() || output->GetStrips()) ) 
        vtkWarningMacro(<<"No topology read!");
    }

  else if ( !strncmp(line, "cell_data", 9) )
    {
    vtkWarningMacro(<<"No geometry defined in data file!");
    if (!this->Reader->Read(&ncells))
      {
      vtkErrorMacro(<<"Cannot read cell data!");
      this->Reader->CloseVTKFile ();
      return;
      }

    this->Reader->ReadCellData(output, ncells);
    }

  else if ( !strncmp(line, "point_data", 10) )
    {
    vtkWarningMacro(<<"No geometry defined in data file!");
    if (!this->Reader->Read(&numPts))
      {
      vtkErrorMacro(<<"Cannot read point data!");
      this->Reader->CloseVTKFile ();
      return;
      }

    this->Reader->ReadPointData(output, numPts);
    }

  else 
    {
    vtkErrorMacro(<< "Unrecognized keyword: " << line);
    }
  this->Reader->CloseVTKFile ();
}

static int recursing = 0;
void vtkPolyDataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  // the reader ivar's source will be this reader. we must do this to prevent infinite printing
  if (!recursing)
    { 
    vtkPolyDataSource::PrintSelf(os,indent);
    recursing = 1;
    os << indent << "Reader:\n";
    this->Reader->PrintSelf(os,indent.GetNextIndent());
    }
  recursing = 0;
}
