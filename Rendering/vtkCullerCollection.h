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
// .NAME vtkCullerCollection - a list of Cullers
// .SECTION Description
// vtkCullerCollection represents and provides methods to 
// manipulate a list of Cullers (i.e., vtkCuller and 
// subclasses). The list is unsorted and duplicate entries are not prevented.

// .SECTION see also
// vtkCuller vtkCollection 

#ifndef __vtkCullerC_h
#define __vtkCullerC_h

#include "vtkCollection.h"
class vtkCuller;

class VTK_EXPORT vtkCullerCollection : public vtkCollection
{
 public:
  static vtkCullerCollection *New() {return new vtkCullerCollection;};
  const char *GetClassName() {return "vtkCullerCollection";};

  void AddItem(vtkCuller *a);
  void RemoveItem(vtkCuller *a);
  int IsItemPresent(vtkCuller *a);
  vtkCuller *GetNextItem();
  vtkCuller *GetLastItem();
};

// Description:
// Add an Culler to the list.
inline void vtkCullerCollection::AddItem(vtkCuller *a) 
{
  this->vtkCollection::AddItem((vtkObject *)a);
}

// Description:
// Remove an Culler from the list.
inline void vtkCullerCollection::RemoveItem(vtkCuller *a) 
{
  this->vtkCollection::RemoveItem((vtkObject *)a);
}

// Description:
// Determine whether a particular Culler is present. Returns its position
// in the list.
inline int vtkCullerCollection::IsItemPresent(vtkCuller *a) 
{
  return this->vtkCollection::IsItemPresent((vtkObject *)a);
}

// Description:
// Get the next Culler in the list.
inline vtkCuller *vtkCullerCollection::GetNextItem() 
{ 
  return (vtkCuller *)(this->GetNextItemAsObject());
}

// Description:
// Get the last Culler in the list.
inline vtkCuller *vtkCullerCollection::GetLastItem() 
{ 
  if ( this->Bottom == NULL )
    {
    return NULL;
    }
  else
    {
    return (vtkCuller *)(this->Bottom->Item);
    }
}

#endif





