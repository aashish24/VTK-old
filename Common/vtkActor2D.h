/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Matt Turek who developed this class.

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
// .NAME vtkActor2D - a actor that draws 2D data
// .SECTION Description
// vtkActor2D is similar to vtkActor, but it is made to be used with two
// dimensional images and annotation.  vtkActor2D has a position but does not
// use a transformation matrix like vtkActor.  vtkActor2D has a reference to
// a vtkMapper2D object which does the rendering.

// .SECTION See Also
// vtkProp  vtkMapper2D vtkProperty2D

#ifndef __vtkActor2D_h
#define __vtkActor2D_h

#include "vtkProp.h"
#include "vtkCoordinate.h"
#include "vtkProperty2D.h"
class vtkMapper2D;

class VTK_EXPORT vtkActor2D : public vtkProp
{
public:
  static vtkActor2D* New() {return new vtkActor2D;};
  void PrintSelf(ostream& os, vtkIndent indent);
  const char *GetClassName() {return "vtkActor2D";};

  // Description:
  // Support the standard render methods.
  virtual int RenderOverlay(vtkViewport *viewport);
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentGeometry(vtkViewport *viewport);

  // Description:
  // Creates an actor2D with the following defaults: 
  // position -1, -1 (view coordinates)
  // orientation 0, scale (1,1), layer 0, visibility on
  vtkActor2D();

  // Description:
  // Destroy an actor2D.
  ~vtkActor2D();
  
  // Description:
  // Set/Get the vtkMapper2D which defines the data to be drawn.
  void SetMapper(vtkMapper2D *mapper);
  vtkGetObjectMacro(Mapper, vtkMapper2D);

  // Description:
  // Set/Get the layer number in the overlay planes into which to render.
  vtkSetMacro(LayerNumber, int);
  vtkGetMacro(LayerNumber, int);
  
  // Description:
  // Returns an Prop2D's property2D.  Creates a property if one
  // doesn't already exist.
  vtkProperty2D* GetProperty();

  // Description:
  // Set this vtkProp's vtkProperty2D.
  vtkSetObjectMacro(Property, vtkProperty2D);

  // Description:
  // Get the PositionCoordinate instance of vtkCoordinate.
  // This is used for for complicated or relative positioning.
  vtkViewportCoordinateMacro(Position);
  
  // Description:
  // Set the Prop2D's position in display coordinates.  
  void SetDisplayPosition(int,int);
  
  // Description:
  // Return this objects MTime.
  unsigned long GetMTime();

protected:
  vtkMapper2D *Mapper;
  int LayerNumber;
  vtkProperty2D *Property;
  vtkCoordinate *PositionCoordinate;
};

#endif



