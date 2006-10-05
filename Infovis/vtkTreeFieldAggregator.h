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
// .NAME vtkTreeFieldAggregator - aggregate field values from the leaves up the tree
//
// .SECTION Description

#ifndef __vtkTreeFieldAggregator_h
#define __vtkTreeFieldAggregator_h

class vtkPoints;
class vtkTree;

#include "vtkTreeAlgorithm.h"

class VTK_INFOVIS_EXPORT vtkTreeFieldAggregator : public vtkTreeAlgorithm 
{
public:
  static vtkTreeFieldAggregator *New();

  vtkTypeRevisionMacro(vtkTreeFieldAggregator,vtkTreeAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The field to aggregate.  If this is a string array, the entries are converted to double.
  // TODO: Remove this field and use the ArrayToProcess in vtkAlgorithm.
  vtkGetStringMacro(Field);
  vtkSetStringMacro(Field);

  // Description:
  // If the value of the node is less than MinValue then consider it's value to be minVal.
  vtkGetMacro(MinValue, double);
  vtkSetMacro(MinValue, double);

  // Description:
  // If set, the algorithm will assume a size of 1 for each leaf node.
  vtkSetMacro(LeafNodeUnitSize, bool);
  vtkGetMacro(LeafNodeUnitSize, bool);
  vtkBooleanMacro(LeafNodeUnitSize, bool);
 
  // Description:
  // If set, the leaf values in the tree will be logarithmically scaled (base 10).
  vtkSetMacro(LogScale, bool);
  vtkGetMacro(LogScale, bool);
  vtkBooleanMacro(LogScale, bool);
protected:
  vtkTreeFieldAggregator();
  ~vtkTreeFieldAggregator();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
private:
  char* Field;
  bool LeafNodeUnitSize;
  bool LogScale;
  double MinValue;
  vtkTreeFieldAggregator(const vtkTreeFieldAggregator&);  // Not implemented.
  void operator=(const vtkTreeFieldAggregator&);  // Not implemented.
  double GetDoubleValue(vtkAbstractArray* arr, vtkIdType id);
  static void SetDoubleValue(vtkAbstractArray* arr, vtkIdType id, double value);
};

#endif
