/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


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

// .NAME vtkEncodedGradientEstimator - Superclass for gradient estimation
// .SECTION Description
// vtkEncodedGradientEstimator is an abstract superclass for gradient 
// estimation. It takes a scalar input of vtkStructuredPoints, computes
// a gradient value for every point, and encodes this value into a 
// three byte value (2 for direction, 1 for magnitude) using the 
// vtkDirectionEncoder. The direction encoder is defaulted to a
// vtkRecursiveSphereDirectionEncoder, but can be overridden with the
// SetDirectionEncoder method. The scale and the bias values for the gradient
// magnitude are used to convert it into a one byte value according to
// v = m*scale + bias where m is the magnitude and v is the resulting
// one byte value.
// .SECTION see also
// vtkFiniteDifferenceGradientEstimator vtkDirectionEncoder

#ifndef __vtkEncodedGradientEstimator_h
#define __vtkEncodedGradientEstimator_h

#include "vtkObject.h"
#include "vtkMultiThreader.h"
#include "vtkStructuredPoints.h"
#include "vtkDirectionEncoder.h"

class VTK_EXPORT vtkEncodedGradientEstimator : public vtkObject
{
public:
  vtkEncodedGradientEstimator();
  ~vtkEncodedGradientEstimator();
  const char *GetClassName() {return "vtkEncodedGradientEstimator";};
  void PrintSelf( ostream& os, vtkIndent index );

  // Description:
  // Set/Get the scalar input for which the normals will be 
  // calculated
  vtkSetObjectMacro( ScalarInput, vtkStructuredPoints );
  vtkGetObjectMacro( ScalarInput, vtkStructuredPoints );

  // Description:
  // Set/Get the scale and bias for the gradient magnitude
  vtkSetMacro( GradientMagnitudeScale, float );
  vtkGetMacro( GradientMagnitudeScale, float );
  vtkSetMacro( GradientMagnitudeBias, float );
  vtkGetMacro( GradientMagnitudeBias, float );

  // Description:
  // Recompute the encoded normals and gradient magnitudes.
  void  Update( void );

  // Description:
  // Get the encoded normals.
  unsigned short  *GetEncodedNormals( void ); 

  // Description:
  // Get the encoded normal at an x,y,z location in the volume
  int   GetEncodedNormalIndex( int xyz_index );
  int   GetEncodedNormalIndex( int x_index, int y_index, int z_index );

  // Description:
  // Get the gradient magnitudes
  unsigned char *GetGradientMagnitudes(void);

  // Description:
  // Get/Set the number of threads to create when encoding normals
  // This defaults to the number of available processors on the machine
  vtkSetClampMacro( NumberOfThreads, int, 1, VTK_MAX_THREADS );
  vtkGetMacro( NumberOfThreads, int );

  // Description:
  // Set / Get the direction encoder used to encode normal directions
  // to fit within two bytes
  void SetDirectionEncoder( vtkDirectionEncoder *direnc );
  vtkGetObjectMacro( DirectionEncoder, vtkDirectionEncoder );

  // Description:
  // If you don't want to compute gradient magnitudes (but you
  // do want normals for shading) this can be used. Be careful - if
  // if you a non-constant gradient magnitude transfer function and
  // you turn this on, it may crash
  vtkSetMacro( ComputeGradientMagnitudes, int );
  vtkGetMacro( ComputeGradientMagnitudes, int );
  vtkBooleanMacro( ComputeGradientMagnitudes, int );

  // Description:
  // If the data in each slice is only contained within a circle circumscribed
  // within the slice, and the slice is square, then don't compute anything
  // outside the cirle.
  vtkSetMacro( ClipOutsideCircle, int );
  vtkGetMacro( ClipOutsideCircle, int );
  vtkBooleanMacro( ClipOutsideCircle, int );

  // Description:
  // Get the time required for the last update in seconds or cpu seconds
  vtkGetMacro( LastUpdateTimeInSeconds, float );
  vtkGetMacro( LastUpdateTimeInCPUSeconds, float );

  vtkGetMacro( UseCircleClip, int );
  int *GetCircleLimits() { return this->CircleLimits; };

  // These variables should be protected but are being
  // made public to be accessible to the templated function.
  // We used to have the templated function as a friend, but
  // this does not work with all compilers

  // The input scalar data on which the normals are computed
  vtkStructuredPoints   *ScalarInput;

  // The encoded normals (2 bytes) and the size of the encoded normals
  unsigned short        *EncodedNormals;
  int                   EncodedNormalsSize[3];

  // The magnitude of the gradient array and the size of this array
  unsigned char         *GradientMagnitudes;

  // The time at which the normals were last built
  vtkTimeStamp          BuildTime;

  // The scale and bias values generally copied from the volume property
  float                 GradientMagnitudeScale;
  float                 GradientMagnitudeBias;

  // These are temporary variables used to avoid conflicts with
  // multi threading
  int                   ScalarInputSize[3];
  float                 ScalarInputAspect[3];

  int                   ComputeGradientMagnitudes;

protected:

  // The number of threads to use when encoding normals
  int                        NumberOfThreads;

  vtkMultiThreader           *Threader;

  vtkDirectionEncoder        *DirectionEncoder;

  virtual void               UpdateNormals( void ) = 0;

  float                      LastUpdateTimeInSeconds;
  float                      LastUpdateTimeInCPUSeconds;

  int                        ClipOutsideCircle;
  int                        *CircleLimits;
  int                        CircleLimitsSize;
  int                        UseCircleClip;
  void                       ComputeCircleLimits( int size );
}; 


#endif

