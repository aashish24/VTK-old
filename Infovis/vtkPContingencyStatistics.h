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
// .NAME vtkPContingencyStatistics - A class for parallel univariate contingency statistics
// .SECTION Description
// vtkPContingencyStatistics is vtkContingencyStatistics subclass for parallel datasets.
// It learns and derives the global statistical model on each node, but assesses each 
// individual data points on the node that owns it.

// .NOTE: It is assumed that the keys in the contingency table be contained in the set {0,...,n-1}
// of successive integers, where n is the number of rows of the summary table.
// If this requirement is not fulfilled, then the outcome of the parallel update of contingency 
// tables is unpredictable but will most likely be a crash.
// Note that this requirement is consistent with the way contingency tables are constructed
// by the (serial) superclass and thus, if you are using this class as it is intended to be ran,
// then you do not have to worry about this requirement.

// .SECTION Thanks
// Thanks to Philippe Pebay from Sandia National Laboratories for implementing this class.

#ifndef __vtkPContingencyStatistics_h
#define __vtkPContingencyStatistics_h

#include "vtkContingencyStatistics.h"

#include <vtkstd/vector> // used by Reduce()

class vtkMultiProcessController;

class VTK_INFOVIS_EXPORT vtkPContingencyStatistics : public vtkContingencyStatistics
{
public:
  static vtkPContingencyStatistics* New();
  vtkTypeRevisionMacro(vtkPContingencyStatistics, vtkContingencyStatistics);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the multiprocess controller. If no controller is set,
  // single process is assumed.
  virtual void SetController(vtkMultiProcessController*);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);

  // Description:
  // Execute the parallel calculations required by the Learn option.
  virtual void ExecuteLearn( vtkTable* inData,
                             vtkDataObject* outMeta );

protected:
  vtkPContingencyStatistics();
  ~vtkPContingencyStatistics();

//BTX
  // Description:
  // Reduce the collection of local contingency tables to the global one
  bool Reduce( char* xyPacked_g,
               vtkIdType& xySizeTotal,
               vtkStdString& xyPacked_l,
               vtkIdType*  kcValues_g,
               vtkIdType& kcSizeTotal,
               vtkstd::vector<vtkIdType>& kcValues_l );
//ETX

  vtkMultiProcessController* Controller;
private:
  vtkPContingencyStatistics(const vtkPContingencyStatistics&); // Not implemented.
  void operator=(const vtkPContingencyStatistics&); // Not implemented.
};

#endif

