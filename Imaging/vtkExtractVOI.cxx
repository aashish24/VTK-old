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
#include "vtkExtractVOI.hh"

// Description:
// Construct object to extract all of the input data.
vtkExtractVOI::vtkExtractVOI()
{
  this->VOI[0] = this->VOI[2] = this->VOI[4] = 0;
  this->VOI[1] = this->VOI[3] = this->VOI[5] = VTK_LARGE_INTEGER;

  this->SampleRate[0] = this->SampleRate[1] = this->SampleRate[2] = 1;
}

void vtkExtractVOI::SetVOI(int imin, int imax, int jmin, int jmax, 
                                       int kmin, int kmax)
{
  int dim[6];

  dim[0] = imin;
  dim[1] = imax;
  dim[2] = jmin;
  dim[3] = jmax;
  dim[4] = kmin;
  dim[5] = kmax;

  this->SetVOI(dim);
}

void vtkExtractVOI::Execute()
{
  vtkStructuredPoints *input=(vtkStructuredPoints *)this->Input;
  vtkPointData *pd=input->GetPointData();
  vtkStructuredPoints *output=(vtkStructuredPoints *)this->Output;
  vtkPointData *outPD=output->GetPointData();
  int i, j, k, dims[3], outDims[3], voi[6], dim, idx, newIdx;
  float origin[3], ar[3], outOrigin[3], outAR[3];
  int sliceSize, outSize, jOffset, kOffset, rate[3];

  vtkDebugMacro(<< "Extracting VOI");
//
// Check VOI and clamp as necessary. Compute output parameters,
//
  input->GetDimensions(dims);
  input->GetOrigin(origin);
  input->GetAspectRatio(ar);

  for ( i=0; i < 6; i++ ) voi[i] = this->VOI[i];

  for ( outSize=1, dim=0, i=0; i < 3; i++ )
    {
    if ( voi[2*i+1] >= dims[i] ) voi[2*i+1] = dims[i] - 1;
    else if ( voi[2*i+1] < 0 ) voi[2*i+1] = 0;

    if ( voi[2*i] > voi[2*i+1] ) voi[2*i] = voi[2*i+1];
    else if ( voi[2*i] < 0 ) voi[2*i] = 0;

    if ( (voi[2*i+1]-voi[2*i]) > 0 ) dim++;

    if ( (rate[i] = this->SampleRate[i]) < 1 ) rate[i] = 1;

    outDims[i] = (voi[2*i+1] - voi[2*i]) / rate[i] + 1;
    if ( outDims[i] < 1 ) outDims[i] = 1;

    outAR[i] = ar[i] * this->SampleRate[i];

    outOrigin[i] = origin[i] + voi[2*i]*ar[i];

    outSize *= outDims[i];
    }

  output->SetDimensions(outDims);
  output->SetAspectRatio(outAR);
  output->SetOrigin(outOrigin);

//
// Allocate necessary objects
//
  outPD->CopyAllocate(pd,outSize,outSize);
  sliceSize = dims[0]*dims[1];

//
// Traverse input data and copy point attributes to output
//
  newIdx = 0;
  for ( k=voi[4]; k <= voi[5]; k += rate[2] )
    {
    kOffset = k * sliceSize;
    for ( j=voi[2]; j <= voi[3]; j += rate[1] )
      {
      jOffset = j * dims[0];
      for ( i=voi[0]; i <= voi[1]; i += rate[0] )
        {
        idx = i + jOffset + kOffset;
        outPD->CopyData(pd, idx, newIdx++);
        }
      }
    }

  vtkDebugMacro(<<"Extracted " << newIdx << " point attributes on "
                << dim << "-D dataset\n\tDimensions are (" << outDims[0]
                << "," << outDims[1] << "," << outDims[2] <<")");
}


void vtkExtractVOI::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkStructuredPointsFilter::PrintSelf(os,indent);

  os << indent << "VOI: \n";
  os << indent << "  Imin,Imax: (" << this->VOI[0] << ", " << this->VOI[1] << ")\n";
  os << indent << "  Jmin,Jmax: (" << this->VOI[2] << ", " << this->VOI[3] << ")\n";
  os << indent << "  Kmin,Kmax: (" << this->VOI[4] << ", " << this->VOI[5] << ")\n";

  os << indent << "Sample Rate: (" << this->SampleRate[0] << ", "
               << this->SampleRate[1] << ", "
               << this->SampleRate[2] << ")\n";
}


