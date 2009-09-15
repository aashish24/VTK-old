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
#include "vtkInterpolatedVelocityField.h"

#include "vtkDataSet.h"
#include "vtkGenericCell.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkCxxRevisionMacro( vtkInterpolatedVelocityField, "$Revision$" );
vtkStandardNewMacro( vtkInterpolatedVelocityField ); 

//----------------------------------------------------------------------------
vtkInterpolatedVelocityField::vtkInterpolatedVelocityField()
{
  this->NumFuncs     = 3; // u, v, w
  this->NumIndepVars = 4; // x, y, z, t
  this->Weights      = 0;
  this->WeightsSize  = 0;
 
  this->Caching    = true; // Caching on by default
  this->CacheHit   = 0;
  this->CacheMiss  = 0;
  
  this->LastCellId = -1;
  this->LastDataSet= 0;
  this->LastDataSetIndex = 0;
  
  this->VectorsSelection = 0;
  this->NormalizeVector  = false;

  this->Cell     = vtkGenericCell::New(); // referenced in the parent class!
  this->GenCell  = vtkGenericCell::New();
  this->DataSets = new vtkAbstractInterpolatedVelocityFieldDataSetsType;
}
//----------------------------------------------------------------------------
vtkInterpolatedVelocityField::~vtkInterpolatedVelocityField()
{
  this->NumFuncs     = 0;
  this->NumIndepVars = 0;
  delete[] this->Weights;
  this->Weights      = 0;
  this->LastDataSet  = 0;
  
  this->Cell->Delete();                   // referenced in the parent class!
  this->GenCell->Delete();
  this->SetVectorsSelection( 0 );

  // Ungister datasets from this velocity field interpolator
  for ( DataSetsTypeBase::iterator dsIt  = this->DataSets->begin(); 
        dsIt != this->DataSets->end(); dsIt ++ )
    {
    if ( *dsIt )
      {
      ( *dsIt )->UnRegister( this );
      }
    ( *dsIt ) = NULL;
    }
  delete this->DataSets;
  this->DataSets = NULL;
}

//----------------------------------------------------------------------------
void vtkInterpolatedVelocityField::AddDataSet( vtkDataSet * dataset )
{
  if ( !dataset )
    {
    return;
    }

  this->DataSets->push_back( dataset );
  dataset->Register( this ); // register this dataset

  int size = dataset->GetMaxCellSize();
  if ( size > this->WeightsSize )
    {
    this->WeightsSize = size;
    delete[] this->Weights;
    this->Weights = new double[size]; 
    }
}

//----------------------------------------------------------------------------
void vtkInterpolatedVelocityField::SetLastCellId( vtkIdType c, int dataindex )
{
  this->LastCellId  = c; 
  this->LastDataSet = ( *this->DataSets )[dataindex];
  
  // if the dataset changes, then the cached cell is invalidated
  // we might as well prefetch the cached cell either way
  if ( this->LastCellId != -1 )
  {
    this->LastDataSet->GetCell( this->LastCellId, this->GenCell );
  } 
  
  this->LastDataSetIndex = dataindex;
}

//----------------------------------------------------------------------------
int vtkInterpolatedVelocityField::FunctionValues( double * x, double * f )
{
  vtkDataSet * ds;
  if(!this->LastDataSet && !this->DataSets->empty())
    {
    ds = ( *this->DataSets )[0];
    this->LastDataSet      = ds;
    this->LastDataSetIndex = 0;
    }
  else
    {
    ds = this->LastDataSet;
    }
    
  int retVal = this->FunctionValues( ds, x, f );
  
  if ( !retVal )
    {
    for( this->LastDataSetIndex = 0; 
         this->LastDataSetIndex < static_cast<int>( this->DataSets->size() );
         this->LastDataSetIndex ++ )
      {
      ds = this->DataSets->operator[]( this->LastDataSetIndex );
      if( ds && ds != this->LastDataSet )
        {
        this->ClearLastCellId();
        retVal = this->FunctionValues( ds, x, f );
        if ( retVal ) 
          {
          this->LastDataSet = ds;
          return retVal;
          }
        }
      }
    this->LastCellId  = -1;
    this->LastDataSetIndex = 0;
    this->LastDataSet = (*this->DataSets)[0];
    return 0;
    }
    
  return retVal;
}

//----------------------------------------------------------------------------
void vtkInterpolatedVelocityField::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}
