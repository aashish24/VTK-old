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
// .NAME vtkPolyDataReader - read vtk polygonal data file
// .SECTION Description
// vtkPolyDataReader is a source object that reads ASCII or binary 
// polygonal data files in vtk format. See text for format details.

// .SECTION Caveats
// Binary files written on one system may not be readable on other systems.

#ifndef __vtkPolyDataReader_h
#define __vtkPolyDataReader_h

#include "vtkPolyDataSource.h"
#include "vtkDataReader.h"

class VTK_EXPORT vtkPolyDataReader : public vtkPolyDataSource
{
public:
  static vtkPolyDataReader *New() {return new vtkPolyDataReader;};
  const char *GetClassName() {return "vtkPolyDataReader";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return the MTime also considering the vtkDataReader ivar
  unsigned long GetMTime();

  // Description:
  // Set / get the file name of vtk polygonal data file to read.
  void SetFileName(char *name);
  char *GetFileName();

  // Description:
  // Get the header from the vtk data file.
  char *GetHeader() {return this->Reader->GetHeader();};

  // Description:
  // Specify the InputString for use when reading from a character array.
  void SetInputString(char *in) {this->Reader->SetInputString(in);}
  void SetInputString(char *in,int len) {this->Reader->SetInputString(in,len);}
  char *GetInputString() { return this->Reader->GetInputString();}

  // Description:
  // Set/Get reading from an InputString instead of the default, a file.
  void SetReadFromInputString(int i) {this->Reader->SetReadFromInputString(i);}
  int GetReadFromInputString() {return this->Reader->GetReadFromInputString();}
  vtkBooleanMacro(ReadFromInputString,int);

  // Description:
  // Get the type of file (ASCII or BINARY)
  int GetFileType();

  // Description:
  // Set / get the name of the scalar data to extract. If not specified, first 
  // scalar data encountered is extracted.
  void SetScalarsName(char *name);
  char *GetScalarsName();

  // Description:
  // Set / get the name of the vector data to extract. If not specified, first 
  // vector data encountered is extracted.
  void SetVectorsName(char *name);
  char *GetVectorsName();

  // Description:
  // Set / get the name of the tensor data to extract. If not specified, first 
  // tensor data encountered is extracted.
  void SetTensorsName(char *name);
  char *GetTensorsName();

  // Description:
  // Set / get the name of the normal data to extract. If not specified, first 
  // normal data encountered is extracted.
  void SetNormalsName(char *name);
  char *GetNormalsName();

  // Description:
  // Set / get the name of the texture coordinate data to extract. If not
  // specified, first texture coordinate data encountered is extracted.
  void SetTCoordsName(char *name);
  char *GetTCoordsName();

  // Description:
  // Set / get the name of the lookup table data to extract. If not
  // specified, uses lookup table named by scalar. Otherwise, this
  // specification supersedes.
  void SetLookupTableName(char *name);
  char *GetLookupTableName();

  // Description:
  // Set / get the name of the field data to extract. If not specified, uses 
  // first field data encountered in file.
  void SetFieldDataName(char *name);
  char *GetFieldDataName();

protected:
  vtkPolyDataReader();
  ~vtkPolyDataReader();

  void Execute();
  vtkDataReader *Reader;
};

#endif


