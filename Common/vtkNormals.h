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
  static vtkNormals *New(int dataType=VTK_FLOAT) {return new vtkNormals(dataType);};

  const char *GetClassName() {return "vtkNormals";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a copy of this object.
  vtkAttributeData *MakeObject(){return new vtkNormals(this->GetDataType());};
  
  // Description:
  // Return number of normals in array.
  int GetNumberOfNormals() {return this->Data->GetNumberOfTuples();};

  // Description:
  // Return a pointer to a float normal n[3] for a specific id.
  float *GetNormal(int id) {return this->Data->GetTuple(id);};
  
  // Description:
  // Copy normal components into user provided array n[3] for specified
  // id.
  void GetNormal(int id, float n[3]){this->Data->GetTuple(id,n);};
  
  // Description:
  // Insert normal into object. No range checking performed (fast!).
  // Make sure you use SetNumberOfNormals() to allocate memory prior
  // to using SetNormal().
  void SetNormal(int id, float n[3]){this->Data->SetTuple(id,n);};
  void SetNormal(int id, float x, float y, float z);

  // Description:
  // Insert normal into object. Range checking performed and memory
  // allocated as necessary.
  void InsertNormal(int id, float n[3]){this->Data->InsertTuple(id,n);};

  // Description:
  // Insert normal into next available slot. Returns id of slot.
  int InsertNextNormal(float n[3]){return this->Data->InsertNextTuple(n);};

  // Description:
  // Specify the number of normals for this object to hold. Does an
  // allocation as well as setting the MaxId ivar. Used in conjunction with
  // SetNormal() method for fast insertion.
  void SetNumberOfNormals(int number);

  // Description:
  // Insert normal into position indicated.
  void InsertNormal(int id, float nx, float ny, float nz);

  // Description:
  // Insert normal at end of array and return its location (id) in the array.
  int InsertNextNormal(float nx, float ny, float nz);

  // Description:
  // Given a list of pt ids, return an array of normals.
  void GetNormals(vtkIdList *ptId, vtkNormals *fn);

  // Description:
  // For legacy compatibility. 
  void GetNormals(vtkIdList& ptId, vtkNormals& fn) {this->GetNormals(&ptId, &fn);}
  
protected:
  vtkNormals(int dataType=VTK_FLOAT);
  ~vtkNormals() {};
  
};


inline void vtkNormals::SetNumberOfNormals(int number)
{
  this->Data->SetNumberOfComponents(3);
  this->Data->SetNumberOfTuples(number);
}

inline void vtkNormals::SetNormal(int id, float nx, float ny, float nz)
{
  float n[3];
  n[0] = nx;
  n[1] = ny;
  n[2] = nz;
  this->Data->SetTuple(id,n);
}

inline void vtkNormals::InsertNormal(int id, float nx, float ny, float nz)
{
  float n[3];

  n[0] = nx;
  n[1] = ny;
  n[2] = nz;
  this->Data->InsertTuple(id,n);
}

inline int vtkNormals::InsertNextNormal(float nx, float ny, float nz)
{
  float n[3];

  n[0] = nx;
  n[1] = ny;
  n[2] = nz;
  return this->Data->InsertNextTuple(n);
}


// These include files are placed here so that if Normals.h is included 
// all other classes necessary for compilation are also included. 
#include "vtkIdList.h"

#endif
