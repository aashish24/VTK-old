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
// vtkAlgorithm::ProcessRequest calls.  The information and
// data referenced by the instance on a particular input or output
// define the request made to the vtkAlgorithm instance.

#ifndef __vtkInformation_h
#define __vtkInformation_h

#include "vtkObject.h"

class vtkDataObject;
class vtkExecutive;
class vtkInformationDataObjectKey;
class vtkInformationDoubleKey;
class vtkInformationDoubleVectorKey;
class vtkInformationExecutiveKey;
class vtkInformationInformationKey;
class vtkInformationInformationVectorKey;
class vtkInformationIntegerKey;
class vtkInformationIntegerVectorKey;
class vtkInformationInternals;
class vtkInformationKey;
class vtkInformationKeyToInformationFriendship;
class vtkInformationKeyVectorKey;
class vtkInformationObjectBaseKey;
class vtkInformationStringKey;
class vtkInformationUnsignedLongKey;
class vtkInformationVector;

class VTK_FILTERING_EXPORT vtkInformation : public vtkObject
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
  // Copy the key/value pair associated with the given key in the
  // given information object.
  void CopyEntry(vtkInformation* from, vtkInformationKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationDataObjectKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationDoubleVectorKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationExecutiveKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationInformationKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationInformationVectorKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationIntegerKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationIntegerVectorKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationStringKey* key);
  void CopyEntry(vtkInformation* from, vtkInformationUnsignedLongKey* key);

  // Description:
  // Use the given key to lookup a list of other keys in the given
  // information object.  The key/value pairs associated with these
  // other keys will be copied.
  void CopyEntries(vtkInformation* from, vtkInformationKeyVectorKey* key);

  // Description:
  // Get/Set an integer-valued entry.
  void Set(vtkInformationIntegerKey* key, int value);
  int Get(vtkInformationIntegerKey* key);
  void Remove(vtkInformationIntegerKey* key);
  int Has(vtkInformationIntegerKey* key);

  // Description:
  // Get/Set an integer-valued entry.
  void Set(vtkInformationDoubleKey* key, double value);
  double Get(vtkInformationDoubleKey* key);
  void Remove(vtkInformationDoubleKey* key);
  int Has(vtkInformationDoubleKey* key);

  // Description:
  // Get/Set an integer-vector-valued entry.
  void Append(vtkInformationIntegerVectorKey* key, int value);
  void Set(vtkInformationIntegerVectorKey* key, int* value, int length);
  void Set(vtkInformationIntegerVectorKey* key, int value1, 
           int value2, int value3);
  void Set(vtkInformationIntegerVectorKey* key, 
           int value1, int value2, int value3,
           int value4, int value5, int value6);
  int* Get(vtkInformationIntegerVectorKey* key);
  void Get(vtkInformationIntegerVectorKey* key, int* value);
  int Length(vtkInformationIntegerVectorKey* key);
  void Remove(vtkInformationIntegerVectorKey* key);
  int Has(vtkInformationIntegerVectorKey* key);

  // Description:
  // Get/Set an unsigned-long-valued entry.
  void Set(vtkInformationUnsignedLongKey* key, unsigned long value);
  unsigned long Get(vtkInformationUnsignedLongKey* key);
  void Remove(vtkInformationUnsignedLongKey* key);
  int Has(vtkInformationUnsignedLongKey* key);

  // Description:
  // Get/Set an double-vector-valued entry.
  void Append(vtkInformationDoubleVectorKey* key, double value);
  void Set(vtkInformationDoubleVectorKey* key, double* value, int length);
  void Set(vtkInformationDoubleVectorKey* key, double value1, 
           double value2, double value3);
  void Set(vtkInformationDoubleVectorKey* key, 
           double value1, double value2, double value3,
           double value4, double value5, double value6);
  double* Get(vtkInformationDoubleVectorKey* key);
  void Get(vtkInformationDoubleVectorKey* key, double* value);
  int Length(vtkInformationDoubleVectorKey* key);
  void Remove(vtkInformationDoubleVectorKey* key);
  int Has(vtkInformationDoubleVectorKey* key);

  // Description:
  // Get/Set an InformationKey-vector-valued entry.
  void Append(vtkInformationKeyVectorKey* key, vtkInformationKey* value);
  void Set(vtkInformationKeyVectorKey* key, vtkInformationKey** value, int length);
  vtkInformationKey** Get(vtkInformationKeyVectorKey* key);
  void Get(vtkInformationKeyVectorKey* key, vtkInformationKey** value);
  int Length(vtkInformationKeyVectorKey* key);
  void Remove(vtkInformationKeyVectorKey* key);
  int Has(vtkInformationKeyVectorKey* key);

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
  // Get/Set an entry storing a vtkObjectBase instance.
  void Set(vtkInformationObjectBaseKey* key, vtkObjectBase*);
  vtkObjectBase* Get(vtkInformationObjectBaseKey* key);
  void Remove(vtkInformationObjectBaseKey* key);
  int Has(vtkInformationObjectBaseKey* key);

  // Description:
  // Get/Set an entry storing a vtkDataObject instance.
  void Set(vtkInformationDataObjectKey* key, vtkDataObject*);
  vtkDataObject* Get(vtkInformationDataObjectKey* key);
  void Remove(vtkInformationDataObjectKey* key);
  int Has(vtkInformationDataObjectKey* key);

  // Description:
  // Get/Set an entry storing a vtkExecutive instance.
  void Set(vtkInformationExecutiveKey* key, vtkExecutive*);
  vtkExecutive* Get(vtkInformationExecutiveKey* key);
  void Remove(vtkInformationExecutiveKey* key);
  int Has(vtkInformationExecutiveKey* key);

  // Description:
  // Upcast the given key instance.
  static vtkInformationKey* GetKey(vtkInformationDataObjectKey* key);
  static vtkInformationKey* GetKey(vtkInformationDoubleKey* key);
  static vtkInformationKey* GetKey(vtkInformationDoubleVectorKey* key);
  static vtkInformationKey* GetKey(vtkInformationExecutiveKey* key);
  static vtkInformationKey* GetKey(vtkInformationInformationKey* key);
  static vtkInformationKey* GetKey(vtkInformationInformationVectorKey* key);
  static vtkInformationKey* GetKey(vtkInformationIntegerKey* key);
  static vtkInformationKey* GetKey(vtkInformationIntegerVectorKey* key);
  static vtkInformationKey* GetKey(vtkInformationStringKey* key);
  static vtkInformationKey* GetKey(vtkInformationKey* key);
  static vtkInformationKey* GetKey(vtkInformationUnsignedLongKey* key);

  // Description:
  // Initiate garbage collection when a reference is removed.
  virtual void Register(vtkObjectBase* o);
  virtual void UnRegister(vtkObjectBase* o);

protected:
  vtkInformation();
  ~vtkInformation();

  // Get/Set a map entry directly through the vtkObjectBase instance
  // representing the value.  Used internally to manage the map.
  void SetAsObjectBase(vtkInformationKey* key, vtkObjectBase* value);
  vtkObjectBase* GetAsObjectBase(vtkInformationKey* key);

  // Internal implementation details.
  vtkInformationInternals* Internal;

  // Garbage collection support.
  virtual void ReportReferences(vtkGarbageCollector*);

  // Report the object associated with the given key to the collector.
  void ReportAsObjectBase(vtkInformationKey* key,
                          vtkGarbageCollector* collector);

private:
  //BTX
  friend class vtkInformationKeyToInformationFriendship;
  //ETX
private:
  vtkInformation(const vtkInformation&);  // Not implemented.
  void operator=(const vtkInformation&);  // Not implemented.
};

#endif
