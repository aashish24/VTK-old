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
// .NAME vtkBYUReader - read MOVIE.BYU polygon files
// .SECTION Description
// vtkBYUReader is a source object that reads MOVIE.BYU polygon files.
// These files consist of a geometry file (.g), a scalar file (.s), a 
// displacement or vector file (.d), and a 2D texture coordinate file
// (.t).

#ifndef __vtkBYUReader_h
#define __vtkBYUReader_h

#include <stdio.h>
#include "vtkPolyDataSource.h"

class VTK_EXPORT vtkBYUReader : public vtkPolyDataSource 
{
public:
  vtkBYUReader();
  ~vtkBYUReader();
  static vtkBYUReader *New() {return new vtkBYUReader;};
  const char *GetClassName() {return "vtkBYUReader";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify name of geometry FileName.
  vtkSetStringMacro(GeometryFileName);
  vtkGetStringMacro(GeometryFileName);

  // Description:
  // Specify name of displacement FileName.
  vtkSetStringMacro(DisplacementFileName);
  vtkGetStringMacro(DisplacementFileName);

  // Description:
  // Specify name of scalar FileName.
  vtkSetStringMacro(ScalarFileName);
  vtkGetStringMacro(ScalarFileName);

  // Description:
  // Specify name of texture coordinates FileName.
  vtkSetStringMacro(TextureFileName);
  vtkGetStringMacro(TextureFileName);

  // Description:
  // Turn on/off the reading of the displacement file.
  vtkSetMacro(ReadDisplacement,int)
  vtkGetMacro(ReadDisplacement,int)
  vtkBooleanMacro(ReadDisplacement,int)
  
  // Description:
  // Turn on/off the reading of the scalar file.
  vtkSetMacro(ReadScalar,int)
  vtkGetMacro(ReadScalar,int)
  vtkBooleanMacro(ReadScalar,int)
  
  // Description:
  // Turn on/off the reading of the texture coordinate file.
  // Specify name of geometry FileName.
  vtkSetMacro(ReadTexture,int)
  vtkGetMacro(ReadTexture,int)
  vtkBooleanMacro(ReadTexture,int)

  vtkSetClampMacro(PartNumber,int,1,VTK_LARGE_INTEGER);
  vtkGetMacro(PartNumber,int);

protected:
  void Execute();

  char *GeometryFileName;
  char *DisplacementFileName;
  char *ScalarFileName;
  char *TextureFileName;
  int ReadDisplacement;
  int ReadScalar;
  int ReadTexture;
  int PartNumber;

  void ReadGeometryFile(FILE *fp, int &numPts);
  void ReadDisplacementFile(int numPts);
  void ReadScalarFile(int numPts);
  void ReadTextureFile(int numPts);
};

#endif


