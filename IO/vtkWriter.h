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
// .NAME vtkWriter - abstract class to write data to file(s)
// .SECTION Description
// vtkWriter is an abstract class for mapper objects that write their data
// to disk (or into a communications port). All writers respond to Write()
// method. This method insures that there is input and input is up to date.
//
// Since vtkWriter is a subclass of vtkProcessObject, StartMethod(), 
// EndMethod(), and ProgressMethod() are all available to writers.
// These methods are executed before and after execution of the Write() 
// method. You can also specify arguments to these methods.

// .SECTION Caveats
// Every subclass of vtkWriter must implement a WriteData() method. Most likely
// will have to create SetInput() method as well.

// .SECTION See Also
// vtkBYUWriter vtkDataWriter vtkSTLWriter vtkVoxelWriter vtkMCubesWriter

#ifndef __vtkWriter_h
#define __vtkWriter_h

#include "vtkProcessObject.h"
#include "vtkDataObject.h"

#define VTK_ASCII 1
#define VTK_BINARY 2

class VTK_EXPORT vtkWriter : public vtkProcessObject
{
public:
  vtkWriter();
  ~vtkWriter();
  const char *GetClassName() {return "vtkWriter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write data to output. Method executes subclasses WriteData() method, as 
  // well as StartMethod() and EndMethod() methods.
  virtual void Write();

  // Description:
  // Convenient alias for Write() method.
  void Update();
  
//BTX
  vtkDataObject *GetInput();
//ETX
protected:
  virtual void WriteData() = 0; //internal method subclasses must respond to
};

#endif


