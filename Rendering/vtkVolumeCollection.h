/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVolumeCollection - a list of volumes
// .SECTION Description
// vtkVolumeCollection represents and provides methods to manipulate a 
// list of volumes (i.e., vtkVolume and subclasses). The list is unsorted 
// and duplicate entries are not prevented.

// .SECTION see also
// vtkCollection vtkVolume

#ifndef __vtkVolumeC_h
#define __vtkVolumeC_h

#include "vtkPropCollection.h"
#include "vtkVolume.h"

class VTK_RENDERING_EXPORT vtkVolumeCollection : public vtkPropCollection
{
 public:
  static vtkVolumeCollection *New();
  vtkTypeRevisionMacro(vtkVolumeCollection,vtkPropCollection);

  // Description:
  // Add a Volume to the list.
  void AddItem(vtkVolume *a) {
    this->vtkCollection::AddItem((vtkObject *)a);};
    
  // Description:
  // Get the next Volume in the list. Return NULL when at the end of the 
  // list.
  vtkVolume *GetNextVolume() {
      return static_cast<vtkVolume *>(this->GetNextItemAsObject());};


  // Description:
  // Access routine provided for compatibility with previous
  // versions of VTK.  Please use the GetNextVolume() variant
  // where possible.
  vtkVolume *GetNextItem() { return this->GetNextVolume(); };

protected:
  vtkVolumeCollection() {};
  ~vtkVolumeCollection() {};

private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->vtkCollection::AddItem(o); };
  void AddItem(vtkProp *o) { this->vtkPropCollection::AddItem(o); };

private:
  vtkVolumeCollection(const vtkVolumeCollection&);  // Not implemented.
  void operator=(const vtkVolumeCollection&);  // Not implemented.
};


#endif





