/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.


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
// .NAME vtkImageWriter - Writes images to files.
// .SECTION Description
// vtkImageWriter writes images to files with any data type. The data type of
// the file is the same scalar type as the input.  The dimensionality
// determines whether the data will be written in one or multiple files.
// This class is used as the superclass of most image writing classes 
// such as vtkBMPWriter etc. It supports streaming.

#ifndef __vtkImageWriter_h
#define __vtkImageWriter_h

#include <iostream.h>
#include <fstream.h>
#include "vtkProcessObject.h"
#include "vtkImageData.h"

class VTK_EXPORT vtkImageWriter : public vtkProcessObject
{
public:
  static vtkImageWriter *New() {return new vtkImageWriter;};
  const char *GetClassName() {return "vtkImageWriter";};
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Specify file name for the image file. You should specify either
  // a FileName or a FilePrefix. Use FilePrefix if the data is stored 
  // in multiple files.
  void SetFileName(char *);
  vtkGetStringMacro(FileName);

  // Description:
  // Specify file prefix for the image file(s).You should specify either
  // a FileName or FilePrefix. Use FilePrefix if the data is stored
  // in multiple files.
  void SetFilePrefix(char *filePrefix);
  vtkGetStringMacro(FilePrefix);

  // Description:
  // The sprintf format used to build filename from FilePrefix and number.
  void SetFilePattern(char *filePattern);
  vtkGetStringMacro(FilePattern);

  // Description:
  // What dimension are the files to be written. Usually this is 2, or 3.
  // If it is 2 and the input is a volume then the volume will be 
  // written as a series of 2d slices.
  vtkSetMacro(FileDimensionality, int);
  vtkGetMacro(FileDimensionality, int);
  
  // Description:
  // Set/Get the input object from the image pipeline.
  void SetInput(vtkImageData *input);
  vtkImageData *GetInput();

  // Description:
  // The main interface which triggers the writer to start.
  virtual void Write();

protected:
  vtkImageWriter();
  ~vtkImageWriter();

  int FileDimensionality;
  char *FilePrefix;
  char *FilePattern;
  char *FileName;
  int FileNumber;
  int FileLowerLeft;
  char *InternalFileName;

  
  void RecursiveWrite(int dim, vtkImageData *region, ofstream *file);
  void RecursiveWrite(int dim, vtkImageData *cache, 
		      vtkImageData *data, ofstream *file);
  virtual void WriteFile(ofstream *file, vtkImageData *data, int extent[6]);
  virtual void WriteFileHeader(ofstream *, vtkImageData *) {};
  virtual void WriteFileTrailer(ofstream *, vtkImageData *) {};
};

#endif


