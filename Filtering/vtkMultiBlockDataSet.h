/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMultiBlockDataSet - collection of data objects
// .SECTION Description
// vtkMultiBlockDataSet represents a collection of data objects.
// The data objects can be primitive datasets as well as other
// composite datasets.
// No relation (spatial or hierarchical) between data objects is
// specified or enforced.

#ifndef __vtkMultiBlockDataSet_h
#define __vtkMultiBlockDataSet_h

#include "vtkCompositeDataSet.h"

class vtkDataSet;
class vtkMultiBlockDataIterator;
class vtkMultiBlockDataSetInternal;

class VTK_FILTERING_EXPORT vtkMultiBlockDataSet : public vtkCompositeDataSet
{
public:
  static vtkMultiBlockDataSet *New();

  vtkTypeRevisionMacro(vtkMultiBlockDataSet,vtkCompositeDataSet);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return a new iterator (has to be deleted by user)
  virtual vtkCompositeDataIterator* NewIterator();

  // Description:
  // Return class name of data type (see vtkSystemIncludes.h for
  // definitions.
  virtual int GetDataObjectType() {return VTK_MULTI_BLOCK_DATA_SET;}

  // Description:
  // Add a dataset to the collection. Returns the index of
  // the added dataset.
  unsigned int AddDataSet(vtkDataObject* data);

  // Description:
  // Calls AddDataSet(dobj)
  virtual void AddDataSet(vtkInformation*, vtkDataObject* dobj)
    {
      this->AddDataSet(dobj);
    }

  // Description:
  // Restore data object to initial state,
  virtual void Initialize();

  // Description:
  // Returns the number of datasets
  unsigned int GetNumberOfDataSets();

  // Description:
  // Returns dataset with given id
  vtkDataObject* GetDataSet(unsigned int idx);

  // Description:
  // Passes the INDEX() key to GetDataSet(idx)
  virtual vtkDataObject* GetDataSet(vtkInformation* index);
  
//BTX
  // Note that vtkMultiBlockDataIterator is dependent on the implementation
  // of the data structure in this class. Changes to the data structure
  // might require changes to vtkMultiBlockDataIterator.
  friend class vtkMultiBlockDataIterator;
//ETX

protected:
  vtkMultiBlockDataSet();
  ~vtkMultiBlockDataSet();

  vtkMultiBlockDataSetInternal* Internal;

private:
  vtkMultiBlockDataSet(const vtkMultiBlockDataSet&);  // Not implemented.
  void operator=(const vtkMultiBlockDataSet&);  // Not implemented.
};

#endif

