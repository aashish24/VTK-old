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

// .NAME vtkEncodedGradientShader - Compute shading tables for encoded normals.
//
// .SECTION Description
// vtkEncodedGradientShader computes shading tables for encoded normals 
// that indicates the amount of diffuse and specular illumination that is 
// recieved from all light sources at a surface location with that normal.
// For diffuse illumination this is accurate, but for specular illumination
// it is approximate for perspective projections since the center view
// direction is always used as the view direction. Since the shading table is
// dependent on the volume (for the transformation that must be applied to
// the normals to put them into world coordinates) there is a shading table
// per volume. This is necessary because multiple volumes can share a 
// volume mapper.

#ifndef __vtkEncodedGradientShader_h
#define __vtkEncodedGradientShader_h

#include "vtkObject.h"

class vtkVolume;
class vtkRenderer;
class vtkEncodedGradientEstimator;

#define VTK_MAX_SHADING_TABLES   100

class VTK_EXPORT vtkEncodedGradientShader : public vtkObject
{
public:
  static vtkEncodedGradientShader *New();
  vtkTypeMacro(vtkEncodedGradientShader,vtkObject);

  // Description:
  // Print the vtkEncodedGradientShader
  void PrintSelf( ostream& os, vtkIndent index );

  // Description:
  // Set / Get the intensity diffuse / specular light used for the
  // zero normals. 
  vtkSetClampMacro( ZeroNormalDiffuseIntensity,  float, 0.0, 1.0);
  vtkGetMacro( ZeroNormalDiffuseIntensity, float );
  vtkSetClampMacro( ZeroNormalSpecularIntensity, float, 0.0, 1.0);
  vtkGetMacro( ZeroNormalSpecularIntensity, float );

  // Description:
  // Cause the shading table to be updated
  void UpdateShadingTable( vtkRenderer *ren, vtkVolume *vol,
			   vtkEncodedGradientEstimator *gradest);

  // Description:
  // Get the red/green/blue shading table.
  float *GetRedDiffuseShadingTable(    vtkVolume *vol );
  float *GetGreenDiffuseShadingTable(  vtkVolume *vol );
  float *GetBlueDiffuseShadingTable(   vtkVolume *vol );
  float *GetRedSpecularShadingTable(   vtkVolume *vol );
  float *GetGreenSpecularShadingTable( vtkVolume *vol );
  float *GetBlueSpecularShadingTable(  vtkVolume *vol );

protected:
  vtkEncodedGradientShader();
  ~vtkEncodedGradientShader();
  vtkEncodedGradientShader(const vtkEncodedGradientShader&) {};
  void operator=(const vtkEncodedGradientShader&) {};

  // Description:
  // Build a shading table for a light with the specified direction,
  // and color for an object of the specified material properties.
  // material[0] = ambient, material[1] = diffuse, material[2] = specular
  // and material[3] = specular exponent.  If the ambient flag is 1, 
  // then ambient illumination is added. If not, then this means we 
  // are calculating the "other side" of two sided lighting, so no 
  // ambient intensity is added in. If the update flag is 0,
  // the shading table is overwritten with these new shading values.
  // If the updateFlag is 1, then the computed light contribution is
  // added to the current shading table values. There is one shading
  // table per volume, and the index value indicated which index table
  // should be used. It is computed in the UpdateShadingTable method.
  void  BuildShadingTable( int index,
			   float lightDirection[3],
			   float lightColor[3],
			   float lightIntensity,
			   float viewDirection[3],
			   float material[4],
			   int twoSided,
			   vtkEncodedGradientEstimator *gradest,
			   int updateFlag );
  
  // The six shading tables (r diffuse ,g diffuse ,b diffuse, 
  // r specular, g specular, b specular ) - with an entry for each
  // encoded normal plus one entry at the end for the zero normal
  // There is one shading table per volume listed in the ShadingTableVolume
  // array. A null entry indicates an available slot.
  float                        *ShadingTable[VTK_MAX_SHADING_TABLES][6];
  vtkVolume                    *ShadingTableVolume[VTK_MAX_SHADING_TABLES];
  int                          ShadingTableSize[VTK_MAX_SHADING_TABLES];

  // The intensity of light used for the zero normals, since it
  // can not be computed from the normal angles. Defaults to 0.0.
  float    ZeroNormalDiffuseIntensity;
  float    ZeroNormalSpecularIntensity;
}; 


#endif
