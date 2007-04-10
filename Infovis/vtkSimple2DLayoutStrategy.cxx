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
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/


#include "vtkSimple2DLayoutStrategy.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

#include "vtkAbstractGraph.h"
#include "vtkGraph.h"
#include "vtkTree.h"


vtkCxxRevisionMacro(vtkSimple2DLayoutStrategy, "$Revision$");
vtkStandardNewMacro(vtkSimple2DLayoutStrategy);

// This is just a convenient macro for smart pointers
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()
  
#ifndef MIN
#define MIN(x, y)       ((x) < (y) ? (x) : (y))
#endif


// Cool-down function.
static inline float CoolDown(float t, float r) 
{  
  return t-(t/r);
}

// ----------------------------------------------------------------------

vtkSimple2DLayoutStrategy::vtkSimple2DLayoutStrategy()
{

  // Create internal vtk classes
  this->RepulsionArray = vtkSmartPointer<vtkFloatArray>::New();
  this->AttractionArray = vtkSmartPointer<vtkFloatArray>::New();
    
  this->IterationsPerLayout = 100;
  this->InitialTemperature = 5;
  this->CoolDownRate = 50.0;
  this->LayoutComplete = 0;
  this->EdgeWeightField = 0;
  this->RestDistance = 0;
  this->Jitter = true;
}

// ----------------------------------------------------------------------

vtkSimple2DLayoutStrategy::~vtkSimple2DLayoutStrategy()
{
  this->SetEdgeWeightField(0);
}

// ----------------------------------------------------------------------

// Set the graph that will be laid out
void vtkSimple2DLayoutStrategy::Initialize()
{
  // Set up some quick access variables
  vtkPoints* pts = this->Graph->GetPoints();
  vtkIdType numVertices = this->Graph->GetNumberOfVertices();
  vtkIdType numEdges = this->Graph->GetNumberOfEdges();
  
  // Create new float array that will be the graph's
  // geometric points
  VTK_CREATE(vtkFloatArray, graphPoints);
  graphPoints->SetNumberOfComponents(3);
  graphPoints->SetNumberOfTuples(numVertices);
  
  // Copy current points to new float array
  double pointCoords[3];
  for (vtkIdType i=0; i<numVertices; ++i)
    {
    pts->GetPoint(i, pointCoords);
    graphPoints->SetValue(i*3, static_cast<float>(pointCoords[0]));
    graphPoints->SetValue(i*3+1, static_cast<float>(pointCoords[1]));
    graphPoints->SetValue(i*3+2, static_cast<float>(pointCoords[2]));
    }
  
  // Set the graph points
  pts->SetData(graphPoints);
  
  // Get a quick pointer to the point data
  vtkFloatArray *array = vtkFloatArray::SafeDownCast(pts->GetData());
  float *rawPointData = array->GetPointer(0);
  
  // The optimal distance between vertices.
  if (this->RestDistance == 0)
    {
    this->RestDistance = sqrt(1.0 / static_cast<float>(numVertices));
    }
    
  // Set up array to store repulsion values
  this->RepulsionArray->SetNumberOfComponents(3);
  this->RepulsionArray->SetNumberOfTuples(numVertices);
  for (vtkIdType i=0; i<numVertices*3; ++i)
    {
    this->RepulsionArray->SetValue(i, 0);
    }
    
  // Set up array to store attraction values
  this->AttractionArray->SetNumberOfComponents(3);
  this->AttractionArray->SetNumberOfTuples(numVertices);
  for (vtkIdType i=0; i<numVertices*3; ++i)
    {
    this->AttractionArray->SetValue(i, 0);
    }
    
  // Put the edge data into compact, fast access edge data structure
  this->EdgeArray =  new vtkLayoutEdge[numEdges];
  
  // If jitter then do it now at initialization
  if (Jitter)
    {
    
    // Jitter x and y, skip z
    for (vtkIdType i=0; i<numVertices*3; i+=3)
      {
      rawPointData[i] += this->RestDistance*(static_cast<float>(rand())/RAND_MAX - .5);
      rawPointData[i+1] += this->RestDistance*(static_cast<float>(rand())/RAND_MAX - .5);
      }
    }

  // Get the weight array
  vtkDataArray* weightArray = NULL;
  double weight, maxWeight = 0;
  if (this->EdgeWeightField != NULL)
    {
    weightArray = vtkDataArray::SafeDownCast(this->Graph->GetEdgeData()->GetAbstractArray(this->EdgeWeightField));
    if (weightArray != NULL)
      {
      for (vtkIdType w = 0; w < weightArray->GetNumberOfTuples(); w++)
        {
        weight = weightArray->GetTuple1(w);
        if (weight > maxWeight)
          {
          maxWeight = weight;
          }
        }
      }
    }
    
  // Load up the edge data structures
  for (vtkIdType i=0; i<numEdges; ++i)
    {
    this->EdgeArray[i].from = this->Graph->GetSourceVertex(i);
    this->EdgeArray[i].to = this->Graph->GetTargetVertex(i);
    if (weightArray != NULL)
      {
      weight = weightArray->GetTuple1(i);
      this->EdgeArray[i].weight = weight / maxWeight;
      }
    else
      {
      this->EdgeArray[i].weight = 1.0;
      }
    }
    
  // Set some vars
  this->TotalIterations = 0;
  this->LayoutComplete = 0;
  this->Temp = this->InitialTemperature;

}

// ----------------------------------------------------------------------

// Simple graph layout method
void vtkSimple2DLayoutStrategy::Layout()
{
  // Do I have a graph to layout
  if (this->Graph == NULL)
    {
    vtkErrorMacro("Graph Layout called with Graph==NULL, call SetGraph(g) first");
    this->LayoutComplete = 1;
    return;
    }
  
  // Set up some variables
  vtkPoints* pts = this->Graph->GetPoints();
  vtkIdType numVertices = this->Graph->GetNumberOfVertices();
  vtkIdType numEdges = this->Graph->GetNumberOfEdges();
  
  // Get a quick pointer to the point data
  vtkFloatArray *array = vtkFloatArray::SafeDownCast(pts->GetData());
  float *rawPointData = array->GetPointer(0);

  // This is the mega, uber, triple inner loop
  // ye of weak hearts, tread no further!
  float delta[]={0,0,0};
  float disSquared;
  float attractValue;
  vtkIdType pointIndex1=0;
  vtkIdType pointIndex2=0;
  for(int i = 0; i < this->IterationsPerLayout; ++i)
    {
    
    // Initialize the repulsion and attraction arrays
    for (vtkIdType j=0; j<numVertices*3; ++j)
      {
      this->RepulsionArray->SetValue(j, 0);
      }
      
    // Set up array to store attraction values
    for (vtkIdType j=0; j<numVertices*3; ++j)
      {
      this->AttractionArray->SetValue(j, 0);
      }
  
    // Calculate the repulsive forces
    float *rawRepulseArray = this->RepulsionArray->GetPointer(0);
    for(vtkIdType j=0; j<numVertices; ++j)
      {
      pointIndex1 = j * 3;
        
      for(vtkIdType k=0; k<numVertices; ++k)
        {
        // Don't repluse against yourself :)
        if (k == j) continue;
          
        pointIndex2 = k * 3;
          
        delta[0] = rawPointData[pointIndex1] - 
                  rawPointData[pointIndex2];
        delta[1] = rawPointData[pointIndex1+1] - 
                  rawPointData[pointIndex2+1];
        disSquared = delta[0]*delta[0] + delta[1]*delta[1];
        rawRepulseArray[pointIndex1]   += delta[0]/disSquared;
        rawRepulseArray[pointIndex1+1] += delta[1]/disSquared;      
        }
      }
      
    // Calculate the attractive forces
    float *rawAttractArray = this->AttractionArray->GetPointer(0);
    for (vtkIdType j=0; j<numEdges; ++j)
      {
      pointIndex1 = this->EdgeArray[j].to * 3;
      pointIndex2 = this->EdgeArray[j].from * 3;
      
      // No need to attract points to themselves
      if (pointIndex1 == pointIndex2) continue;
      
      delta[0] = rawPointData[pointIndex1] - 
             rawPointData[pointIndex2];
      delta[1] = rawPointData[pointIndex1+1] - 
              rawPointData[pointIndex2+1];
      disSquared = delta[0]*delta[0] + delta[1]*delta[1];
       
      // Perform weight adjustment
      attractValue = this->EdgeArray[j].weight*disSquared-this->RestDistance;

      rawAttractArray[pointIndex1]   -= delta[0] * attractValue;
      rawAttractArray[pointIndex1+1] -= delta[1] * attractValue;
      rawAttractArray[pointIndex2]   += delta[0] * attractValue;
      rawAttractArray[pointIndex2+1] += delta[1] * attractValue;
      }
      
    // Okay now set new positions based on replusion 
    // and attraction 'forces'
    for(vtkIdType j=0; j<numVertices; ++j)
      {
      pointIndex1 = j * 3;
      
      // Get forces for this node
      float forceX = rawAttractArray[pointIndex1] + rawRepulseArray[pointIndex1];
      float forceY = rawAttractArray[pointIndex1+1] + rawRepulseArray[pointIndex1+1];
      //forceX = rawRepulseArray[pointIndex1];
      //forceY = rawRepulseArray[pointIndex1+1];
      
      // Forces can get extreme so limit them
      // Note: This is psuedo-normalization of the
      //       force vector, just to save some cycles
      float pNormalize = MIN(1, 1.0/(fabs(forceX) + fabs(forceY)));
      pNormalize *= this->Temp;
      forceX *= pNormalize;
      forceY *= pNormalize;
  
      rawPointData[pointIndex1] += forceX;
      rawPointData[pointIndex1+1] += forceY;
      }
      
    // The point coordinates have been modified
    this->Graph->GetPoints()->Modified();
      
    // Reduce temperature as layout approaches a better configuration.
    this->Temp = CoolDown(this->Temp, this->CoolDownRate);

    // Announce progress
    double progress = (i+this->TotalIterations) / 
                      static_cast<double>(this->MaxNumberOfIterations);
    this->InvokeEvent(vtkCommand::ProgressEvent, static_cast<void *>(&progress));

   } // End loop this->IterationsPerLayout

    
  // Check for completion of layout
  this->TotalIterations += this->IterationsPerLayout;
  if (this->TotalIterations >= this->MaxNumberOfIterations)
    {
    // I'm done
    this->LayoutComplete = 1;
    }
}

void vtkSimple2DLayoutStrategy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "InitialTemperature: " << this->InitialTemperature << endl;
  os << indent << "MaxNumberOfIterations: " << this->MaxNumberOfIterations << endl;
  os << indent << "IterationsPerLayout: " << this->IterationsPerLayout << endl;
  os << indent << "CoolDownRate: " << this->CoolDownRate << endl;
  os << indent << "EdgeWeightField: " << (this->EdgeWeightField ? this->EdgeWeightField : "(none)") << endl;
  os << indent << "Jitter: " << (this->Jitter ? "True" : "False") << endl;
  os << indent << "RestDistance: " << this->RestDistance << endl;
}
