/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Matt Turek who developed this class.

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
// .NAME vtkActor2DCollection
// .SECTION Description
// vtkActor2DCollection is a subclass of vtkCollection.  vtkActor2DCollection
// maintains a collection of vtkActor2D objects that is sorted by layer
// number, with lower layer numbers at the start of the list.  This allows
// the vtkActor2D objects to be rendered in the correct order. 

// .SECTION See Also
// vtkActor2D vtkCollection

#ifndef __vtkActor2DCollection_h
#define __vtkActor2DCollection_h

#include "vtkPropCollection.h"
class vtkActor2D;
class vtkViewport;

class VTK_EXPORT vtkActor2DCollection : public vtkPropCollection
{
 public:
  // Description:
  // Desctructor for the vtkActor2DCollection class. This removes all 
  // objects from the collection.
  static vtkActor2DCollection *New();
  const char *GetClassName() {return "vtkActor2DCollection";};

  // Description:
  // Sorts the vtkActor2DCollection by layer number.  Smaller layer
  // numbers are first.  Layer numbers can be any integer value.
  void Sort();
  
  // Description:
  // Add an actor to the list.  The new actor is inserted in the list
  // according to it's layer number.
  void AddItem(vtkActor2D *a);

  // Description:
  // Standard Collection methods
  int IsItemPresent(vtkActor2D *a);
  vtkActor2D *GetNextActor2D();
  vtkActor2D *GetLastActor2D();

  // Description:
 // Access routines that are provided for compatibility with previous
 // version of VTK.  Please use the GetNextActor2D(), GetLastActor2D()
 // variants where possible.
 vtkActor2D *GetNextItem();
 vtkActor2D *GetLastItem();

    
  // Description:
  // Sort and then render the collection of 2D actors.  
  void RenderOverlay(vtkViewport* viewport);


protected:
  vtkActor2DCollection() {};
  ~vtkActor2DCollection();
  vtkActor2DCollection(const vtkActor2DCollection&) {};
  void operator=(const vtkActor2DCollection&) {};

  virtual void DeleteElement(vtkCollectionElement *); 

private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->vtkCollection::AddItem(o); };
  void AddItem(vtkProp *o) { this->vtkPropCollection::AddItem(o); };
  int IsItemPresent(vtkObject *o) { return this->vtkCollection::IsItemPresent(o); };

};

inline int vtkActor2DCollection::IsItemPresent(vtkActor2D *a) 
{
  return this->vtkCollection::IsItemPresent((vtkObject *)a);
}

inline vtkActor2D *vtkActor2DCollection::GetNextActor2D() 
{ 
    return (vtkActor2D *)(this->GetNextItemAsObject());
}

inline vtkActor2D *vtkActor2DCollection::GetLastActor2D() 
{ 
  if ( this->Bottom == NULL )
    {
    return NULL;
    }
  else
    {
    return (vtkActor2D *)(this->Bottom->Item);
    }
}

inline vtkActor2D *vtkActor2DCollection::GetNextItem() 
{ 
  return this->GetNextActor2D();
}

inline vtkActor2D *vtkActor2DCollection::GetLastItem() 
{
  return this->GetLastActor2D();
}

#endif





