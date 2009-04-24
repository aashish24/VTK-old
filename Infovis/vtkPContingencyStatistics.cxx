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
#if defined(_MSC_VER)
#pragma warning (disable:4503)
#endif

#include "vtkToolkits.h"

#include "vtkPContingencyStatistics.h"
#include "vtkBivariateStatisticsAlgorithmPrivate.h"

#include "vtkCommunicator.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkMultiProcessController.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"

#include <vtkstd/map>
#include <vtkstd/set>
#include <vtkstd/vector>

vtkStandardNewMacro(vtkPContingencyStatistics);
vtkCxxRevisionMacro(vtkPContingencyStatistics, "$Revision$");
vtkCxxSetObjectMacro(vtkPContingencyStatistics, Controller, vtkMultiProcessController);
//-----------------------------------------------------------------------------
vtkPContingencyStatistics::vtkPContingencyStatistics()
{
  this->Controller = 0;
  this->SetController( vtkMultiProcessController::GetGlobalController() );
}

//-----------------------------------------------------------------------------
vtkPContingencyStatistics::~vtkPContingencyStatistics()
{
  this->SetController( 0 );
}

//-----------------------------------------------------------------------------
void vtkPContingencyStatistics::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Controller: " << this->Controller << endl;
}

//-----------------------------------------------------------------------------
void PackValues( const vtkstd::vector<vtkStdString>& values, 
                 vtkStdString& buffer )
{
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
    buffer.resize( 0 );
#else // defined(_MSC_VER) && (_MSC_VER <= 1200)
    buffer.clear();
#endif // defined(_MSC_VER) && (_MSC_VER <= 1200)

  for( vtkstd::vector<vtkStdString>::const_iterator it = values.begin();
       it != values.end(); ++ it )
    {
    buffer.append( *it );
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
    buffer.append( 1, 0 );
#else // defined(_MSC_VER) && (_MSC_VER <= 1200)
    buffer.push_back( 0 );
#endif // defined(_MSC_VER) && (_MSC_VER <= 1200)
    }
}

//-----------------------------------------------------------------------------
void UnpackValues( const vtkStdString& buffer,
                   vtkstd::vector<vtkStdString>& values )
{
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
    values.resize( 0 );
#else // defined(_MSC_VER) && (_MSC_VER <= 1200)
    values.clear();
#endif // defined(_MSC_VER) && (_MSC_VER <= 1200)

  const char* const bufferEnd = &buffer[0] + buffer.size();

  for( const char* start = &buffer[0]; start != bufferEnd; ++ start )
    {
    for( const char* finish = start; finish != bufferEnd; ++ finish )
      {
      if( ! *finish )
        {
        values.push_back( vtkStdString( start ) );
        start = finish;
        break;
        }
      }
    }
}

// ----------------------------------------------------------------------
void vtkPContingencyStatistics::ExecuteLearn( vtkTable* inData,
                                              vtkDataObject* outMetaDO )
{
  vtkMultiBlockDataSet* outMeta = vtkMultiBlockDataSet::SafeDownCast( outMetaDO );
  if ( ! outMeta )
    {
    return;
    }

  // First calculate contingency statistics on local data set
  this->Superclass::ExecuteLearn( inData, outMeta );

  // Get a hold of the contingency table
  vtkTable* contingencyTab = vtkTable::SafeDownCast( outMeta->GetBlock( 1 ) );
  if ( ! contingencyTab )
    {
    return;
    }

  vtkIdType nRowCont = contingencyTab->GetNumberOfRows();
  if ( nRowCont <= 0 )
    {
    // No statistics were calculated in serial.
    return;
    }

  // Make sure that parallel updates are needed, otherwise leave it at that.
  int np = this->Controller->GetNumberOfProcesses();
  if ( np < 2 )
    {
    return;
    }

  // Downcast meta columns to string arrays for efficient data access
  vtkIdTypeArray* keys = vtkIdTypeArray::SafeDownCast( contingencyTab->GetColumnByName( "Key" ) );
  vtkStringArray* valx = vtkStringArray::SafeDownCast( contingencyTab->GetColumnByName( "x" ) );
  vtkStringArray* valy = vtkStringArray::SafeDownCast( contingencyTab->GetColumnByName( "y" ) );
  vtkIdTypeArray* card = vtkIdTypeArray::SafeDownCast( contingencyTab->GetColumnByName( "Cardinality" ) );
  if ( ! keys || ! valx || ! valy || ! card )
    {
    return;
    }

  vtkstd::vector<vtkStdString> xyValues_l; // local consecutive xy pairs
  vtkstd::vector<vtkIdType>    kcValues_l; // local consecutive kc  pairs

  for ( vtkIdType r = 1; r < nRowCont; ++ r ) // Skip first row which is reserved for data set cardinality
    {
    // Push back x and y to list of strings
    xyValues_l.push_back( valx->GetValue( r ) );
    xyValues_l.push_back( valy->GetValue( r ) );

    // Push back (X,Y) index and #(x,y) to list of strings
    kcValues_l.push_back( keys->GetValue( r ) );
    kcValues_l.push_back( card->GetValue( r ) );
    }

  // Concatenate vector of strings into single string
  vtkStdString xyPacked_l;
  PackValues( xyValues_l, xyPacked_l );

  // Get ready for parallel calculations
  vtkCommunicator* com = this->Controller->GetCommunicator();

  // Use process 0 as sole reducer for now
  int reduceProc = 0; 

  // (All) gather all xy and kc sizes
  vtkIdType xySize_l = xyPacked_l.size();
  vtkIdType* xySize_g = new vtkIdType[np];
  com->AllGather( &xySize_l,
                  xySize_g,
                  1 );

  vtkIdType kcSize_l = kcValues_l.size();
  vtkIdType* kcSize_g = new vtkIdType[np];
  com->AllGather( &kcSize_l,
                  kcSize_g,
                  1 );

  // Calculate total size and displacement arrays
  vtkIdType* xyOffset = new vtkIdType[np];
  vtkIdType* kcOffset = new vtkIdType[np];

  vtkIdType xySizeTotal = 0;
  vtkIdType kcSizeTotal = 0;

  for ( vtkIdType i = 0; i < np; ++ i )
    {
    xyOffset[i] = xySizeTotal;
    kcOffset[i] = kcSizeTotal;

    xySizeTotal += xySize_g[i];
    kcSizeTotal += kcSize_g[i];
    }

  vtkIdType myRank = com->GetLocalProcessId();
  // Allocate receive buffers on reducer process, based on the global sizes obtained above
  char* xyPacked_g = 0;
  vtkIdType*  kcValues_g = 0;
  if ( myRank == reduceProc )
    {
    xyPacked_g = new char[xySizeTotal];
    kcValues_g = new vtkIdType[kcSizeTotal];
    }
  
  // Gather all xyPacked and kcValues on process reduceProc
  // NB: GatherV because the packets have variable lengths
  if ( ! com->GatherV( &(*xyPacked_l.begin()),
                       xyPacked_g,
                       xySize_l,
                       xySize_g,
                       xyOffset,
                       reduceProc ) )
    {
    vtkErrorMacro("Process "
                  << myRank
                  << "could not gather (x,y) values.");
    return;
    }
  
  if ( ! com->GatherV( &(*kcValues_l.begin()),
                       kcValues_g,
                       kcSize_l,
                       kcSize_g,
                       kcOffset,
                       reduceProc ) )
    {
    vtkErrorMacro("Process "
                  << myRank
                  << "could not gather (k,c) values.");
    return;
    }

  // Reduction step: have process reduceProc perform the reduction of the global contingency table
  if ( myRank == reduceProc )
    {
    this->Reduce( myRank,
                  xyPacked_g,
                  xySizeTotal,
                  xyValues_l,
                  xyPacked_l,
                  kcValues_g,
                  kcSizeTotal,
                  kcValues_l );
    } // if ( myRank == reduceProc )

  // Broadcast the xy and kc buffer sizes
  if ( ! com->Broadcast( &xySizeTotal,
                         1,
                         reduceProc ) )
    {
    vtkErrorMacro("Process "
                  << myRank
                  << "could not broadcast (x,y) buffer size.");
    return;
    }

  if ( ! com->Broadcast( &kcSizeTotal,
                         1,
                         reduceProc ) )
    {
    vtkErrorMacro("Process "
                  << myRank
                  << "could not broadcast (k,c) buffer size.");
    return;
    }
  
  // Resize vectors so they can receive the broadcasted xy and kc values
  xyPacked_l.resize( xySizeTotal );
  kcValues_l.resize( kcSizeTotal );

  // Broadcast the contents of contingency table to everyone
  if ( ! com->Broadcast( &(*xyPacked_l.begin()),
                         xySizeTotal,
                         reduceProc ) )
    {
    vtkErrorMacro("Process "
                  << myRank
                  << "could not broadcast (x,y) values.");
    return;
    }
  
  if ( ! com->Broadcast( &(*kcValues_l.begin()),
                         kcSizeTotal,
                         reduceProc ) )
    {
    vtkErrorMacro("Process "
                  << myRank
                  << "could not broadcast (k,c) values.");
    return;
    }
  
  // Unpack the packet of strings
  UnpackValues( xyPacked_l, xyValues_l );

  // Finally, fill the new, global contigency table (everyone does this so everyone ends up with the same model)
  vtkVariantArray* row4 = vtkVariantArray::New();
  row4->SetNumberOfValues( 4 );

  vtkstd::vector<vtkStdString>::iterator xyit = xyValues_l.begin();
  vtkstd::vector<vtkIdType>::iterator    kcit = kcValues_l.begin();

  // First replace existing rows
  // Start with row 1 and not 0 because of cardinality row (cf. superclass for a detailed explanation)
  for ( vtkIdType r = 1 ; r < nRowCont; ++ r, xyit += 2, kcit += 2 )
    {
    row4->SetValue( 0, *kcit );
    row4->SetValue( 1, *xyit );
    row4->SetValue( 2, *(xyit + 1) );
    row4->SetValue( 3, *(kcit + 1) );

    contingencyTab->SetRow( r, row4 );
    }

  // Then insert new rows
  for ( ; xyit != xyValues_l.end() ; xyit += 2, kcit += 2 )
    {
    row4->SetValue( 0, *kcit );
    row4->SetValue( 1, *xyit );
    row4->SetValue( 2, *(xyit + 1) );
    row4->SetValue( 3, *(kcit + 1) );

   contingencyTab->InsertNextRow( row4 );
   }
    
  // Clean up
  row4->Delete();

  if ( myRank == reduceProc )
    {
    delete [] xyPacked_g;
    delete [] kcValues_g;
    }

  delete [] xySize_g;
  delete [] kcSize_g;
  delete [] xyOffset;
  delete [] kcOffset;
}

// ----------------------------------------------------------------------
void vtkPContingencyStatistics::Reduce( vtkIdType myRank,
                                        char* xyPacked_g,
                                        vtkIdType& xySizeTotal,
                                        vtkstd::vector<vtkStdString>& xyValues_l,
                                        vtkStdString& xyPacked_l,
                                        vtkIdType*  kcValues_g,
                                        vtkIdType& kcSizeTotal,
                                        vtkstd::vector<vtkIdType>& kcValues_l )
{
  // First, unpack the packet of strings
  vtkstd::vector<vtkStdString> xyValues_g; 
  UnpackValues( vtkStdString ( xyPacked_g, xySizeTotal ), xyValues_g );
  
  // Second, check consistency: we must have the same number of xy and kc entries
  if ( vtkIdType( xyValues_g.size() ) != kcSizeTotal )
    {
    vtkErrorMacro("Process "
                  << myRank
                  << " collected inconsistent number of (x,y) and (k,c) pairs: "
                  << xyValues_g.size()
                  << " <> "
                  << kcSizeTotal
                  << ".");
    return;
    }

  // Third, reduce to the global contingency table
  typedef vtkstd::map<vtkStdString,vtkIdType> Distribution;
  typedef vtkstd::map<vtkStdString,Distribution> Bidistribution;
  vtkstd::map<vtkIdType,Bidistribution> contingencyTable;

  vtkIdType i = 0;
  for ( vtkstd::vector<vtkStdString>::iterator vit = xyValues_g.begin(); vit != xyValues_g.end(); vit += 2, i += 2 )
    {
    contingencyTable
      [kcValues_g[i]]
      [*vit]
      [*(vit + 1)] 
      += kcValues_g[i + 1];
    }
    
  // Fourth, prepare send buffers of (global) xy and kc values
  xyValues_l.clear();
  kcValues_l.clear();
  for ( vtkstd::map<vtkIdType,Bidistribution>::iterator ait = contingencyTable.begin();
        ait != contingencyTable.end(); ++ ait )
    {
    Bidistribution bidi = ait->second;
    for ( Bidistribution::iterator bit = bidi.begin();
          bit != bidi.end(); ++ bit )
      {
      Distribution di = bit->second;
      for ( Distribution::iterator dit = di.begin();
            dit != di.end(); ++ dit )
        {
        // Push back x and y to list of strings
        xyValues_l.push_back( bit->first );  // x
        xyValues_l.push_back( dit->first );  // y
          
        // Push back (X,Y) index and #(x,y) to list of strings
        kcValues_l.push_back( ait->first );  // k
        kcValues_l.push_back( dit->second ); // c
        }
      }
    }
  PackValues( xyValues_l, xyPacked_l );
    
  // Last, update xy and kc buffer sizes (which have changed because of the reduction)
  xySizeTotal = xyPacked_l.size();
  kcSizeTotal = kcValues_l.size();
}


