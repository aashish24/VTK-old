/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMeasureRepresentation - represent the vtkMeasureWidget
// .SECTION Description
// The vtkMeasureRepresentation is a superclass for various types of
// representations for the vtkMeasureWidget. Logically subclasses consist of
// an axis and two handles for placing/manipulating the end points.

// .SECTION See Also
// vtkMeasureWidget vtkHandleRepresentation vtkMeasureRepresentation2D vtkMeasureRepresentation


#ifndef __vtkMeasureRepresentation_h
#define __vtkMeasureRepresentation_h

#include "vtkWidgetRepresentation.h"

class vtkHandleRepresentation;


class VTK_WIDGETS_EXPORT vtkMeasureRepresentation : public vtkWidgetRepresentation
{
public:
  // Description:
  // Standard VTK methods.
  vtkTypeRevisionMacro(vtkMeasureRepresentation,vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Methods to Set/Get the coordinates of the two points defining
  // this representation. Note that methods are available for both
  // display and world coordinates.
  virtual void GetPoint1WorldPosition(double pos[3]) = 0;
  virtual void GetPoint2WorldPosition(double pos[3]) = 0;
  virtual void SetPoint1DisplayPosition(double pos[3]) = 0;
  virtual void SetPoint2DisplayPosition(double pos[3]) = 0;
  virtual void GetPoint1DisplayPosition(double pos[3]) = 0;
  virtual void GetPoint2DisplayPosition(double pos[3]) = 0;

  // Description:
  // This method is used to specify the type of handle representation to
  // use for the two internal vtkHandleWidgets within vtkMeasureWidget.
  // To use this method, create a dummy vtkHandleWidget (or subclass),
  // and then invoke this method with this dummy. Then the 
  // vtkMeasureRepresentation uses this dummy to clone two vtkHandleWidgets
  // of the same type. Make sure you set the handle representation before
  // the widget is enabled. (The method InstantiateHandleRepresentation()
  // is invoked by the vtkMeasure widget.)
  void SetHandleRepresentation(vtkHandleRepresentation *handle);
  void InstantiateHandleRepresentation();

  // Description:
  // Set/Get the two handle representations used for the vtkMeasureWidget.
  vtkGetObjectMacro(Point1Representation,vtkHandleRepresentation);
  vtkGetObjectMacro(Point2Representation,vtkHandleRepresentation);

  // Description:
  // The tolerance representing the distance to the widget (in pixels) in
  // which the cursor is considered near enough to the end points of
  // thewidget to be active.
  vtkSetClampMacro(Tolerance,int,1,100);
  vtkGetMacro(Tolerance,int);

//BTX -- used to communicate about the state of the representation
  enum {Outside=0,NearP1,NearP2};
//ETX

  // Description:
  // These are methods that satisfy vtkWidgetRepresentation's API.
  virtual void BuildRepresentation();
  virtual int ComputeInteractionState(int X, int Y, int modify=0);
  virtual void StartWidgetInteraction(double e[2]);
  virtual void WidgetInteraction(double e[2]);
  
protected:
  vtkMeasureRepresentation();
  ~vtkMeasureRepresentation();

  // The handle and the rep used to close the handles
  vtkHandleRepresentation *HandleRepresentation;
  vtkHandleRepresentation *Point1Representation;
  vtkHandleRepresentation *Point2Representation;

  // Selection tolerance for the handles
  int Tolerance;

private:
  vtkMeasureRepresentation(const vtkMeasureRepresentation&);  //Not implemented
  void operator=(const vtkMeasureRepresentation&);  //Not implemented
};

#endif
