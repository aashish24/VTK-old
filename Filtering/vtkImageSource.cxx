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
#include "vtkImageSource.hh"


//----------------------------------------------------------------------------
// Description:
// This method should a region filled with the requested data.
vtkImageRegion *vtkImageSource::RequestRegion(int outOffset[3], int outSize[3])
{
  vtkDebugMacro(<< "RequestRegion: offset = ("
                << outOffset[0] << ", " << outOffset[1] << ", " << outOffset[2]
                << "), size = ("
                << outSize[0] << ", " << outSize[1] << ", " << outSize[2]
                << ")");

  vtkErrorMacro(<< "RequestRegion has not been defined for this source");
  // Indicate that splitting will not help.
  this->SplitFactor = 0;
  
  return NULL;
}


//----------------------------------------------------------------------------
// Description:
// This method returns an object which will satisfy requests.
// For non cached sources, it returns the source itself.
vtkImageSource *vtkImageSource::GetOutput()
{
  return this;
}


//----------------------------------------------------------------------------
// Description:
// This method returns in "offset" and "size" the boundary of data
// in the image. Requests for regions of the image out side of these
// bounds will have unpridictable effects.
// i.e. no error checking is performed.
void vtkImageSource::GetBoundary(int *offset, int *size)
{
  int idx;
  
  vtkErrorMacro(<< "GetBoundary: Not defined for this source.");
  
  for (idx = 0; idx < 3; ++idx)
    {
    offset[idx] = 0;
    size[idx] = 0;
    }
}


//----------------------------------------------------------------------------
// Description:
// This method returns the maximum MTime of this source and all the objects
// that come before this source (that can change this sources output).
unsigned long vtkImageSource::GetPipelineMTime()
{
  return this->GetMTime();
}


  





