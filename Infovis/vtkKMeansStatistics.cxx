#include "vtkKMeansStatistics.h"
#include "vtkStringArray.h"
#include "vtkKMeansAssessFunctor.h"
#include "vtkKMeansDistanceFunctor.h"

#include "vtkObjectFactory.h"
#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkStatisticsAlgorithmPrivate.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"
#include "vtkIntArray.h"
#include "vtkIdTypeArray.h"

#include <vtkstd/map>
#include <vtkstd/vector>
#include <vtksys/ios/sstream>

vtkCxxRevisionMacro(vtkKMeansStatistics,"$Revision$");
vtkStandardNewMacro(vtkKMeansStatistics);

// ----------------------------------------------------------------------
vtkKMeansStatistics::vtkKMeansStatistics()
{
  this->AssessNames->SetNumberOfValues( 2 );
  this->AssessNames->SetValue( 0, "distance" ); 
  this->AssessNames->SetValue( 1, "closest id" ); 
  this->DefaultNumberOfClusters = 3;
  this->Tolerance = 0.01;                  
  this->KValuesArrayName = 0;
  this->SetKValuesArrayName( "K" );
  this->MaxNumIterations = 50;
  this->DistanceFunctor = 0;
}

// ----------------------------------------------------------------------
vtkKMeansStatistics::~vtkKMeansStatistics()
{
  this->SetKValuesArrayName( 0 );
  if( this->DistanceFunctor ) 
    {
      delete this->DistanceFunctor;
    }
}

// ----------------------------------------------------------------------
void vtkKMeansStatistics::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
  os << indent << "DefaultNumberofClusters: " 
               << this->DefaultNumberOfClusters << endl;
  os << indent << "KValuesArrayName: \"" 
               << ( this->KValuesArrayName ? this->KValuesArrayName : "NULL" ) 
               << "\"\n";
  os << indent << "MaxNumIterations: " << this->MaxNumIterations << endl;
  os << indent << "Tolerance: " << this->Tolerance << endl;
  os << indent << "DistanceFunctor: " << this->DistanceFunctor << endl;
}


// ----------------------------------------------------------------------
int vtkKMeansStatistics::FillOutputPortInformation( int port, vtkInformation* info )
{
  int stat;
  if ( port == vtkStatisticsAlgorithm::OUTPUT_MODEL )
    {
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet" );
    stat = 1;
    }
  else 
    {
    stat = this->Superclass::FillOutputPortInformation( port, info );
    }
  return stat;
}

// ----------------------------------------------------------------------
int vtkKMeansStatistics::FillInputPortInformation( int port, vtkInformation* info )
{
  int stat;
  if ( port == INPUT_MODEL )
    {
    info->Set( vtkAlgorithm::INPUT_IS_OPTIONAL(), 1 );
    info->Set( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet" );
    stat = 1;
    }
  else 
    {
    stat = this->Superclass::FillInputPortInformation( port, info );
    }
  return stat;
}

// ----------------------------------------------------------------------
int vtkKMeansStatistics::InitializeDataAndClusterCenters(vtkTable* inParameters,
                                                         vtkTable* inData,
                                                         vtkTable*  dataElements,
                                                         vtkIdTypeArray*  numberOfClusters,
                                                         vtkTable*  curClusterElements,
                                                         vtkTable*  newClusterElements,
                                                         vtkIdTypeArray*  startRunID,
                                                         vtkIdTypeArray*  endRunID)
{
  vtkstd::set<vtkstd::set<vtkStdString> >::iterator reqIt;
  if( this->Internals->Requests.size() > 1 ) 
    {
    static int num=0;
    num++;
    if( num < 10 )
      {
      vtkWarningMacro( "Only the first request will be processed -- the rest will be ignored." );
      }
    }

  if( this->Internals->Requests.size() == 0 ) 
    {
    vtkErrorMacro( "No requests were made." );
    return 0;
    }
  reqIt = this->Internals->Requests.begin(); 

  vtkIdType numToAllocate;
  vtkIdType numRuns=0;
  // process parameter input table
  if ( inParameters && inParameters->GetNumberOfRows() > 0 && 
       inParameters->GetNumberOfColumns() > 1 )
    {
    numToAllocate = inParameters->GetNumberOfRows();
    cout << "InitDataAndClusterCenters: START numberOfClusters setting number of values " << numToAllocate << endl;
    numberOfClusters->SetNumberOfValues( numToAllocate );
    cout << "InitDataAndClusterCenters: END numberOfClusters setting number of values " << numToAllocate << endl;
    numberOfClusters->SetName( inParameters->GetColumn( 0 )->GetName() );
    vtkIdTypeArray* counts = vtkIdTypeArray::SafeDownCast( inParameters->GetColumn( 0 ) );

    for ( vtkIdType i=0; i < numToAllocate; i++ ) 
      {
      numberOfClusters->SetValue( i, counts->GetValue( i ) );
      }
    vtkIdType curRow = 0;
    while ( curRow < inParameters->GetNumberOfRows() )
      {
      numRuns++;
      cout << "InitDataAndClusterCenters: START startRunID  insertingNextValue" << curRow << endl;
      startRunID->InsertNextValue( curRow );
      cout << "InitDataAndClusterCenters: END startRunID  insertingNextValue" << curRow << endl;
      curRow += inParameters->GetValue( curRow, 0 ).ToInt();
      cout << "InitDataAndClusterCenters: START endRunID  insertingNextValue" << curRow << endl;
      endRunID->InsertNextValue( curRow );
      cout << "InitDataAndClusterCenters: END endRunID  insertingNextValue" << curRow << endl;
      }
    vtkTable* condensedTable = vtkTable::New();
    vtkstd::set<vtkStdString>::iterator colItr;
    for ( colItr = reqIt->begin(); colItr != reqIt->end(); ++ colItr )
      {
      vtkAbstractArray* pArr = inParameters->GetColumnByName( colItr->c_str() );
      vtkAbstractArray* dArr = inData->GetColumnByName( colItr->c_str() );
      if( pArr && dArr )
        {
        condensedTable->AddColumn( pArr );
        dataElements->AddColumn( dArr );
        } 
      else 
        {
        vtkWarningMacro( "Skipping requested column \"" << colItr->c_str() << "\"." );
        }
      }
    cout << "InitDataAndClusterCenters: START newClusterElements DeepCopy " << endl;
    newClusterElements->DeepCopy( condensedTable );
    cout << "InitDataAndClusterCenters: END newClusterElements DeepCopy " << endl;
    cout << "InitDataAndClusterCenters: START curClusterElements DeepCopy " << endl;
    curClusterElements->DeepCopy( condensedTable );
    cout << "InitDataAndClusterCenters: END curClusterElements DeepCopy " << endl;
    condensedTable->Delete();

    }
  else
    {
    // otherwise create an initial set of cluster coords
    numRuns = 1;
    numToAllocate = this->DefaultNumberOfClusters < inData->GetNumberOfRows() ? 
                    this->DefaultNumberOfClusters : inData->GetNumberOfRows();
    cout << "InitDataAndClusterCenters: START startRunID  insertingNextValue" << 0  << endl;
    startRunID->InsertNextValue( 0 );
    cout << "InitDataAndClusterCenters: END startRunID  insertingNextValue" << 0  << endl;
    cout << "InitDataAndClusterCenters: START endRunID  insertingNextValue" << numToAllocate  << endl;
    endRunID->InsertNextValue( numToAllocate );
    cout << "InitDataAndClusterCenters: END endRunID  insertingNextValue" << numToAllocate  << endl;
    numberOfClusters->SetName( this->KValuesArrayName );

    for ( vtkIdType j = 0; j < inData->GetNumberOfColumns(); j++ )
      {
      if(reqIt->find( inData->GetColumnName( j ) ) != reqIt->end() ) 
        {
        vtkVariantArray* curCoords = vtkVariantArray::New();
        vtkVariantArray* newCoords = vtkVariantArray::New();
        curCoords->SetName( inData->GetColumnName( j ) );
        newCoords->SetName( inData->GetColumnName( j ) );
        curClusterElements->AddColumn( curCoords );
        newClusterElements->AddColumn( newCoords );
        curCoords->Delete();
        newCoords->Delete();
        dataElements->AddColumn( inData->GetColumnByName( inData->GetColumnName( j ) ) );
        }
      }
    for ( vtkIdType i = 0; i < numToAllocate; i++ )
      {
      cout << "InitDataAndClusterCenters: START numberOfClusters  insertingNextValue" << numToAllocate  << endl;
      numberOfClusters->InsertNextValue( numToAllocate );
      cout << "InitDataAndClusterCenters: END numberOfClusters  insertingNextValue" << numToAllocate  << endl;
      vtkVariantArray *curRow = vtkVariantArray::New();
      vtkVariantArray *newRow = vtkVariantArray::New();
      for ( int j = 0; j < inData->GetNumberOfColumns(); j++ )
        {
        if(reqIt->find( inData->GetColumnName( j ) ) != reqIt->end()  )
          {
          cout << "InitDataAndClusterCenters: START curRow  insertingNextValue" << endl;
          curRow->InsertNextValue( inData->GetValue( i, j ) );
          cout << "InitDataAndClusterCenters: END curRow  insertingNextValue" << endl;
          cout << "InitDataAndClusterCenters: START nextRow  insertingNextValue" << endl;
          newRow->InsertNextValue( inData->GetValue( i, j ) );
          cout << "InitDataAndClusterCenters: END nextRow  insertingNextValue" << endl;
          }
        }
      cout << "InitDataAndClusterCenters: START curClusterELements insertingNextRow" << endl;
      curClusterElements->InsertNextRow( curRow );
      cout << "InitDataAndClusterCenters: END curClusterELements insertingNextRow" << endl;
      cout << "InitDataAndClusterCenters: START newClusterELements insertingNextRow" << endl;
      newClusterElements->InsertNextRow( newRow );
      cout << "InitDataAndClusterCenters: END newClusterELements insertingNextRow" << endl;
      curRow->Delete();
      newRow->Delete();
      }
    }
    if(curClusterElements->GetNumberOfColumns() == 0 )
      {
      return 0;
      }
    return numRuns;
}

// ----------------------------------------------------------------------
vtkIdType vtkKMeansStatistics::GetTotalNumberOfObservations( vtkIdType numObservations )
{
  return numObservations;
}

// ----------------------------------------------------------------------
void vtkKMeansStatistics::UpdateClusterCenters( vtkTable* newClusterElements, 
                                                vtkTable* curClusterElements, 
                                                vtkIdTypeArray* vtkNotUsed( numMembershipChanges ), 
                                                vtkIdTypeArray* numDataElementsInCluster, 
                                                vtkDoubleArray* vtkNotUsed( error ), 
                                                vtkIdTypeArray* startRunID, 
                                                vtkIdTypeArray* endRunID, 
                                                vtkIntArray* computeRun ) 
{
  for(vtkIdType runID = 0; runID < startRunID->GetNumberOfTuples(); runID++) 
    {
    if( computeRun->GetValue(runID) )
      {
      for(vtkIdType i = startRunID->GetValue(runID); i < endRunID->GetValue(runID); i++) 
        {
        if( numDataElementsInCluster->GetValue( i ) == 0 )
          {
          vtkWarningMacro("cluster center " << i-startRunID->GetValue(runID) 
                                            << " in run " << runID 
                                            << " is degenerate. Attempting to perturb");
          this->DistanceFunctor->PerturbElement(newClusterElements, 
                                                curClusterElements, 
                                                i, 
                                                startRunID->GetValue(runID), 
                                                endRunID->GetValue(runID),
                                                0.8 ) ;
          }
        }
      }
    }
}

// ----------------------------------------------------------------------
void vtkKMeansStatistics::ExecuteLearn( vtkTable* inData, 
                                        vtkTable* inParameters,
                                        vtkDataObject* outMetaDO )
{
  vtkMultiBlockDataSet* outMeta = vtkMultiBlockDataSet::SafeDownCast( outMetaDO );
  if ( !outMeta )
    {
    return;
    }
  vtkIdType numObservations = inData->GetNumberOfRows();
  if ( numObservations <= 0 )
    {
    return;
    }
  vtkIdType totalNumberOfObservations = this->GetTotalNumberOfObservations( numObservations );

  if ( inData->GetNumberOfColumns() <= 0 )
    {
    return;
    }

  if ( !this->DistanceFunctor )
    {
    this->DistanceFunctor = vtkKMeansDefaultDistanceFunctor::New();
    }

  // Data initialization
  vtkIdTypeArray* numberOfClusters = vtkIdTypeArray::New();
  vtkTable* curClusterElements = vtkTable::New();
  vtkTable* newClusterElements = vtkTable::New();
  vtkIdTypeArray* startRunID = vtkIdTypeArray::New();
  vtkIdTypeArray* endRunID = vtkIdTypeArray::New();
  vtkTable* dataElements = vtkTable::New();
  int numRuns = InitializeDataAndClusterCenters(inParameters,
                                                inData,
                                                dataElements,
                                                numberOfClusters,
                                                curClusterElements,
                                                newClusterElements,
                                                startRunID,
                                                endRunID);
  if( numRuns == 0 )
    {
    numberOfClusters->Delete();
    curClusterElements->Delete();
    newClusterElements->Delete();
    startRunID->Delete();
    endRunID->Delete();
    dataElements->Delete();
    return;
    }
                                 
  vtkIdType numToAllocate = curClusterElements->GetNumberOfRows();
  vtkIdTypeArray* numIterations = vtkIdTypeArray::New();
  vtkIdTypeArray* numDataElementsInCluster = vtkIdTypeArray::New();
  vtkDoubleArray* error = vtkDoubleArray::New();
  vtkIdTypeArray* clusterMemberID = vtkIdTypeArray::New();
  vtkIdTypeArray* numMembershipChanges = vtkIdTypeArray::New();
  vtkIntArray* computeRun = vtkIntArray::New();
  vtkIdTypeArray* clusterRunIDs = vtkIdTypeArray::New(); 

  cout << "ExecuteLearn: START numDataElementsInCluster SetNumberOfValues " << numToAllocate << endl;
  numDataElementsInCluster->SetNumberOfValues( numToAllocate );
  cout << "ExecuteLearn: END numDataElementsInCluster SetNumberOfValues " << numToAllocate << endl;
  numDataElementsInCluster->SetName( "Num Elements" );
  cout << "ExecuteLearn: START clusterRunIDs SetNumberOfValues " << numToAllocate << endl;
  clusterRunIDs->SetNumberOfValues( numToAllocate );
  cout << "ExecuteLearn: END clusterRunIDs SetNumberOfValues " << numToAllocate << endl;
  clusterRunIDs->SetName( "Run ID" );
  cout << "ExecuteLearn: START error SetNumberOfValues " << numToAllocate << endl;
  error->SetNumberOfValues( numToAllocate );
  cout << "ExecuteLearn: END error SetNumberOfValues " << numToAllocate << endl;
  error->SetName( "Error" );
  cout << "ExecuteLearn: START numIterations SetNumberOfValues " << numToAllocate << endl;
  numIterations->SetNumberOfValues( numToAllocate );
  cout << "ExecuteLearn: END numIterations SetNumberOfValues " << numToAllocate << endl;
  numIterations->SetName( "Iterations" );
  cout << "ExecuteLearn: START numMembershipChanges SetNumberOfValues " << numRuns << endl;
  numMembershipChanges->SetNumberOfValues( numRuns );
  cout << "ExecuteLearn: END numMembershipChanges SetNumberOfValues " << numRuns << endl;
  cout << "ExecuteLearn: START computeRun SetNumberOfValues " << numRuns << endl;
  computeRun->SetNumberOfValues( numRuns );
  cout << "ExecuteLearn: END computeRun SetNumberOfValues " << numRuns << endl;
  cout << "ExecuteLearn: START clusterMemberID SetNumberOfValues " << numObservations << "*" << numRuns << " = " << numObservations*numRuns << endl;
  clusterMemberID->SetNumberOfValues( numObservations*numRuns );
  cout << "ExecuteLearn: END clusterMemberID SetNumberOfValues " << numObservations << "*" << numRuns << " = " << numObservations*numRuns << endl;
  clusterMemberID->SetName( "cluster member id" );

  for ( int i = 0; i < numRuns; i++ )
    {
    for ( vtkIdType j = startRunID->GetValue(i); j < endRunID->GetValue(i); j++ )
      {
      clusterRunIDs->SetValue( j, i );
      }
    }

  numIterations->FillComponent( 0, 0 );
  computeRun->FillComponent( 0, 1 ); 
  int allConverged, numIter=0;    
  clusterMemberID->FillComponent( 0, -1 );


  // Iterate until new cluster centers have converged OR we have reached a max number of iterations
  do 
    {
    // Initialize coordinates, cluster sizes and errors
    numMembershipChanges->FillComponent( 0, 0 );
    for( int runID = 0; runID < numRuns; runID++)
      {
      if(computeRun->GetValue( runID ))
        {
        for( vtkIdType j = startRunID->GetValue(runID); j < endRunID->GetValue(runID); j++ )
          {
          curClusterElements->SetRow( j, newClusterElements->GetRow( j ) );
          newClusterElements->SetRow( j, 
                   this->DistanceFunctor->GetEmptyTuple( newClusterElements->GetNumberOfColumns() ) );
          numDataElementsInCluster->SetValue( j, 0 );
          error->SetValue( j, 0.0 );
          }
        }
      }

    // find minimum distance between each data object and cluster center
    vtkIdType localMemberID, offsetLocalMemberID;
    double minDistance, curDistance;
    for ( vtkIdType observation=0; observation < dataElements->GetNumberOfRows(); observation++ )
      {
      for( int runID = 0; runID < numRuns; runID++)
        {
        if(computeRun->GetValue( runID ))
          {
          vtkIdType runStartIdx = startRunID->GetValue( runID );
          vtkIdType runEndIdx = endRunID->GetValue( runID );
          if ( runStartIdx >= runEndIdx )
            {
            continue;
            }
          vtkIdType j = runStartIdx;
          localMemberID = 0;
          offsetLocalMemberID = runStartIdx;
          (*this->DistanceFunctor)( minDistance, 
                                    curClusterElements->GetRow( j ), 
                                    dataElements->GetRow( observation ) );
          for( /* no init */; j < runEndIdx; j++ )
            {
            (*this->DistanceFunctor)( curDistance, 
                                      curClusterElements->GetRow( j ), 
                                      dataElements->GetRow( observation ) );
            if( curDistance < minDistance )
              {
              minDistance = curDistance;
              localMemberID = j-runStartIdx;
              offsetLocalMemberID = j;
              }
            }
            if ( clusterMemberID->GetValue( observation*numRuns+runID) != localMemberID )
              {
              numMembershipChanges->SetValue( runID, numMembershipChanges->GetValue( runID ) + 1 );
              clusterMemberID->SetValue( observation*numRuns+runID, localMemberID );
              }
            // change this to online update
            vtkIdType newCardinality = numDataElementsInCluster->GetValue( offsetLocalMemberID ) + 1;
            numDataElementsInCluster->SetValue( offsetLocalMemberID, newCardinality );
            this->DistanceFunctor->PairwiseUpdate( newClusterElements, offsetLocalMemberID,
                                     dataElements->GetRow( observation ), 1, newCardinality );
            error->SetValue( offsetLocalMemberID, error->GetValue( offsetLocalMemberID ) + minDistance );
            }
         }
      }
    // update cluster centers
    this->UpdateClusterCenters( newClusterElements, curClusterElements, numMembershipChanges, 
                                numDataElementsInCluster, error, startRunID, endRunID, computeRun );

    // check for convergence 
    numIter++ ;
    allConverged = 0;

    for( int j = 0; j < numRuns; j++ ) 
      {
      if( computeRun->GetValue( j ) ) 
        { 
        double percentChanged = static_cast<double>( numMembershipChanges->GetValue( j ) )/
                                static_cast<double>( totalNumberOfObservations ) ;
        if( percentChanged < this->Tolerance || numIter == this->MaxNumIterations) 
          {
          allConverged++;
          computeRun->SetValue( j, 0 );
          for(int k = startRunID->GetValue( j ); k < endRunID->GetValue( j ); k++)
            {
            numIterations->SetValue( k, numIter );
            }
          }
        }
      else 
        {
        allConverged++;
        }
      }
    }
  while ( allConverged < numRuns && numIter  < this->MaxNumIterations );


  // add columns to output table
  vtkTable* outputTable = vtkTable::New(); 
  outputTable->AddColumn(clusterRunIDs);
  outputTable->AddColumn(numberOfClusters);
  outputTable->AddColumn(numIterations);
  outputTable->AddColumn(error);
  outputTable->AddColumn(numDataElementsInCluster);
  for( vtkIdType i = 0; i < newClusterElements->GetNumberOfColumns(); i++ )
    {
      outputTable->AddColumn( newClusterElements->GetColumn( i ) );
    }

  outMeta->SetNumberOfBlocks( 1 );
  outMeta->SetBlock( 0, outputTable );
  outMeta->GetMetaData( static_cast<unsigned>( 0 ) )->Set( vtkCompositeDataSet::NAME(), 
                                                           "Updated Cluster Centers" );

  clusterRunIDs->Delete();
  numberOfClusters->Delete();
  numDataElementsInCluster->Delete();
  numIterations->Delete();
  error->Delete();
  curClusterElements->Delete();
  newClusterElements->Delete();
  dataElements->Delete();
  clusterMemberID->Delete();
  outputTable->Delete();
  startRunID->Delete();
  endRunID->Delete();
  computeRun->Delete();
  numMembershipChanges->Delete();
}

// ----------------------------------------------------------------------
void vtkKMeansStatistics::ExecuteDerive( vtkDataObject* outMetaDO )
{
  vtkMultiBlockDataSet* outMeta = vtkMultiBlockDataSet::SafeDownCast( outMetaDO );
  vtkTable* outTable;
  vtkIdTypeArray* clusterRunIDs;
  vtkIdTypeArray* numIterations;
  vtkIdTypeArray* numberOfClusters;
  vtkDoubleArray* error;
  
  if (
    ! outMeta || outMeta->GetNumberOfBlocks() < 1 ||
    ! ( outTable = vtkTable::SafeDownCast( outMeta->GetBlock( 0 ) ) ) ||
    ! ( clusterRunIDs = vtkIdTypeArray::SafeDownCast( outTable->GetColumn( 0 ) ) ) ||
    ! ( numberOfClusters = vtkIdTypeArray::SafeDownCast( outTable->GetColumn( 1 ) ) ) ||
    ! ( numIterations = vtkIdTypeArray::SafeDownCast( outTable->GetColumn( 2 ) ) ) ||
    ! ( error = vtkDoubleArray::SafeDownCast( outTable->GetColumn( 3 ) ) ) 
    )
    {
    return;
    }

  // Create an output table 
  // outMeta and which is presumed to exist upon entry to ExecuteDerive).

  cout << "ExecuteDerive: START outMeta SetNumberOfBlocks " << 2 << endl;
  outMeta->SetNumberOfBlocks( 2 );
  cout << "ExecuteDerive: END outMeta SetNumberOfBlocks " << 2 << endl;

  vtkIdTypeArray* totalClusterRunIDs = vtkIdTypeArray::New();
  vtkIdTypeArray* totalNumberOfClusters = vtkIdTypeArray::New();
  vtkIdTypeArray* totalNumIterations = vtkIdTypeArray::New();
  vtkIdTypeArray* globalRank = vtkIdTypeArray::New();
  vtkIdTypeArray* localRank = vtkIdTypeArray::New();
  vtkDoubleArray* totalError = vtkDoubleArray::New();

  totalClusterRunIDs->SetName( clusterRunIDs->GetName() );
  totalNumberOfClusters->SetName( numberOfClusters->GetName() );
  totalNumIterations->SetName( numIterations->GetName() );
  totalError->SetName( "Total Error" );
  globalRank->SetName( "Global Rank" );
  localRank->SetName( "Local Rank" );

  vtkstd::multimap<double, vtkIdType> globalErrorMap;
  vtkstd::map<vtkIdType, vtkstd::multimap<double, vtkIdType> > localErrorMap;

  vtkIdType curRow = 0;
  while ( curRow < outTable->GetNumberOfRows() )
    {
    cout << "ExecuteDerive: START totalClusterRunIDs InsertNextValue " << 2 << endl;
    totalClusterRunIDs->InsertNextValue( clusterRunIDs->GetValue( curRow ) );
    cout << "ExecuteDerive: END totalClusterRunIDs InsertNextValue " << endl;
    cout << "ExecuteDerive: START totalNumIterations InsertNextValue " << endl;
    totalNumIterations->InsertNextValue( numIterations->GetValue( curRow ) );
    cout << "ExecuteDerive: END totalNumIterations InsertNextValue " << endl;
    cout << "ExecuteDerive: START totalNumberOfClusters InsertNextValue " << endl;
    totalNumberOfClusters->InsertNextValue( numberOfClusters->GetValue( curRow ) );
    cout << "ExecuteDerive: END totalNumberOfClusters InsertNextValue " << endl;
    double totalErr = 0.0;
    for(vtkIdType i=curRow; i < curRow+numberOfClusters->GetValue( curRow ); i++ )
      {
      totalErr+= error->GetValue( i );
      }
    cout << "ExecuteDerive: START totalError InsertNextValue " << endl;
    totalError->InsertNextValue( totalErr );
    cout << "ExecuteDerive: END totalError InsertNextValue " << endl;
    globalErrorMap.insert(vtkstd::multimap<double, vtkIdType>::value_type( totalErr, 
                                                                     clusterRunIDs->GetValue( curRow ) ) );
    localErrorMap[numberOfClusters->GetValue( curRow )].insert(
                vtkstd::multimap<double, vtkIdType>::value_type( totalErr, clusterRunIDs->GetValue( curRow ) ) );
    curRow += numberOfClusters->GetValue( curRow );
    }
 
  cout << "ExecuteDerive: START globalRank SetNumberOfValues " << totalClusterRunIDs->GetNumberOfTuples() << endl;
  globalRank->SetNumberOfValues( totalClusterRunIDs->GetNumberOfTuples() );
  cout << "ExecuteDerive: END globalRank SetNumberOfValues " << totalClusterRunIDs->GetNumberOfTuples() << endl;
  cout << "ExecuteDerive: START localRank SetNumberOfValues " << totalClusterRunIDs->GetNumberOfTuples() << endl;
  localRank->SetNumberOfValues( totalClusterRunIDs->GetNumberOfTuples() );
  cout << "ExecuteDerive: END localRank SetNumberOfValues " << totalClusterRunIDs->GetNumberOfTuples() << endl;
  int rankID=1;

  for( vtkstd::multimap<double, vtkIdType>::iterator itr = globalErrorMap.begin(); itr != globalErrorMap.end(); itr++ )
    {
    globalRank->SetValue( itr->second, rankID++ ) ;
    }
  for( vtkstd::map<vtkIdType, vtkstd::multimap<double, vtkIdType> >::iterator itr = localErrorMap.begin(); itr != localErrorMap.end(); itr++ )
    {
    rankID=1;
    for( vtkstd::multimap<double, vtkIdType>::iterator rItr = itr->second.begin(); rItr != itr->second.end(); rItr++ )
      {
      localRank->SetValue( rItr->second, rankID++ ) ;
      }
    }

  vtkTable* ranked = vtkTable::New();
  outMeta->SetBlock( 1, ranked );
  outMeta->GetMetaData( static_cast<unsigned>( 1 ) )->Set( vtkCompositeDataSet::NAME(), 
                                                           "Ranked Cluster Centers" );
  ranked->Delete(); // outMeta now owns ranked
  ranked->AddColumn( totalClusterRunIDs );
  ranked->AddColumn( totalNumberOfClusters );
  ranked->AddColumn( totalNumIterations );
  ranked->AddColumn( totalError );
  ranked->AddColumn( localRank );
  ranked->AddColumn( globalRank );

  totalError->Delete();
  localRank->Delete();
  globalRank->Delete();
  totalClusterRunIDs->Delete();
  totalNumberOfClusters->Delete();
  totalNumIterations->Delete();
}

// ----------------------------------------------------------------------
void vtkKMeansStatistics::ExecuteAssess( vtkTable* inData, 
                                         vtkDataObject* inMetaDO, 
                                         vtkTable* outData, 
                                         vtkDataObject* vtkNotUsed(outMetaDO) )
{
  vtkMultiBlockDataSet* inMeta = vtkMultiBlockDataSet::SafeDownCast( inMetaDO );
  if ( ! inMeta || ! outData )
    {
    return;
    }

  if ( inData->GetNumberOfColumns() <= 0 )
    {
    return;
    }

  vtkIdType nsamples = inData->GetNumberOfRows();
  if ( nsamples <= 0 )
    {
    return;
    }

  // Add a column to the output data related to the each input datum wrt the model in the request.
  // Column names of the metadata and input data are assumed to match (no mapping using 
  // AssessNames or AssessParameters is done).
  // The output columns will be named "this->AssessNames->GetValue(0)(A,B,C)" where 
  // "A", "B", and "C" are the column names specified in the per-request metadata tables.
  AssessFunctor* dfunc = 0;
  // only one request allowed in when learning, so there will only be one 
  vtkTable* reqModel = vtkTable::SafeDownCast( inMeta->GetBlock( 0 ) );
  if ( ! reqModel )
    { 
    // silently skip invalid entries. Note we leave assessValues column in output data even when it's empty.
    return;
    }

  this->SelectAssessFunctor( inData, 
                             reqModel, 
                             0, 
                             dfunc );
  vtkKMeansAssessFunctor* kmfunc = static_cast<vtkKMeansAssessFunctor*>( dfunc );
  if ( ! kmfunc )
    {
    vtkWarningMacro( "Assessment could not be accommodated. Skipping." );
    if ( dfunc )
      {
      delete dfunc;
      }
    return;
    }

  vtkIdType nv = this->AssessNames->GetNumberOfValues();
  int numRuns = kmfunc->GetNumberOfRuns();
  vtkStdString* names = new vtkStdString[nv*numRuns];
  for ( int i = 0; i < numRuns; ++ i )
    {
    for ( vtkIdType v = 0; v < nv; ++ v )
      {
      vtksys_ios::ostringstream assessColName;
      assessColName << this->AssessNames->GetValue( v )
                    << " ("
                    <<  i 
                    << ")";

       vtkVariantArray* assessValues = vtkVariantArray::New();
       names[i*nv+v] = assessColName.str().c_str(); // Storing names to be able to use SetValueByName which is faster than SetValue
       assessValues->SetName( names[i*nv+v] );
       assessValues->SetNumberOfTuples( nsamples );
       outData->AddColumn( assessValues );
       assessValues->Delete();
      }
    }
       
  // Assess each entry of the column
  vtkVariantArray* assessResult = vtkVariantArray::New();
  for ( vtkIdType r = 0; r < nsamples; ++ r )
    {
    (*dfunc)( assessResult, r );
    for ( vtkIdType j = 0; j < nv*numRuns; ++ j )
      {
      outData->SetValueByName( r, names[j], assessResult->GetValue( j ) );
      }
    }
  assessResult->Delete();

  delete dfunc;
  delete [] names;
}

// ----------------------------------------------------------------------
void vtkKMeansStatistics::SelectAssessFunctor( vtkTable* inData,
                                               vtkDataObject* inMetaDO,
                                               vtkStringArray* vtkNotUsed(rowNames),
                                               AssessFunctor*& dfunc )
{
  (void)inData;

  dfunc = 0;
  vtkTable* reqModel = vtkTable::SafeDownCast( inMetaDO );
  if ( ! reqModel )
    {
    return;
    }
  
  vtkKMeansAssessFunctor* kmfunc = vtkKMeansAssessFunctor::New();

  if( !this->DistanceFunctor )
    {
      this->DistanceFunctor = vtkKMeansDefaultDistanceFunctor::New();
    }

  if ( ! kmfunc->Initialize( inData, reqModel, this->DistanceFunctor ) )
    {
    delete kmfunc;
    }
  dfunc = kmfunc;

}

// ----------------------------------------------------------------------
vtkKMeansAssessFunctor* vtkKMeansAssessFunctor::New()
{
  return new vtkKMeansAssessFunctor;
}

// ----------------------------------------------------------------------
vtkKMeansAssessFunctor::~vtkKMeansAssessFunctor()
{
  this->ClusterMemberIDs->Delete();
  this->Distances->Delete();
}

// ----------------------------------------------------------------------
bool vtkKMeansAssessFunctor::Initialize( vtkTable* inData, vtkTable* inModel, vtkKMeansDistanceFunctor* dfunc )
{
  vtkIdType numObservations = inData->GetNumberOfRows();
  vtkTable* dataElements = vtkTable::New();
  vtkTable* curClusterElements = vtkTable::New();
  vtkIdTypeArray* startRunID = vtkIdTypeArray::New();
  vtkIdTypeArray* endRunID = vtkIdTypeArray::New();

  this->Distances = vtkDoubleArray::New();
  this->ClusterMemberIDs = vtkIdTypeArray::New();
  this->NumRuns = 0;

  // cluster coordinates start in column 5 of the inModel table
  for ( vtkIdType i=5; i < inModel->GetNumberOfColumns(); i++ )
    {
    curClusterElements->AddColumn( inModel->GetColumn( i ) );
    dataElements->AddColumn( inData->GetColumnByName( inModel->GetColumnName( i ) ) );
    }

  vtkIdType curRow = 0;
  while ( curRow < inModel->GetNumberOfRows() )
    {
    this->NumRuns++;
    cout << "Assess::Initialize: START startRunID InsertNextValue " << endl;
    startRunID->InsertNextValue( curRow );
    cout << "Assess::Initialize: END startRunID InsertNextValue " << endl;
    // number of clusters "K" is stored in column 1 of the inModel table 
    curRow += inModel->GetValue( curRow, 1 ).ToInt();
    cout << "Assess::Initialize: START endRunID InsertNextValue " << endl;
    endRunID->InsertNextValue( curRow );
    cout << "Assess::Initialize: END endRunID InsertNextValue " << endl;
    }

  cout << "Assess::Initialize: START Distances SetNumberOfValues " << numObservations << "*" << this->NumRuns << "=" << numObservations*this->NumRuns << endl;
  this->Distances->SetNumberOfValues( numObservations * this->NumRuns );
  cout << "Assess::Initialize: END Distances SetNumberOfValues " << numObservations << "*" << this->NumRuns << "=" << numObservations*this->NumRuns << endl;

  cout << "Assess::Initialize: START ClusterMemberIDs SetNumberOfValues " << numObservations << "*" << this->NumRuns << "=" << numObservations*this->NumRuns << endl;
  this->ClusterMemberIDs->SetNumberOfValues( numObservations * this->NumRuns );
  cout << "Assess::Initialize: END ClusterMemberIDs SetNumberOfValues " << numObservations << "*" << this->NumRuns << "=" << numObservations*this->NumRuns << endl;

  // find minimum distance between each data object and cluster center
  for ( vtkIdType observation = 0; observation < numObservations; ++ observation )
    {
    for( int runID = 0; runID < this->NumRuns; ++ runID )
      {
      vtkIdType runStartIdx = startRunID->GetValue( runID );
      vtkIdType runEndIdx = endRunID->GetValue( runID );
      if ( runStartIdx >= runEndIdx )
        {
        continue;
        }
      // Find the closest cluster center to the observation across all centers in the runID-th run.
      vtkIdType j = runStartIdx;
      double minDistance;
      double curDistance;
      (*dfunc)( minDistance, curClusterElements->GetRow( j ), dataElements->GetRow( observation ) );
      vtkIdType localMemberID = 0;
      for( /* no init */; j < runEndIdx; ++ j )
        {
        (*dfunc)( curDistance, curClusterElements->GetRow( j ), dataElements->GetRow( observation ) );
        if ( curDistance < minDistance )
          {
          minDistance = curDistance;
          localMemberID = j - runStartIdx;
          }
        }
      this->ClusterMemberIDs->SetValue( observation * this->NumRuns + runID, localMemberID );
      this->Distances->SetValue( observation * this->NumRuns + runID, minDistance );
      }
   }

  dataElements->Delete();
  curClusterElements->Delete();
  startRunID->Delete();
  endRunID->Delete();
  return true;
}

// ----------------------------------------------------------------------
void vtkKMeansAssessFunctor::operator () ( vtkVariantArray* result, vtkIdType row )
{

  cout << "Assess::(): START result SetNumberOfValues " << 2*this->NumRuns << endl;
  result->SetNumberOfValues( 2*this->NumRuns );
  cout << "Assess::(): END result SetNumberOfValues " << 2*this->NumRuns << endl;
  vtkIdType resIndex=0;
  for( int runID = 0; runID < this->NumRuns; runID++)
    {
    result->SetValue( resIndex++, this->Distances->GetValue( row*this->NumRuns+runID ) );
    result->SetValue( resIndex++, this->ClusterMemberIDs->GetValue( row*this->NumRuns+runID ) );
    }
}
