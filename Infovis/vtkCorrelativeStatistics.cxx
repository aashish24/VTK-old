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
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkToolkits.h"

#include "vtkCorrelativeStatistics.h"

#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"

#include <vtkstd/set>

// = Start Private Implementation =======================================
class vtkCorrelativeStatisticsPrivate
{
public:
  vtkCorrelativeStatisticsPrivate();
  ~vtkCorrelativeStatisticsPrivate();

  vtkstd::set<vtkstd::pair<vtkIdType,vtkIdType> > ColumnPairs;
};

vtkCorrelativeStatisticsPrivate::vtkCorrelativeStatisticsPrivate()
{
}

vtkCorrelativeStatisticsPrivate::~vtkCorrelativeStatisticsPrivate()
{
}

// = End Private Implementation =========================================

vtkCxxRevisionMacro(vtkCorrelativeStatistics, "$Revision$");
vtkStandardNewMacro(vtkCorrelativeStatistics);

// ----------------------------------------------------------------------
vtkCorrelativeStatistics::vtkCorrelativeStatistics()
{
  this->Internals = new vtkCorrelativeStatisticsPrivate;
}

// ----------------------------------------------------------------------
vtkCorrelativeStatistics::~vtkCorrelativeStatistics()
{
  delete this->Internals;
}

// ----------------------------------------------------------------------
void vtkCorrelativeStatistics::PrintSelf( ostream &os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

// ----------------------------------------------------------------------
void vtkCorrelativeStatistics::ResetColumnPairs()
{
  this->Internals->ColumnPairs.clear();
  this->Modified();
}

// ----------------------------------------------------------------------
void vtkCorrelativeStatistics::AddColumnPair( vtkIdType idxColX, vtkIdType idxColY )
{
  vtkstd::pair<vtkIdType,vtkIdType> idxPair( idxColX, idxColY );
  this->Internals->ColumnPairs.insert( idxPair );
  this->Modified();
}

// ----------------------------------------------------------------------
void vtkCorrelativeStatistics::ExecuteLearn( vtkTable* dataset,
                                             vtkTable* output,
                                             bool finalize )
{
  vtkIdType nCol = dataset->GetNumberOfColumns();
  if ( ! nCol )
    {
    vtkWarningMacro( "Dataset table does not have any columns. Doing nothing." );
    this->SampleSize = 0;
    return;
    }

  this->SampleSize = dataset->GetNumberOfRows();
  if ( ! this->SampleSize )
    {
    vtkWarningMacro( "Dataset table does not have any rows. Doing nothing." );
    return;
    }

  vtkStringArray* stringCol = vtkStringArray::New();
  stringCol->SetName( "Column X" );
  output->AddColumn( stringCol );
  stringCol->Delete();

  stringCol = vtkStringArray::New();
  stringCol->SetName( "Column Y" );
  output->AddColumn( stringCol );
  stringCol->Delete();

  if ( finalize )
    {
    vtkDoubleArray* doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Mean X" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Mean Y" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Var(X)" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Var(Y)" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Cov(X,Y)" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    stringCol = vtkStringArray::New();
    stringCol->SetName( "Linear Correlation" );
    output->AddColumn( stringCol );
    stringCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Slope Y/X" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Intersect Y/X" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Slope X/Y" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Intersect X/Y" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Corr. Coeff." );
    output->AddColumn( doubleCol );
    doubleCol->Delete();
    }
  else
    {
    vtkDoubleArray* doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Sum x" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Sum y" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Sum x2" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Sum y2" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    doubleCol = vtkDoubleArray::New();
    doubleCol->SetName( "Sum xy" );
    output->AddColumn( doubleCol );
    doubleCol->Delete();

    vtkIdTypeArray* idTypeCol = vtkIdTypeArray::New();
    idTypeCol->SetName( "Cardinality" );
    output->AddColumn( idTypeCol );
    idTypeCol->Delete();
    }


  for ( vtkstd::set<vtkstd::pair<vtkIdType,vtkIdType> >::iterator it = this->Internals->ColumnPairs.begin(); 
        it != this->Internals->ColumnPairs.end(); ++ it )
    {
    vtkIdType idX = it->first;
    if ( idX >= nCol )
      {
      vtkWarningMacro( "Dataset table does not have a column with index "<<idX<<". Ignoring this pair." );
      continue;
      }

    vtkIdType idY = it->second;
    if ( idY >= nCol )
      {
      vtkWarningMacro( "Dataset table does not have a column with index "<<idY<<". Ignoring this pair." );
      continue;
      }
    
    double x   = 0.;
    double y   = 0.;
    double sx  = 0.;
    double sy  = 0.;
    double sx2 = 0.;
    double sy2 = 0.;
    double sxy = 0.;
    for ( vtkIdType r = 0; r < this->SampleSize; ++ r )
      {
      x = dataset->GetValue( r, idX ).ToDouble();
      y = dataset->GetValue( r, idY ).ToDouble();
      
      sx  += x;
      sy  += y;
      sx2 += x * x;
      sy2 += y * y;
      sxy += x * y;
      }

    vtkVariantArray* row = vtkVariantArray::New();

    if ( finalize )
      {
      row->SetNumberOfValues( 13 );

      double correlations[5];
      int res = this->CalculateFromSums( this->SampleSize, sx, sy, sx2, sy2, sxy, correlations );
      
      row->SetValue( 0, dataset->GetColumnName( idX ) );
      row->SetValue( 1, dataset->GetColumnName( idY ) );
      row->SetValue( 2, sx );
      row->SetValue( 3, sy );
      row->SetValue( 4, sx2 );
      row->SetValue( 5, sy2 );
      row->SetValue( 6, sxy );
      row->SetValue( 7, ( res ? "invalid" : "valid" ) );
      for ( int i = 0; i < 5; ++ i )
        {
        row->SetValue( i + 8, correlations[i] );
        }
      }
    else 
      {
      row->SetNumberOfValues( 8 );

      row->SetValue( 0, dataset->GetColumnName( idX ) );
      row->SetValue( 1, dataset->GetColumnName( idY ) );
      row->SetValue( 2, sx );
      row->SetValue( 3, sy );
      row->SetValue( 4, sx2 );
      row->SetValue( 5, sy2 );
      row->SetValue( 6, sxy );
      row->SetValue( 7, this->SampleSize );
      }

    output->InsertNextRow( row );

    row->Delete();
    }
    
  return;
}

// ----------------------------------------------------------------------
void vtkCorrelativeStatistics::ExecuteValidate( vtkTable*,
                                                vtkTable*,
                                                vtkTable* )
{
  // Not implemented for this statistical engine
}

// ----------------------------------------------------------------------
void vtkCorrelativeStatistics::ExecuteEvince( vtkTable* dataset,
                                              vtkTable* params,
                                              vtkTable* output)
{
  vtkIdType nColD = dataset->GetNumberOfColumns();
  if ( ! nColD )
    {
    vtkWarningMacro( "Dataset table does not have any columns. Doing nothing." );
    return;
    }

  vtkIdType nRowD = dataset->GetNumberOfRows();
  if ( ! nRowD )
    {
    vtkWarningMacro( "Dataset table does not have any rows. Doing nothing." );
    return;
    }

  vtkIdType nColP = params->GetNumberOfColumns();
  if ( nColP != 8 )
    {
    vtkWarningMacro( "Parameter table has " 
                     << nColP
                     << " != 8 columns. Doing nothing." );
    return;
    }

  vtkIdType nRowP = params->GetNumberOfRows();
  if ( ! nRowP )
    {
    vtkWarningMacro( "Parameter table does not have any rows. Doing nothing." );
    return;
    }

  vtkIdTypeArray* idTypeCol = vtkIdTypeArray::New();
  idTypeCol->SetName( "Column X" );
  output->AddColumn( idTypeCol );
  idTypeCol->Delete();

  idTypeCol = vtkIdTypeArray::New();
  idTypeCol->SetName( "Column Y" );
  output->AddColumn( idTypeCol );
  idTypeCol->Delete();

  idTypeCol = vtkIdTypeArray::New();
  idTypeCol->SetName( "Row" );
  output->AddColumn( idTypeCol );
  idTypeCol->Delete();

  vtkDoubleArray* doubleCol = vtkDoubleArray::New();
  doubleCol->SetName( "Relative PDF" );
  output->AddColumn( doubleCol );
  doubleCol->Delete();

  vtkVariantArray* row = vtkVariantArray::New();
  row->SetNumberOfValues( 4 );
  
  for ( vtkstd::set<vtkstd::pair<vtkIdType,vtkIdType> >::iterator it = this->Internals->ColumnPairs.begin(); 
        it != this->Internals->ColumnPairs.end(); ++ it )
    {
    vtkIdType idX = it->first;
    if ( idX >= nColD )
      {
      vtkWarningMacro( "Dataset table does not have a column with index "<<idX<<". Ignoring it." );
      continue;
      }

    vtkIdType idY = it->second;
    if ( idY >= nColD )
      {
      vtkWarningMacro( "Dataset table does not have a column with index "<<idY<<". Ignoring it." );
      continue;
      }

    bool unfound = true;
    for ( int i = 0; i < nRowP; ++ i )
      {
      vtkIdType c1 = params->GetValue( i, 0 ).ToInt();
      vtkIdType c2 = params->GetValue( i, 1 ).ToInt();
      if ( ( c1 == it->first && c2 == it->second ) ||  ( c2 == it->first && c1 == it->second ) )
        {
        unfound = false;

        double varX = params->GetValue( i, 4 ).ToDouble();
        double varY = params->GetValue( i, 5 ).ToDouble();
        double covr = params->GetValue( i, 6 ).ToDouble();
        
        double d = varX * varY - covr * covr;
        if ( d <= 0. )
          {
          vtkWarningMacro( "Incorrect parameters for column pair ("
                           <<c1
                           <<", "
                           <<c2
                           <<"): variance/covariance matrix has non-positive determinant." );
          continue;
          }
        
        double nominalX = params->GetValue( i, 2 ).ToDouble();
        double nominalY = params->GetValue( i, 3 ).ToDouble();
        double thresPDF = params->GetValue( i, 7 ).ToDouble();
        double eFac = -.5 / d;
        covr *= 2.;
        
        if ( c2 == it->first )
          {
          vtkIdType tmp = idX;
          idX = idY;
          idY = tmp;
          }

        double x, y, rPDF;
        for ( vtkIdType r = 0; r < nRowD; ++ r )
          {
          x = dataset->GetValue( r, idX ).ToDouble() - nominalX;
          y = dataset->GetValue( r, idY ).ToDouble() - nominalY;
          
          rPDF = exp( eFac * ( varY * x * x - covr * x * y + varX * y * y ) );
          if ( rPDF < thresPDF )
            {
            row->SetValue( 0, idX );
            row->SetValue( 1, idY );
            row->SetValue( 2, r );
            row->SetValue( 3, rPDF );

            output->InsertNextRow( row );
            }
          }

        break;
        }
      }

    if ( unfound )
      {
      vtkWarningMacro( "Parameter table does not a row for dataset table column pair ("
                       <<it->first
                       <<", "
                       <<it->second
                       <<"). Ignoring it." );
      continue;
      }
    }
  row->Delete();

  return;
}

// ----------------------------------------------------------------------
int vtkCorrelativeStatistics::CalculateFromSums( int n,
                                                 double& sx,
                                                 double& sy,
                                                 double& sx2,
                                                 double& sy2,
                                                 double& sxy,
                                                 double* correlations )
{
  if ( n < 2 ) 
    {
    return -1;
    }

  double nd = static_cast<double>( n );

  // (unbiased) estimation of the means
  sx /= nd;
  sy /= nd;

  if ( n == 1 )
    {
    sx2 = sy2 = sxy = 0.;
    return 0;
    }

  // (unbiased) estimation of the variances and covariance
  double f = 1. / ( nd - 1. );
  sx2 = ( sx2 - sx * sx * nd ) * f;
  sy2 = ( sy2 - sy * sy * nd ) * f;
  sxy = ( sxy - sx * sy * nd ) * f;

  // linear regression

  if ( sx2 > 0. && sy2 > 0. )
    {
    // variable Y on variable X:
    //   slope
    correlations[0] = sxy / sx2;
    //   intersect
    correlations[1] = sy - correlations[0] * sx;

    //   variable X on variable Y:
    //   slope
    correlations[2] = sxy / sy2;
    //   intersect
    correlations[3] = sx - correlations[2] * sy;

    // correlation coefficient
    double d = sx2 * sy2;
    if ( d > 0 )
      {
      correlations[4] = sxy / sqrt( d );
      return 0;
      }
    else
      {
      correlations[4] = 0.;
      return 1;
      }
    }
  else
    {
    correlations[0] = 0.;
    correlations[1] = 0.;
    correlations[2] = 0.;
    correlations[3] = 0.;
    correlations[4] = 0.;
    return 1;
    }
}
