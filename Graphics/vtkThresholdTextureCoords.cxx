/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkThresholdTextureCoords.hh"

// Construct with lower threshold=0, upper threshold=1, threshold 
// function=upper, and texture dimension = 2.
vtkThresholdTextureCoords::vtkThresholdTextureCoords()
{
  this->LowerThreshold = 0.0;
  this->UpperThreshold = 1.0;
  this->TextureDimension = 2;

  this->ThresholdFunction = &vtkThresholdTextureCoords::Upper;
}

// Description:
// Criterion is cells whose scalars are less than lower threshold.
void vtkThresholdTextureCoords::ThresholdByLower(float lower) 
{
  if ( this->LowerThreshold != lower )
    {
    this->LowerThreshold = lower; 
    this->ThresholdFunction = &vtkThresholdTextureCoords::Lower;
    this->Modified();
    }
}
                           
// Description:
// Criterion is cells whose scalars are less than upper threshold.
void vtkThresholdTextureCoords::ThresholdByUpper(float upper)
{
  if ( this->UpperThreshold != upper )
    {
    this->UpperThreshold = upper; 
    this->ThresholdFunction = &vtkThresholdTextureCoords::Upper;
    this->Modified();
    }
}
                           
// Description:
// Criterion is cells whose scalars are between lower and upper thresholds.
void vtkThresholdTextureCoords::ThresholdBetween(float lower, float upper)
{
  if ( this->LowerThreshold != lower || this->UpperThreshold != upper )
    {
    this->LowerThreshold = lower; 
    this->UpperThreshold = upper;
    this->ThresholdFunction = vtkThresholdTextureCoords::Between;
    this->Modified();
    }
}
  
void vtkThresholdTextureCoords::Execute()
{
  int numPts;
  vtkFloatTCoords *newTCoords;
  int ptId;
  float inTC[3], outTC[3];
  vtkScalars *inScalars;
  vtkDataSet *input=this->Input;
  vtkDataSet *output=this->Output;

  vtkDebugMacro(<< "Executing texture threshold filter");
  output->Initialize();

  if ( ! (inScalars = input->GetPointData()->GetScalars()) )
    {
    vtkErrorMacro(<<"No scalar data to texture threshold");
    return;
    }
     
  numPts = input->GetNumberOfPoints();
  newTCoords = new vtkFloatTCoords(this->TextureDimension);
  inTC[0] = inTC[1] = inTC[2] = 1.0;
  outTC[0] = outTC[1] = outTC[2] = 0.0;

  // Check that the scalars of each cell satisfy the threshold criterion
  for (ptId=0; ptId < input->GetNumberOfCells(); ptId++)
    {
    if ( ((this->*(this->ThresholdFunction))(inScalars->GetScalar(ptId))) )
      {
      newTCoords->InsertTCoord(ptId,inTC);
      }
    else //doesn't satisfy criterion
      {
      newTCoords->InsertTCoord(ptId,outTC);
      }

    } //for all points

  output->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();
}

void vtkThresholdTextureCoords::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToDataSetFilter::PrintSelf(os,indent);

  if ( this->ThresholdFunction == &vtkThresholdTextureCoords::Upper )
    os << indent << "Threshold By Upper\n";

  else if ( this->ThresholdFunction == &vtkThresholdTextureCoords::Lower )
    os << indent << "Threshold By Lower\n";

  else if ( this->ThresholdFunction == &vtkThresholdTextureCoords::Between )
    os << indent << "Threshold Between\n";

  os << indent << "Lower Threshold: " << this->LowerThreshold << "\n";;
  os << indent << "Upper Threshold: " << this->UpperThreshold << "\n";;
  os << indent << "Texture Dimension: " << this->TextureDimension << "\n";;
}
