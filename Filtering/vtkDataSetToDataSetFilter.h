/*=========================================================================

  Program:   Visualization Toolkit
  Module:    DS2DSF.h
  Language:  C++
  Date:      2/17/94
  Version:   1.8


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkDataSetToDataSetFilter - abstract filter class
// .SECTION Description
// vtkDataSetToDataSetFilter is an abstract filter class. Subclasses of
// vtkDataSetToDataSetFilter take a dataset as input and create a dataset as
// output. The form of the input geometry is not changed in these filters,
// only the point attributes (e.g. scalars, vectors, etc.).
//
// This is an abstract filter type. What that means is that the output of the
// filter is an abstract type (i.e., vtkDataSet), no matter what the input of
// the filter is. This can cause problems connecting together filters due to
// the change in dataset type. (For example, in a series of filters
// processing vtkPolyData, when a vtkDataSetToDataSetFilter or subclass is
// introduced into the pipeline, if the filter downstream of it takes
// vtkPolyData as input, the pipeline connection cannot be made.) To get
// around this problem, use one of the convenience methods to return a
// concrete type (e.g., vtkGetPolyDataOutput(), GetStructuredPointsOutput(),
// etc.).

// .SECTION See Also
// vtkBrownianPoints vtkProbeFilter vtkThresholdTextureCoords vtkDicer
// vtkElevationFilter vtkImplicitTextureCoords vtkTextureMapToBox 
// vtkTextureMapToPlane vtkVectorDot vtkVectorNorm

#ifndef __vtkDataSetToDataSetFilter_h
#define __vtkDataSetToDataSetFilter_h

#include "vtkDataSetSource.h"
#include "vtkDataSet.h"
#include "vtkImageToStructuredPoints.h"

class vtkPolyData;
class vtkStructuredPoints;
class vtkStructuredGrid;
class vtkUnstructuredGrid;
class vtkRectilinearGrid;

class VTK_EXPORT vtkDataSetToDataSetFilter : public vtkDataSetSource
{

public:
  static vtkDataSetToDataSetFilter *New();
  const char *GetClassName() {return "vtkDataSetToDataSetFilter";};

  // Description:
  // Specify the input data or filter.
  void SetInput(vtkDataSet *input);
  void SetInput(vtkImageData *cache)
    {vtkImageToStructuredPoints *tmp = cache->MakeImageToStructuredPoints();
    this->SetInput(((vtkDataSet *)tmp->GetOutput())); tmp->Delete();}

  // Description:
  // Get the output of this filter. If output is NULL then input
  // hasn't been set which is necessary for abstract objects.
  vtkDataSet *GetOutput();
  vtkDataSet *GetOutput(int idx)
    {return (vtkDataSet *) this->vtkDataSetSource::GetOutput(idx); };

  // Description:
  // Get the output as vtkPolyData.
  vtkPolyData *GetPolyDataOutput();

  // Description:
  // Get the output as vtkStructuredPoints.
  vtkStructuredPoints *GetStructuredPointsOutput();

  // Description:
  // Get the output as vtkStructuredGrid.
  vtkStructuredGrid *GetStructuredGridOutput();

  // Description:
  // Get the output as vtkUnstructuredGrid.
  vtkUnstructuredGrid *GetUnstructuredGridOutput();

  // Description:
  // Get the output as vtkRectilinearGrid. 
  vtkRectilinearGrid *GetRectilinearGridOutput();
  
  // Description:
  // Get the input data or filter.
  vtkDataSet *GetInput();

  // Description:
  // This method is called by the data object. It assumes UpdateInformation
  // has been called.  vtkDataSetToDataSetFilter has a special version of
  // this method because it needs to "CopyStructure" from input to output.
  // Also, this version will not let subclasses initiate stremaing.
  void InternalUpdate(vtkDataObject *output);
  
protected:
  vtkDataSetToDataSetFilter();
  ~vtkDataSetToDataSetFilter();
  vtkDataSetToDataSetFilter(const vtkDataSetToDataSetFilter&) {};
  void operator=(const vtkDataSetToDataSetFilter&) {};


  // Since we know Inputs[0] is the same type as Outputs[0] we can
  // use CopyUpdateExtent of the data object to propagate extents.
  // It the filter has more than one input, all bets are off.
  // It is then up to the subclass to implement this method.
  int ComputeInputUpdateExtents(vtkDataObject *output);

};

#endif



