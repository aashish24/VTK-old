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
#include "vtkUnstructuredGridReader.h"
#include "vtkByteSwap.h"

#ifdef read
#undef read
#endif

vtkUnstructuredGridReader::vtkUnstructuredGridReader()
{
  this->Reader = vtkDataReader::New();
  this->Reader->SetSource(this);
  this->Recursing = 0;
}

vtkUnstructuredGridReader::~vtkUnstructuredGridReader()
{
  this->Reader->Delete();
}

unsigned long int vtkUnstructuredGridReader::GetMTime()
{
  unsigned long dtime = this->vtkSource::GetMTime();
  unsigned long rtime = this->Reader->GetMTime();
  return (dtime > rtime ? dtime : rtime);
}

// Specify file name of vtk polygonal data file to read.
void vtkUnstructuredGridReader::SetFileName(char *name) 
{
  this->Reader->SetFileName(name);
}
char *vtkUnstructuredGridReader::GetFileName() 
{
  return this->Reader->GetFileName();
}

// Get the type of file (ASCII or BINARY)
int vtkUnstructuredGridReader::GetFileType() 
{
  return this->Reader->GetFileType();
}

// Set the name of the scalar data to extract. If not specified, first 
// scalar data encountered is extracted.
void vtkUnstructuredGridReader::SetScalarsName(char *name) 
{
  this->Reader->SetScalarsName(name);
}
char *vtkUnstructuredGridReader::GetScalarsName() 
{
  return this->Reader->GetScalarsName();
}

// Set the name of the vector data to extract. If not specified, first 
// vector data encountered is extracted.
void vtkUnstructuredGridReader::SetVectorsName(char *name) 
{
  this->Reader->SetVectorsName(name);
}
char *vtkUnstructuredGridReader::GetVectorsName() 
{
  return this->Reader->GetVectorsName();
}

// Set the name of the tensor data to extract. If not specified, first 
// tensor data encountered is extracted.
void vtkUnstructuredGridReader::SetTensorsName(char *name) 
{
  this->Reader->SetTensorsName(name);
}
char *vtkUnstructuredGridReader::GetTensorsName() 
{
  return this->Reader->GetTensorsName();
}

// Set the name of the normal data to extract. If not specified, first 
// normal data encountered is extracted.
void vtkUnstructuredGridReader::SetNormalsName(char *name) 
{
  this->Reader->SetNormalsName(name);
}
char *vtkUnstructuredGridReader::GetNormalsName() 
{
  return this->Reader->GetNormalsName();
}

// Set the name of the texture coordinate data to extract. If not specified,
// first texture coordinate data encountered is extracted.
void vtkUnstructuredGridReader::SetTCoordsName(char *name) 
{
  this->Reader->SetTCoordsName(name);
}
char *vtkUnstructuredGridReader::GetTCoordsName() 
{
  return this->Reader->GetTCoordsName();
}

// Set the name of the lookup table data to extract. If not specified, uses 
// lookup table named by scalar. Otherwise, this specification supersedes.
void vtkUnstructuredGridReader::SetLookupTableName(char *name) 
{
  this->Reader->SetLookupTableName(name);
}
char *vtkUnstructuredGridReader::GetLookupTableName() 
{
  return this->Reader->GetLookupTableName();
}

// Set the name of the field data to extract. If not specified, uses 
// first field data encountered in file.
void vtkUnstructuredGridReader::SetFieldDataName(char *name) 
{
  this->Reader->SetFieldDataName(name);
}
char *vtkUnstructuredGridReader::GetFieldDataName() 
{
  return this->Reader->GetFieldDataName();
}

void vtkUnstructuredGridReader::Execute()
{
  int i, numPts=0, numCells=0;
  char line[256];
  int npts, size, ncells;
  int piece, numPieces, skip1, read2, skip3, tmp;
  vtkCellArray *cells=NULL;
  int *types=NULL;
  vtkUnstructuredGrid *output = this->GetOutput();

  // to test streaming
  //output->SetUpdateExtent(1, 2);

  vtkDebugMacro(<<"Reading vtk unstructured grid...");
  if ( this->Debug )
    {
    this->Reader->DebugOn();
    }
  else
    {
    this->Reader->DebugOff();
    }

  if (!this->Reader->OpenVTKFile() || !this->Reader->ReadHeader())
    {
    return;
    }

  // Read unstructured grid specific stuff
  //
  if (!this->Reader->ReadString(line))
    {
    vtkErrorMacro(<<"Data file ends prematurely!");
    this->Reader->CloseVTKFile ();
    return;
    }

  if ( !strncmp(this->Reader->LowerCase(line),"dataset",(unsigned long)7) )
    {
    // Make sure we're reading right type of geometry
    //
    if (!this->Reader->ReadString(line))
      {
      vtkErrorMacro(<<"Data file ends prematurely!");
      this->Reader->CloseVTKFile ();
      return;
      } 

    if ( strncmp(this->Reader->LowerCase(line),"unstructured_grid",17) )
      {
      vtkErrorMacro(<< "Cannot read dataset type: " << line);
      this->Reader->CloseVTKFile ();
      return;
      }

    // Might find points, cells, and cell types
    //
    while (1)
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

        if (!this->Reader->ReadPoints(output, numPts))
          {
          this->Reader->CloseVTKFile ();
          return;
          }
        }

      else if ( !strncmp(line,"cells",5))
        {
	output->GetUpdateExtent(piece, numPieces);
        if (!(this->Reader->Read(&ncells) && this->Reader->Read(&size)))
          {
          vtkErrorMacro(<<"Cannot read cells!");
          this->Reader->CloseVTKFile ();
          return;
          }

	// the number of ints to read befor we get to the piece.
	skip1 = piece * ncells / numPieces;
	// the number of ints to read as part of piece.
	read2 = ((piece+1) * ncells / numPieces) - skip1;
	// the number of ints after the piece
	skip3 = ncells - skip1 - read2;

        cells = vtkCellArray::New();
        if (!this->Reader->ReadCells(size, cells->WritePointer(read2,size),
				     skip1, read2, skip3) )
          {
          this->Reader->CloseVTKFile ();
          return;
          }
        if (cells && types)
	  {
	  output->SetCells(types, cells);
	  }
        }

      else if (!strncmp(line,"cell_types",10))
        {
	output->GetUpdateExtent(piece, numPieces);
        if (!this->Reader->Read(&ncells))
          {
          vtkErrorMacro(<<"Cannot read cell types!");
          this->Reader->CloseVTKFile ();
          return;
          }
	// the number of ints to read befor we get to the piece.
	skip1 = piece * ncells / numPieces;
	// the number of ints to read as part of piece.
	read2 = ((piece+1) * ncells / numPieces) - skip1;
	// the number of ints after the piece
	skip3 = ncells - skip1 - read2;

	//cerr << skip1 << " --- " << read2 << " --- " << skip3 << endl;
	// allocate array for piece cell types
        types = new int[read2];
        if (this->Reader->GetFileType() == VTK_BINARY)
          {
          // suck up newline
          this->Reader->GetIStream()->getline(line,256);
	  // skip
	  if (skip1 != 0)
	    {
	    this->Reader->GetIStream()->seekg(sizeof(int)*skip1, ios::cur);
	    }
          this->Reader->GetIStream()->read((char *)types,sizeof(int)*read2);
	  // skip
	  if (skip3 != 0)
	    {
	    this->Reader->GetIStream()->seekg(sizeof(int)*skip3, ios::cur);
	    }

          if (this->Reader->GetIStream()->eof())
            {
            vtkErrorMacro(<<"Error reading binary cell types!");
            this->Reader->CloseVTKFile ();
            return;
            }
          vtkByteSwap::Swap4BERange(types,read2);
          }
        else //ascii
          {
	  // skip types before piece
          for (i=0; i<skip1; i++)
            {
            if (!this->Reader->Read(&tmp))
              {
              vtkErrorMacro(<<"Error reading cell types!");
              this->Reader->CloseVTKFile ();
              return;
              }
            }
	  // read types for piece
          for (i=0; i<read2; i++)
            {
            if (!this->Reader->Read(types+i))
              {
              vtkErrorMacro(<<"Error reading cell types!");
              this->Reader->CloseVTKFile ();
              return;
              }
            }
	  // skip types after piece
          for (i=0; i<skip3; i++)
            {
            if (!this->Reader->Read(&tmp))
              {
              vtkErrorMacro(<<"Error reading cell types!");
              this->Reader->CloseVTKFile ();
              return;
              }
            }
          }
        if ( cells && types )
	  {
	  output->SetCells(types, cells);
	  }
        }

      else if ( ! strncmp(line, "cell_data", 9) )
        {
        if (!this->Reader->Read(&numCells))
          {
          vtkErrorMacro(<<"Cannot read cell data!");
          this->Reader->CloseVTKFile ();
          return;
          }
        
        if ( ncells != numCells )
          {
          vtkErrorMacro(<<"Number of cells don't match!");
          this->Reader->CloseVTKFile ();
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
          vtkErrorMacro(<<"Number of points don't match!");
          this->Reader->CloseVTKFile ();
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
      if ( ! (cells && types) )  vtkWarningMacro(<<"No topology read!");
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

  // Clean-up and get out
  //
  if (types)
    {
    delete [] types;
    }
  if (cells)
    {
    cells->Delete();
    }

  vtkDebugMacro(<<"Read " <<output->GetNumberOfPoints() <<" points," 
                <<output->GetNumberOfCells() <<" cells.\n");

  this->Reader->CloseVTKFile ();
  return;
}

void vtkUnstructuredGridReader::PrintSelf(ostream& os, vtkIndent indent)
{
  // the reader ivar's source will be this Reader-> we must do this to prevent 
  // infinite printing
  if (!this->Recursing)
    { 
    vtkUnstructuredGridSource::PrintSelf(os,indent);
    this->Recursing = 1;
    os << indent << "Reader:\n";
    this->Reader->PrintSelf(os,indent.GetNextIndent());
    }
  this->Recursing = 0;
}
