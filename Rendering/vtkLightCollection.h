/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkLightCollection - a list of lights
// .SECTION Description
// vtkLightCollection represents and provides methods to manipulate list of
// lights (i.e., vtkLight and subclasses). The list is unsorted and duplicate
// entries are not prevented.

#ifndef __vtkLightC_hh
#define __vtkLightC_hh

#include "Collect.hh"
#include "Light.hh"

class vtkLightCollection : public vtkCollection
{
 public:
  char *GetClassName() {return "vtkLightCollection";};

  void AddItem(vtkLight *a);
  void RemoveItem(vtkLight *a);
  int IsItemPresent(vtkLight *a);
  vtkLight *GetNextItem();
};

// Description:
// Add an light to the list.
inline void vtkLightCollection::AddItem(vtkLight *a) 
{
  this->vtkCollection::AddItem((vtkObject *)a);
}

// Description:
// Remove an light from the list.
inline void vtkLightCollection::RemoveItem(vtkLight *a) 
{
  this->vtkCollection::RemoveItem((vtkObject *)a);
}

// Description:
// Determine whether a particular light is present. Returns its position
// in the list.
inline int vtkLightCollection::IsItemPresent(vtkLight *a) 
{
  return this->vtkCollection::IsItemPresent((vtkObject *)a);
}

// Description:
// Get the next light in the list.
inline vtkLight *vtkLightCollection::GetNextItem() 
{ 
  return (vtkLight *)(this->vtkCollection::GetNextItem());
}

#endif

