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
#include "vtkDataArray.h"
#include "vtkBitArray.h"
#include "vtkCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkShortArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkIntArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkLongArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkLookupTable.h"
#include "vtkCriticalSection.h"
#include "vtkIdList.h"
#include "vtkMath.h"

// Construct object with default tuple dimension (number of components) of 1.
vtkDataArray::vtkDataArray(vtkIdType numComp)
{
  this->Range[0] = 0;
  this->Range[1] = 1;
  this->ComponentForLastRange = -1;

  this->Size = 0;
  this->MaxId = -1;
  this->LookupTable = NULL;

  this->NumberOfComponents = (numComp < 1 ? 1 : numComp);
  this->Name = 0;
}

vtkDataArray::~vtkDataArray()
{
  if ( this->LookupTable )
    {
    this->LookupTable->Delete();
    }
  delete[] this->Name;
}

void vtkDataArray::SetName(const char* name)
{
  delete[] this->Name;
  this->Name = 0;
  if (name)
    {
    int size = strlen(name);
    this->Name = new char[size+1];
    strcpy(this->Name, name);
    }
}

const char* vtkDataArray::GetName()
{
  return this->Name;
}

void vtkDataArray::DeepCopy(vtkDataArray *da)
{

  // Match the behavior of the old AttributeData
  if ( da == NULL )
    {
    return;
    }

  if ( this != da )
    {
    int numTuples = da->GetNumberOfTuples();
    this->NumberOfComponents = da->NumberOfComponents;
    this->SetNumberOfTuples(numTuples);

    for (int i=0; i < numTuples; i++)
      {
      this->SetTuple(i, da->GetTuple(i));
      }
    }
}

// These can be overridden for more efficiency
float vtkDataArray::GetComponent(const vtkIdType i, const int j)
{
  float *tuple=new float[this->NumberOfComponents], c;

  this->GetTuple(i,tuple);
  c =  tuple[j];
  delete [] tuple;

  return c;
}

void vtkDataArray::SetComponent(const vtkIdType i, const int j, const float c)
{
  float *tuple=new float[this->NumberOfComponents];

  if ( i < this->GetNumberOfTuples() )
    {
    this->GetTuple(i,tuple);
    }
  else
    {
    for (int k=0; k<this->NumberOfComponents; k++)
      {
      tuple[k] = 0.0;
      }
    }

  tuple[j] = c;
  this->SetTuple(i,tuple);

  delete [] tuple;
}

void vtkDataArray::InsertComponent(const vtkIdType i, const int j,
                                   const float c)
{
  float *tuple=new float[this->NumberOfComponents];

  if ( i < this->GetNumberOfTuples() )
    {
    this->GetTuple(i,tuple);
    }
  else
    {
    for (int k=0; k<this->NumberOfComponents; k++)
      {
      tuple[k] = 0.0;
      }
    }

  tuple[j] = c;
  this->InsertTuple(i,tuple);

  delete [] tuple;
}

void vtkDataArray::GetData(vtkIdType tupleMin, vtkIdType tupleMax, int compMin,
                           int compMax, vtkFloatArray* data)
{
  int i, j;
  int numComp=this->GetNumberOfComponents();
  float *tuple=new float[numComp];
  float *ptr=data->WritePointer(0,(tupleMax-tupleMin+1)*(compMax-compMin+1));
  
  for (j=tupleMin; j <= tupleMax; j++)
    {
    this->GetTuple(j,tuple);
    for (i=compMin; i <= compMax; i++)
      {
      *ptr++ = tuple[i];
      }
    }
  delete [] tuple;
}

void vtkDataArray::CreateDefaultLookupTable()
{
  if ( this->LookupTable )
    {
    this->LookupTable->UnRegister(this);
    }
  this->LookupTable = vtkLookupTable::New();
  // make sure it is built 
  // otherwise problems with InsertScalar trying to map through 
  // non built lut
  this->LookupTable->Build();
  this->LookupTable->Register(this);
}

void vtkDataArray::SetLookupTable(vtkLookupTable* lut)
{
  if ( this->LookupTable != lut ) 
    {
    if ( this->LookupTable )
      {
      this->LookupTable->UnRegister(this);
      }
    this->LookupTable = lut;
    this->LookupTable->Register(this);
    this->Modified();
    }
}

// default double behaviour
void vtkDataArray::GetTuple(const vtkIdType i, double * tuple)
{
  int c;
  int numComp=this->GetNumberOfComponents();
  float *ftuple=new float[numComp];
  this->GetTuple(i,ftuple);
  for (c = 0; c < numComp;  c++)
    {
    tuple[c] = ftuple[c];
    }
  delete [] ftuple;
}


float* vtkDataArray::GetTupleN(const vtkIdType i, int n)
{
  int numComp = this->GetNumberOfComponents();
  if (numComp != n)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != " << n);
    }
  return this->GetTuple(i);
}

float vtkDataArray::GetTuple1(const vtkIdType i)
{
  int numComp = this->GetNumberOfComponents();
  if (numComp != 1)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 1");
    }
  return *(this->GetTuple(i));
}
float* vtkDataArray::GetTuple2(const vtkIdType i)
{
  return this->GetTupleN(i, 2);
}
float* vtkDataArray::GetTuple3(const vtkIdType i)
{
  return this->GetTupleN(i, 3);
}
float* vtkDataArray::GetTuple4(const vtkIdType i)
{
  return this->GetTupleN(i, 4);
}
float* vtkDataArray::GetTuple9(const vtkIdType i)
{
  return this->GetTupleN(i, 9);
}

void vtkDataArray::SetTuple(const vtkIdType i, const double * tuple)
{
  int c;
  int numComp=this->GetNumberOfComponents();
  float *ftuple=new float[numComp];
  for (c = 0; c < numComp;  c++)
    {
    ftuple[c] = (float)(tuple[c]);
    }
  this->SetTuple(i,ftuple);
  delete [] ftuple;
}

void vtkDataArray::SetTuple1(const vtkIdType i, float value)
{
  int numComp = this->GetNumberOfComponents();
  if (numComp != 1)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 1");
    }
  this->SetTuple(i, &value);
}
void vtkDataArray::SetTuple2(const vtkIdType i, float val0, float val1)
{
  float tuple[2];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 2)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 2");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  this->SetTuple(i, tuple);
}
void vtkDataArray::SetTuple3(const vtkIdType i, float val0, float val1, 
                               float val2)
{
  float tuple[3];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 3)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 3");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  this->SetTuple(i, tuple);
}
void vtkDataArray::SetTuple4(const vtkIdType i, float val0, float val1, 
                             float val2, float val3)
{
  float tuple[4];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 4)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 4");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  tuple[3] = val3;
  this->SetTuple(i, tuple);
}
void vtkDataArray::SetTuple9(const vtkIdType i, float val0, float val1, 
                             float val2,  float val3, float val4, 
                             float val5, float val6,float val7, float val8)
{
  float tuple[9];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 9)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 9");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  tuple[3] = val3;
  tuple[4] = val4;
  tuple[5] = val5;
  tuple[6] = val6;
  tuple[7] = val7;
  tuple[8] = val8;
  this->SetTuple(i, tuple);
}

void vtkDataArray::InsertTuple(const vtkIdType i, const double * tuple)
{
  int c;
  int numComp=this->GetNumberOfComponents();
  float *ftuple=new float[numComp];
  for (c = 0; c < numComp;  c++)
    {
    ftuple[c] = (float)(tuple[c]);
    }
  this->InsertTuple(i,ftuple);
  delete [] ftuple;
}

void vtkDataArray::InsertTuple1(const vtkIdType i, float value)
{
  int numComp = this->GetNumberOfComponents();
  if (numComp != 1)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 1");
    }
  this->InsertTuple(i, &value);
}
void vtkDataArray::InsertTuple2(const vtkIdType i, float val0, float val1)
{
  float tuple[2];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 2)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 2");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  this->InsertTuple(i, tuple);
}
void vtkDataArray::InsertTuple3(const vtkIdType i, float val0, float val1, 
                                float val2)
{
  float tuple[3];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 3)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 3");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  this->InsertTuple(i, tuple);
}
void vtkDataArray::InsertTuple4(const vtkIdType i, float val0, float val1, 
                                float val2, float val3)
{
  float tuple[4];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 4)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 4");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  tuple[3] = val3;
  this->InsertTuple(i, tuple);
}
void vtkDataArray::InsertTuple9(const vtkIdType i, float val0, float val1, 
                                float val2,  float val3, float val4, 
                                float val5, float val6,float val7, float val8)
{
  float tuple[9];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 9)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 9");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  tuple[3] = val3;
  tuple[4] = val4;
  tuple[5] = val5;
  tuple[6] = val6;
  tuple[7] = val7;
  tuple[8] = val8;
  this->InsertTuple(i, tuple);
}

vtkIdType vtkDataArray::InsertNextTuple(const double * tuple)
{
  int c;
  int numComp=this->GetNumberOfComponents();
  float *ftuple=new float[numComp];
  for (c = 0; c < numComp;  c++)
    {
    ftuple[c] = (float)(tuple[c]);
    }
  int ret = this->InsertNextTuple(ftuple);
  delete [] ftuple;
  return ret;
}

void vtkDataArray::InsertNextTuple1(float value)
{
  int numComp = this->GetNumberOfComponents();
  if (numComp != 1)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 1");
    }
  this->InsertNextTuple(&value);
}
void vtkDataArray::InsertNextTuple2(float val0, float val1)
{
  float tuple[2];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 2)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 2");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  this->InsertNextTuple(tuple);
}
void vtkDataArray::InsertNextTuple3(float val0, float val1, 
                                    float val2)
{
  float tuple[3];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 3)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 3");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  this->InsertNextTuple(tuple);
}
void vtkDataArray::InsertNextTuple4(float val0, float val1, 
                                    float val2, float val3)
{
  float tuple[4];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 4)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 4");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  tuple[3] = val3;
  this->InsertNextTuple(tuple);
}
void vtkDataArray::InsertNextTuple9(float val0, float val1, 
                                    float val2,  float val3, float val4, 
                                    float val5, float val6,float val7, 
                                    float val8)
{
  float tuple[9];
  int numComp = this->GetNumberOfComponents();
  if (numComp != 9)
    {
    vtkErrorMacro("The number of components do not match the number requested: "
                  << numComp << " != 9");
    }
  tuple[0] = val0;
  tuple[1] = val1;
  tuple[2] = val2;
  tuple[3] = val3;
  tuple[4] = val4;
  tuple[5] = val5;
  tuple[6] = val6;
  tuple[7] = val7;
  tuple[8] = val8;
  this->InsertNextTuple(tuple);
}

unsigned long vtkDataArray::GetActualMemorySize()
{
  unsigned long numPrims;
  float size = 0.0;
  numPrims = this->GetNumberOfTuples() * this->GetNumberOfComponents();

  switch (this->GetDataType())
    {
    case VTK_BIT:
      size = (float)sizeof(char)/8.0;
      break;

    case VTK_CHAR:
      size = (float)sizeof(char);
      break;

    case VTK_UNSIGNED_CHAR:
      size = (float)sizeof(unsigned char);
      break;

    case VTK_SHORT:
      size = (float)sizeof(short);
      break;

    case VTK_UNSIGNED_SHORT:
      size = (float)sizeof(unsigned short);
      break;

    case VTK_INT:
      size = (float)sizeof(int);
      break;

    case VTK_UNSIGNED_INT:
      size = (float)sizeof(unsigned int);
      break;

    case VTK_LONG:
      size = (float)sizeof(long);
      break;

    case VTK_UNSIGNED_LONG:
      size = (float)sizeof(unsigned long);
      break;

    case VTK_FLOAT:
      size = (float)sizeof(float);
      break;

    case VTK_DOUBLE:
      size = (float)sizeof(double);
      break;

    case VTK_ID_TYPE:
      size = (float)sizeof(vtkIdType);
      break;

    default:
      vtkErrorMacro(<<"Unsupported data type!");
    }

  return (unsigned long)ceil((size * numPrims)/1000.0); //kilobytes
}

vtkDataArray* vtkDataArray::CreateDataArray(int dataType)
{
  switch (dataType)
    {
    case VTK_BIT:
      return vtkBitArray::New();

    case VTK_CHAR:
      return vtkCharArray::New();

    case VTK_UNSIGNED_CHAR:
      return vtkUnsignedCharArray::New();

    case VTK_SHORT:
      return vtkShortArray::New();

    case VTK_UNSIGNED_SHORT:
      return vtkUnsignedShortArray::New();

    case VTK_INT:
      return vtkIntArray::New();

    case VTK_UNSIGNED_INT:
      return vtkUnsignedIntArray::New();

    case VTK_LONG:
      return vtkLongArray::New();

    case VTK_UNSIGNED_LONG:
      return vtkUnsignedLongArray::New();

    case VTK_FLOAT:
      return vtkFloatArray::New();

    case VTK_DOUBLE:
      return vtkDoubleArray::New();

    case VTK_ID_TYPE:
      return vtkIdTypeArray::New();

    default:
      vtkGenericWarningMacro(<<"Unsupported data type! Setting to VTK_FLOAT");
      return vtkFloatArray::New();
    }
}

template <class IT, class OT>
static void CopyTuples(IT* input, OT* output, int nComp, vtkIdList* ptIds )
{
  int i, j;
  int num=ptIds->GetNumberOfIds();
  for (i=0; i<num; i++)
    {
    for (j=0; j<nComp; j++)
      {
      output[i*nComp+j] = static_cast<OT>(input[ptIds->GetId(i)*nComp+j]);
      }
    }
}

template <class IT>
static void CopyTuples1(IT* input, vtkDataArray* output, vtkIdList* ptIds)
{
  switch (output->GetDataType())
    {
    vtkTemplateMacro4(CopyTuples, input, (VTK_TT *)output->GetVoidPointer(0), 
                      output->GetNumberOfComponents(), ptIds );

    default:
      vtkGenericWarningMacro(<<"Sanity check failed: Unsupported data type.");
      return;
    }
}

void vtkDataArray::GetTuples(vtkIdList *ptIds, vtkDataArray *da)
{

  if ((da->GetNumberOfComponents() != this->GetNumberOfComponents()))
    {
    vtkWarningMacro("Number of components for input and output do not match");
    return;
    }

  switch (this->GetDataType())
    {
    vtkTemplateMacro3(CopyTuples1, (VTK_TT *)this->GetVoidPointer(0), da,
                      ptIds );
    // This is not supported by the template macro.
    // Switch to using the float interface.
    case VTK_BIT:
      {
      vtkIdType num=ptIds->GetNumberOfIds();
      for (vtkIdType i=0; i<num; i++)
        {
        da->SetTuple(i,this->GetTuple(ptIds->GetId(i)));
        }
      }
      break;
    default:
      vtkErrorMacro(<<"Sanity check failed: Unsupported data type.");
      return;
    }
}

template <class IT, class OT>
static void CopyTuples(IT* input, OT* output, int nComp, 
                       vtkIdType p1, vtkIdType p2)
{
  int i, j;
  int num=p2-p1+1;
  for (i=0; i<num; i++)
    {
    for (j=0; j<nComp; j++)
      {
      output[i*nComp+j] = static_cast<OT>(input[(p1+i)*nComp+j]);
      }
    }
}

template <class IT>
static void CopyTuples1(IT* input, vtkDataArray* output, 
                        vtkIdType p1, vtkIdType p2)
{
  switch (output->GetDataType())
    {
    vtkTemplateMacro5(CopyTuples, input, (VTK_TT *)output->GetVoidPointer(0), 
                      output->GetNumberOfComponents(), p1, p2 );

    default:
      vtkGenericWarningMacro(<<"Sanity check failed: Unsupported data type.");
      return;
    }
}


void vtkDataArray::GetTuples(vtkIdType p1, vtkIdType p2, vtkDataArray *da)
{

  if ((da->GetNumberOfComponents() != this->GetNumberOfComponents()))
    {
    vtkWarningMacro("Number of components for input and output do not match");
    return;
    }

  switch (this->GetDataType())
    {
    vtkTemplateMacro4(CopyTuples1, (VTK_TT *)this->GetVoidPointer(0), da,
                      p1, p2 );
    // This is not supported by the template macro.
    // Switch to using the float interface.
    case VTK_BIT:
      {
      vtkIdType num=p2-p1+1;
      for (vtkIdType i=0; i<num; i++)
        {
        da->SetTuple(i,this->GetTuple(p1+i));
        }
      }
      break;
    default:
      vtkErrorMacro(<<"Sanity check failed: Unsupported data type.");
      return;
    }
}

void vtkDataArray::FillComponent(const int j, const float c)
{
  if (j < 0 || j >= this->GetNumberOfComponents())
    {
    vtkErrorMacro(<< "Specified component " << j << " is not in [0, "
    << this->GetNumberOfComponents() << ")" );
    return;
    }
  
  vtkIdType i;

  for (i = 0; i < this->GetNumberOfTuples(); i++)
    {
    this->SetComponent(i, j, c);
    }
}


void vtkDataArray::CopyComponent(const int j, vtkDataArray *from,
                                 const int fromComponent)
{
  if (this->GetNumberOfTuples() != from->GetNumberOfTuples())
    {
    vtkErrorMacro(<< "Number of tuples in 'from' ("
    << from->GetNumberOfTuples() << ") and 'to' ("
    << this->GetNumberOfTuples() << ") do not match.");
    return;
    }

  if (j < 0 || j >= this->GetNumberOfComponents())
    {
    vtkErrorMacro(<< "Specified component " << j << " in 'to' array is not in [0, "
    << this->GetNumberOfComponents() << ")" );
    return;
    }

  if (fromComponent < 0 || fromComponent >= from->GetNumberOfComponents())
    {
    vtkErrorMacro(<< "Specified component " << fromComponent << " in 'from' array is not in [0, "
    << from->GetNumberOfComponents() << ")" );
    return;
    }

  vtkIdType i;
  for (i = 0; i < this->GetNumberOfTuples(); i++)
    {
    this->SetComponent(i, j, from->GetComponent(i, fromComponent));
    }
}

float vtkDataArray::GetMaxNorm()
{
  vtkIdType i;
  float norm, maxNorm;
  int nComponents = this->GetNumberOfComponents();

  maxNorm = 0.0;
  for (i=0; i<this->GetNumberOfTuples(); i++)
    {
    norm = vtkMath::Norm(this->GetTuple(i), nComponents);
    if ( norm > maxNorm )
      {
      maxNorm = norm;
      }
    }

  return maxNorm;
}

void vtkDataArray::ComputeRange(int comp)
{
  float s;
  vtkIdType numTuples;

  if ( (this->GetMTime() > this->ComputeTimeForLastRange) ||
       (comp != this->ComponentForLastRange))
    {
    numTuples=this->GetNumberOfTuples();
    this->Range[0] =  VTK_LARGE_FLOAT;
    this->Range[1] =  -VTK_LARGE_FLOAT;

    for (vtkIdType i=0; i<numTuples; i++)
      {
      s = this->GetComponent(i,comp);
      if ( s < this->Range[0] )
        {
        this->Range[0] = s;
        }
      if ( s > this->Range[1] )
        {
        this->Range[1] = s;
        }
      }
    this->ComputeTimeForLastRange.Modified();
    this->ComponentForLastRange = comp;
    }

}

void vtkDataArray::GetDataTypeRange(double range[2])
{
  range[0] = this->GetDataTypeMin();
  range[1] = this->GetDataTypeMax();
}

double vtkDataArray::GetDataTypeMin()
{
  int dataType=this->GetDataType();
  switch (dataType)
    {
    case VTK_BIT:            return (double)VTK_BIT_MIN;
    case VTK_UNSIGNED_CHAR:  return (double)VTK_UNSIGNED_CHAR_MIN;
    case VTK_CHAR:           return (double)VTK_CHAR_MIN;
    case VTK_UNSIGNED_SHORT: return (double)VTK_UNSIGNED_SHORT_MIN;
    case VTK_SHORT:          return (double)VTK_SHORT_MIN;
    case VTK_UNSIGNED_INT:   return (double)VTK_UNSIGNED_INT_MIN;
    case VTK_INT:            return (double)VTK_INT_MIN;
    case VTK_UNSIGNED_LONG:  return (double)VTK_UNSIGNED_LONG_MIN;
    case VTK_LONG:           return (double)VTK_LONG_MIN;
    case VTK_FLOAT:          return (double)VTK_FLOAT_MIN;
    case VTK_DOUBLE:         return (double)VTK_DOUBLE_MIN;
    default: return 0;
    }
}

double vtkDataArray::GetDataTypeMax()
{
  int dataType=this->GetDataType();
  switch (dataType)
    {
    case VTK_BIT:            return (double)VTK_BIT_MAX;
    case VTK_UNSIGNED_CHAR:  return (double)VTK_UNSIGNED_CHAR_MAX;
    case VTK_CHAR:           return (double)VTK_CHAR_MAX;
    case VTK_UNSIGNED_SHORT: return (double)VTK_UNSIGNED_SHORT_MAX;
    case VTK_SHORT:          return (double)VTK_SHORT_MAX;
    case VTK_UNSIGNED_INT:   return (double)VTK_UNSIGNED_INT_MAX;
    case VTK_INT:            return (double)VTK_INT_MAX;
    case VTK_UNSIGNED_LONG:  return (double)VTK_UNSIGNED_LONG_MAX;
    case VTK_LONG:           return (double)VTK_LONG_MAX;
    case VTK_FLOAT:          return (double)VTK_FLOAT_MAX;
    case VTK_DOUBLE:         return (double)VTK_DOUBLE_MAX;
    default: return 1;
    }
}

void vtkDataArray::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);

  const char* name = this->GetName();
  if (name)
    {
    os << indent << "Name: " << name << "\n";
    }
  else
    {
    os << indent << "Name: (none)\n";
    }
  os << indent << "Number Of Components: " << this->NumberOfComponents << "\n";
  os << indent << "Number Of Tuples: " << this->GetNumberOfTuples() << "\n";
  os << indent << "Size: " << this->Size << "\n";
  os << indent << "MaxId: " << this->MaxId << "\n";
  if ( this->LookupTable )
    {
    os << indent << "Lookup Table:\n";
    this->LookupTable->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "LookupTable: (none)\n";
    }
}
