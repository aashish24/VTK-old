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
#include "vtkExtractVectorComponents.h"

#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

vtkCxxRevisionMacro(vtkExtractVectorComponents, "$Revision$");
vtkStandardNewMacro(vtkExtractVectorComponents);

vtkExtractVectorComponents::vtkExtractVectorComponents()
{
  this->ExtractToFieldData = 0;
}

vtkExtractVectorComponents::~vtkExtractVectorComponents()
{
}

// Get the output dataset containing the indicated component. The component is 
// specified by an index between (0,2) corresponding to the x, y, or z vector
// component. By default, the x component is extracted.
vtkDataSet *vtkExtractVectorComponents::GetOutput(int i)
{
  if ( this->NumberOfOutputs < 3 )
    {
    vtkErrorMacro(<<"Abstract filters require input to be set before output can be retrieved");
    return NULL;
    }
  
  if ( i < 0 || i > 2 )
    {
    vtkErrorMacro(<<"Vector component must be between (0,2)");
    if ( i < 0 )
      {
      return (vtkDataSet *)this->Outputs[0];
      }
    if ( i > 2 )
      {
      return (vtkDataSet *)this->Outputs[2];
      }
    }

  return (vtkDataSet *)this->Outputs[i];
}

// Get the output dataset representing velocity x-component. If output is NULL
// then input hasn't been set, which is necessary for abstract objects. (Note:
// this method returns the same information as the GetOutput() method with an
// index of 0.)
vtkDataSet *vtkExtractVectorComponents::GetVxComponent()
{
  if ( this->NumberOfOutputs < 1)
    {
    vtkErrorMacro(<<"Abstract filters require input to be set before VxComponent can be retrieved");
    return 0;
    }
  return static_cast<vtkDataSet *>(this->Outputs[0]);
}

// Get the output dataset representing velocity y-component. If output is NULL
// then input hasn't been set, which is necessary for abstract objects. (Note:
// this method returns the same information as the GetOutput() method with an
// index of 1.)
vtkDataSet *vtkExtractVectorComponents::GetVyComponent()
{
  if ( this->NumberOfOutputs < 2)
    {
    vtkErrorMacro(<<"Abstract filters require input to be set before VyComponent can be retrieved");
    return 0;
    }
  return static_cast<vtkDataSet *>(this->Outputs[1]);
}

// Get the output dataset representing velocity z-component. If output is NULL
// then input hasn't been set, which is necessary for abstract objects. (Note:
// this method returns the same information as the GetOutput() method with an
// index of 2.)
vtkDataSet *vtkExtractVectorComponents::GetVzComponent()
{
  if ( this->NumberOfOutputs < 3)
    {
    vtkErrorMacro(<<"Abstract filters require input to be set before VzComponent can be retrieved");
    return 0;
    }
  return static_cast<vtkDataSet *>(this->Outputs[2]);
}

// Specify the input data or filter.
void vtkExtractVectorComponents::SetInput(vtkDataSet *input)
{
  if (this->NumberOfInputs > 0 && this->Inputs[0] == input )
    {
    return;
    }

  this->vtkProcessObject::SetNthInput(0, input);

  if ( input == NULL )
    {
    return;
    }

  if (this->NumberOfOutputs < 3)
    {
    this->SetNthOutput(0,input->NewInstance());
    this->Outputs[0]->Delete();
    this->SetNthOutput(1,input->NewInstance());
    this->Outputs[1]->Delete();
    this->SetNthOutput(2,input->NewInstance());
    this->Outputs[2]->Delete();
    return;
    }

  // since the input has changed we might need to create a new output
  if (strcmp(this->Outputs[0]->GetClassName(),input->GetClassName()))
    {
    this->SetNthOutput(0,input->NewInstance());
    this->Outputs[0]->Delete();
    this->SetNthOutput(1,input->NewInstance());
    this->Outputs[1]->Delete();
    this->SetNthOutput(2,input->NewInstance());
    this->Outputs[2]->Delete();
    vtkWarningMacro(<<" a new output had to be created since the input type changed.");
    }
}

template <class T>
void vtkExtractComponents(int numVectors, T* vectors, T* vx, T* vy, T* vz)
{
  for (int i=0; i<numVectors; i++)
    {
    vx[i] = vectors[3*i];
    vy[i] = vectors[3*i+1];
    vz[i] = vectors[3*i+2];
    }
}

void vtkExtractVectorComponents::Execute()
{
  int numVectors = 0, numVectorsc = 0;
  vtkDataArray *vectors, *vectorsc;
  vtkDataArray *vx, *vy, *vz;
  vtkDataArray *vxc, *vyc, *vzc;
  vtkPointData *pd, *outVx, *outVy=0, *outVz=0;
  vtkCellData *cd, *outVxc, *outVyc=0, *outVzc=0;

  vtkDebugMacro(<<"Extracting vector components...");

  // taken out of previous update method.
  this->GetOutput()->CopyStructure(this->GetInput());
  if (!this->ExtractToFieldData)
    {
    this->GetVyComponent()->CopyStructure(this->GetInput());
    this->GetVzComponent()->CopyStructure(this->GetInput());
    }
  
  pd = this->GetInput()->GetPointData();
  cd = this->GetInput()->GetCellData();
  outVx = this->GetOutput()->GetPointData();  
  outVxc = this->GetOutput()->GetCellData();  
  if (!this->ExtractToFieldData)
    {
    outVy = this->GetVyComponent()->GetPointData();  
    outVz = this->GetVzComponent()->GetPointData();  
    outVyc = this->GetVyComponent()->GetCellData();  
    outVzc = this->GetVzComponent()->GetCellData();  
    }

  vectors = pd->GetVectors();
  vectorsc = cd->GetVectors();
  if ( (vectors == NULL ||
        ((numVectors = vectors->GetNumberOfTuples()) < 1) ) && 
       (vectorsc == NULL ||
        ((numVectorsc = vectorsc->GetNumberOfTuples()) < 1)))  
    {
    vtkErrorMacro(<<"No vector data to extract!");
    return;
    }

  const char* name;
  if (vectors)
    {
    name = vectors->GetName();
    }
  else if (vectorsc)
    {
    name = vectorsc->GetName();
    }
  else 
    {
    name = 0;
    }

  char* newName=0;
  if (name)
    {
    newName = new char[strlen(name)+10];
    }
  else
    {
    newName = new char[10];
    name = "";
    }

  if (vectors)
    {
    vx = vtkDataArray::CreateDataArray(vectors->GetDataType());
    vx->SetNumberOfTuples(numVectors);
    sprintf(newName, "%s-x", name);
    vx->SetName(newName);
    vy = vtkDataArray::CreateDataArray(vectors->GetDataType());
    vy->SetNumberOfTuples(numVectors);
    sprintf(newName, "%s-y", name);
    vy->SetName(newName);
    vz = vtkDataArray::CreateDataArray(vectors->GetDataType());
    vz->SetNumberOfTuples(numVectors);
    sprintf(newName, "%s-z", name);
    vz->SetName(newName);

    switch (vectors->GetDataType())
      {
      vtkTemplateMacro5(vtkExtractComponents, numVectors,
                        (VTK_TT *)vectors->GetVoidPointer(0),
                        (VTK_TT *)vx->GetVoidPointer(0),
                        (VTK_TT *)vy->GetVoidPointer(0),
                        (VTK_TT *)vz->GetVoidPointer(0));
      }

    outVx->CopyScalarsOff();
    outVx->PassData(pd);
    outVx->SetScalars(vx);
    vx->Delete();
    
    if (this->ExtractToFieldData)
      {
      outVx->AddArray(vy);
      outVx->AddArray(vz);
      }
    else
      {
      outVy->CopyScalarsOff();
      outVy->PassData(pd);
      outVy->SetScalars(vy);
      
      outVz->CopyScalarsOff();
      outVz->PassData(pd);
      outVz->SetScalars(vz);
      }
    vy->Delete();
    vz->Delete();
    }

  if (vectorsc)
    {
    vxc = vtkDataArray::CreateDataArray(vectorsc->GetDataType());
    vxc->SetNumberOfTuples(numVectorsc);
    sprintf(newName, "%s-x", name);
    vxc->SetName(newName);
    vyc = vtkDataArray::CreateDataArray(vectorsc->GetDataType());
    vyc->SetNumberOfTuples(numVectorsc);
    sprintf(newName, "%s-y", name);
    vyc->SetName(newName);
    vzc = vtkDataArray::CreateDataArray(vectorsc->GetDataType());
    vzc->SetNumberOfTuples(numVectorsc);
    sprintf(newName, "%s-z", name);
    vzc->SetName(newName);

    switch (vectorsc->GetDataType())
      {
      vtkTemplateMacro5(vtkExtractComponents, numVectorsc,
                        (VTK_TT *)vectorsc->GetVoidPointer(0),
                        (VTK_TT *)vxc->GetVoidPointer(0),
                        (VTK_TT *)vyc->GetVoidPointer(0),
                        (VTK_TT *)vzc->GetVoidPointer(0));
      }

    outVxc->CopyScalarsOff();
    outVxc->PassData(cd);
    outVxc->SetScalars(vxc);
    vxc->Delete();
    
    if (this->ExtractToFieldData)
      {
      outVxc->AddArray(vyc);
      outVxc->AddArray(vzc);
      }
    else
      {
      outVyc->CopyScalarsOff();
      outVyc->PassData(cd);
      outVyc->SetScalars(vyc);
      
      outVzc->CopyScalarsOff();
      outVzc->PassData(cd);
      outVzc->SetScalars(vzc);
      }
    vyc->Delete();
    vzc->Delete();
    }
  delete[] newName;

}


//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkDataSet *vtkExtractVectorComponents::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkDataSet *)(this->Inputs[0]);
}

void vtkExtractVectorComponents::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "ExtractToFieldData: " << this->ExtractToFieldData << endl;
}
