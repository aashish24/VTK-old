/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkPointDataToCellData.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkPointDataToCellData* vtkPointDataToCellData::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPointDataToCellData");
  if(ret)
    {
    return (vtkPointDataToCellData*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPointDataToCellData;
}




// Instantiate object so that point data is not passed to output.
vtkPointDataToCellData::vtkPointDataToCellData()
{
  this->PassPointData = 0;
}

void vtkPointDataToCellData::Execute()
{
  int cellId, ptId;
  int numCells, numPts;
  vtkDataSet *input= this->GetInput();
  vtkDataSet *output= this->GetOutput();
  vtkPointData *inPD=input->GetPointData();
  vtkCellData *outPD=output->GetCellData();
  int maxCellSize=input->GetMaxCellSize();
  vtkIdList *cellPts;
  float weight, *weights=new float[maxCellSize];

  vtkDebugMacro(<<"Mapping point data to cell data");

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  if ( (numCells=input->GetNumberOfCells()) < 1 )
    {
    vtkErrorMacro(<<"No input cells!");
    return;
    }
  
  cellPts = vtkIdList::New();
  cellPts->Allocate(maxCellSize);

  // notice that inPD and outPD are vtkPointData and vtkCellData; respectively.
  // It's weird, but it works.
  outPD->CopyAllocate(inPD,numCells);

  for (cellId=0; cellId < numCells; cellId++)
    {
    input->GetCellPoints(cellId, cellPts);
    numPts = cellPts->GetNumberOfIds();
    if ( numPts > 0 )
      {
      weight = 1.0 / numPts;
      for (ptId=0; ptId < numPts; ptId++)
	{
	weights[ptId] = weight;
	}
      outPD->InterpolatePoint(inPD, cellId, cellPts, weights);
      }
    }

  // Pass through any cell data that's in the input 
  // and not defined in the output.
  output->GetCellData()->PassNoReplaceData(input->GetCellData());
  
  if ( this->PassPointData )
    {
    output->GetPointData()->PassData(input->GetPointData());
    }
  
  cellPts->Delete();
  delete [] weights;
}

void vtkPointDataToCellData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToDataSetFilter::PrintSelf(os,indent);

  os << indent << "Pass Point Data: " << (this->PassPointData ? "On\n" : "Off\n");
}
