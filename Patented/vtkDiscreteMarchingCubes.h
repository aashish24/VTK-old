/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.


     THIS CLASS IS PATENTED UNDER UNITED STATES PATENT NUMBER 4,710,876
     "System and Method for the Display of Surface Structures Contained
     Within the Interior Region of a Solid Body".
     Application of this software for commercial purposes requires 
     a license grant from GE. Contact:

         Carl B. Horton
         Sr. Counsel, Intellectual Property
         3000 N. Grandview Blvd., W-710
         Waukesha, WI  53188
         Phone:  (262) 513-4022
         E-Mail: Carl.Horton@med.ge.com

     for more information.

// .SECTION Thanks
// Jim Miller at GE Research implemented the original version of this
// filter.
// This work was supported by PHS Research Grant No. 1 P41 RR13218-01
// from the National Center for Research Resources and supported by a
// grant from the DARPA, executed by the U.S. Army Medical Research
// and Materiel Command/TATRC Cooperative Agreement,
// Contract # W81XWH-04-2-0012.

=========================================================================*/
// .NAME vtkDiscreteMarchingCubes - generate object boundaries from
// labelled volumes
// .SECTION Description vtkDiscreteMarchingCubes is a filter that
// takes as input a volume (e.g., 3D structured point set) of
// segmentation labels and generates on output one or more
// models representing the boundaries between the specified label and
// the adjacent structures.  One or more label values must be specified to
// generate the models.  The boundary positions are always defined to
// be half-way between adjacent voxels. This filter works best with
// integral scalar values.
// If ComputeScalars is on (the default), each output cell will have
// cell data that corresponds to the scalar value (segmentation label)
// of the corresponding cube. Note that this differs from vtkMarchingCubes,
// which stores the scalar value as point data. The rationale for this
// difference is that cell vertices may be shared between multiple
// cells. This also means that the resultant polydata may be
// non-manifold (cell faces may be coincident). To further process the
// polydata, users should either: 1) extract cells that have a common
// scalar value using vtkThreshold, or 2) process the data with
// filters that can handle non-manifold polydata
// (e.g. vtkWindowedSincPolyDataFilter).
// Also note, Normals and Gradients are not computed.
// .SECTION Caveats
// This filter is specialized to volumes. If you are interested in 
// contouring other types of data, use the general vtkContourFilter. If you
// want to contour an image (i.e., a volume slice), use vtkMarchingSquares.
// .SECTION See Also
// vtkContourFilter vtkSliceCubes vtkMarchingSquares vtkDividingCubes

#ifndef __vtkDiscreteMarchingCubes_h
#define __vtkDiscreteMarchingCubes_h

#include "vtkMarchingCubes.h"

class VTK_EXPORT vtkDiscreteMarchingCubes : public vtkMarchingCubes
{
public:
  static vtkDiscreteMarchingCubes *New();
  vtkTypeRevisionMacro(vtkDiscreteMarchingCubes,vtkMarchingCubes);

protected:
  vtkDiscreteMarchingCubes();
  ~vtkDiscreteMarchingCubes();

  void Execute();

private:
  vtkDiscreteMarchingCubes(const vtkDiscreteMarchingCubes&);  // Not implemented.
  void operator=(const vtkDiscreteMarchingCubes&);  // Not implemented.

};

#endif


