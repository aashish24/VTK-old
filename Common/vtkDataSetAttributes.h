/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkDataSetAttributes - represent and manipulate attribute data in a dataset
// .SECTION Description
// vtkDataSetAttributes is a class that is used to represent and manipulate
// attribute data (e.g., scalars, vectors, normals, texture coordinates,
// tensors, and field data) Special methods are provided to work with filter
// objects, such as passing data through filter, copying data from one
// attribute set to another, and interpolating data given cell interpolation
// weights.

#ifndef __vtkDataSetAttributes_h
#define __vtkDataSetAttributes_h

#include "vtkObject.h"
#include "vtkScalars.h"
#include "vtkVectors.h"
#include "vtkNormals.h"
#include "vtkTCoords.h"
#include "vtkTensors.h"
#include "vtkFieldData.h"

class VTK_EXPORT vtkDataSetAttributes : public vtkFieldData
{
public:
  // Description:
  // Construct object with copying turned on for all data.
  static vtkDataSetAttributes *New();
  
  vtkTypeMacro(vtkDataSetAttributes,vtkFieldData);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initialize all of the object's data to NULL
  virtual void Initialize();

  // Description:
  // Attributes have a chance to bring themselves up to date; right
  // now this is ignored.
  virtual void Update() {}

  // Description:
  // Pass entire arrays of input data through to output. Obey the "copy"
  // flags. Note that if the copy flag of an attribute is off,
  // the corresponding array will not be passed. However if the
  // copy flag of an attribute is on, the corresponding array
  // will be copied even if the array is in not the list of arrays
  // to be copied.
  void PassData(vtkDataSetAttributes* pd);

  // Description:
  // Allocates point data for point-by-point (or cell-by-cell) copy operation.
  // If sze=0, then use the input DataSetAttributes to create (i.e., find 
  // initial size of) new objects; otherwise use the sze variable.
  // Note that pd HAS to be the vtkDataSetAttributes object which
  // will later be used with CopyData. If this is not the case,
  // consider using the alternative forms of CopyAllocate and CopyData.
  // ext is no longer used.
  void CopyAllocate(vtkDataSetAttributes* pd, int sze=0, int ext=1000);

  // Description:
  // Copy the attribute data from one id to another. Make sure CopyAllocate()
  // has been invoked before using this method.
  void CopyData(vtkDataSetAttributes *fromPd, int fromId, int toId);

  // Description:
  // Initialize point interpolation method.
  // Note that pd HAS to be the vtkDataSetAttributes object which
  // will later be used with InterpolatePoint or InterpolateEdge.
  // ext is no longer used.
  void InterpolateAllocate(vtkDataSetAttributes* pd, int sze=0, int ext=1000);
  
  // Description:
  // Interpolate data set attributes from other data set attributes
  // given cell or point ids and associated interpolation weights.
  void InterpolatePoint(vtkDataSetAttributes *fromPd, int toId, 
                        vtkIdList *ids, float *weights);
  
  // Description:
  // Interpolate data from the two points p1,p2 (forming an edge) and an 
  // interpolation factor, t, along the edge. The weight ranges from (0,1), 
  // with t=0 located at p1. Make sure that the method InterpolateAllocate() 
  // has been invoked before using this method.
  void InterpolateEdge(vtkDataSetAttributes *fromPd, int toId, int p1, int p2,
                       float t);

  // Description:
  // Interpolate data from the same id (point or cell) at different points
  // in time (parameter t). Two input data set attributes objects are input.
  // The parameter t lies between (0<=t<=1). IMPORTANT: it is assumed that
  // the number of attributes and number of components is the same for both
  // from1 and from2, and the type of data for from1 and from2 are the same.
  // Make sure that the method InterpolateAllocate() has been invoked before 
  // using this method.
  void InterpolateTime(vtkDataSetAttributes *from1, 
		       vtkDataSetAttributes *from2,
                       int id, float t);

  // Description:
  // Deep copy of data (i.e., create new data arrays and
  // copy from input data).
  virtual void DeepCopy(vtkFieldData *pd);

  // Description:
  // Shallow copy of data (i.e., use reference counting).
  virtual void ShallowCopy(vtkFieldData *pd);

  // Description:
  // Set/Get the scalar data.
  int SetScalars(vtkDataArray* da) 
    { return this->SetAttribute(da, SCALARS); }
  void SetScalars(vtkScalars* scalars)
    { this->SetAttributeData(scalars, SCALARS); }
  int SetActiveScalars(const char* name)
    { return this->SetActiveAttribute(name, SCALARS); }
  vtkScalars* GetScalars();
  vtkDataArray* GetActiveScalars() 
    { return this->GetActiveAttribute(SCALARS); }

  // Description:
  // Set/Get the vector data.
  int SetVectors(vtkDataArray* da) 
    { return this->SetAttribute(da, VECTORS); }
  void SetVectors(vtkVectors* vectors)
    { this->SetAttributeData(vectors, VECTORS); }
  int SetActiveVectors(const char* name)
    { return this->SetActiveAttribute(name, VECTORS); }
  vtkVectors* GetVectors();
  vtkDataArray* GetActiveVectors() 
    { return this->GetActiveAttribute(VECTORS); }

  // Description:
  // Set/get the normal data.
  int SetNormals(vtkDataArray* da) 
    { return this->SetAttribute(da, NORMALS); }
  void SetNormals(vtkNormals* normals)
    { this->SetAttributeData(normals, NORMALS); }
  int SetActiveNormals(const char* name)
    { return this->SetActiveAttribute(name, NORMALS); }
  vtkNormals* GetNormals();
  vtkDataArray* GetActiveNormals() 
    { return this->GetActiveAttribute(NORMALS); }

  // Description:
  // Set/Get the texture coordinate data.
  int SetTCoords(vtkDataArray* da) 
    { return this->SetAttribute(da, TCOORDS); }
  void SetTCoords(vtkTCoords* tcoords)
    { this->SetAttributeData(tcoords, TCOORDS); }
  int SetActiveTCoords(const char* name)
    { return this->SetActiveAttribute(name, TCOORDS); }
  vtkTCoords* GetTCoords();
  vtkDataArray* GetActiveTCoords() 
    { return this->GetActiveAttribute(TCOORDS); }

  // Description:
  // Set/Get the tensor data.
  int SetTensors(vtkDataArray* da) 
    { return this->SetAttribute(da, TENSORS); }
  void SetTensors(vtkTensors* Tensors)
    { this->SetAttributeData(Tensors, TENSORS); }
  int SetActiveTensors(const char* name)
    { return this->SetActiveAttribute(name, TENSORS); }
  vtkTensors* GetTensors();
  vtkDataArray* GetActiveTensors() 
    { return this->GetActiveAttribute(TENSORS); }

  // Description:
  // Make the array with the given name the active attribute.
  // Attribute types are:
  //  vtkDataSetAttributes::SCALARS = 0
  //  vtkDataSetAttributes::VECTORS = 1
  //  vtkDataSetAttributes::NORMALS = 2
  //  vtkDataSetAttributes::TCOORDS = 3
  //  vtkDataSetAttributes::TENSORS = 4
  // Returns the index of the array if succesful, -1 if the array 
  // is not in the list of arrays.
  int SetActiveAttribute(const char* name, int attributeType);

  // Description:
  // Get the field data.
  vtkFieldData* GetFieldData() { return this; }

  // Description:
  // Specify whether to copy the data attribute referred to by index i.
  void SetCopyAttribute (int index, int value) 
    { 
      if (this->CopyAttributeFlags[ index ] != value) 
	{ 
	this->CopyAttributeFlags[ index ] = value; 
	this->Modified(); 
	} 
    } 

  // Description:
  // Turn on/off the copying of scalar data.
  void SetCopyScalars(int i) { this->SetCopyAttribute(SCALARS, i); }
  int GetCopyScalars() { return this->CopyAttributeFlags[SCALARS]; }
  vtkBooleanMacro(CopyScalars, int);

  // Description:
  // Turn on/off the copying of vector data.
  void SetCopyVectors(int i) { this->SetCopyAttribute(VECTORS, i); }
  int GetCopyVectors() { return this->CopyAttributeFlags[VECTORS]; }
  vtkBooleanMacro(CopyVectors, int);

  // Description:
  // Turn on/off the copying of normals data.
  void SetCopyNormals(int i) { this->SetCopyAttribute(NORMALS, i); }
  int GetCopyNormals() { return this->CopyAttributeFlags[NORMALS]; }
  vtkBooleanMacro(CopyNormals, int);

  // Description:
  // Turn on/off the copying of texture coordinates data.
  void SetCopyTCoords(int i) { this->SetCopyAttribute(TCOORDS, i); }
  int GetCopyTCoords() { return this->CopyAttributeFlags[TCOORDS]; }
  vtkBooleanMacro(CopyTCoords, int);

  // Description:
  // Turn on/off the copying of tensor data.
  void SetCopyTensors(int i) { this->SetCopyAttribute(TENSORS, i); }
  int GetCopyTensors() { return this->CopyAttributeFlags[TENSORS]; }
  vtkBooleanMacro(CopyTensors, int);

  // Description:
  // Turn on/off the copying of the field specified by name.
  void CopyFieldOn(const char* name);
  void CopyFieldOff(const char* name);

  // Description:
  // Turn on copying of all data.
  void CopyAllOn();

  // Description:
  // Turn off copying of all data.
  void CopyAllOff();

  // Description:
  // Copy a tuple of data from one data array to another. This method (and
  // following ones) assume that the fromData and toData objects are of the
  // same type, and have the same number of components. This is true if you
  // invoke CopyAllocate() or InterpolateAllocate().
  void CopyTuple(vtkDataArray *fromData, vtkDataArray *toData, int fromId, 
                 int toId);

  // Description:
  // Get the field data array indices corresponding to scalars, 
  // vectors, tensors, etc.
  void GetAttributeIndices(int* indexArray);

  // Description:
  // Determine whether a data array of index idx is considered a data set
  // attribute (i.e., scalar, vector, tensor, etc). Return less-than zero 
  // if it is, otherwise an index 0<=idx<NUM_ATTRIBUTES to indicate 
  // which attribute.
  int IsArrayAnAttribute(int idx);

  // Description:
  // Return an attribute given the attribute type
  // (see vtkDataSetAttributes::AttributeTypes).
  vtkDataArray* GetActiveAttribute(int attributeType);

  // Description:
  // Remove an array (with the given name) from the list of arrays.
  virtual void RemoveArray(const char *name)
    {
      int i;
      this->GetArray(name, i);
      this->RemoveArray(i);
    }

//BTX
  // Always keep NUM_ATTRIBUTES as the last entry
  enum AttributeTypes {
    SCALARS=0,
    VECTORS=1,
    NORMALS=2,
    TCOORDS=3,
    TENSORS=4,
    NUM_ATTRIBUTES
  };

  enum AttributeLimitTypes {
    MAX, EXACT
  };

  // This public class is used to perform set operations, other misc. 
  // operations on fields. For example, vtkAppendFilter uses it to 
  // determine which attributes the input datasets share in common.
  class VTK_EXPORT FieldList
  {
  public:
    FieldList(int numInputs);
    ~FieldList();

    void InitializeFieldList(vtkDataSetAttributes* dsa);
    void IntersectFieldList(vtkDataSetAttributes* dsa);

    //Determine whether data is available
    int IsAttributePresent(int attrType); //true/false attributes specified
    int IsFieldPresent(const char *name); //return idx into field arrays

    friend class vtkDataSetAttributes;

  protected:
    FieldList(const FieldList&) {} //prevent these methods from being used
    void operator=(const FieldList&) {}

  private:
    void SetField(int index, vtkDataArray *da);
    void RemoveField(const char *name);
    void ClearFields();
    
    //These keep track of what is common across datasets
    char** Fields; //the names of the fields (first five are named attributes)
    int *FieldTypes; //the types of the fields (first five are named 
                     //attributes)
    int *FieldComponents; //the number of components in each  fields 
                          // (first five are named attributes)
    int *FieldIndices; //output data array index 
                       // (first five are named attributes)
    vtkLookupTable **LUT; //luts associated with each array
    int NumberOfTuples; //a running total of values
    int NumberOfFields; //the number of fields
    
    //For every vtkDataSetAttributes that are processed, keep track of the 
    //indices into various things. The indices are organized so that the
    //first NUM_ATTRIBUTES refer to attributes, the next refer to the 
    //non-attribute fields, for a total of NUM_ATTRIBUTES + NumberOfFields.
    //CurrentInput is the current input being processed.
    int **DSAIndices;
    int NumberOfDSAIndices;
    int CurrentInput;
  };

  // Description:
  // A special form of CopyAllocate() to be used with FieldLists. Use it 
  // when you are copying data from a set of vtkDataSetAttributes.
  void CopyAllocate(vtkDataSetAttributes::FieldList& list, int sze=0, 
                    int ext=1000);

  // Description:
  // A special form of CopyData() to be used with FieldLists. Use it when 
  // you are copying data from a set of vtkDataSetAttributes. Make sure 
  // that you have called the special form of CopyAllocate that accepts 
  // FieldLists.
  void CopyData(vtkDataSetAttributes::FieldList& list, 
		vtkDataSetAttributes* dsa, int idx, int fromId, int toId);

  friend class vtkDataSetAttributes::FieldList;
//ETX

protected:
  vtkDataSetAttributes();
  ~vtkDataSetAttributes();
  vtkDataSetAttributes(const vtkDataSetAttributes&) {}
  void operator=(const vtkDataSetAttributes&) {}

  // special methods to support managing data
  void InterpolateTuple(vtkDataArray *fromData, vtkDataArray *toData, int toId,
                        vtkIdList *ptIds, float *weights);
  void InterpolateTuple(vtkDataArray *fromData, vtkDataArray *toData, int toId,
                        int id1, int id2, float t);
  void InterpolateTuple(vtkDataArray *fromData1, vtkDataArray *fromData2, 
                        vtkDataArray *toData, int id, float t);

  // Manage data arrays as the  attributes scalars, vectors, tensors, etc...
  vtkAttributeData* Attributes[NUM_ATTRIBUTES]; //pointer to attributes arrays
  int AttributeIndices[NUM_ATTRIBUTES]; //index to attribute array in field data
  int CopyAttributeFlags[NUM_ATTRIBUTES]; //copy flag for attribute data
  char** CopyFieldOffFlags; //the names of fields not to be copied
  int NumberOfFieldFlags; //the number of fields not to be copied

//BTX
  vtkFieldData::BasicIterator RequiredArrays;
//ETX

  int* TargetIndices;

  virtual void RemoveArray(int index);

  static int NumberOfAttributeComponents[NUM_ATTRIBUTES];
  static int AttributeLimits[NUM_ATTRIBUTES];
  static char AttributeNames[NUM_ATTRIBUTES][10];

private:
  int SetAttribute(vtkDataArray* da, int attributeType);
  void SetAttributeData(vtkAttributeData* newAtt, int attributeType);
  vtkAttributeData* GetAttributeData(int attributeType);
  int SetActiveAttribute(int index, int attributeType);
  int FindOffFlag(const char* field);
  static int CheckNumberOfComponents(vtkDataArray* da, int attributeType);
  void CopyFieldFlags(const vtkDataSetAttributes* source);
  void ClearFieldFlags();

//BTX

  vtkFieldData::BasicIterator  ComputeRequiredArrays(vtkDataSetAttributes* pd);

//ETX

};

#endif


