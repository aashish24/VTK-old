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
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
  -------------------------------------------------------------------------*/
// .NAME vtkDescriptiveStatistics - Base class for univariate statistics 
// algorithms
//
// .SECTION Description
// This class specializes statistics algorithms to the univariate case, where
// a number of columns of interest can be selected in the input data set.
// This is done by the means of the following functions:
//
// ResetColumns() - reset the list of columns of interest.
// Add/RemoveColum( i ) - try to add/remove column with index to the list.
// Add/RemoveColumnRange( i1, i2 ) - try to add/remove all columns with indices
// at least equal to i1 and stricly smaller to i2 to the list.
// The verb "try" is used in the sense that attempting to neither attempting to 
// repeat an existing entry nor to remove a non-existent entry will work.
// 
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this class.

#ifndef __vtkUnivariateStatisticsAlgorithm_h
#define __vtkUnivariateStatisticsAlgorithm_h

#include "vtkStatisticsAlgorithm.h"

class vtkUnivariateStatisticsAlgorithmPrivate;
class vtkTable;

class VTK_INFOVIS_EXPORT vtkUnivariateStatisticsAlgorithm : public vtkStatisticsAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkUnivariateStatisticsAlgorithm, vtkStatisticsAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reset list of columns of interest
  void ResetColumns();

  // Description:
  // Add column index \p idxCol to the list of columns of interest
  // Warning: no range checking is performed on \p idxCol; it is the user's
  // responsibility to use valid column indices.
  void AddColumn( vtkIdType idxCol );

  // Description:
  // Remove (if it exists) column index \p idxCol to the list of columns of interest
  void RemoveColumn( vtkIdType idxCol );

  // Description:
  // Add column indices from \p idxColBegin (included) to \p idxColEnd (excluded).
  // Warning: no range checking is performed on \p idxColBegin nor \p idxColEnd; it is 
  // the user's responsibility to use valid column indices.
  void AddColumnRange( vtkIdType idxColBegin, vtkIdType idxColEnd );

  // Description:
  // Remove column indices from \p idxColBegin (included) to \p idxColEnd (excluded),
  // for those which are present.
  void RemoveColumnRange( vtkIdType idxColBegin, vtkIdType idxColEnd );

protected:
  vtkUnivariateStatisticsAlgorithm();
  ~vtkUnivariateStatisticsAlgorithm();

  vtkUnivariateStatisticsAlgorithmPrivate* Internals;

private:
  vtkUnivariateStatisticsAlgorithm(const vtkUnivariateStatisticsAlgorithm&); // Not implemented
  void operator=(const vtkUnivariateStatisticsAlgorithm&);   // Not implemented
};

#endif

