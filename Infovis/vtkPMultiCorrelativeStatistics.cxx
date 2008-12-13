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
#include "vtkToolkits.h"

#include "vtkPMultiCorrelativeStatistics.h"

#include "vtkCommunicator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkMultiProcessController.h"
#include "vtkTable.h"
#include "vtkVariant.h"

#include <vtkstd/map>

vtkStandardNewMacro(vtkPMultiCorrelativeStatistics);
vtkCxxRevisionMacro(vtkPMultiCorrelativeStatistics, "$Revision$");
vtkCxxSetObjectMacro(vtkPMultiCorrelativeStatistics, Controller, vtkMultiProcessController);
//-----------------------------------------------------------------------------
vtkPMultiCorrelativeStatistics::vtkPMultiCorrelativeStatistics()
{
  this->Controller = 0;
  this->SetController( vtkMultiProcessController::GetGlobalController() );
}

//-----------------------------------------------------------------------------
vtkPMultiCorrelativeStatistics::~vtkPMultiCorrelativeStatistics()
{
  this->SetController( 0 );
}

//-----------------------------------------------------------------------------
void vtkPMultiCorrelativeStatistics::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Controller: " << this->Controller << endl;
}

// ----------------------------------------------------------------------
void vtkPMultiCorrelativeStatistics::ExecuteLearn( vtkTable* inData,
                                                   vtkDataObject* outMetaDO )
{
  vtkMultiBlockDataSet* outMeta = vtkMultiBlockDataSet::SafeDownCast( outMetaDO );
  if ( ! outMeta )
    {
    return;
    }

  // First calculate correlative statistics on local data set
  this->Superclass::ExecuteLearn( inData, outMeta );

  // Get a hold of the (sparse) covariance matrix
  vtkTable* sparseCov = vtkTable::SafeDownCast( outMeta->GetBlock( 0 ) );
  if ( ! sparseCov )
    {
    return;
    }
  
  vtkIdType nRow = sparseCov->GetNumberOfRows();
  if ( ! nRow )
    {
    // No statistics were calculated.
    return;
    }

  // Make sure that parallel updates are needed, otherwise leave it at that.
  int np = this->Controller->GetNumberOfProcesses();
  if ( np < 2 )
    {
    return;
    }

  // Now get ready for parallel calculations
  vtkCommunicator* com = this->Controller->GetCommunicator();
  
  // (All) gather all sample sizes
  int n_l = sparseCov->GetValueByName( 0, "Entries" ).ToInt(); // Sample Size
  int* n_g = new int[np];
  com->AllGather( &n_l, n_g, 1 ); 
  
  // Iterate over all mean and MXY entries
  // NB: two passes are required as there is no guarantee that all means
  //     are stored before MXYs
  int nM = nRow - 1;
  double* M_l = new double[nM];

  // First, load all means and create a name-to-index lookup table
  vtkstd::map<vtkStdString, vtkIdType> meanIndex;
  for ( vtkIdType r = 1; r < nRow; ++ r )
    {
    if ( sparseCov->GetValueByName( r, "Column2" ).ToString() == "" )
      {
      meanIndex[sparseCov->GetValueByName( r, "Column1" ).ToString()] = r - 1;

      M_l[r - 1] = sparseCov->GetValueByName( r, "Entries" ).ToDouble();
      }
    }
  vtkIdType nMeans = meanIndex.size();

  // Second, load all MXYs and create an index-to-index-pair lookup table
  vtkstd::map<vtkIdType, vtkstd::pair<vtkIdType, vtkIdType> > covToMeans;
  for ( vtkIdType r = 1; r < nRow; ++ r )
    {
    vtkStdString col2 = sparseCov->GetValueByName( r, "Column2" ).ToString();
    if ( col2  != "" )
      {
      covToMeans[r - 1] = vtkstd::pair<vtkIdType, vtkIdType> 
        ( meanIndex[sparseCov->GetValueByName( r, "Column1" ).ToString()], 
          meanIndex[col2] );          

      M_l[r - 1] = sparseCov->GetValueByName( r, "Entries" ).ToDouble();
      }
    }

  // (All) gather all local means and MXY statistics
  double* M_g = new double[nM * np];
  com->AllGather( M_l, M_g, nM );

  // Aggregate all local nM-tuples of M statistics into global ones
  int ns = n_g[0];
  for ( int i = 0; i < nM; ++ i )
    {
    M_l[i] = M_g[i];
    }

  for ( int i = 1; i < np; ++ i )
    {
    int ns_l = n_g[i];
    ns += ns_l;
    int prod_ns = ns * ns_l;
    
    double* M_part = new double[nM];
    double* delta  = new double[nMeans];
    double* delta_sur_n  = new double[nMeans];
    int o = nM * i;

    // First, calculate deltas for all means
    for ( int j = 0; j < nMeans; ++ j )
      {
      M_part[j] = M_g[o + j];

      delta[j] = M_part[j] - M_l[j];
      delta_sur_n[j] = delta[j] / static_cast<double>( ns );
      }

    // Then, update covariances
    for ( int j = nMeans; j < nM; ++ j )
      {
      M_part[j] = M_g[o + j];
    
      M_l[j] += M_part[j]
        + prod_ns * delta[covToMeans[j].first] * delta_sur_n[covToMeans[j].second];
      }

    // Last, update means
    for ( int j = 0; j < nMeans; ++ j )
      {
      M_l[j] += ns_l * delta_sur_n[j];
      }

    // Clean-up
    delete [] M_part;
    delete [] delta;
    delete [] delta_sur_n;
    }

  for ( int i = 0; i < nM; ++ i )
    {
    sparseCov->SetValueByName( i + 1, "Entries", M_l[i] );
    }

  // Set global statistics
  sparseCov->SetValueByName( 0, "Entries", ns );
  this->SampleSize = ns;

  // Clean-up
  delete [] M_l;
  delete [] M_g;
  delete [] n_g;
}
