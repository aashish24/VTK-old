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
#include "vtkIdList.h"

vtkIdList::vtkIdList(const int sz, const int ext)
{
  this->Ia = vtkIntArray::New(); //reference count is 1
  this->Ia->Allocate(sz,ext);
}

vtkIdList::~vtkIdList()
{
  this->Ia->Delete();
}

// Description:
// Copy an id list by reference counting internal array.
void vtkIdList::ShallowCopy(vtkIdList& ids)
{
  if ( ids.Ia != NULL )
    {
    this->Ia->Delete();
    this->Ia = ids.Ia;
    this->Ia->Register(this);
    }
}

// Description:
// Copy an id list by explicitly copying the internal array.
void vtkIdList::DeepCopy(vtkIdList& ids)
{
  if ( ids.Ia != NULL )
    {
    this->Ia->DeepCopy(*ids.Ia);
    }
}

// Description:
// Delete specified id from list. Will replace all occurences of id in list.
void vtkIdList::DeleteId(int Id)
{
  int i=0, numIds=this->GetNumberOfIds();

  // while loop is necessary to delete all occurences of Id
  while ( i < numIds )
    {
    // first find id
    for ( ; i < numIds; i++)
      if ( this->GetId(i) == Id ) 
        break;

    // if found; replace current id with last
    if ( i < numIds )
      {
      this->SetId(i,this->Ia->GetValue(this->Ia->GetMaxId()));
      this->Ia->SetNumberOfValues(--numIds);
      }
    }
}

#define VTK_TMP_ARRAY_SIZE 500
// Description:
// Intersect this list with another vtkIdList. Updates current list according
// to result of intersection operation.
void vtkIdList::IntersectWith(vtkIdList& otherIds)
{
  // Fast method due to Dr. Andreas Mueller of ISE Integrated Systems Engineering (CH)
  int thisNumIds = this->GetNumberOfIds();

  if (thisNumIds <= VTK_TMP_ARRAY_SIZE) 
    {//Use fast method if we can fit in temporary storage
    int  thisIds[VTK_TMP_ARRAY_SIZE];
    int  i, id;
    
    for (i=0; i < thisNumIds; i++) thisIds[i] = this->GetId(i);
    for (this->Reset(), i=0; i < thisNumIds; i++) 
      {
      id = thisIds[i];
      if (otherIds.IsId(id)) this->InsertNextId(id);
      }
    } 
  else 
    {//use slower method for extreme cases
    int  *thisIds = new int [thisNumIds];
    int  i, id;
    
    for (i=0; i < thisNumIds; i++) *(thisIds + i) = this->GetId(i);
    for (this->Reset(), i=0; i < thisNumIds; i++) 
      {
      id = *(thisIds + i);
      if (otherIds.IsId(id)) this->InsertNextId(id);
      }
    delete [] thisIds;
    }
}
#undef VTK_TMP_ARRAY_SIZE


void vtkIdList::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);

  os << indent << "Number of Ids: " << this->Ia->GetMaxId()+1 << "\n";
}
