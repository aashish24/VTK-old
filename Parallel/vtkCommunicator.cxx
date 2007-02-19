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
#include "vtkCommunicator.h"

#include "vtkBoundingBox.h"
#include "vtkCharArray.h"
#include "vtkDataSetReader.h"
#include "vtkDataSetWriter.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGenericDataObjectReader.h"
#include "vtkGenericDataObjectWriter.h"
#include "vtkIdTypeArray.h"
#include "vtkImageClip.h"
#include "vtkImageData.h"
#include "vtkIntArray.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkStructuredPointsReader.h"
#include "vtkStructuredPointsWriter.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedLongArray.h"

vtkCxxRevisionMacro(vtkCommunicator, "$Revision$");

template <class T>
int SendDataArray(T* data, int length, int handle, int tag, vtkCommunicator *self)
{

  self->Send(data, length, handle, tag);

  return 1;
}


vtkCommunicator::vtkCommunicator()
{
  this->MarshalString = 0;
  this->MarshalStringLength = 0;
  this->MarshalDataLength = 0;
}

vtkCommunicator::~vtkCommunicator()
{
  this->DeleteAndSetMarshalString(0, 0);
}

int vtkCommunicator::UseCopy = 0;
void vtkCommunicator::SetUseCopy(int useCopy)
{
  vtkCommunicator::UseCopy = useCopy;
}

void vtkCommunicator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Marshal string: ";
  if ( this->MarshalString )
    {
    os << this->MarshalString << endl;
    }
  else
    {
    os << "(None)" << endl;
    }
  os << indent << "Marshal string length: " << this->MarshalStringLength
     << endl;
  os << indent << "Marshal data length: " << this->MarshalDataLength
     << endl;
}

//----------------------------------------------------------------------------
// Internal method.  Assumes responsibility for deleting the string
void vtkCommunicator::DeleteAndSetMarshalString(char *str, int strLength)
{
  // delete any previous string
  if (this->MarshalString)
    {
    delete [] this->MarshalString;
    this->MarshalString = 0;
    this->MarshalStringLength = 0;
    this->MarshalDataLength = 0;
    }
  
  this->MarshalString = str;
  this->MarshalStringLength = strLength;
}

// Need to add better error checking
int vtkCommunicator::Send(vtkDataObject* data, int remoteHandle, 
                          int tag)
{

  if (data == NULL)
    {
    this->MarshalDataLength = 0;
    this->Send( &this->MarshalDataLength, 1,      
                remoteHandle, tag);
    return 1;
    }
  if (this->WriteObject(data))
    {
    this->Send( &this->MarshalDataLength, 1,      
                remoteHandle, tag);
    // then send the string.
    this->Send( this->MarshalString, this->MarshalDataLength, 
                remoteHandle, tag);
    // Send data extents. These make sense only for structured data.
    // However, we still send them. We need to send extents separately
    // because the Legacy writers discard extents.
    int extent[6] = {0,0,0,0,0,0};
    if (data->GetExtentType() == VTK_3D_EXTENT)
      {
      vtkRectilinearGrid* rg = vtkRectilinearGrid::SafeDownCast(data);
      vtkStructuredGrid* sg = vtkStructuredGrid::SafeDownCast(data);
      vtkImageData* id = vtkImageData::SafeDownCast(data);
      if (rg)
        {
        rg->GetExtent(extent);
        }
      else if (sg)
        {
        sg->GetExtent(extent);
        }
      else if (id)
        {
        id->GetExtent(extent);
        }
      }
    this->Send(extent, 6, remoteHandle, tag);

    return 1;
    }
  
  // could not marshal data
  return 0;
}

int vtkCommunicator::Send(vtkDataArray* data, int remoteHandle, int tag)
{

  int type = -1;
  if (data == NULL)
    {
      this->MarshalDataLength = 0;
      this->Send( &type, 1, remoteHandle, tag);
      return 1;
    }

  // send array type
  type = data->GetDataType();
  this->Send( &type, 1, remoteHandle, tag);

  // send array tuples
  vtkIdType numTuples = data->GetNumberOfTuples();
  this->Send( &numTuples, 1, remoteHandle, tag);

  // send number of components in array
  int numComponents = data->GetNumberOfComponents();
  this->Send( &numComponents, 1, remoteHandle, tag);

  vtkIdType size = numTuples*numComponents;

  
  const char* name = data->GetName();
  int len = 0;
  if (name)
    {
    len = static_cast<int>(strlen(name)) + 1;
    }

  // send length of name
  this->Send( &len, 1, remoteHandle, tag);

  if (len > 0)
    {
    // send name
    this->Send( const_cast<char*>(name), len, remoteHandle, tag);
    }

  // do nothing if size is zero.
  if (size == 0)
    {
    return 1;
    }

  // now send the raw array
  switch (type)
    {

    case VTK_CHAR:
      return SendDataArray(static_cast<char*>(data->GetVoidPointer(0)), 
                          size, remoteHandle, tag, this);

    case VTK_UNSIGNED_CHAR:
      return SendDataArray(static_cast<unsigned char*>(data->GetVoidPointer(0)), 
                          size, remoteHandle, tag, this);

    case VTK_INT:
      return SendDataArray(static_cast<int*>(data->GetVoidPointer(0)), 
                          size, remoteHandle, tag, this);

    case VTK_UNSIGNED_LONG:
      return SendDataArray(static_cast<unsigned long*>(data->GetVoidPointer(0)), 
                          size, remoteHandle, tag, this);

    case VTK_FLOAT:
      return SendDataArray(static_cast<float*>(data->GetVoidPointer(0)), 
                          size, remoteHandle, tag, this);

    case VTK_DOUBLE:
      return SendDataArray(static_cast<double*>(data->GetVoidPointer(0)), 
                          size, remoteHandle, tag, this);

    case VTK_ID_TYPE:
      return SendDataArray(static_cast<vtkIdType*>(data->GetVoidPointer(0)), 
                          size, remoteHandle, tag, this);

    default:
      vtkErrorMacro(<<"Unsupported data type!");
      return 0; // could not marshal data

    }

}


int vtkCommunicator::Receive(vtkDataObject* data, int remoteHandle, 
                             int tag)
{
  int dataLength;

  // First receive the data length.
  if (!this->Receive( &dataLength, 1, remoteHandle, tag))
    {
    vtkErrorMacro("Could not receive data!");
    return 0;
    }
  
  if (dataLength < 0)
    {
    vtkErrorMacro("Bad data length");
    return 0;
    }
  
  if (dataLength == 0)
    { // This indicates a NULL object was sent. Do nothing.
    return 1;   
    }
  
  // if we cannot reuse the string, allocate a new one.
  if (dataLength > this->MarshalStringLength)
    {
    char *str = new char[dataLength + 10]; // maybe a little extra?
    this->DeleteAndSetMarshalString(str, dataLength + 10);
    }
  
  // Receive the string
  if (!this->Receive(this->MarshalString, dataLength, 
                     remoteHandle, tag))
    {
    return 0;
    }

  int extent[6];
  // Receive the extents.
  if (!this->Receive(extent, 6, remoteHandle, tag))
    {
    return 0;
    }

  this->MarshalDataLength = dataLength;

  this->ReadObject(data);

  // Set the extents if the dataobject supports it.
  if (data->GetExtentType() == VTK_3D_EXTENT)
    {
    vtkRectilinearGrid* rg = vtkRectilinearGrid::SafeDownCast(data);
    vtkStructuredGrid* sg = vtkStructuredGrid::SafeDownCast(data);
    vtkImageData* id = vtkImageData::SafeDownCast(data);
    if (rg)
      {
      rg->SetExtent(extent);
      }
    else if (sg)
      {
      sg->SetExtent(extent);
      }
    else if (id)
      {
      id->SetExtent(extent);
      }
    }

  // we should really look at status to determine success
  return 1;
}

int vtkCommunicator::Receive(vtkDataArray* data, int remoteHandle, 
                             int tag)
{
  vtkIdType numTuples;
  int type;
  int numComponents;
  int nameLength;

  char *c = 0;
  unsigned char *uc = 0;
  int *i = 0;
  unsigned long *ul = 0;
  float *f = 0;
  double *d = 0;
  vtkIdType *idt = 0;
  

  // First receive the data type.
  if (!this->Receive( &type, 1, remoteHandle, tag))
    {
    vtkErrorMacro("Could not receive data!");
    return 0;
    }

  if (type == -1) 
    { // This indicates a NULL object was sent. Do nothing.
    return 1;   
    }

  // Next receive the number of tuples.
  if (!this->Receive( &numTuples, 1, remoteHandle, tag))
    {
    vtkErrorMacro("Could not receive data!");
    return 0;
    }

  // Next receive the number of components.
  this->Receive( &numComponents, 1, remoteHandle, tag);

  vtkIdType size = numTuples*numComponents;

  // Next receive the length of the name.
  this->Receive( &nameLength, 1, remoteHandle, tag);

  if ( nameLength > 0 )
    {
    char *str = new char[nameLength]; 
    this->DeleteAndSetMarshalString(str, nameLength);
    
    // Receive the name
    this->Receive(this->MarshalString, nameLength, remoteHandle, tag);
    this->MarshalDataLength = nameLength;
    }

  if (size < 0)
    {
    vtkErrorMacro("Bad data length");
    return 0;
    }
  
  // Do nothing if size is zero.
  if (size == 0)
    {
    return 1;   
    }
  
  // Receive the raw data array
  switch (type)
    {

    case VTK_CHAR:
      c = new char[size];
      this->Receive(c, size, remoteHandle, tag);
      static_cast<vtkCharArray*>(data)->SetArray(c, size, 0);
      break;

    case VTK_UNSIGNED_CHAR:
      uc = new unsigned char[size];
      this->Receive(uc, size, remoteHandle, tag);
      static_cast<vtkUnsignedCharArray*>(data)->SetArray(uc, size, 0);
      break;

    case VTK_INT:
      i = new int[size];
      this->Receive(i, size, remoteHandle, tag);
      static_cast<vtkIntArray*>(data)->SetArray(i, size, 0);
      break;

    case VTK_UNSIGNED_LONG:
      ul = new unsigned long[size];
      this->Receive(ul, size, remoteHandle, tag);
      static_cast<vtkUnsignedLongArray*>(data)->SetArray(ul, size, 0);
      break;

    case VTK_FLOAT:
      f = new float[size];
      this->Receive(f, size, remoteHandle, tag);
      static_cast<vtkFloatArray*>(data)->SetArray(f, size, 0);
      break;

    case VTK_DOUBLE:

      d = new double[size];
      this->Receive(d, size, remoteHandle, tag);
      static_cast<vtkDoubleArray*>(data)->SetArray(d, size, 0);
      break;

    case VTK_ID_TYPE:
      idt = new vtkIdType[size];
      this->Receive(idt, size, remoteHandle, tag);
      static_cast<vtkIdTypeArray*>(data)->SetArray(idt, size, 0);
      break;

    default:
      vtkErrorMacro(<<"Unsupported data type!");
      return 0; // could not marshal data

    }

  if (nameLength > 0)
    {
    data->SetName(this->MarshalString);
    }
  else
    {
    data->SetName(0);
    }
  data->SetNumberOfComponents(numComponents);

  return 1;

}

int vtkCommunicator::WriteObject(vtkDataObject *object)
{
  vtkGenericDataObjectWriter* writer = vtkGenericDataObjectWriter::New();

  vtkDataObject* copy = object->NewInstance();
  copy->ShallowCopy(object);

  writer->SetFileTypeToBinary();
  // There is a problem with binary files with no data.
  if (vtkDataSet::SafeDownCast(copy) != NULL)
    {
    vtkDataSet* ds = vtkDataSet::SafeDownCast(copy);
    if (ds->GetNumberOfCells() + ds->GetNumberOfPoints() == 0)
      {
      writer->SetFileTypeToASCII();
      }
    }
  writer->WriteToOutputStringOn();
  writer->SetInput(copy);
  
  writer->Write();
  unsigned int size = writer->GetOutputStringLength();
  this->DeleteAndSetMarshalString(writer->RegisterAndGetOutputString(), size);
  this->MarshalDataLength = size;

  writer->Delete();
  copy->Delete();
  return 1;
}

int vtkCommunicator::ReadObject(vtkDataObject *object)
{
  if (this->MarshalString == NULL || this->MarshalStringLength <= 0)
    {
    return 0;
    }
  
  vtkGenericDataObjectReader* reader = vtkGenericDataObjectReader::New();
  reader->ReadFromInputStringOn();

  vtkCharArray* mystring = vtkCharArray::New();
  // mystring should not delete the string when it's done,
  // that's our job.
  mystring->SetArray(this->MarshalString, this->MarshalDataLength, 1);
  reader->SetInputArray(mystring);
  mystring->Delete();

  reader->Update();
  object->ShallowCopy(reader->GetOutput());

  reader->Delete();
  return 1;
}

// The processors are views as a heap tree. The root is the processor of
// id 0.
//-----------------------------------------------------------------------------
int vtkCommunicator::GetParentProcessor(int proc)
{
  int result;
  if(proc%2==1)
    {
    result=proc>>1; // /2
    }
  else
    {
    result=(proc-1)>>1; // /2
    }
  return result;
}

int vtkCommunicator::GetLeftChildProcessor(int proc)
{
  return (proc<<1)+1; // *2+1
}

int vtkCommunicator::ComputeGlobalBounds(int processNumber, int numProcessors,
                                         vtkBoundingBox *bounds,
                                         int *rhb, int *lhb,
                                         int hasBoundsTag,
                                         int localBoundsTag,
                                         int globalBoundsTag)
{
  int parent = 0;
  int leftHasBounds = 0, rightHasBounds = 0;
  int left = this->GetLeftChildProcessor(processNumber);
  int right=left+1;
  if(processNumber>0) // not root (nothing to do if root)
    {
    parent=this->GetParentProcessor(processNumber);
    }
  
  double otherBounds[6];
  if(left<numProcessors)
    {
    this->Receive(&leftHasBounds, 1, left, hasBoundsTag);
    if (lhb)
      {
      *lhb = leftHasBounds;
      }

    if(leftHasBounds)
      {
      this->Receive(otherBounds, 6, left, localBoundsTag);
      bounds->AddBounds(otherBounds);
      }
    }
  if(right<numProcessors)
    {
    // Grab the bounds from right child
    this->Receive(&rightHasBounds, 1, right, hasBoundsTag);
    
    if (rhb)
      {
      *rhb = rightHasBounds;
      }

    if(rightHasBounds)
      {
      this->Receive(otherBounds, 6, right, localBoundsTag);
      bounds->AddBounds(otherBounds);
      }
    }
  
  // If there are bounds to send do so
  int boundsHaveBeenSet = bounds->IsValid();
  double b[6];
  // Send local to parent, Receive global from the parent.
  if(processNumber > 0) // not root (nothing to do if root)
    {
    this->Send(&boundsHaveBeenSet, 1, parent, hasBoundsTag);
    if(boundsHaveBeenSet)
      {
      // Copy the bounds to an array so we can send them
      
      bounds->GetBounds(b);
      this->Send(b, 6, parent, localBoundsTag);
      
      this->Receive(b, 6, parent, globalBoundsTag);
      bounds->AddBounds(b);
      }
    }
  
  if(!boundsHaveBeenSet) // empty, no bounds, nothing to do
    {
    return 1;
    }
  
  // Send it to children.
  bounds->GetBounds(b);
  if(left<numProcessors)
    {
    if(leftHasBounds)
      {
      this->Send(b, 6, left, globalBoundsTag);
      }
    if(right<numProcessors)
      {
      if(rightHasBounds)
        {
        this->Send(b, 6, right, globalBoundsTag);
        }
      }
    }
  return 1;
}
