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
// .NAME vtkDebugLeaks - cell represents a 1D line
// .SECTION Description
// vtkDebugLeaks is used to report memory leaks at the exit of the program.
// It uses the vtkObjectFactory to intercept the construction of all vtk
// objects.   It uses the UnRegister method of vtkObject to intercept
// the destruction of all objects.   A table of object name to 
// number of instances is kept.   At the exit of the program if there
// are still vtk objects around it will print them out.  To enable this
// class add the flag -DVTK_DEBUG_LEAKS to the compile line, and rebuild
// vtkObject and vtkObjectFactory.

#ifndef _vtkDebugLeaks_h
#define _vtkDebugLeaks_h

#include "vtkObject.h"

class vtkDebugLeaksHashTable;

class vtkDebugLeaks : public vtkObject
{
public: 
  static vtkDebugLeaks *New();
  vtkTypeMacro(vtkDebugLeaks,vtkObject);
  // Description:
  // Call this when creating a class of a given name
  static void ConstructClass(const char* classname);
  // Call this when deleting a class of a given name
  static void DestructClass(const char* classname);
  // Print all the values in the table.
  static void PrintCurrentLeaks();
  // Clean up the table memory
  static void DeleteTable();
private:
  vtkDebugLeaks(){}; 
  virtual ~vtkDebugLeaks(){}; 
  vtkDebugLeaks(const vtkDebugLeaks&) {};
  void operator=(const vtkDebugLeaks&) {};
private:
  static vtkDebugLeaksHashTable* MemoryTable;
};

#endif // _DebugMem_h
