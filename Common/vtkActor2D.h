/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Matt Turek who developed this class.

Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkActor2D - a actor that draws 2D data
// .SECTION Description
// vtkActor2D is similar to vtkActor, but it is made to be used with two
// dimensional images and annotation.  vtkActor2D has a position but does not
// use a transformation matrix like vtkActor (see the superclass vtkProp
// for information on positioning vtkActor2D).  vtkActor2D has a reference to
// a vtkMapper2D object which does the rendering.

// .SECTION See Also
// vtkProp  vtkMapper2D vtkProperty2D

#ifndef __vtkActor2D_h
#define __vtkActor2D_h

#include "vtkProp.h"
#include "vtkCoordinate.h"
#include "vtkProperty2D.h"
class vtkMapper2D;

class VTK_COMMON_EXPORT vtkActor2D : public vtkProp
{
public:
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkActor2D,vtkProp);

  // Description:
  // Creates an actor2D with the following defaults: 
  // position (0,0) (coordinate system is viewport);
  // at layer 0.
  static vtkActor2D* New();
  
  // Description:
  // Support the standard render methods.
  int RenderOverlay(vtkViewport *viewport);
  int RenderOpaqueGeometry(vtkViewport *viewport);
  int RenderTranslucentGeometry(vtkViewport *viewport);

  // Description:
  // Set/Get the vtkMapper2D which defines the data to be drawn.
  void SetMapper(vtkMapper2D *mapper);
  vtkGetObjectMacro(Mapper, vtkMapper2D);

  // Description:
  // Set/Get the layer number in the overlay planes into which to render.
  vtkSetMacro(LayerNumber, int);
  vtkGetMacro(LayerNumber, int);

  // Description:
  // Returns this actor's vtkProperty2D.  Creates a property if one
  // doesn't already exist.
  vtkProperty2D* GetProperty();

  // Description:
  // Set this vtkProp's vtkProperty2D.
  vtkSetObjectMacro(Property, vtkProperty2D);

  // Description:
  // Get the PositionCoordinate instance of vtkCoordinate.
  // This is used for for complicated or relative positioning.
  // The position variable controls the lower left corner of the Actor2D
  vtkViewportCoordinateMacro(Position);

  // Description:
  // Set the Prop2D's position in display coordinates.
  void SetDisplayPosition(int,int);

  // Description:
  // Access the Position2 instance variable. This variable controls
  // the upper right corner of the Actor2D. It is by default
  // relative to Position and in normalized viewport coordinates.
  // Some 2D actor subclasses ignore the position2 variable
  vtkViewportCoordinateMacro(Position2);

  // Description:
  // Set/Get the height and width of the Actor2D. The value is expressed
  // as a fraction of the viewport. This really is just another way of
  // setting the Position2 instance variable.
  void SetWidth(float w);
  float GetWidth();
  void SetHeight(float h);
  float GetHeight();

  // Description:
  // Return this objects MTime.
  unsigned long GetMTime();

  // Description:
  // For some exporters and other other operations we must be
  // able to collect all the actors or volumes. These methods
  // are used in that process.
  virtual void GetActors2D(vtkPropCollection *pc);

  // Description:
  // Shallow copy of this vtkActor2D. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *);

protected:
  vtkActor2D();
  ~vtkActor2D();

  vtkMapper2D *Mapper;
  int LayerNumber;
  vtkProperty2D *Property;
  vtkCoordinate *PositionCoordinate;
  vtkCoordinate *Position2Coordinate;
private:
  vtkActor2D(const vtkActor2D&);  // Not implemented.
  void operator=(const vtkActor2D&);  // Not implemented.
};

#endif



