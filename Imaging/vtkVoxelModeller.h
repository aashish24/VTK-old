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
// .NAME vtkVoxelModeller - convert arbitrary dataset to voxel (structured point) representation
// .SECTION Description
// vtkVoxelModeller is a filter that converts an arbitrary data set to a
// structured point (i.e., voxel) representation.

#ifndef __vtkVoxelModeller_h
#define __vtkVoxelModeller_h

#include "vtkDataSetToStructuredPointsFilter.hh"

class vtkVoxelModeller : public vtkDataSetToStructuredPointsFilter 
{
public:
  vtkVoxelModeller();
  ~vtkVoxelModeller() {};
  char *GetClassName() {return "vtkVoxelModeller";};
  void PrintSelf(ostream& os, vtkIndent indent);

  float ComputeModelBounds();

  void SetSampleDimensions(int i, int j, int k);
  void SetSampleDimensions(int dim[3]);
  vtkGetVectorMacro(SampleDimensions,int,3);

  // Description:
  // Specify distance away from surface of input geometry to sample. Smaller
  // values make large increases in performance.
  vtkSetClampMacro(MaximumDistance,float,0.0,1.0);
  vtkGetMacro(MaximumDistance,float);

  // Specify the position in space to perform the sampling.
  void SetModelBounds(float *bounds);
  void SetModelBounds(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
  vtkGetVectorMacro(ModelBounds,float,6);

  void Write(char *);

protected:
  void Execute();
  int SampleDimensions[3];
  float MaximumDistance;
  float ModelBounds[6];
};

#endif


