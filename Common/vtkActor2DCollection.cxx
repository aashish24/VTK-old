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

#include "vtkCollection.h"
#include "vtkActor2D.h"
#include "vtkActor2DCollection.h"

// protected function to delete an element. Internal use only.
void vtkActor2DCollection::DeleteElement(vtkCollectionElement *e)
{
  vtkCollection::DeleteElement(e);
}

// Desctructor for the vtkActor2DCollection class. This removes all 
// objects from the collection.
vtkActor2DCollection::~vtkActor2DCollection()
{
  this->RemoveAllItems();
}

// Sort and then render the collection of 2D actors.  
void vtkActor2DCollection::Render(vtkViewport* viewport)
{
  if (this->NumberOfItems != 0)
    {
    this->Sort();  
    vtkActor2D* tempActor;
    for ( this->InitTraversal(); 
           (tempActor = this->GetNextItem());)
      {
      // Make sure that the actor is visible before rendering
      if (tempActor->GetVisibility() == 1)
	{
	tempActor->Render(viewport);
	}
      }
    }
}

// Add an actor to the list.  The new actor is 
// inserted in the list according to it's layer
// number.
void vtkActor2DCollection::AddItem(vtkActor2D *a)
{
  vtkCollectionElement* indexElem;
  vtkCollectionElement* elem = new vtkCollectionElement;

  // Check if the top item is NULL
  if (this->Top == NULL)
    {
    vtkDebugMacro(<<"vtkActor2DCollection::AddItem - Adding item to top of the list");
  
    this->Top = elem;
    elem->Item = a;
    elem->Next = NULL;
    this->Bottom = elem;
    this->NumberOfItems++;
    a->Register(this);
    return;
    }

  for (indexElem = this->Top;
         indexElem != NULL;
           indexElem = indexElem->Next)
    {

    vtkActor2D* tempActor = (vtkActor2D*) indexElem->Item;
    if (a->GetLayerNumber() < tempActor->GetLayerNumber())
      {
      // The indexElem item's layer number is larger, so swap
      // the new item and the indexElem item.
      vtkDebugMacro(<<"vtkActor2DCollection::AddItem - Inserting item");
      elem->Item = indexElem->Item;
      elem->Next = indexElem->Next;
      indexElem->Item = a;
      indexElem->Next = elem;
      this->NumberOfItems++;
      a->Register(this);
      return;
      }

    }

  //End of list found before a larger layer number
  vtkDebugMacro(<<"vtkActor2DCollection::AddItem - Adding item to end of the list");
  elem->Item = a;
  elem->Next = NULL;
  this->Bottom->Next = elem;
  this->Bottom = elem;
  this->NumberOfItems++;
  a->Register(this);

}

// Sorts the vtkActor2DCollection by layer number.  Smaller layer
// numbers are first.  Layer numbers can be any integer value.
void vtkActor2DCollection::Sort()
{
   int index;
   
   vtkDebugMacro(<<"vtkActor2DCollection::Sort");

   int numElems  = this->GetNumberOfItems();

   // Create an array of pointers to actors
   vtkActor2D** actorPtrArr = new vtkActor2D* [numElems];

   vtkDebugMacro(<<"vtkActor2DCollection::Sort - Getting actors from collection");

   // Start at the beginning of the collection
   this->InitTraversal();

   // Fill the actor array with the items in the collection
   for (index = 0; index < numElems; index++)
     {
     actorPtrArr[index] = this->GetNextItem();
     }

  vtkDebugMacro(<<"vtkActor2DCollection::Sort - Starting selection sort");
   // Start the sorting - selection sort
  int i, j, min;
  vtkActor2D* t;

  for (i = 0; i < numElems - 1; i++)
    {
    min = i;
    for (j = i + 1; j < numElems ; j++)
      {
      if(actorPtrArr[j]->GetLayerNumber() < actorPtrArr[min]->GetLayerNumber()) 
        {
        min = j;
        }
      }
    t = actorPtrArr[min];
    actorPtrArr[min] = actorPtrArr[i];
    actorPtrArr[i] = t;
    }

   vtkDebugMacro(<<"vtkActor2DCollection::Sort - Selection sort done.");

   for (index = 0; index < numElems; index++)
     {
     vtkDebugMacro(<<"vtkActor2DCollection::Sort - actorPtrArr["<<index<<"] layer: " <<
     actorPtrArr[index]->GetLayerNumber());
     }

  vtkDebugMacro(<<"vtkActor2DCollection::Sort - Rearraging the linked list.");
  // Now move the items around in the linked list -
  // keep the links the same, but swap around the items
 
  vtkCollectionElement* elem = this->Top;
  elem->Item = actorPtrArr[0];

  for (i = 1; i < numElems; i++)
    {
    elem = elem->Next;
    elem->Item = actorPtrArr[i];
    }

  delete[] actorPtrArr;
}

















