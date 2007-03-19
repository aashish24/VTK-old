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
// .NAME vtkSelectionSource - Generate selection from given set of ids
// vtkSelectionSource generates a vtkSelection from a set of 
// (piece id, cell id) pairs. It will only generate the selection values
// that match UPDATE_PIECE_NUMBER (i.e. piece == UPDATE_PIECE_NUMBER).

#ifndef __vtkSelectionSource_h
#define __vtkSelectionSource_h

#include "vtkSelectionAlgorithm.h"

//BTX
struct vtkSelectionSourceInternals;
//ETX

class VTK_GRAPHICS_EXPORT vtkSelectionSource : public vtkSelectionAlgorithm
{
public:
  static vtkSelectionSource *New();
  vtkTypeRevisionMacro(vtkSelectionSource,vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Add a (piece, id) to the selection set. The source will generate
  // only the ids for which piece == UPDATE_PIECE_NUMBER.
  void AddID(vtkIdType piece, vtkIdType id);

  // Description:
  // Removes all IDs.
  void RemoveAllIDs();

protected:
  vtkSelectionSource();
  ~vtkSelectionSource();

  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  vtkSelectionSourceInternals* Internal;

private:
  vtkSelectionSource(const vtkSelectionSource&);  // Not implemented.
  void operator=(const vtkSelectionSource&);  // Not implemented.
};

#endif
