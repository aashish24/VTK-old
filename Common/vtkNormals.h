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
// .NAME vtkNormals - represent and manipulate 3D normals
// .SECTION Description
// vtkNormals represents 3D normals. The data model for vtkNormals is an 
// array of nx-ny-nz triplets accessible by (point or cell) id. Each normal
// is assumed to have magnitude |n| = 1.

#ifndef __vtkNormals_h
#define __vtkNormals_h

#include "vtkAttributeData.h"

class vtkIdList;
class vtkNormals;

class VTK_EXPORT vtkNormals : public vtkAttributeData
{
public:
  static vtkNormals *New(int dataType);
  static vtkNormals *New();

  vtkTypeMacro(vtkNormals,vtkAttributeData);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a copy of this object.
  vtkAttributeData *MakeObject()
    {return vtkNormals::New(this->GetDataType());}
  
  // Description:
  // Return number of normals in array.
  int GetNumberOfNormals() 
    {return this->Data->GetNumberOfTuples();}

  // Description:
  // Return a pointer to a float normal n[3] for a specific id.
  float *GetNormal(int id) 
    {return this->Data->GetTuple(id);}
  
  // Description:
  // Copy normal components into user provided array n[3] for specified
  // id.
  void GetNormal(int id, float n[3]) 
    {this->Data->GetTuple(id,n);}
  void GetNormal(int id, double n[3]) 
    {this->Data->GetTuple(id,n);}
  
  // Description:
  // Insert normal into object. No range checking performed (fast!).
  // Make sure you use SetNumberOfNormals() to allocate memory prior
  // to using SetNormal().
  void SetNormal(int id, float n[3]) 
    {this->Data->SetTuple(id,n);}
  void SetNormal(int id, double n[3]) 
    {this->Data->SetTuple(id,n);}
  void SetNormal(int id, double nx, double ny, double nz);

  // Description:
  // Insert normal into object. Range checking performed and memory
  // allocated as necessary.
  void InsertNormal(int id, double n[3]) 
    {this->Data->InsertTuple(id,n);}
  void InsertNormal(int id, float n[3]) 
    {this->Data->InsertTuple(id,n);}
  void InsertNormal(int id, double nx, double ny, double nz);

  // Description:
  // Insert normal into next available slot. Returns id of slot.
  int InsertNextNormal(float n[3]) 
    {return this->Data->InsertNextTuple(n);}
  int InsertNextNormal(double n[3]) 
    {return this->Data->InsertNextTuple(n);}
  int InsertNextNormal(double nx, double ny, double nz);

  // Description:
  // Specify the number of normals for this object to hold. Does an
  // allocation as well as setting the MaxId ivar. Used in conjunction with
  // SetNormal() method for fast insertion.
  void SetNumberOfNormals(int number);

  // Description:
  // Given a list of pt ids, return an array of normals.
  void GetNormals(vtkIdList *ptId, vtkNormals *fn);

#ifndef VTK_REMOVE_LEGACY_CODE
  // Description:
  // For legacy compatibility. Do not use.
  void GetNormals(vtkIdList& ptId, vtkNormals& fn) 
    {VTK_LEGACY_METHOD(GetNormals,"3.2"); this->GetNormals(&ptId, &fn);}
#endif
  
protected:
  vtkNormals();
  ~vtkNormals() {};
  vtkNormals(const vtkNormals&) {};
  void operator=(const vtkNormals&) {};
  
};


inline void vtkNormals::SetNumberOfNormals(int number)
{
  this->Data->SetNumberOfComponents(3);
  this->Data->SetNumberOfTuples(number);
}

inline void vtkNormals::SetNormal(int id, double nx, double ny, double nz)
{
  double n[3];
  n[0] = nx;
  n[1] = ny;
  n[2] = nz;
  this->Data->SetTuple(id,n);
}

inline void vtkNormals::InsertNormal(int id, double nx, double ny, double nz)
{
  double n[3];

  n[0] = nx;
  n[1] = ny;
  n[2] = nz;
  this->Data->InsertTuple(id,n);
}

inline int vtkNormals::InsertNextNormal(double nx, double ny, double nz)
{
  double n[3];

  n[0] = nx;
  n[1] = ny;
  n[2] = nz;
  return this->Data->InsertNextTuple(n);
}


// These include files are placed here so that if Normals.h is included 
// all other classes necessary for compilation are also included. 
#include "vtkIdList.h"

#endif
