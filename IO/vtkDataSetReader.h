/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkDataSetReader - class to read any type of vtk dataset
// .SECTION Description
// vtkDataSetReader is a class that provides instance variables 
// and methods to read any type of dataset in Visualization Toolkit (vtk) format. 
// The output type of this class will vary depending upon the type of data
// file. Convenience methods are provided to keep the data as a particular
// type.

#ifndef __vtkDataSetReader_h
#define __vtkDataSetReader_h

#include "vtkSource.h"
#include "vtkDataReader.h"

class vtkPolyData;
class vtkStructuredPoints;
class vtkStructuredGrid;
class vtkUnstructuredGrid;
class vtkRectilinearGrid;

class VTK_EXPORT vtkDataSetReader : public vtkSource
{
public:
  static vtkDataSetReader *New();
  vtkTypeMacro(vtkDataSetReader,vtkSource);
  void PrintSelf(ostream& os, vtkIndent indent);
  unsigned long int GetMTime();

  // Description:
  // Set / get the file name of vtk data file to read.
  void SetFileName(char *name);
  char *GetFileName();

  // Description:
  // Specify the InputString for use when reading from a character array.
  void SetInputString(char *in) {this->Reader->SetInputString(in);}
  void SetInputString(char *in,int len) {this->Reader->SetInputString(in,len);}
  char *GetInputString() { return this->Reader->GetInputString();}

  // Description:
  // Set/Get reading from an InputString instead of the default, a file.
  void SetReadFromInputString(int i){this->Reader->SetReadFromInputString(i);}
  int GetReadFromInputString() {return this->Reader->GetReadFromInputString();}
  vtkBooleanMacro(ReadFromInputString,int);

  // Description:
  // Get the type of file (VTK_ASCII or VTK_BINARY).
  int GetFileType();

  // Description:
  // Set / get the name of the scalar data to extract. If not specified,
  // first scalar data encountered is extracted.
  void SetScalarsName(char *name);
  char *GetScalarsName();

  // Description:
  // Set / get the name of the vector data to extract. If not specified,
  // first vector data encountered is extracted.
  void SetVectorsName(char *name);
  char *GetVectorsName();

  // Description:
  // Set / get the name of the tensor data to extract. If not specified,
  // first tensor data encountered is extracted.
  void SetTensorsName(char *name);
  char *GetTensorsName();

  // Description:
  // Set / get the name of the normal data to extract. If not specified,
  // first normal data encountered is extracted.
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

  // Description:
  // Get the output of this source as a general vtkDataSet. Since we need 
  // to know the type of the data, the FileName must be set before GetOutput 
  // is applied.
  vtkDataSet *GetOutput();
  vtkDataSet *GetOutput(int idx)
    {return (vtkDataSet *) this->vtkSource::GetOutput(idx); };

  // Description:
  // Get the output as various concrete types. This method is typically used
  // when you know exactly what type of data is being read.  Otherwise, use
  // the general GetOutput() method. Warning: the method is dangerous because
  // of the cast; make sure you know the type you request is actually the
  // type in the file. (You must also set the filename of the object prior
  // to getting the output.)
  vtkPolyData *GetPolyDataOutput() {
    return (vtkPolyData *)this->GetOutput();};
  vtkStructuredPoints *GetStructuredPointsOutput() {
    return (vtkStructuredPoints *)this->GetOutput();};
  vtkStructuredGrid *GetStructuredGridOutput() {
    return (vtkStructuredGrid *)this->GetOutput();};
  vtkUnstructuredGrid *GetUnstructuredGridOutput() {
    return (vtkUnstructuredGrid *)this->GetOutput();};
  vtkRectilinearGrid *GetRectilinearGridOutput() {
    return (vtkRectilinearGrid *)this->GetOutput();};

  // Description:
  // If there is no output, execute anyway.  Execute creates an output.
  void Update();
  
protected:
  vtkDataSetReader();
  ~vtkDataSetReader();
  vtkDataSetReader(const vtkDataSetReader&) {};
  void operator=(const vtkDataSetReader&) {};

  void Execute();
  vtkDataReader *Reader;

};

#endif


