/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994

=========================================================================*/
#include <stdlib.h>
#include <math.h>

#include "Collect.hh"

// Description:
// Construct with empty list.
vlCollection::vlCollection()
{
  this->NumberOfItems = 0;
  this->Top = NULL;
  this->Bottom = NULL;
  this->Current = NULL;
}

vlCollection::~vlCollection()
{
  this->RemoveAllItems();
}

// Description:
// Add an object to the list. Does not prevent duplicate entries.
void vlCollection::AddItem(vlObject *a)
{
  vlCollectionElement *elem;

  elem = new vlCollectionElement;
  
  if (!this->Top)
    {
    this->Top = elem;
    }
  else
    {
    this->Bottom->Next = elem;
    }
  this->Bottom = elem;

  elem->Item = a;
  elem->Next = NULL;

  this->NumberOfItems++;
}

// Description:
// Remove an object from the list. Removes the first object found, not
// all occurences. If no object found, list is unaffected.
void vlCollection::RemoveItem(vlObject *a)
{
  int i;
  vlCollectionElement *elem,*prev;
  
  if (!this->Top) return;

  elem = this->Top;
  prev = NULL;
  for (i = 0; i < this->NumberOfItems; i++)
    {
    if (elem->Item == a)
      {
      if (prev)
	{
	prev->Next = elem->Next;
	}
      else
	{
	this->Top = elem->Next;
	}
      if (!elem->Next)
	{
	this->Bottom = prev;
	}
      
      delete elem;
      this->NumberOfItems--;
      return;
      }
    else
      {
      prev = elem;
      elem = elem->Next;
      }
    }
}

// Description:
// Remove all object from the list.
void vlCollection::RemoveAllItems()
{
  vlCollectionElement *p;  

  for ( p=this->Top; p != NULL; p = p->Next )
    {
    delete p;
    }

  this->NumberOfItems = 0;
  this->Top = this->Bottom = this->Current = NULL;
}

// Description:
// Search for an object and return location in list. If location == 0,
// object was not found.
int vlCollection::IsItemPresent(vlObject *a)
{
  int i;
  vlCollectionElement *elem;
  
  if (!this->Top) return 0;

  elem = this->Top;
  for (i = 0; i < this->NumberOfItems; i++)
    {
    if (elem->Item == a)
      {
      return i + 1;
      }
    else
      {
      elem = elem->Next;
      }
    }

  return 0;
}


// Description:
// Return the number of objects in the list.
int vlCollection::GetNumberOfItems()
{
  return this->NumberOfItems;
}


void vlCollection::PrintSelf(ostream& os, vlIndent indent)
{
  vlObject::PrintSelf(os,indent);

  os << indent << "Number Of Items: " << this->NumberOfItems << "\n";
}
