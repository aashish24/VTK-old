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
#include "vtkKitwareContourFilter.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkContourValues.h"
#include "vtkGridSynchronizedTemplates3D.h"
#include "vtkImageData.h"
#include "vtkMergePoints.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkRectilinearSynchronizedTemplates.h"
#include "vtkScalarTree.h"
#include "vtkStructuredGrid.h"
#include "vtkSynchronizedTemplates2D.h"
#include "vtkSynchronizedTemplates3D.h"

#include <math.h>

vtkCxxRevisionMacro(vtkKitwareContourFilter, "$Revision$");
vtkStandardNewMacro(vtkKitwareContourFilter);

// Construct object with initial range (0,1) and single contour value
// of 0.0.
vtkKitwareContourFilter::vtkKitwareContourFilter()
{
  this->ArrayComponent = 0;
}

vtkKitwareContourFilter::~vtkKitwareContourFilter()
{
}

int vtkKitwareContourFilter::RequestUpdateExtent(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));
  
  if (!input || !output) 
    {
    return 0;
    }

  const char* inputObjectType = inInfo->Get(vtkDataObject::DATA_TYPE_NAME());

  if ( !strcmp(inputObjectType, "vtkStructuredPoints") ||
       !strcmp(inputObjectType, "vtkImageData"))
    {     
    int ext[6], dims[3], dim=0;
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext);
    for(int j=0; j<3; j++)
      {
      dims[j] = ext[2*j+1]-ext[2*j];
      if ( dims[j] != 0 )
        {
        dim++;
        }
      }
     
    if (dim == 2)
      {
      vtkSynchronizedTemplates2D *syncTemp2D = 
        vtkSynchronizedTemplates2D::New();
      syncTemp2D->SetInput(vtkImageData::SafeDownCast(input));
      syncTemp2D->SetDebug(this->Debug);
      syncTemp2D->RequestUpdateExtent(request, inputVector, outputVector);
      syncTemp2D->Delete();
      return 1;
      }
    else if (dim == 3)
      {
      vtkSynchronizedTemplates3D *syncTemp3D =
        vtkSynchronizedTemplates3D::New();
      syncTemp3D->SetInput(vtkImageData::SafeDownCast(input));
      syncTemp3D->SetDebug(this->Debug);
      syncTemp3D->SetComputeNormals (this->ComputeNormals);
      syncTemp3D->SetComputeGradients (this->ComputeGradients);
      syncTemp3D->SetComputeScalars (this->ComputeScalars);
      syncTemp3D->ComputeInputUpdateExtents(output);
      syncTemp3D->Delete();
      return 1;
      }
    }

  if ( !strcmp(inputObjectType, "vtkStructuredGrid") )
    {
    int ext[6], dim=0;
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext);
    for(int j=0; j<3; j++)
      {
      if ( ( ext[2*j+1]-ext[2*j] ) != 0 )
        {
        dim++;
        }
      }
    if (dim == 3)
      {
      vtkGridSynchronizedTemplates3D *gridTemp3D =
        vtkGridSynchronizedTemplates3D::New();
      gridTemp3D->SetInput(vtkStructuredGrid::SafeDownCast(input));
      gridTemp3D->SetComputeNormals (this->ComputeNormals);
      gridTemp3D->SetComputeGradients (this->ComputeGradients);
      gridTemp3D->SetComputeScalars (this->ComputeScalars);
      gridTemp3D->SetDebug(this->Debug);
      gridTemp3D->ComputeInputUpdateExtents(output);
      gridTemp3D->Delete();
      return 1;
      }
    }

  if ( !strcmp(inputObjectType, "vtkRectilinearGrid") )
    {
    int ext[6], dim=0;
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext);
    for(int j=0; j<3; j++)
      {
      if ( ( ext[2*j+1]-ext[2*j] ) != 0 )
        {
        dim++;
        }
      }
    if (dim == 3)
      {
      vtkRectilinearSynchronizedTemplates *rTemp =
        vtkRectilinearSynchronizedTemplates::New();
      rTemp->SetInput(vtkRectilinearGrid::SafeDownCast(input));
      rTemp->SetComputeNormals (this->ComputeNormals);
      rTemp->SetComputeGradients (this->ComputeGradients);
      rTemp->SetComputeScalars (this->ComputeScalars);
      rTemp->SetDebug(this->Debug);
      rTemp->RequestUpdateExtent(request, inputVector, outputVector);
      rTemp->Delete();
      return 1;
      }
    }
   
  return this->Superclass::RequestUpdateExtent(request, 
                                               inputVector, 
                                               outputVector);
}

//
// General contouring filter.  Handles arbitrary input.
//
int vtkKitwareContourFilter::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!input) {return 0;}

  vtkDataArray *inScalars;
  vtkIdType numCells;
  
  vtkDebugMacro(<< "Executing contour filter");

  numCells = input->GetNumberOfCells();
  inScalars = input->GetPointData()->GetScalars(this->InputScalarsSelection);
  if ( ! inScalars || numCells < 1 )
    {
    vtkDebugMacro(<<"No data to contour");
    return 1;
    }

  // If structured points and structured grid, use more efficient algorithms
  if ( input->GetDataObjectType() == VTK_STRUCTURED_POINTS || 
       input->GetDataObjectType() == VTK_IMAGE_DATA )
    {
    int dim = 3;
    int *uExt = input->GetUpdateExtent();
    if (uExt[0] == uExt[1])
      {
      --dim;
      }
    if (uExt[2] == uExt[3])
      {
      --dim;
      }
    if (uExt[4] == uExt[5])
      {
      --dim;
      }

    if ( dim >= 2 ) 
      {
      this->StructuredPointsContour(dim, input);
      return 1;
      }
    }

  if ( input->GetDataObjectType() == VTK_STRUCTURED_GRID )
    {
    int dim = 3;
    int *uExt = input->GetUpdateExtent();
    if (uExt[0] == uExt[1])
      {
      --dim;
      }
    if (uExt[2] == uExt[3])
      {
      --dim;
      }
    if (uExt[4] == uExt[5])
      {
      --dim;
      }

    // only do 3D structured grids (to be extended in the future)
    if ( dim == 3 ) 
      {
      this->StructuredGridContour(dim, input);
      return 1;
      }
    }

  if ( input->GetDataObjectType() == VTK_RECTILINEAR_GRID )
    {
    int dim = 3;
    int *uExt = input->GetUpdateExtent();
    if (uExt[0] == uExt[1])
      {
      --dim;
      }
    if (uExt[2] == uExt[3])
      {
      --dim;
      }
    if (uExt[4] == uExt[5])
      {
      --dim;
      }

    if ( dim == 3 ) 
      {
      this->RectilinearGridContour(dim, input);
      return 1;
      }
    }

  // otherwise just use the normal one
  return this->Superclass::RequestData(request, inputVector, outputVector);
}


//
// Special method handles structured points
//
void vtkKitwareContourFilter::StructuredPointsContour(int dim, vtkDataSet* input)
{
  vtkPolyData *output;
  vtkPolyData *thisOutput = this->GetOutput();
  int numContours=this->ContourValues->GetNumberOfContours();
  double *values=this->ContourValues->GetValues();

  if ( dim == 2 )
    {
    vtkSynchronizedTemplates2D *syncTemp2D;
    int i;
    
    syncTemp2D = vtkSynchronizedTemplates2D::New();
    syncTemp2D->SetInput(vtkImageData::SafeDownCast(input));
    syncTemp2D->SetDebug(this->Debug);
    syncTemp2D->SetNumberOfContours(numContours);
    syncTemp2D->SetArrayComponent(this->ArrayComponent);
    for (i=0; i < numContours; i++)
      {
      syncTemp2D->SetValue(i,values[i]);
      }
         
    syncTemp2D->GetOutput()->SetUpdateExtent(thisOutput->GetUpdatePiece(),
                                             thisOutput->GetUpdateNumberOfPieces(),
                                             thisOutput->GetUpdateGhostLevel());
    syncTemp2D->SelectInputScalars(this->InputScalarsSelection);
    syncTemp2D->Update();
    output = syncTemp2D->GetOutput();
    output->Register(this);
    syncTemp2D->Delete();
    }

  else 
    {
    vtkSynchronizedTemplates3D *syncTemp3D;
    int i;
    
    syncTemp3D = vtkSynchronizedTemplates3D::New();
    
    syncTemp3D->SetInput(vtkImageData::SafeDownCast(input));
    syncTemp3D->SetComputeNormals (this->ComputeNormals);
    syncTemp3D->SetComputeGradients (this->ComputeGradients);
    syncTemp3D->SetComputeScalars (this->ComputeScalars);
    syncTemp3D->SetDebug(this->Debug);
    syncTemp3D->SetNumberOfContours(numContours);
    syncTemp3D->SetArrayComponent(this->ArrayComponent);
    for (i=0; i < numContours; i++)
      {
      syncTemp3D->SetValue(i,values[i]);
      }

    syncTemp3D->GetOutput()->SetUpdateExtent(thisOutput->GetUpdatePiece(),
                                             thisOutput->GetUpdateNumberOfPieces(),
                                             thisOutput->GetUpdateGhostLevel());
    syncTemp3D->SelectInputScalars(this->InputScalarsSelection);
    syncTemp3D->Update();
    output = syncTemp3D->GetOutput();
    output->Register(this);
    syncTemp3D->Delete();
    }
  
  thisOutput->CopyStructure(output);
  thisOutput->GetPointData()->ShallowCopy(output->GetPointData());
  thisOutput->GetCellData()->ShallowCopy(output->GetCellData());
  output->UnRegister(this);
}
//
// Special method handles structured grids
//
void vtkKitwareContourFilter::StructuredGridContour(int dim, vtkDataSet* input)
{
  vtkPolyData *output = NULL;
  vtkPolyData *thisOutput = this->GetOutput();
  int numContours=this->ContourValues->GetNumberOfContours();
  double *values=this->ContourValues->GetValues();

  if ( dim == 3 )
    {
    vtkGridSynchronizedTemplates3D *gridTemp3D;
    int i;
    
    gridTemp3D = vtkGridSynchronizedTemplates3D::New();
    gridTemp3D->SetInput(vtkStructuredGrid::SafeDownCast(input));
    gridTemp3D->SetComputeNormals (this->ComputeNormals);
    gridTemp3D->SetComputeGradients (this->ComputeGradients);
    gridTemp3D->SetComputeScalars (this->ComputeScalars);
    gridTemp3D->SetDebug(this->Debug);
    gridTemp3D->SetNumberOfContours(numContours);
    for (i=0; i < numContours; i++)
      {
      gridTemp3D->SetValue(i,values[i]);
      }

    output = gridTemp3D->GetOutput();
    output->SetUpdateNumberOfPieces(thisOutput->GetUpdateNumberOfPieces());
    output->SetUpdatePiece(thisOutput->GetUpdatePiece());
    output->SetUpdateGhostLevel(thisOutput->GetUpdateGhostLevel());
    gridTemp3D->SelectInputScalars(this->InputScalarsSelection);
    gridTemp3D->Update();
    output->Register(this);
    gridTemp3D->Delete();
    }
  
  thisOutput->CopyStructure(output);
  thisOutput->GetPointData()->ShallowCopy(output->GetPointData());
  thisOutput->GetCellData()->ShallowCopy(output->GetCellData());
  output->UnRegister(this);
}

//
// Special method handles rectilinear grids
//
void vtkKitwareContourFilter::RectilinearGridContour(int dim, vtkDataSet* input)
{
  vtkPolyData *output = NULL;
  vtkPolyData *thisOutput = this->GetOutput();
  int numContours=this->ContourValues->GetNumberOfContours();
  double *values=this->ContourValues->GetValues();

  if ( dim == 3 )
    {
    vtkRectilinearSynchronizedTemplates *rTemp;
    int i;
    
    rTemp = vtkRectilinearSynchronizedTemplates::New();
    rTemp->SetInput(vtkRectilinearGrid::SafeDownCast(input));
    rTemp->SetComputeNormals (this->ComputeNormals);
    rTemp->SetComputeGradients (this->ComputeGradients);
    rTemp->SetComputeScalars (this->ComputeScalars);
    rTemp->SetDebug(this->Debug);
    rTemp->SetNumberOfContours(numContours);
    for (i=0; i < numContours; i++)
      {
      rTemp->SetValue(i,values[i]);
      }

    output = rTemp->GetOutput();
    output->SetUpdateNumberOfPieces(thisOutput->GetUpdateNumberOfPieces());
    output->SetUpdatePiece(thisOutput->GetUpdatePiece());
    output->SetUpdateGhostLevel(thisOutput->GetUpdateGhostLevel());
    rTemp->SelectInputScalars(this->InputScalarsSelection);
    rTemp->Update();
    output->Register(this);
    rTemp->Delete();
    }
  
  thisOutput->CopyStructure(output);
  thisOutput->GetPointData()->ShallowCopy(output->GetPointData());
  thisOutput->GetCellData()->ShallowCopy(output->GetCellData());
  output->UnRegister(this);
}

//----------------------------------------------------------------------------
void vtkKitwareContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ArrayComponent: " << this->ArrayComponent << endl;
}
