/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to William A. Hoffman who developed this class


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
// .NAME vtkObjectFactoryCollection - maintain a list of object factories
// .SECTION Description
// vtkObjectFactoryCollection is an object that creates amn manipulates lists
// of object of type vtkObjectFactory.

// .SECTION see also
// vtkCollection vtkObjectFactory

#ifndef __vtkObjectFactoryCollection_h
#define __vtkObjectFactoryCollection_h

#include "vtkCollection.h"
#include "vtkObjectFactory.h"

class VTK_EXPORT vtkObjectFactoryCollection : public vtkCollection
{
public:
  vtkTypeMacro(vtkObjectFactoryCollection,vtkCollection);
  static vtkObjectFactoryCollection *New() {return new vtkObjectFactoryCollection;};

  // Description:
  // Add an ObjectFactory from the list.
  void AddItem(vtkObjectFactory *t) { this->vtkCollection::AddItem((vtkObject *)t); }
  
  // Description:
  // Get the next ObjectFactory in the list. Return NULL when the end of the
  // list is reached.
  vtkObjectFactory *GetNextItem() { return (vtkObjectFactory *)(this->GetNextItemAsObject());}

protected:
  vtkObjectFactoryCollection() {};
  ~vtkObjectFactoryCollection() {};
  vtkObjectFactoryCollection(const vtkObjectFactoryCollection&) {};
  void operator=(const vtkObjectFactoryCollection&) {};


private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->vtkCollection::AddItem(o); };

};


#endif
