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
// .NAME vtkActor - represents an object (geomtry & properties) in a scene 
// .SECTION Description
// vtkActor is used to represent an entity in a rendering scene.  It
// handles functions related to the actors position, orientation and
// scaling. It combines these instance variables into one four by four
// transformation matrix as follows: [x y z 1] = [x y z 1]
// Translate(-origin) Scale(scale) Rot(y) Rot(x) Rot (z) Trans(origin)
// Trans(position). The actor also maintains a reference to the
// defining geometry (i.e., the mapper), rendering properties and
// possibly a texture map.

// .SECTION See Also
// vtkProperty vtkTexture vtkMapper

#ifndef __vtkActor_hh
#define __vtkActor_hh

#include "vtkObject.hh"
#include "vtkProperty.hh"
#include "vtkTexture.hh"
#include "vtkMapper.hh"
#include "vtkTransform.hh"

class vtkRenderer;

class vtkActor : public vtkObject
{
 public:
  vtkActor();
  ~vtkActor();
  char *GetClassName() {return "vtkActor";};
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Render(vtkRenderer *ren);

  // Description: 
  // Set/Get the property object that controls this
  // actors surface properties.  This is should be an instance of a
  // vtkProperty object.  Every Actor must have a property associated
  // with it.  If one isn't specified then one will be generated
  // automatically. Multiple actors can share one property object.
  void SetProperty(vtkProperty *lut);
  void SetProperty(vtkProperty& lut) {this->SetProperty(&lut);};
  vtkProperty *GetProperty();

  // Description: 
  // Set/Get the Texture object to control rendering
  // texture maps.  This will be a vtkTexture object. An actor does
  // not need to have an associated texture map and mutliple actors
  // can share one texture.
  vtkSetObjectMacro(Texture,vtkTexture);
  vtkGetObjectMacro(Texture,vtkTexture);

  // Description:
  // This is the method that is used to connect an actor to the end of a
  // visualization pipeline, i.e. the Mapper. This should be a subclass
  // of vtkMapper. Typically vtkPolyMapper and vtkDataSetMapper will
  // be used.
  vtkSetObjectMacro(Mapper,vtkMapper);

  // Description:
  // Returns the Mapper that this actor is getting it's data from.
  vtkGetObjectMacro(Mapper,vtkMapper);

  // Description:
  // In addition to the instance variables such as position and orientation,
  // you can specify your own four by four transformation matrix that will
  // get concatenated with the actor's four by four matrix as determined
  // by the other instance variables. If the other instance variables such
  // as position and orientation are left with  their default values then 
  // they will result in the identity matrix. And the resulting matrix
  // will be the user defined matrix.
  vtkSetObjectMacro(UserMatrix,vtkMatrix4x4);
  vtkGetObjectMacro(UserMatrix,vtkMatrix4x4);

  // Description:
  // Set/Get/Add the position of the actor in world coordinates.
  vtkSetVector3Macro(Position,float);
  vtkGetVectorMacro(Position,float,3);
  void AddPosition(float deltaPosition[3]);
  void AddPosition(float deltaX,float deltaY,float deltaZ);

  // Description:
  // Set/Get the origin of the actor. This is the point about which all 
  // rotations take place.
  vtkSetVector3Macro(Origin,float);
  vtkGetVectorMacro(Origin,float,3);

  // Description:
  // Set/Get the scale of the actor. Scaling in performed independently on the
  // X, Y and Z axis. A scale of zero is illegal and will be replace with one.
  vtkSetVector3Macro(Scale,float);
  vtkGetVectorMacro(Scale,float,3);

  // Description:
  // Set/Get the visibility of the actor. Visibility is like a light switch
  // for actors. Use it to turn them on or off.
  vtkSetMacro(Visibility,int);
  vtkGetMacro(Visibility,int);
  vtkBooleanMacro(Visibility,int);

  // Description:
  // Set/Get the pickable instance variable.  This determines if the actor can 
  // be picked (typically using the mouse). Also see dragable.
  vtkSetMacro(Pickable,int);
  vtkGetMacro(Pickable,int);
  vtkBooleanMacro(Pickable,int);

  // Description:
  // Set/Get the value of the dragable instance variable. This determines if 
  // an actor once picked, can be dragged (translated) through space.
  // This is typically done through an interactive mouse interface.
  // This does not affect methods such as SetPosition which will continue
  // to work.  It is just intended to prevent some actors from being
  // dragged from within a user interface..
  vtkSetMacro(Dragable,int);
  vtkGetMacro(Dragable,int);
  vtkBooleanMacro(Dragable,int);

  vtkMatrix4x4& GetMatrix();
  virtual void GetMatrix(vtkMatrix4x4& m);

  float *GetBounds();
  float *GetCenter();
  float *GetXRange();
  float *GetYRange();
  float *GetZRange();

  void RotateX(float);
  void RotateY(float);
  void RotateZ(float);
  void RotateWXYZ(float,float,float,float);

  void SetOrientation(float,float,float);
  void SetOrientation(float a[3]);
  float *GetOrientation();
  void AddOrientation(float,float,float);
  void AddOrientation(float a[3]);

protected:
  vtkMatrix4x4 *UserMatrix;
  vtkProperty *Property; 
  vtkTexture *Texture; 
  vtkMapper *Mapper;
  float Origin[3];
  float Position[3];
  float Orientation[3];
  float Scale[3];
  int   Visibility;
  int   Pickable;
  int   Dragable;
  vtkTransform Transform;
  float Bounds[6];
  int SelfCreatedProperty;
  float Center[3];
};

#endif

