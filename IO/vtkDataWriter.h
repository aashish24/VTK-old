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
// .NAME vtkDataWriter - helper class for objects that write vtk data files
// .SECTION Description
// vtkDataWriter is a helper class that opens and writes the vtk header and 
// point data (e.g., scalars, vectors, normals, etc.) from a vtk data file. 
// See text for various formats.

// .SECTION See Also
// vtkDataSetWriter vtkPolyDataWriter vtkStructuredGridWriter
// vtkStructuredPointsWriter vtkUnstructuredGridWriter
// vtkFieldDataWriter vtkRectilinearGridWriter

#ifndef __vtkDataWriter_h
#define __vtkDataWriter_h

#include <stdio.h>
#include "vtkWriter.h"

class vtkDataSet;
class vtkPoints;
class vtkCellArray;
class vtkScalars;
class vtkVectors;
class vtkNormals;
class vtkTCoords;
class vtkTensors;
class vtkDataArray;

class VTK_EXPORT vtkDataWriter : public vtkWriter
{
public:
  const char *GetClassName() {return "vtkDataWriter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Created object with default header, ASCII format, and default names for 
  // scalars, vectors, tensors, normals, and texture coordinates.
  static vtkDataWriter *New() {return new vtkDataWriter;};

  // Description:
  // Specify file name of vtk polygon data file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Specify the OutputString for use when writing to a character array.
  // If no string is specified, ostrstream allocates one of its own.
  // If a string is given, it is the users responsibility to delete the string.
  void SetOutputString(char *str, int length);
  vtkGetStringMacro(OutputString);
  vtkGetMacro(OutputStringLength, int);

  // Description:
  // Enable writing to an OutputString instead of the default, a file.
  vtkSetMacro(WriteToOutputString,int);
  vtkGetMacro(WriteToOutputString,int);
  vtkBooleanMacro(WriteToOutputString,int);

  // Description:
  // Specify the header for the vtk data file.
  vtkSetStringMacro(Header);
  vtkGetStringMacro(Header);

  // Description:
  // Specify file type (ASCII or BINARY) for vtk data file.
  vtkSetClampMacro(FileType,int,VTK_ASCII,VTK_BINARY);
  vtkGetMacro(FileType,int);
  void SetFileTypeToASCII() {this->SetFileType(VTK_ASCII);};
  void SetFileTypeToBinary() {this->SetFileType(VTK_BINARY);};

  // Description:
  // Give a name to the scalar data. If not specified, uses default
  // name "scalars".
  vtkSetStringMacro(ScalarsName);
  vtkGetStringMacro(ScalarsName);

  // Description:
  // Give a name to the vector data. If not specified, uses default
  // name "vectors".
  vtkSetStringMacro(VectorsName);
  vtkGetStringMacro(VectorsName);

  // Description:
  // Give a name to the tensors data. If not specified, uses default
  // name "tensors".
  vtkSetStringMacro(TensorsName);
  vtkGetStringMacro(TensorsName);

  // Description:
  // Give a name to the normals data. If not specified, uses default
  // name "normals".
  vtkSetStringMacro(NormalsName);
  vtkGetStringMacro(NormalsName);

  // Description:
  // Give a name to the texture coordinates data. If not specified, uses 
  // default name "textureCoords".
  vtkSetStringMacro(TCoordsName);
  vtkGetStringMacro(TCoordsName);

  // Description:
  // Give a name to the lookup table. If not specified, uses default
  // name "lookupTable".
  vtkSetStringMacro(LookupTableName);
  vtkGetStringMacro(LookupTableName);

  // Description:
  // Give a name to the field data. If not specified, uses default 
  // name "field".
  vtkSetStringMacro(FieldDataName);
  vtkGetStringMacro(FieldDataName);

  // Description:
  // Open a vtk data file. Returns NULL if error.
  virtual ostream *OpenVTKFile();

  // Description:
  // Write the header of a vtk data file. Returns 0 if error.
  int WriteHeader(ostream *fp);

  // Description:
  // Write out the points of the data set.
  int WritePoints(ostream *fp, vtkPoints *p);

  // Description:
  // Write out coordinates for rectilinear grids.
  int WriteCoordinates(ostream *fp, vtkScalars *coords, int axes);

  // Description:
  // Write out the cells of the data set.
  int WriteCells(ostream *fp, vtkCellArray *cells, char *label);

  // Description:
  // Write the cell data (e.g., scalars, vectors, ...) of a vtk dataset.
  // Returns 0 if error.
  int WriteCellData(ostream *fp, vtkDataSet *ds);

  // Description:
  // Write the point data (e.g., scalars, vectors, ...) of a vtk dataset.
  // Returns 0 if error.
  int WritePointData(ostream *fp, vtkDataSet *ds);

  // Description:
  // Write out the field data.
  int WriteFieldData(ostream *fp, vtkFieldData *f);
  
  // Description:
  // Close a vtk file.
  void CloseVTKFile(ostream *fp);


protected:
  vtkDataWriter();
  ~vtkDataWriter();
  vtkDataWriter(const vtkDataWriter&) {};
  void operator=(const vtkDataWriter&) {};

  int WriteToOutputString;
  char *OutputString;
  int OutputStringLength;
  int UserOwnsOutputString;

  void WriteData(); //dummy method to allow this class to be instantiated and delegated to

  char *FileName;
  char *Header;
  int FileType;

  char *ScalarsName;
  char *VectorsName;
  char *TensorsName;
  char *TCoordsName;
  char *NormalsName;
  char *LookupTableName;
  char *FieldDataName;

  int WriteArray(ostream *fp, int dataType, vtkDataArray *data, char *format, 
		 int num, int numComp);
  int WriteScalarData(ostream *fp, vtkScalars *s, int num);
  int WriteVectorData(ostream *fp, vtkVectors *v, int num);
  int WriteNormalData(ostream *fp, vtkNormals *n, int num);
  int WriteTCoordData(ostream *fp, vtkTCoords *tc, int num);
  int WriteTensorData(ostream *fp, vtkTensors *t, int num);

};

#endif


