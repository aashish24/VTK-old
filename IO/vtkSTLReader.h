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
// .NAME vtkSTLReader - read ASCII or binary stereo lithography files
// .SECTION Description
// vtkSTLReader is a source object that reads ASCII or binary stereo 
// lithography files (.stl files). The FileName must be specified to
// vtkSTLReader. The object automatically detects whether the file is
// ASCII or binary.
//
// .stl files are quite inefficient since they duplicate vertex 
// definitions. By setting the Merging boolean you can control wether the 
// point data is merged after reading. Merging is performed by default, 
// however, merging requires a large amount of temporary storage since a 
// 3D hash table must be constructed.

// .SECTION Caveats
// Binary files written on one system may not be readable on other systems.
// vtkSTLWriter uses VAX or PC byte ordering and swaps bytes on other systems.

#ifndef __vtkSTLReader_h
#define __vtkSTLReader_h

#include <stdio.h>
#include "vtkPolyDataSource.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"

class VTK_EXPORT vtkSTLReader : public vtkPolyDataSource 
{
public:
  vtkTypeMacro(vtkSTLReader,vtkPolyDataSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with merging set to true.
  static vtkSTLReader *New();

  // Description:
  // Overload standard modified time function. If locator is modified,
  // then this object is modified as well.
  unsigned long GetMTime();

  // Description:
  // Specify file name of stereo lithography file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Turn on/off merging of points/triangles.
  vtkSetMacro(Merging,int);
  vtkGetMacro(Merging,int);
  vtkBooleanMacro(Merging,int);

  // Description:
  // Specify a spatial locator for merging points. By
  // default an instance of vtkMergePoints is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified.
  void CreateDefaultLocator();

  // Description:
  // For legacy compatibiltiy. Do not use.
  void SetLocator(vtkPointLocator& locator) {this->SetLocator(&locator);};

protected:
  vtkSTLReader();
  ~vtkSTLReader();
  vtkSTLReader(const vtkSTLReader&) {};
  void operator=(const vtkSTLReader&) {};

  char *FileName;
  int Merging;
  vtkPointLocator *Locator;

  void Execute();
  int ReadBinarySTL(FILE *fp, vtkPoints*, vtkCellArray*);
  int ReadASCIISTL(FILE *fp, vtkPoints*, vtkCellArray*);
  int GetSTLFileType(FILE *fp);
};

#endif


