/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkScalars - abstract interface to array of scalar data
// .SECTION Description
// vtkScalars provides an abstract interface to an array of scalar data. 
// The data model for vtkScalars is an array accessible by point id.
// The subclasses of vtkScalars are concrete data types (float, int, etc.) 
// that implement the interface of vtkScalars.
//
// Scalars typically provide a single value per point. However, there are
// types of scalars that have multiple values per point (e.g., vtkPixmap or
// vtkAPixmap that provide three and four values per point, respectively).
// These are used when reading data in rgb and rgba form (e.g., images 
// and volumes).
//
// Because of the close relationship between scalars and colors, scalars 
// also maintain in internal lookup table. If provided, this table is used 
// to map scalars into colors, rather than the lookup table that the vtkMapper
// objects are associated with.

#ifndef __vtkScalars_h
#define __vtkScalars_h

#include "vtkRefCount.hh"

class vtkIdList;
class vtkFloatScalars;
class vtkLookupTable;

class vtkScalars : public vtkRefCount 
{
public:
  vtkScalars();
  virtual ~vtkScalars();
  char *GetClassName() {return "vtkScalars";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a copy of this object.
  virtual vtkScalars *MakeObject(int sze, int ext=1000) = 0;

  // Description:
  // Return data type. One of "bit", "unsigned char", "short", "int", "float", or
  // "double".
  virtual char *GetDataType() = 0;

  // Description:
  // Return the type of scalar. Want to differentiate between
  // single-valued scalars and multiple-valued (e.g., "color" scalars).
  // Returns either "SingleValued" or "ColorScalar".
  virtual char *GetScalarType() {return "SingleValued";};

  // Description:
  // Return the number of values per scalar. Should range between (1,4).
  virtual int GetNumberOfValuesPerScalar() {return 1;};

  // Description:
  // Return number of scalars in this object.
  virtual int GetNumberOfScalars() = 0;

  // Description:
  // Return a float scalar value for a particular point id.
  virtual float GetScalar(int id) = 0;

  // Description:
  // Insert scalar into array. No range checking performed (fast!).
  virtual void SetScalar(int id, float s) = 0;

  // Description:
  // Insert scalar into array. Range checking performed and memory
  // allocated as necessary.
  virtual void InsertScalar(int id, float s) = 0;

  // Description:
  // Insert scalar into next available slot. Returns point id of slot.
  virtual int InsertNextScalar(float s) = 0;

  // Description:
  // Reclaim any extra memory.
  virtual void Squeeze() = 0;

  // Description:
  // Get the scalar values for the point ids specified.
  virtual void GetScalars(vtkIdList& ptIds, vtkFloatScalars& fs);

  virtual void ComputeRange();
  float *GetRange();
  void GetRange(float range[2]);

  // Description:
  // Create default lookup table. Generally used to create one when none
  // is available.
  virtual void CreateDefaultLookupTable();

  void SetLookupTable(vtkLookupTable *lut);
  vtkGetObjectMacro(LookupTable,vtkLookupTable);

protected:
  float Range[8];
  vtkTimeStamp ComputeTime; // Time at which range computed
  vtkLookupTable *LookupTable;
};

// These include files are placed here so that if Scalars.hh is included 
// all other classes necessary for compilation are also included. 
#include "vtkIdList.hh"
#include "vtkFloatScalars.hh"

#endif
