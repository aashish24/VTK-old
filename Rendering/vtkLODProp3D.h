/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkLODProp3D - level of detail 3D prop
// .SECTION Description
// vtkLODProp3D is a class to support level of detail rendering for Prop3D.
// Any number of mapper/property/texture items can be added to this object.
// Render time will be measured, and will be used to select a LOD based on
// the AllocatedRenderTime of this Prop3D. Depending on the type of the
// mapper/property, a vtkActor or a vtkVolume will be created behind the
// scenes. 

// .SECTION See Also
// vtkProp3D vtkActor vtkVolume vtkLODActor

#ifndef __vtkLODProp3D_h
#define __vtkLODProp3D_h

#include "vtkProp3D.h"
#include "vtkTransform.h"

class vtkRenderer;
class vtkMapper;
class vtkVolumeMapper;
class vtkProperty;
class vtkVolumeProperty;
class vtkTexture;

typedef struct
{
  vtkProp3D   *Prop3D;
  int         Prop3DType;
  int         ID;
  float       EstimatedTime;
  int         State;
} vtkLODProp3DEntry;

class VTK_EXPORT vtkLODProp3D : public vtkProp3D
{
public:
  // Description:
  // Create an instance of this class.
  static vtkLODProp3D *New();

  vtkTypeMacro(vtkLODProp3D,vtkProp3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Standard vtkProp method to get 3D bounds of a 3D prop
  float *GetBounds();
  void GetBounds(float bounds[6]) { this->vtkProp3D::GetBounds( bounds ); };

  // Description:
  // Do we need to ray cast this prop?
  int RequiresRayCasting();

  // Description:
  // Does this prop render into an image?
  int RequiresRenderingIntoImage();
 
  // Description:
  // Add a level of detail with a given mapper, property, texture, and
  // guess of rendering time.  The property and texture fields can be set
  // to NULL (the other methods are included for script access where null
  // variables are not allowed). The time field can be set to 0.0 indicating
  // that no initial guess for rendering time is being supplied.
  // The returned integer value is an ID that can be used later to delete
  // this LOD, or set it as the selected LOD.
  int AddLOD( vtkMapper *m, vtkProperty *p, vtkTexture *t, float time );
  int AddLOD( vtkMapper *m, vtkProperty *p, float time );
  int AddLOD( vtkMapper *m, vtkTexture *t, float time );
  int AddLOD( vtkMapper *m, float time );
  int AddLOD( vtkVolumeMapper *m, vtkVolumeProperty *p, float time );
  int AddLOD( vtkVolumeMapper *m, float time );

  // Description:
  // Delete a level of detail given an ID. This is the ID returned by the
  // AddLOD method
  void RemoveLOD( int id );

  // Description:
  // Methods to set / get the property of an LOD. Since the LOD could be
  // a volume or an actor, you have to pass in the pointer to the property
  // to get it. The returned property will be NULL if the id is not valid,
  // or the property is of the wrong type for the corresponding Prop3D.
  void SetLODProperty( int id, vtkProperty  *p );
  void GetLODProperty( int id, vtkProperty  **p );
  void SetLODProperty( int id, vtkVolumeProperty  *p );
  void GetLODProperty( int id, vtkVolumeProperty  **p );

  // Description:
  // Methods to set / get the mapper of an LOD. Since the LOD could be
  // a volume or an actor, you have to pass in the pointer to the mapper
  // to get it. The returned mapper will be NULL if the id is not valid,
  // or the mapper is of the wrong type for the corresponding Prop3D.
  void SetLODMapper( int id, vtkMapper  *m );
  void GetLODMapper( int id, vtkMapper  **m );
  void SetLODMapper( int id, vtkVolumeMapper  *m );
  void GetLODMapper( int id, vtkVolumeMapper  **m );

  // Description:
  // Methods to set / get the texture of an LOD. This method is only
  // valid for LOD ids that are Actors (not Volumes)
  void SetLODTexture( int id, vtkTexture *t );
  void GetLODTexture( int id, vtkTexture **t );

  // Description:
  // Enable / disable a particular LOD. If it is disabled, it will not
  // be used during automatic selection, but can be selected as the
  // LOD if automatic LOD selection is off.
  void EnableLOD( int id );
  void DisableLOD( int id );
  
  // Description:
  // Access method that can be used to find out the estimated render time
  // (the thing used to select an LOD) for a given LOD ID or index. 
  // Value is returned in seconds.
  float GetLODEstimatedRenderTime( int id );
  float GetLODIndexEstimatedRenderTime( int index );

  // Description:
  // Turn on / off automatic selection of LOD. 
  // This is on by default. If it is off, then the SelectedLODID is 
  // rendered regardless of rendering time or desired update rate. 
  vtkSetClampMacro( AutomaticLODSelection, int, 0, 1 );
  vtkGetMacro( AutomaticLODSelection, int );
  vtkBooleanMacro( AutomaticLODSelection, int );

  // Description:
  // Set the id of the LOD that is to be drawn when automatic LOD selection
  // is turned off.
  vtkSetMacro( SelectedLODID, int );
  vtkGetMacro( SelectedLODID, int );

  // Description:
  // Get the ID of the previously (during the last render) selected LOD index
  int GetLastRenderedLODID();

  // Description: 
  // For some exporters and other other operations we must be
  // able to collect all the actors or volumes. These methods
  // are used in that process.
  virtual void GetActors(vtkPropCollection *);

  // Description:
  // This method is invoked when an instance of vtkProp (or subclass, 
  // e.g., vtkActor) is picked by vtkPicker.
  void SetPickMethod(void (*f)(void *), void *arg);
  void SetPickMethodArgDelete(void (*f)(void *));

  // Description:
  // Set the id of the LOD that is to be used for picking when  automatic 
  // LOD pick selection is turned off.
  void SetSelectedPickLODID(int id);
  vtkGetMacro( SelectedPickLODID, int );

  // Description:
  // Turn on / off automatic selection of picking LOD. 
  // This is on by default. If it is off, then the SelectedLODID is 
  // rendered regardless of rendering time or desired update rate. 
  vtkSetClampMacro( AutomaticPickLODSelection, int, 0, 1 );
  vtkGetMacro( AutomaticPickLODSelection, int );
  vtkBooleanMacro( AutomaticPickLODSelection, int );

  // Description:
  // Shallow copy of this vtkLODProp3D.
  void ShallowCopy(vtkProp *prop);

//BTX

  // Description:
  // Support the standard render methods.
  int RenderOpaqueGeometry(vtkViewport *viewport);
  int RenderTranslucentGeometry(vtkViewport *viewport);
  int RenderIntoImage(vtkViewport *viewport);
  int CastViewRay( VTKRayCastRayInfo *rayInfo );
  int InitializeRayCasting( vtkViewport *viewport);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Used by the culler / renderer to set the allocated render time for this
  // prop. This is based on the desired update rate, and possibly some other
  // properties such as potential screen coverage of this prop.
  void SetAllocatedRenderTime( float t );

  // Description:
  // Override method from vtkProp in order to push this call down to the
  // selected LOD as well.
  virtual void AddEstimatedRenderTime( float t );

//ETX

protected:
  vtkLODProp3D();
  ~vtkLODProp3D();
  vtkLODProp3D(const vtkLODProp3D&) {};
  void operator=(const vtkLODProp3D&) {};

  int GetAutomaticPickPropIndex(void);

  vtkLODProp3DEntry *LODs;
  int               NumberOfEntries;
  int               NumberOfLODs;
  int               CurrentIndex;

  int               GetNextEntryIndex();
  int               ConvertIDToIndex( int id );
  int               SelectedLODIndex;

  int               AutomaticLODSelection;
  int               SelectedLODID;
  int               SelectedPickLODID;
  int               AutomaticPickLODSelection;
  vtkProp*          PreviousPickProp;
  void (*PreviousPickMethod)(void *);
  void *            PreviousPickMethodArg;
};

#endif

