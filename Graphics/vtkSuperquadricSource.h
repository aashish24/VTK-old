/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Mike Halle, Brigham and Women's Hospital


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
// .NAME vtkSuperquadricSource - create a polygonal superquadric centered 
// at the origin
// .SECTION Description
// vtkSuperquadricSource creates a superquadric (represented by polygons) 
// of specified
// size centered at the origin. The resolution (polygonal discretization)
// in both the latitude (phi) and longitude (theta) directions can be
// specified. Roundness parameters (PhiRoundness and ThetaRoundness) control
// the shape of the superquadric.  The Toroidal boolean controls whether
// a toroidal superquadric is produced.  If so, the Thickness parameter
// controls the thickness of the toroid:  0 is the thinnest allowable
// toroid, and 1 has a minimum sized hole.  The Scale parameters allow 
// the superquadric to be scaled in x, y, and z (normal vectors are correctly
// generated in any case).  The Size parameter controls size of the 
// superquadric.
//
// This code is based on "Rigid physically based superquadrics", A. H. Barr,
// in "Graphics Gems III", David Kirk, ed., Academic Press, 1992.
//
// .SECTION Caveats
// Resolution means the number of latitude or longitude lines for a complete 
// superquadric. The resolution parameters are rounded to the nearest 4
// in phi and 8 in theta.  
//
// Texture coordinates are not equally distributed around all superquadrics.
// 
// The Size and Thickness parameters control coefficients of superquadric
// generation, and may do not exactly describe the size of the superquadric.
//

#ifndef __vtkSuperquadricSource_h
#define __vtkSuperquadricSource_h

#include "vtkPolyDataSource.h"

#define VTK_MAX_SUPERQUADRIC_RESOLUTION 1024
#define VTK_MIN_SUPERQUADRIC_THICKNESS  1e-4
#define VTK_MIN_SUPERQUADRIC_ROUNDNESS  1e-24

class VTK_EXPORT vtkSuperquadricSource : public vtkPolyDataSource 
{
public:
  // Description:
  // Create a default superquadric with a radius of 0.5, non-toroidal, 
  // spherical, and centered at the origin.
  static vtkSuperquadricSource *New();

  vtkTypeMacro(vtkSuperquadricSource,vtkPolyDataSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the center of the superquadric. Default is 0,0,0.
  vtkSetVector3Macro(Center,float);
  vtkGetVectorMacro(Center,float,3);

  // Description:
  // Set the scale factors of the superquadric. Default is 1,1,1.
  vtkSetVector3Macro(Scale,float);
  vtkGetVectorMacro(Scale,float,3);

  // Description:
  // Set the number of points in the longitude direction.
  vtkGetMacro(ThetaResolution,int);
  void SetThetaResolution(int i);

  // Description:
  // Set the number of points in the latitude direction.
  vtkGetMacro(PhiResolution,int);
  void SetPhiResolution(int i);

  // Description:
  // Set/Get Superquadric ring thickness (toriods only).
  // Changing thickness maintains the outside diameter of the toroid.
  vtkGetMacro(Thickness,float);
  vtkSetClampMacro(Thickness,float,VTK_MIN_SUPERQUADRIC_THICKNESS,1.0);

  // Description:
  // Set/Get Superquadric north/south roundness. 
  // Values range from 0 (rectanglar) to 1 (circular) to higher orders.
  vtkGetMacro(PhiRoundness,float);
  void SetPhiRoundness(float e); 

  // Description:
  // Set/Get Superquadric east/west roundness.
  // Values range from 0 (rectanglar) to 1 (circular) to higher orders.
  vtkGetMacro(ThetaRoundness,float);
  void SetThetaRoundness(float e);

  // Description:
  // Set/Get Superquadric isotropic size.
  vtkSetMacro(Size,float);
  vtkGetMacro(Size,float);

  // Description:
  // Set/Get whether or not the superquadric is toroidal (1) or episoidal (0).
  vtkBooleanMacro(Toroidal,int);
  vtkGetMacro(Toroidal,int);
  vtkSetMacro(Toroidal,int);

protected:
  vtkSuperquadricSource(int res=16);
  ~vtkSuperquadricSource() {};
  vtkSuperquadricSource(const vtkSuperquadricSource&) {};
  void operator=(const vtkSuperquadricSource&) {};

  int Toroidal;
  float Thickness;
  float Size;
  float PhiRoundness;
  float ThetaRoundness;
  void Execute();
  float Center[3];
  float Scale[3];
  int ThetaResolution;
  int PhiResolution;

};

#endif


