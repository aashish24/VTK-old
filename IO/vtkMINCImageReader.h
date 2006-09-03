/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

Copyright (c) 2006 Atamai, Inc.

Use, modification and redistribution of the software, in source or
binary forms, are permitted provided that the following terms and
conditions are met:

1) Redistribution of the source code, in verbatim or modified
   form, must retain the above copyright notice, this license,
   the following disclaimer, and any notices that refer to this
   license and/or the following disclaimer.

2) Redistribution in binary form must include the above copyright
   notice, a copy of this license and the following disclaimer
   in the documentation or with other materials provided with the
   distribution.

3) Modified copies of the source code must be clearly marked as such,
   and must not be misrepresented as verbatim copies of the source code.

THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE SOFTWARE "AS IS"
WITHOUT EXPRESSED OR IMPLIED WARRANTY INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE.  IN NO EVENT SHALL ANY COPYRIGHT HOLDER OR OTHER PARTY WHO MAY
MODIFY AND/OR REDISTRIBUTE THE SOFTWARE UNDER THE TERMS OF THIS LICENSE
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA OR DATA BECOMING INACCURATE
OR LOSS OF PROFIT OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF
THE USE OR INABILITY TO USE THE SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.

=========================================================================*/
// .NAME vtkMINCImageReader - A reader for MINC files.
// .SECTION Description
// MINC is a medical image file format that was developed at the Montreal
// Neurological Institute in 1992. It is based on the NetCDF format.
// .SECTION Thanks
// Thanks to David Gobbi for writing this class and Atamai Inc. for
// contributing it to VTK.

#ifndef __vtkMINCImageReader_h
#define __vtkMINCImageReader_h

#include "vtkImageReader2.h"

class vtkStringArray;
class vtkIdTypeArray;
class vtkDoubleArray;
class vtkMatrix4x4;

// A special class that holds the attributes
class vtkMINCImageAttributes;

class VTK_IO_EXPORT vtkMINCImageReader : public vtkImageReader2
{
public:
  vtkTypeRevisionMacro(vtkMINCImageReader,vtkImageReader2);

  static vtkMINCImageReader *New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the file name.
  virtual void SetFileName(const char *name);

  // Description:
  // Get the entension for this file format. 
  virtual const char* GetFileExtensions() {
    return ".mnc"; }

  // Description:
  // Get the name of this file format.
  virtual const char* GetDescriptiveName() {
    return "MINC"; }

  // Description:
  // Test whether the specified file can be read.
  virtual int CanReadFile(const char* name);

  // Description:
  // Get a matrix that describes the orientation of the data.
  // The three columns of the matrix are the direction cosines
  // for the x, y and z dimensions respectively.
  virtual vtkMatrix4x4 *GetOrientationMatrix();

  // Description:
  // Get the slope and intercept for rescaling the scalar values
  // to real data values.
  virtual double GetRescaleSlope();
  virtual double GetRescaleIntercept();

  // Description:
  // Get the ValidRange of the data as stored in the file.
  // The ScalarRange of the output data will be equal to this.
  virtual double *GetValidRange();
  virtual void GetValidRange(double range[2]) {
    double *r = this->GetValidRange();
    range[0] = r[0]; range[1] = r[1]; };

  // Description:
  // Get the number of time steps in the file.
  virtual int GetNumberOfTimeSteps();

  // Description:
  // Set the time step to read.
  vtkSetMacro(TimeStep, int);
  vtkGetMacro(TimeStep, int);

  // Description:
  // Get the image attributes, which contain patient information and
  // other useful metadata.
  virtual vtkMINCImageAttributes *GetImageAttributes();

protected:
  vtkMINCImageReader();
  ~vtkMINCImageReader();

  int MINCImageType;
  int MINCImageTypeSigned;

  double ValidRange[2];
  double ImageRange[2];

  int NumberOfTimeSteps;
  int TimeStep;
  vtkMatrix4x4 *OrientationMatrix;
  double RescaleSlope;
  double RescaleIntercept;
  vtkMINCImageAttributes *ImageAttributes;

  int FileNameHasChanged;

  virtual int OpenNetCDFFile(const char *filename, int& ncid);
  virtual int CloseNetCDFFile(int ncid);
  virtual int IndexFromDimensionName(const char *dimName);
  virtual int ReadMINCFileAttributes();
  const char *ConvertDataArrayToString(vtkDataArray *array);
  static int ConvertMINCTypeToVTKType(int minctype, int mincsigned);

  virtual void ExecuteInformation();
  virtual void ExecuteData(vtkDataObject *out);

private:
  vtkMINCImageReader(const vtkMINCImageReader&); // Not implemented
  void operator=(const vtkMINCImageReader&);  // Not implemented

};

#endif
