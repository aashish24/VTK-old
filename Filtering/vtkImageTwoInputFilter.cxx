/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

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
#include "vtkImageRegion.h"
#include "vtkImageCache.h"
#include "vtkImageTwoInputFilter.h"


//----------------------------------------------------------------------------
vtkImageTwoInputFilter::vtkImageTwoInputFilter()
{
  this->Input1 = NULL;
  this->Input2 = NULL;

  // Invalid
  this->ExecuteDimensionality = -1;
}

//----------------------------------------------------------------------------
void vtkImageTwoInputFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageCachedSource::PrintSelf(os,indent);
  os << indent << "Input1: (" << this->Input1 << ")\n";
  os << indent << "Input2: (" << this->Input2 << ")\n";
}

//----------------------------------------------------------------------------
// Description:
// This Method returns the MTime of the pipeline upto and including this filter
// Note: current implementation may create a cascade of GetPipelineMTime calls.
// Each GetPipelineMTime call propagates the call all the way to the original
// source.  This works, but is not elegant.
// An Executor would probably be the best solution if this is a problem.
// (The pipeline could vote before it starts processing, but one object
// has to initiate the voting.)
unsigned long int vtkImageTwoInputFilter::GetPipelineMTime()
{
  unsigned long int time1, time2;

  // This objects MTime
  // (Super class considers cache in case cache did not originate message)
  time1 = this->vtkImageCachedSource::GetPipelineMTime();
  if ( ! this->Input1 )
    {
    vtkWarningMacro(<< "GetPipelineMTime: Input1 not set.");
    time2 = time1;
    }
  else
    {
    time2 = this->Input1->GetPipelineMTime();
    }
    
  // Keep the larger of the two 
  if (time2 > time1)
    time1 = time2;

  if ( ! this->Input2 )
    {
    vtkWarningMacro(<< "GetPipelineMTime: Input2 not set.");
    return time1;
    }
  time2 = this->Input2->GetPipelineMTime();

  // Keep the larger of the two 
  if (time2 > time1)
    time1 = time2;

  return time1;
}


//----------------------------------------------------------------------------
// Description:
// Set the Input1 of this filter. If a ScalarType has not been set,
// then the ScalarType of the input is used.
void vtkImageTwoInputFilter::SetInput1(vtkImageCache *input)
{
  vtkDebugMacro(<< "SetInput1: input = " << input->GetClassName()
		<< " (" << input << ")");

  // does this change anything?
  if (input == this->Input1)
    {
    return;
    }
  
  this->Input1 = input;
  this->Modified();

  // Should we use the data type from the input?
  this->CheckCache();      // make sure a cache exists
  if (this->Output->GetScalarType() == VTK_VOID)
    {
    this->Output->SetScalarType(input->GetScalarType());
    if (this->Output->GetScalarType() == VTK_VOID)
      {
      vtkErrorMacro(<< "SetInput1: Cannot determine ScalarType of input.");
      }
    }
}



//----------------------------------------------------------------------------
// Description:
// Set the Input2 of this filter. If a ScalarType has not been set,
// then the ScalarType of the input is used.
void vtkImageTwoInputFilter::SetInput2(vtkImageCache *input)
{
  vtkDebugMacro(<< "SetInput2: input = " << input->GetClassName()
		<< " (" << input << ")");

  // does this change anything?
  if (input == this->Input2)
    {
    return;
    }
  
  this->Input2 = input;
  this->Modified();

  // Should we use the data type from the input?
  this->CheckCache();      // make sure a cache exists

  // Do not use the scalar type from the second input.
}



//----------------------------------------------------------------------------
// Description:
// This method is called by the cache.  
void vtkImageTwoInputFilter::Update(vtkImageRegion *outRegion)
{
  vtkImageRegion *inRegion1, *inRegion2;
  
  // Make sure the subclss has defined the execute dimensionality
  // It is needed to terminate recursion.
  if (this->ExecuteDimensionality < 0)
    {
    vtkErrorMacro(<< "Subclass has not set ExecuteDimensionality");
    return;
    }

  // If outBBox is empty return imediately.
  if (outRegion->IsEmpty())
    {
    return;
    }
    
  // Make sure the Input has been set.
  if ( ! this->Input1 || ! this->Input2)
    {
    vtkErrorMacro(<< "An input has not been set.");
    return;
    }
  
  // Make the input regions that will be used to generate the output region
  inRegion1 = vtkImageRegion::New();
  inRegion2 = vtkImageRegion::New();
  // Fill in image information
  this->Input1->UpdateImageInformation(inRegion1);
  this->Input2->UpdateImageInformation(inRegion2);
  // Translate to local coordinate system
  inRegion1->SetAxes(VTK_IMAGE_DIMENSIONS, this->Axes);
  inRegion2->SetAxes(VTK_IMAGE_DIMENSIONS, this->Axes);
  
  // Compute the required input region extent.
  // Copy to fill in extent of extra dimensions.
  inRegion1->SetExtent(VTK_IMAGE_DIMENSIONS, outRegion->GetExtent());
  inRegion2->SetExtent(VTK_IMAGE_DIMENSIONS, outRegion->GetExtent());
  this->ComputeRequiredInputRegionExtent(outRegion, inRegion1, inRegion2);

  // Use the input to fill the data of the region.
  this->Input1->Update(inRegion1);
  if ( ! inRegion1->AreScalarsAllocated())
    {
    vtkErrorMacro("Update(region): Cannot get input1");
    return;
    }
  this->Input2->Update(inRegion2);
  if ( ! inRegion2->AreScalarsAllocated())
    {
    vtkErrorMacro("Update(region): Cannot get input2");
    inRegion1->Delete();
    return;
    }
  
  // fill the output region 
  if ( this->StartMethod ) (*this->StartMethod)(this->StartMethodArg);
  this->RecursiveLoopExecute(VTK_IMAGE_DIMENSIONS,
			     inRegion1, inRegion2, outRegion);
  if ( this->EndMethod ) (*this->EndMethod)(this->EndMethodArg);
  
  // free the input regions
  inRegion1->Delete();
  inRegion2->Delete();

  // Save the new region in cache.
  // I am not happy with the necessity of the call CacheRegion.
  // Getting the region from the cache originally seems more consistent
  // and more compatable with the visualization pipeline.
  this->Output->CacheRegion(outRegion);  
}


//----------------------------------------------------------------------------
// Description:
// This method gets the boundary of the inputs then computes and returns 
// the boundary of the largest region that can be generated. 
void vtkImageTwoInputFilter::UpdateImageInformation(vtkImageRegion *outRegion)
{
  vtkImageRegion *inRegion2;
  
  // Make sure the Input has been set.
  if ( ! this->Input1 || ! this->Input2)
    {
    vtkErrorMacro(<< "UpdateImageInformation: An input is not set.");
    return;
    }

  inRegion2 = vtkImageRegion::New();
  
  this->Input1->UpdateImageInformation(outRegion);
  this->Input2->UpdateImageInformation(inRegion2);
  this->ComputeOutputImageInformation(outRegion, inRegion2, outRegion);

  inRegion2->Delete();
}



//----------------------------------------------------------------------------
// Description:
// This method is passed an inRegion that holds the image information
// (image extent ...) of this filters input, and fills outRegion with
// the image information after this filter is finished.
// outImage is identical to inImage when this method is envoked, and
// outImage may be the same object as in image.
void 
vtkImageTwoInputFilter::ComputeOutputImageInformation(vtkImageRegion *inRegion1,
						    vtkImageRegion *inRegion2,
						    vtkImageRegion *outRegion)

{
  // Default: Image information does not change (do nothing).
  // Avoid warnings
  inRegion1 = inRegion1;
  inRegion2 = inRegion2;
  outRegion = outRegion;
}



//----------------------------------------------------------------------------
// Description:
// This method computes the extent of the input region necessary to generate
// an output region.  Before this method is called "region" should have the 
// extent of the output region.  After this method finishes, "region" should 
// have the extent of the required input region.  The default method assumes
// the required input extent are the same as the output extent.
// Note: The splitting methods call this method with outRegion = inRegion.
void vtkImageTwoInputFilter::ComputeRequiredInputRegionExtent(
					       vtkImageRegion *outRegion,
					       vtkImageRegion *inRegion1,
					       vtkImageRegion *inRegion2)
{
  inRegion1->SetExtent(outRegion->GetExtent());
  inRegion2->SetExtent(outRegion->GetExtent());
}



//----------------------------------------------------------------------------
// Description:
// This execute method recursively loops over extra dimensions and
// calls the subclasses Execute method with lower dimensional regions.
void vtkImageTwoInputFilter::RecursiveLoopExecute(int dim, 
						  vtkImageRegion *inRegion1,
						  vtkImageRegion *inRegion2,
						  vtkImageRegion *outRegion)
{
  // Terminate recursion?
  if (dim <= this->ExecuteDimensionality)
    {
    this->Execute(inRegion1, inRegion2, outRegion);
    return;
    }
  else
    {
    int coordinate, axis;
    int inMin, inMax;
    int outMin, outMax;
    
    // Get the extent of the forth dimension to be eliminated.
    axis = this->Axes[dim - 1];
    inRegion1->GetAxisExtent(axis, inMin, inMax);
    outRegion->GetAxisExtent(axis, outMin, outMax);
    
    // Extra axis of in and out must have the same extent
    if (inMin != outMin || inMax != outMax)
      {
      vtkErrorMacro(<< "Execute: Extra axis can not be eliminated.");
      return;
      }
    
    // loop over the samples along the extra axis.
    for (coordinate = inMin; coordinate <= inMax; ++coordinate)
      {
      // set up the lower dimensional regions.
      inRegion1->SetAxisExtent(axis, coordinate, coordinate);
      inRegion2->SetAxisExtent(axis, coordinate, coordinate);
      outRegion->SetAxisExtent(axis, coordinate, coordinate);
      this->RecursiveLoopExecute(dim - 1, inRegion1, inRegion2, outRegion);
      }
    // restore the original extent
    inRegion1->SetAxisExtent(axis, inMin, inMax);
    inRegion2->SetAxisExtent(axis, inMin, inMax);
    outRegion->SetAxisExtent(axis, outMin, outMax);
    }
}

  
  
//----------------------------------------------------------------------------
// Description:
// The execute method created by the subclass.
void vtkImageTwoInputFilter::Execute(vtkImageRegion *inRegion1, 
				     vtkImageRegion *inRegion2, 
				     vtkImageRegion *outRegion)
{
  inRegion1 = inRegion2 = outRegion;
  vtkErrorMacro(<< "Subclass needs to supply an execute function.");
}

  







