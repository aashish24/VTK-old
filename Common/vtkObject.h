/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkObject - abstract base class for most of the vtk objects
// .SECTION Description
// vtkObject is the base class for many objects in the visualization 
// toolkit. vtkObject provides methods for tracking modification times, 
// debugging, and printing. Most objects created within the vtk 
// framework should be a subclass of vtkObject or one of its children.
// The few exceptions tend to be very small helper classes that usually
// never get instantiated or situations where multiple inheritance
// gets in the way. 

#ifndef __vtkObject_h
#define __vtkObject_h

#include <iostream.h>
#ifdef _WIN32
#include "vtkWin32Header.h"
#endif
#include "vtkTimeStamp.h"
#include "vtkSetGet.h"
#include "vtkIndent.h"

class vtkObject 
{
public:
  vtkObject(); //create a vtk object
  virtual void Delete(); //delete a vtk object.
  virtual ~vtkObject(); //use Delete() whenever possible
  virtual char *GetClassName() {return "vtkObject";};

  // debugging
  virtual void DebugOn();
  virtual void DebugOff();
  int GetDebug();
  void SetDebug(int debugFlag);

  // modified time
  virtual unsigned long int GetMTime();
  virtual void Modified();

  // printing
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  void Print(ostream& os);
  virtual void PrintHeader(ostream& os, vtkIndent indent);
  virtual void PrintTrailer(ostream& os, vtkIndent indent);

protected:
  int Debug;         // Enable debug messages
  vtkTimeStamp MTime; // Keep track of modification time

private:
  //BTX
#ifdef _WIN32
#ifdef VTKDLL
  friend __declspec(dllexport) 
    ostream& operator<<(ostream& os, vtkObject& o);  
#else
  friend __declspec(dllimport) 
     ostream& operator<<(ostream& os, vtkObject& o);
#endif
#else  
  friend ostream& operator<<(ostream& os, vtkObject& o);
#endif 
  //ETX
};

// Description:
// Update the modification time for this object. Many filters rely on
// the modification time to determine if they need to recompute their
// data.
inline void vtkObject::Modified()
{
  this->MTime.Modified();
}

#endif

