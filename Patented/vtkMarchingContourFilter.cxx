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
#include "vtkMarchingContourFilter.h"

#include "vtkCell.h"
#include "vtkContourFilter.h"
#include "vtkContourValues.h"
#include "vtkImageMarchingCubes.h"
#include "vtkMarchingCubes.h"
#include "vtkMarchingSquares.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkScalarTree.h"
#include "vtkStructuredPoints.h"

#include <math.h>

vtkCxxRevisionMacro(vtkMarchingContourFilter, "$Revision$");
vtkStandardNewMacro(vtkMarchingContourFilter);

// Construct object with initial range (0,1) and single contour value
// of 0.0.
vtkMarchingContourFilter::vtkMarchingContourFilter()
{
  this->ContourValues = vtkContourValues::New();

  this->ComputeNormals = 1;
  this->ComputeGradients = 0;
  this->ComputeScalars = 1;

  this->Locator = NULL;

  this->UseScalarTree = 0;
  this->ScalarTree = NULL;
}

vtkMarchingContourFilter::~vtkMarchingContourFilter()
{
  this->ContourValues->Delete();
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  if ( this->ScalarTree )
    {
    this->ScalarTree->Delete();
    }
}

// Overload standard modified time function. If contour values are modified,
// then this object is modified as well.
unsigned long vtkMarchingContourFilter::GetMTime()
{
  unsigned long mTime=this->vtkDataSetToPolyDataFilter::GetMTime();
  unsigned long time;

  if (this->ContourValues)
    {
    time = this->ContourValues->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }
  if (this->Locator)
    {
    time = this->Locator->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

//
// General contouring filter.  Handles arbitrary input.
//
void vtkMarchingContourFilter::Execute()
{
  vtkDataArray *inScalars;
  vtkDataSet *input=this->GetInput();
  vtkIdType numCells;
  
  vtkDebugMacro(<< "Executing marching contour filter");

  if (input == NULL)
    {
      vtkErrorMacro(<<"Input is NULL");
      return;
    }

  numCells = input->GetNumberOfCells();
  inScalars = input->GetPointData()->GetScalars();
  if ( ! inScalars || numCells < 1 )
    {
    vtkErrorMacro(<<"No data to contour");
    return;
    }

  // If structured points, use more efficient algorithms
  if ( (input->GetDataObjectType() == VTK_STRUCTURED_POINTS))
    {
    if (inScalars->GetDataType() != VTK_BIT)
      {
      int dim = input->GetCell(0)->GetCellDimension();
      
      if ( input->GetCell(0)->GetCellDimension() >= 2 ) 
        {
        vtkDebugMacro(<< "Structured Points");
        this->StructuredPointsContour(dim);
        return;
        }
      }
    }
  
  if ( (input->GetDataObjectType() == VTK_IMAGE_DATA)) 
    {
    if (inScalars->GetDataType() != VTK_BIT)
      {
      int dim = input->GetCell(0)->GetCellDimension();
      
      if ( input->GetCell(0)->GetCellDimension() >= 2 ) 
        {
        vtkDebugMacro(<< "Image");
        this->ImageContour(dim);
        return;
        }
      }
    }
  
  vtkDebugMacro(<< "Unoptimized");
  this->DataSetContour();
}

void vtkMarchingContourFilter::StructuredPointsContour(int dim)
{
  vtkPolyData *output;
  vtkPolyData *thisOutput = this->GetOutput();
  int numContours=this->ContourValues->GetNumberOfContours();
  float *values=this->ContourValues->GetValues();

  if ( dim == 2 ) //marching squares
    {
    vtkMarchingSquares *msquares;
    int i;
    
    msquares = vtkMarchingSquares::New();
    msquares->SetInput((vtkImageData *)this->GetInput());
    msquares->SetDebug(this->Debug);
    msquares->SetNumberOfContours(numContours);
    for (i=0; i < numContours; i++)
      {
      msquares->SetValue(i,values[i]);
      }
         
    msquares->Update();
    output = msquares->GetOutput();
    output->Register(this);
    msquares->Delete();
    }

  else //marching cubes
    {
    vtkMarchingCubes *mcubes;
    int i;
    
    mcubes = vtkMarchingCubes::New();
    mcubes->SetInput((vtkImageData *)this->GetInput());
    mcubes->SetComputeNormals (this->ComputeNormals);
    mcubes->SetComputeGradients (this->ComputeGradients);
    mcubes->SetComputeScalars (this->ComputeScalars);
    mcubes->SetDebug(this->Debug);
    mcubes->SetNumberOfContours(numContours);
    for (i=0; i < numContours; i++)
      {
      mcubes->SetValue(i,values[i]);
      }

    mcubes->Update();
    output = mcubes->GetOutput();
    output->Register(this);
    mcubes->Delete();
    }
  
  thisOutput->CopyStructure(output);
  thisOutput->GetPointData()->ShallowCopy(output->GetPointData());
  output->UnRegister(this);
}

void vtkMarchingContourFilter::DataSetContour()
{
  vtkPolyData *output = this->GetOutput();
  int numContours=this->ContourValues->GetNumberOfContours();
  float *values=this->ContourValues->GetValues();

  vtkContourFilter *contour = vtkContourFilter::New();
  contour->SetInput((vtkImageData *)this->GetInput());
  contour->SetOutput(output);
  contour->SetComputeNormals (this->ComputeNormals);
  contour->SetComputeGradients (this->ComputeGradients);
  contour->SetComputeScalars (this->ComputeScalars);
  contour->SetDebug(this->Debug);
  contour->SetNumberOfContours(numContours);
  for (int i=0; i < numContours; i++)
    {
    contour->SetValue(i,values[i]);
    }

  contour->Update();
  this->SetOutput(output);
  contour->Delete();
}

void vtkMarchingContourFilter::ImageContour(int dim)
{
  vtkPolyData *output = this->GetOutput();
  int numContours=this->ContourValues->GetNumberOfContours();
  float *values=this->ContourValues->GetValues();

  if ( dim == 2 ) //marching squares
    {
    vtkMarchingSquares *msquares;
    int i;
    
    msquares = vtkMarchingSquares::New();
    msquares->SetInput((vtkImageData *)this->GetInput());
    msquares->SetOutput(output);
    msquares->SetDebug(this->Debug);
    msquares->SetNumberOfContours(numContours);
    for (i=0; i < numContours; i++)
      {
      msquares->SetValue(i,values[i]);
      }
         
    msquares->Update();
    this->SetOutput(output);
    msquares->Delete();
    }

  else //image marching cubes
    {
    vtkImageMarchingCubes *mcubes;
    int i;
    
    mcubes = vtkImageMarchingCubes::New();
    mcubes->SetInput((vtkImageData *)this->GetInput());
    mcubes->SetOutput(output);
    mcubes->SetComputeNormals (this->ComputeNormals);
    mcubes->SetComputeGradients (this->ComputeGradients);
    mcubes->SetComputeScalars (this->ComputeScalars);
    mcubes->SetDebug(this->Debug);
    mcubes->SetNumberOfContours(numContours);
    for (i=0; i < numContours; i++)
      {
      mcubes->SetValue(i,values[i]);
      }

    mcubes->Update();
    this->SetOutput(output);
    mcubes->Delete();
    }
}

// Specify a spatial locator for merging points. By default, 
// an instance of vtkMergePoints is used.
void vtkMarchingContourFilter::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator == locator ) 
    {
    return;
    }
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  if ( locator )
    {
    locator->Register(this);
    }
  this->Locator = locator;
  this->Modified();
}

void vtkMarchingContourFilter::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    }
}

void vtkMarchingContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Compute Gradients: " << (this->ComputeGradients ? "On\n" : "Off\n");
  os << indent << "Compute Normals: " << (this->ComputeNormals ? "On\n" : "Off\n");
  os << indent << "Compute Scalars: " << (this->ComputeScalars ? "On\n" : "Off\n");
  os << indent << "Use Scalar Tree: " << (this->UseScalarTree ? "On\n" : "Off\n");

  this->ContourValues->PrintSelf(os,indent);

  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }
}
