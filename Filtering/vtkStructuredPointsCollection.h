/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME vlStructuredPointsCollection - maintain a list of structured points data objects
// .SECTION Description
// vlStructuredPointsCollection is an object that creates and manipulates lists of
// structured points datasets. See also vlCollection and subclasses.

#ifndef __vlStructuredPointsCollection_hh
#define __vlStructuredPointsCollection_hh

#include "StrPts.hh"

class vlStructuredPointsCollectionElement
{
 public:
  vlStructuredPoints *Item;
  vlStructuredPointsCollectionElement *Next;
};

class vlStructuredPointsCollection : public vlObject
{
public:
  vlStructuredPointsCollection();
  void PrintSelf(ostream& os, vlIndent indent);
  char *GetClassName() {return "vlStructuredPointsCollection";};

  void AddItem(vlStructuredPoints *);
  void RemoveItem(vlStructuredPoints *);
  int IsItemPresent(vlStructuredPoints *);
  int GetNumberOfItems();
  vlStructuredPoints *GetItem(int num);

private:
  int NumberOfItems;
  vlStructuredPointsCollectionElement *Top;
  vlStructuredPointsCollectionElement *Bottom;

};

#endif
