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
// .NAME vtkProp - abstract superclass for all actors, volumes and annotations
// .SECTION Description
// vtkProp is an abstract superclass for actor type objects. Instances of
// vtkProp may respond to RenderGeometry, RenderVolume, and RenderPostSwap
// calls and may repond to more than one.
//
// vt
// .SECTION See Also
// vtkActor2D  vtkActor vtkVolume

#ifndef __vtkProp_h
#define __vtkProp_h

#include "vtkObject.h"
#include "vtkRayCastStructures.h"

class vtkViewport;
class vtkPropCollection;
class vtkWindow;

class VTK_EXPORT vtkProp : public vtkObject
{
public:
  // Description:
  // Creates an instance with visibility=1, pickable=1,
  // and dragable=1.
  static vtkProp* New();

  vtkTypeMacro(vtkProp,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // For some exporters and other other operations we must be
  // able to collect all the actors or volumes. These methods
  // are used in that process.
  virtual void GetActors(vtkPropCollection *) {};
  virtual void GetActors2D(vtkPropCollection *) {};
  virtual void GetVolumes(vtkPropCollection *) {};

  // Description:
  // Set/Get visibility of this vtkProp.
  vtkSetMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkBooleanMacro(Visibility, int);

  // Description:
  // Set/Get the pickable instance variable.  This determines if the vtkProp
  // can be picked (typically using the mouse). Also see dragable.
  vtkSetMacro(Pickable,int);
  vtkGetMacro(Pickable,int);
  vtkBooleanMacro(Pickable,int);

  // Description:
  // This method is invoked when an instance of vtkProp (or subclass, 
  // e.g., vtkActor) is picked by vtkPicker.
  void SetPickMethod(void (*f)(void *), void *arg);
  void SetPickMethodArgDelete(void (*f)(void *));

  // Description:
  // Method invokes PickMethod() if one defined and the prop is picked.
  virtual void Pick();

  // Description:
  // Set/Get the value of the dragable instance variable. This determines if 
  // an Prop, once picked, can be dragged (translated) through space.
  // This is typically done through an interactive mouse interface.
  // This does not affect methods such as SetPosition, which will continue
  // to work.  It is just intended to prevent some vtkProp'ss from being
  // dragged from within a user interface.
  vtkSetMacro(Dragable,int);
  vtkGetMacro(Dragable,int);
  vtkBooleanMacro(Dragable,int);

  // Description:
  // Return the mtime of anything that would cause the rendered image to 
  // appear differently. Usually this involves checking the mtime of the 
  // prop plus anything else it depends on such as properties, textures
  // etc.
  virtual unsigned long GetRedrawMTime() {return this->GetMTime();};
  
  // Description:
  // Get the bounds for this Prop as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
  // in world coordinates. NULL means that the bounds are not defined.
  virtual float *GetBounds() {return NULL;};

  // Description:
  // Shallow copy of this vtkProp.
  void ShallowCopy(vtkProp *Prop);

//BTX  
  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THESE METHODS OUTSIDE OF THE RENDERING PROCESS
  // Do we need to ray cast this prop?
  // For certain types of props (vtkLODProp3D for example) this
  // method is not guaranteed to work outside of the rendering
  // process, since a level of detail must be selected before this
  // question can be answered.
  virtual int RequiresRayCasting() { return 0; };

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THESE METHODS OUTSIDE OF THE RENDERING PROCESS
  // Does this prop render into an image?
  // For certain types of props (vtkLODProp3D for example) this
  // method is not guaranteed to work outside of the rendering
  // process, since a level of detail must be selected before this
  // question can be answered.
  virtual int RequiresRenderingIntoImage() { return 0; };

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THESE METHODS OUTSIDE OF THE RENDERING PROCESS
  // All concrete subclasses must be able to render themselves.
  // There are five key render methods in vtk and they correspond
  // to five different points in the rendering cycle. Any given
  // prop may implement one or more of these methods. The first two 
  // methods are designed to render 3D geometry such as polygons
  // lines, triangles. We render the opaque first then the transparent.
  // Ray casting is different from the other methods in that the
  // rendering process is driven not by the mapper but by the ray caster.
  // Two methods are required to support ray casting - one for 
  // initialization, and the other to cast a ray (in viewing coordinates.)
  // The next three methods are primarily intended for volume rendering
  // and supports any technique that returns an image to be composited.  
  // The RenderIntoImage() method causes the rendering to occur, and the
  // GetRGBAImage() and GetZImage() methods are used to gather results.
  // The last method is to render any 2D annotation or overlays.
  // Except for the ray casting methods, these methods return an integer value 
  // indicating whether or not this render method was applied to this 
  // data. For the ray cast initialization, the integer indicated whether or
  // not the initialization was successful. For ray casting, the integer 
  // return value indicates whether or not the ray intersected something.
  virtual int RenderOpaqueGeometry(      vtkViewport *) { return 0; };
  virtual int RenderTranslucentGeometry( vtkViewport *) { return 0; };
  virtual int InitializeRayCasting(      vtkViewport *) { return 0; };
  virtual int CastViewRay(         VTKRayCastRayInfo *) { return 0; };
  virtual int RenderIntoImage(           vtkViewport *) { return 0; };
  virtual float *GetRGBAImage()                         { return NULL; };
  virtual float *GetZImage()                            { return NULL; };
  virtual int RenderOverlay(             vtkViewport *) { return 0; };

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *) {};

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THESE METHODS OUTSIDE OF THE RENDERING PROCESS
  // The EstimatedRenderTime may be used to select between different props,
  // for example in LODProp it is used to select the level-of-detail.
  // The value is returned in seconds. For simple geometry the accuracy may
  // not be great due to buffering. For ray casting, which is already
  // multi-resolution, the current resolution of the image is factored into
  // the time.
  vtkGetMacro( EstimatedRenderTime, float );

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // This method is intended to allow the renderer to add to the
  // EstimatedRenderTime in props that require information that
  // the renderer has in order to do this. For example, props
  // that are rendered with a ray casting method do not know
  // themselves how long it took for them to render. We don't want to
  // cause a this->Modified() when we set this value since it is not
  // really a modification to the object. (For example, we don't want
  // to rebuild matrices at every render because the estimated render time
  // is changing)
  virtual void AddEstimatedRenderTime(float t){this->EstimatedRenderTime+=t;};

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // The renderer may use the allocated rendering time to determine
  // how to render this actor. 
  // The set method is not a macro in order to avoid resetting the mtime of
  // the prop - otherwise the prop would have been modified during every 
  // render.
  // A side effect of this method is to reset the EstimatedRenderTime to
  // 0.0. This way, each of the ways that this prop may be rendered can
  // be timed and added together into this value.
  virtual void SetAllocatedRenderTime(float t) {
    this->AllocatedRenderTime = t;
    this->EstimatedRenderTime = 0.0;
  };

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  vtkGetMacro(AllocatedRenderTime, float);

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS
  // Get/Set the multiplier for the render time. This is used
  // for culling and is a number between 0 and 1. It is used
  // to create the allocated render time value.
  void SetRenderTimeMultiplier( float t ) { this->RenderTimeMultiplier = t; };
  vtkGetMacro(RenderTimeMultiplier, float);

//ETX

protected:
  vtkProp();
  ~vtkProp();
  vtkProp(const vtkProp&) {};
  void operator=(const vtkProp&) {};

  int Visibility;
  int Pickable;
  void (*PickMethod)(void *);
  void (*PickMethodArgDelete)(void *);
  void *PickMethodArg;
  int Dragable;

  float AllocatedRenderTime;
  float EstimatedRenderTime;
  float RenderTimeMultiplier;
};

#endif


