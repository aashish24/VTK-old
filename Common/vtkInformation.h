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
// .NAME vtkInformation - Store vtkAlgorithm input/output information.
// .SECTION Description
// vtkInformation represents information and/or data for one input or
// one output of a vtkAlgorithm.  It maps from keys to values of
// several data types.  Instances of this class are collected in
// vtkInformationVector instances and passed to
// vtkAlgorithm::ProcessUpstreamRequest and
// vtkAlgorithm::ProcessDownstreamRequest calls.  The information and
// data referenced by the instance on a particular input or output
// define the request made to the vtkAlgorithm instance.

#ifndef __vtkInformation_h
#define __vtkInformation_h

#include "vtkObject.h"

class vtkDataObject;
class vtkInformationDataObjectKey;
class vtkInformationDataObjectVectorKey;
class vtkInformationInformationKey;
class vtkInformationInformationVectorKey;
class vtkInformationIntegerKey;
class vtkInformationIntegerVectorKey;
class vtkInformationInternals;
class vtkInformationKey;
class vtkInformationKeyVectorKey;
class vtkInformationStringKey;
class vtkInformationVector;

class VTK_COMMON_EXPORT vtkInformation : public vtkObject
{
public:
  static vtkInformation *New();
  vtkTypeRevisionMacro(vtkInformation,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Clear all information entries.
  void Clear();

  // Description:
  // Copy all information entries from the given vtkInformation
  // instance.  Any previously existing entries are removed.
  void Copy(vtkInformation* from);

  // Description:
  // Get/Set an integer-valued entry.
  void Set(vtkInformationIntegerKey* key, int value);
  int Get(vtkInformationIntegerKey* key);
  void Remove(vtkInformationIntegerKey* key);
  int Has(vtkInformationIntegerKey* key);

  // Description:
  // Get/Set an integer-vector-valued entry.
  void Set(vtkInformationIntegerVectorKey* key, int* value, int length);
  int* Get(vtkInformationIntegerVectorKey* key);
  void Get(vtkInformationIntegerVectorKey* key, int* value);
  int Length(vtkInformationIntegerVectorKey* key);
  void Remove(vtkInformationIntegerVectorKey* key);
  int Has(vtkInformationIntegerVectorKey* key);

  // Description:
  // Get/Set an InformationKey-vector-valued entry.
  void Set(vtkInformationKeyVectorKey* key, vtkInformationKey** value, int length);
  vtkInformationKey** Get(vtkInformationKeyVectorKey* key);
  void Get(vtkInformationKeyVectorKey* key, vtkInformationKey** value);
  int Length(vtkInformationKeyVectorKey* key);
  void Remove(vtkInformationKeyVectorKey* key);
  int Has(vtkInformationKeyVectorKey* key);

  // Description:
  // Get/Set a DataObject-vector-valued entry.
  void Set(vtkInformationDataObjectVectorKey* key, vtkDataObject** value, int length);
  vtkDataObject** Get(vtkInformationDataObjectVectorKey* key);
  void Get(vtkInformationDataObjectVectorKey* key, vtkDataObject** value);
  int Length(vtkInformationDataObjectVectorKey* key);
  void Remove(vtkInformationDataObjectVectorKey* key);
  int Has(vtkInformationDataObjectVectorKey* key);

  // Description:
  // Get/Set a string-valued entry.
  void Set(vtkInformationStringKey* key, const char*);
  const char* Get(vtkInformationStringKey* key);
  void Remove(vtkInformationStringKey* key);
  int Has(vtkInformationStringKey* key);

  // Description:
  // Get/Set an entry storing another vtkInformation instance.
  void Set(vtkInformationInformationKey* key, vtkInformation*);
  vtkInformation* Get(vtkInformationInformationKey* key);
  void Remove(vtkInformationInformationKey* key);
  int Has(vtkInformationInformationKey* key);

  // Description:
  // Get/Set an entry storing a vtkInformationVector instance.
  void Set(vtkInformationInformationVectorKey* key, vtkInformationVector*);
  vtkInformationVector* Get(vtkInformationInformationVectorKey* key);
  void Remove(vtkInformationInformationVectorKey* key);
  int Has(vtkInformationInformationVectorKey* key);

  // Description:
  // Get/Set an entry storing a vtkDataObject instance.
  void Set(vtkInformationDataObjectKey* key, vtkDataObject*);
  vtkDataObject* Get(vtkInformationDataObjectKey* key);
  void Remove(vtkInformationDataObjectKey* key);
  int Has(vtkInformationDataObjectKey* key);

  //BTX
  // Description:
  // Possible values for the FIELD_ASSOCIATION information entry.
  enum FieldAssociations
  {
    FIELD_ASSOCIATION_POINTS,
    FIELD_ASSOCIATION_CELLS,
    FIELD_ASSOCIATION_NONE
  };
  //ETX

  //BTX
  // Description:
  // Possible values for the FIELD_OPERATION information entry.
  enum FieldOperations
  {
    FIELD_OPERATION_PRESERVED,
    FIELD_OPERATION_REINTERPOLATED,
    FIELD_OPERATION_MODIFIED,
    FIELD_OPERATION_REMOVED
  };
  //ETX

  // Description:
  // Get a key instance for the information entry specified by the
  // method name.
  static vtkInformationIntegerKey* INPUT_IS_OPTIONAL();
  static vtkInformationIntegerKey* INPUT_IS_REPEATABLE();
  static vtkInformationInformationVectorKey* INPUT_CONNECTION_INFORMATION();
  static vtkInformationInformationVectorKey* INPUT_REQUIRED_FIELDS();
  static vtkInformationStringKey* INPUT_REQUIRED_DATA_TYPE();
  static vtkInformationStringKey* OUTPUT_DATA_TYPE();
  static vtkInformationInformationVectorKey* OUTPUT_PROVIDED_FIELDS();
  static vtkInformationDataObjectKey* DATA_OBJECT();
  static vtkInformationIntegerKey* FIELD_ARRAY_TYPE();
  static vtkInformationIntegerKey* FIELD_ASSOCIATION();
  static vtkInformationIntegerKey* FIELD_ATTRIBUTE_TYPE();
  static vtkInformationIntegerKey* FIELD_NUMBER_OF_COMPONENTS();
  static vtkInformationIntegerKey* FIELD_NUMBER_OF_TUPLES();
  static vtkInformationIntegerKey* FIELD_OPERATION();
  static vtkInformationStringKey* FIELD_NAME();
  static vtkInformationIntegerKey* DATA_TYPE();
  static vtkInformationIntegerKey* DATA_EXTENT_TYPE();
  static vtkInformationIntegerVectorKey* DATA_EXTENT();
  static vtkInformationIntegerKey* DATA_PIECE_NUMBER();
  static vtkInformationIntegerKey* DATA_NUMBER_OF_PIECES();

  // Description:
  // Upcast the given key instance.
  vtkInformationKey* GetKey(vtkInformationDataObjectKey* key);
  vtkInformationKey* GetKey(vtkInformationDataObjectVectorKey* key);
  vtkInformationKey* GetKey(vtkInformationInformationKey* key);
  vtkInformationKey* GetKey(vtkInformationInformationVectorKey* key);
  vtkInformationKey* GetKey(vtkInformationIntegerKey* key);
  vtkInformationKey* GetKey(vtkInformationIntegerVectorKey* key);
  vtkInformationKey* GetKey(vtkInformationStringKey* key);
  vtkInformationKey* GetKey(vtkInformationKey* key);
protected:
  vtkInformation();
  ~vtkInformation();

  // Get/Set a map entry directly through the vtkObjectBase instance
  // representing the value.  Used internally to manage the map.
  void SetAsObjectBase(vtkInformationKey* key, vtkObjectBase* value);
  vtkObjectBase* GetAsObjectBase(vtkInformationKey* key);

  // Internal implementation details.
  vtkInformationInternals* Internal;

private:
  vtkInformation(const vtkInformation&);  // Not implemented.
  void operator=(const vtkInformation&);  // Not implemented.
};

#endif
