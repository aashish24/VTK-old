/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  
-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkAnnotation - Stores a collection of annotation artifacts.
//
// .SECTION Description
// vtkAnnotation is a collection of annotation properties along with
// an associated selection indicating the portion of data the annotation
// refers to.
//
// .SECTION Thanks
// Timothy M. Shead (tshead@sandia.gov) at Sandia National Laboratories
// contributed code to this class.

#ifndef __vtkAnnotation_h
#define __vtkAnnotation_h

#include "vtkDataObject.h"

class vtkInformationStringKey;
class vtkInformationDoubleVectorKey;
class vtkSelection;

class VTK_FILTERING_EXPORT vtkAnnotation : public vtkDataObject
{
public:
  vtkTypeRevisionMacro(vtkAnnotation, vtkDataObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkAnnotation* New();

  // Description:
  // The selection to which this set of annotations will apply.
  vtkGetObjectMacro(Selection, vtkSelection);
  virtual void SetSelection(vtkSelection* selection);

  // Description:
  // Retrieve a vtkAnnotation stored inside an information object.
  static vtkAnnotation* GetData(vtkInformation* info);
  static vtkAnnotation* GetData(vtkInformationVector* v, int i=0);

  // Description:
  // The label for this annotation.
  static vtkInformationStringKey* LABEL();

  // Description:
  // The color for this annotation.
  // This is stored as an RGB triple with values between 0 and 1.
  static vtkInformationDoubleVectorKey* COLOR();

  // Description:
  // The color for this annotation.
  // This is stored as a value between 0 and 1.
  static vtkInformationDoubleKey* OPACITY();

  // Description:
  // Initialize the annotation to an empty state.
  virtual void Initialize();

  // Description:
  // Make this annotation have the same properties and have
  // the same selection of another annotation.
  virtual void ShallowCopy(vtkDataObject* other);

  // Description:
  // Make this annotation have the same properties and have
  // a copy of the selection of another annotation.
  virtual void DeepCopy(vtkDataObject* other);

//BTX
protected:
  vtkAnnotation();
  ~vtkAnnotation();

  vtkSelection* Selection;

private:
  vtkAnnotation(const vtkAnnotation&);  // Not implemented.
  void operator=(const vtkAnnotation&);  // Not implemented.
//ETX
};

#endif

