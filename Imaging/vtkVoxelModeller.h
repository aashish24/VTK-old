/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkVoxelModeller - convert an arbitrary dataset to a voxel representation
// .SECTION Description
// vtkVoxelModeller is a filter that converts an arbitrary data set to a
// structured point (i.e., voxel) representation. It is very similar to 
// vtkImplicitModeller, except that it doesn't record distance; instead it
// records occupancy. As such, it stores its results in the more compact
// form of 0/1 bits.
// .SECTION see also
// vtkImplicitModeller

#ifndef __vtkVoxelModeller_h
#define __vtkVoxelModeller_h

#include "vtkDataSetToStructuredPointsFilter.h"

class VTK_EXPORT vtkVoxelModeller : public vtkDataSetToStructuredPointsFilter 
{
public:
  vtkTypeMacro(vtkVoxelModeller,vtkDataSetToStructuredPointsFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct an instance of vtkVoxelModeller with its sample dimensions
  // set to (50,50,50), and so that the model bounds are
  // automatically computed from its input. The maximum distance is set to 
  // examine the whole grid. This could be made much faster, and probably
  // will be in the future.
  static vtkVoxelModeller *New();

  // Description:
  // Compute the ModelBounds based on the input geometry.
  float ComputeModelBounds(float origin[3], float ar[3]);

  // Description:
  // Set the i-j-k dimensions on which to sample the distance function.
  void SetSampleDimensions(int i, int j, int k);

  // Description:
  // Set the i-j-k dimensions on which to sample the distance function.
  void SetSampleDimensions(int dim[3]);
  vtkGetVectorMacro(SampleDimensions,int,3);

  // Description:
  // Specify distance away from surface of input geometry to sample. Smaller
  // values make large increases in performance.
  vtkSetClampMacro(MaximumDistance,float,0.0,1.0);
  vtkGetMacro(MaximumDistance,float);

  // Description:
  // Specify the position in space to perform the voxelization.
  void SetModelBounds(float *bounds);
  void SetModelBounds(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
  vtkGetVectorMacro(ModelBounds,float,6);

  // Description:
  // The the volume out to a specified filename.
  void Write(char *);

protected:
  vtkVoxelModeller();
  ~vtkVoxelModeller() {};
  vtkVoxelModeller(const vtkVoxelModeller&) {};
  void operator=(const vtkVoxelModeller&) {};

  void Execute();
  int SampleDimensions[3];
  float MaximumDistance;
  float ModelBounds[6];
};

#endif


