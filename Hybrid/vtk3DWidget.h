/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtk3DWidget - an abstract superclass for 3D widgets
// .SECTION Description
// vtk3DWidget is an abstract superclass for 3D interactor widgets. These
// 3D widgets represent themselves in the scene, and have special callbacks
// associated with them that allows interactive manipulation of the widget.
// 3D widgets also provide auxiliary functions like producing a transformation,
// creating polydata (for seeding streamlines, probes, etc.) or creating
// implicit functions. See the concrete subclasses for particulars.
//
// Typically the widget is used by specifying a vtkProp3D or VTK dataset as
// input, and then invoking the "On" method to activate it. (You can also
// specify a bounding box to help position the widget.) Prior to invoking the
// On() method, the user may also wish to use the PlaceWidget() to initially
// position it. The 'w' or 'W' (for "widget") keypresses also can be used to
// turn the widgets on and off.
// 
// To support interactive manipulation of objects, this class (and
// subclasses) invoke the events StartInteractionEvent, InteractionEvent, and
// EndInteractionEvent.  These events are invoked when the vtk3DWidget enters
// a state where rapid response is desired: mouse motion, etc. The events can
// be used, for example, to set the desired update frame rate
// (StartInteractionEvent), operate on the vtkProp3D or other object
// (InteractionEvent), and set the desired frame rate back to normal values
// (EndInteractionEvent).

// .SECTION See Also
// vtkBoxWidget vtkLineWidget

#ifndef __vtk3DWidget_h
#define __vtk3DWidget_h

#include "vtkRenderWindowInteractor.h"
#include "vtkCommand.h"

class vtkCallbackCommand;

class VTK_EXPORT vtk3DWidget : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtk3DWidget,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Turn the widget on.
  virtual void On() = 0;

  // Description:
  // Turn the widget off.
  virtual void Off() = 0;

  // Description:
  // This method is used to initially place the widget.
  // The placement of the widget depends on whether a
  // Prop3D or input dataset is provided. If one of these
  // two is provided, they will be used to obtain a bounding
  // box, around which the widget is placed. Otherwise,
  // you can manually specify a bounds with the 
  // PlaceWidget(bounds) method.
  virtual void PlaceWidget();

  // Description:
  // Place the widget within the bounding box provided.
  // Subclasses must provide this method.
  virtual void PlaceWidget(float bounds[6]) = 0;

  // Description:
  // Specify a vtkProp3D around which to place the widget. This 
  // is not required, but if supplied, it is used to initially 
  // position the widget.
  vtkSetObjectMacro(Prop3D,vtkProp3D);
  vtkGetObjectMacro(Prop3D,vtkProp3D);
  
  // Description:
  // Specify the input dataset. This is not required, but if supplied,
  // and no vtkProp3D is specified, it is used to initially position 
  // the widget.
  vtkSetObjectMacro(Input,vtkDataSet);
  vtkGetObjectMacro(Input,vtkDataSet);
  
  // Description:
  // This method is used to associate the widget with the render window interactor. 
  // Observers of the appropriate events invoked in the render window interactor are 
  // set up as a result of this method invocation.
  virtual void SetInteractor(vtkRenderWindowInteractor *interactor) = 0;
  vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);

  // Description:
  // Set/Get the priority at which events are processed. This is used when multiple
  // widgets are used simultaneously. The default value is 1.0 (highest priority.) Note
  // that when multiple widgets (or observers) have the same priority, then the last
  // observer added will process the event first.
  vtkSetClampMacro(Priority,float,0.0,1.0);
  vtkGetMacro(Priority,float);

  // Description:
  // Enable/Disable of the use of a keypress to turn on and off the widget. (By default,
  // the keypress is 'W' for "widget"...note the capital letter W. This may interfere 
  // with the 'W' key for wireframe in the interactor style.)
  vtkSetMacro(KeyPressActivation,int);
  vtkGetMacro(KeyPressActivation,int);
  vtkBooleanMacro(KeyPressActivation,int);
  
  // Description:
  // Specify which key press value to use to activate the widget (if key press
  // activation is enabled). By default, the key press activation value is 'w'.
  // Note: once the SetInteractor() method is invoked, changing the key press
  // activation value will not affect the key press until SetInteractor is 
  // called again.
  vtkSetMacro(KeyPressActivationValue,char);
  vtkGetMacro(KeyPressActivationValue,char);

//BTX
  // Description:
  // Various enums for controlling states and invoking events.
  enum WidgetVisibility
  {
    WidgetOff=0,
    WidgetOn
  };
  enum WidgetState
  {
    Start=0,
    Moving,
    Scaling,
    Outside
  };
//ETX

protected:
  vtk3DWidget();
  ~vtk3DWidget();

  // Sets up the keypress-W event. Should be invoked by subclass' ProcessEvents()
  void OnChar(int ctrl, int shift, char keycode, int repeatcount);
  
  // helper method for subclasses
  void ComputeDisplayToWorld(double x, double y, double z, double *worldPt);
  void ComputeWorldToDisplay(double x, double y, double z, double *displayPt);
    
  vtkProp3D *Prop3D;
  vtkDataSet *Input;
  
  // Maintain internal state in the widgets
  int Mode;
  int State;
  
  //has the widget ever been placed
  int Placed; 
  
  // Used to process events
  vtkCallbackCommand* WidgetCallbackCommand;

  // Priority at which events are processed
  float Priority;

  // Keypress activation
  int KeyPressActivation;
  char KeyPressActivationValue;

  // Used to associate observers with the interactor
  vtkRenderWindowInteractor *Interactor;
  
  // Internal ivars for processing events
  vtkRenderer *CurrentRenderer;
  vtkCamera *CurrentCamera;
  float OldX;
  float OldY;


private:
  vtk3DWidget(const vtk3DWidget&);  // Not implemented.
  void operator=(const vtk3DWidget&);  // Not implemented.
  
};

#endif
