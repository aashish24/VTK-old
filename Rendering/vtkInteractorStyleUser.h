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

// .NAME vtkInteractorStyleUser - provides customizable interaction routines
// 
// .SECTION Description
// The most common way to customize user interaction is to write a subclass
// of vtkInteractorStyle: vtkInteractorStyleUser allows you to customize
// the interaction to without subclassing vtkInteractorStyle.  This is
// particularly useful for setting up custom interaction modes in
// scripting languages such as Tcl and Python.  This class allows you
// to hook into the MouseMove, ButtonPress/Release, KeyPress/Release,
// etc. events.  If you want to hook into just a single mouse button,
// but leave the interaction modes for the others unchanged, you
// must use e.g. SetMiddleButtonPressMethod() instead of the more
// general SetButtonPressMethod().

#ifndef __vtkInteractorStyleUser_h
#define __vtkInteractorStyleUser_h

#include "vtkInteractorStyle.h"

// new motion flag
#define VTKIS_USERINTERACTION 8 

class VTK_RENDERING_EXPORT vtkInteractorStyleUser : public vtkInteractorStyle 
{
public:
  static vtkInteractorStyleUser *New();
  vtkTypeRevisionMacro(vtkInteractorStyleUser,vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set a method that will be called every time the mouse is
  // moved.  You can use GetLastPos() to determine the position of
  // the cursor in display coordinates, and GetOldPos() to determine
  // the previous position.  Use GetButton() to query which mouse
  // button is being held down.  This should be used in conjunction
  // with SetButtonPressMethod()/SetButtonReleaseMethod() or with the
  // individual SetXXButtonPressMethods in vtkInteractorStyle.
  void SetMouseMoveMethod(void (*f)(void *), void *arg);
  void SetMouseMoveMethodArgDelete(void (*f)(void *));

  // Description:
  // Set a method that will be called whenever a mouse button is
  // pressed.  Use GetButton() to query which button was pressed.
  // This simply calls SetLeftButtonPressMethod(method), 
  // SetMiddleButtonPressMethod(method), 
  // SetRightButtonPressMethod(method).
  void SetButtonPressMethod(void (*f)(void *), void *arg);
  void SetButtonPressMethodArgDelete(void (*f)(void *));

  // Description:
  // Set a method that will be called whenever a mouse button is
  // released.  Use GetButton() to query which button was released.
  // This simply calls SetLeftButtonReleaseMethod(method), 
  // SetMiddleButtonReleaseMethod(method), 
  // SetRightButtonReleaseMethod(method).
  void SetButtonReleaseMethod(void (*f)(void *), void *arg);
  void SetButtonReleaseMethodArgDelete(void (*f)(void *));

  // Description:
  // Set a method that will be called every time a key is pressed.
  // Use GetKeySym() to find out which key was pressed.  They keystroke
  // is also converted into a character, which can be retrieved using
  // GetChar().
  void SetKeyPressMethod(void (*f)(void *), void *arg);
  void SetKeyPressMethodArgDelete(void (*f)(void *));

  // Description:
  // Set a method that will be called every time a key is released.
  // Use GetKeySym to find out which key was released.  They keystroke
  // is also converted into a character, which can be retrieved using
  // GetChar().
  void SetKeyReleaseMethod(void (*f)(void *), void *arg);
  void SetKeyReleaseMethodArgDelete(void (*f)(void *));

  // Description:
  // Set a method that will be called every time a character is
  // received.  This is not the same as the KeyPress method, which
  // is called when any key (including shift or control) is pressed.
  // Use GetChar() to find out which char was received.
  void SetCharMethod(void (*f)(void *), void *arg);
  void SetCharMethodArgDelete(void (*f)(void *));

  // Description:
  // Set methods that will be called when the size of the render
  // window changes (this method is called just before the window
  // re-renders after the size change).  Call GetSize() on the
  // interactor to find out the new size.
  void SetConfigureMethod(void (*f)(void *), void *arg);
  void SetConfigureMethodArgDelete(void (*f)(void *));

  // Description:
  // Set methods that will be called when some part of the render
  // window is exposed.
  void SetExposeMethod(void (*f)(void *), void *arg);
  void SetExposeMethodArgDelete(void (*f)(void *));

  // Description:
  // Set methods to be called when the mouse enters or leaves
  // the window.  Use GetLastPos() to determine where the mouse
  // pointer was when the event occurred.
  void SetEnterMethod(void (*f)(void *), void *arg);
  void SetEnterMethodArgDelete(void (*f)(void *));
  void SetLeaveMethod(void (*f)(void *), void *arg);
  void SetLeaveMethodArgDelete(void (*f)(void *));

  // Description:
  // Set a method that will be called continuously at a fairly rapid
  // rate (fast enough to be used for interaction).  For this method
  // to work, it must be called after the RenderWindowInteractor
  // has been Initialized.
  void SetTimerMethod(void (*f)(void *), void *arg);
  void SetTimerMethodArgDelete(void (*f)(void *));

  // Description:
  // Get the most recent mouse position during mouse motion.  
  // In your user interaction method, you must use this to track
  // the mouse movement.  Do not use GetEventPosition(), which records
  // the last position where a mouse button was pressed.
  vtkGetVector2Macro(LastPos,int);

  // Description:
  // Get the previous mouse position during mouse motion, or after
  // a key press.  This can be used to calculate the relative 
  // displacement of the mouse.
  vtkGetVector2Macro(OldPos,int);

  // Description:
  // Test whether modifiers were held down when mouse button or key
  // was pressed
  vtkGetMacro(ShiftKey,int);
  vtkGetMacro(CtrlKey,int);

  // Description:
  // Get the character for a Char event.
  vtkGetMacro(Char,int);

  // Description:
  // Get the KeySym (in the same format as Tk KeySyms) for a 
  // KeyPress or KeyRelease method.
  vtkGetStringMacro(KeySym);

  // Description:
  // Get the mouse button that was last pressed inside the window
  // (returns zero when the button is released).
  vtkGetMacro(Button,int);

  // Description:
  // This method behaves just like OnTimer, but is only called if 
  // StartUserInteraction has been called.  This method cannot be
  // used in conjunction with SetMouseMoveMethod.  Deprecated,
  // do not use.
  void SetUserInteractionMethod(void (*f)(void *), void *arg);
  void SetUserInteractionMethodArgDelete(void (*f)(void *));

  // Description: 
  // Start/Stop user interaction mode.  You must not call these methods
  // before you have Initialized the vtkRenderWindowInteractor.  Deprecated,
  // do not use.
  void StartUserInteraction();
  void EndUserInteraction();

  // Description:
  // Generic event bindings
  virtual void OnMouseMove       (int ctrl, int shift, int x, int y);
  virtual void OnLeftButtonDown  (int ctrl, int shift, int x, int y);
  virtual void OnLeftButtonUp    (int ctrl, int shift, int x, int y);
  virtual void OnMiddleButtonDown(int ctrl, int shift, int x, int y);
  virtual void OnMiddleButtonUp  (int ctrl, int shift, int x, int y);
  virtual void OnRightButtonDown (int ctrl, int shift, int x, int y);
  virtual void OnRightButtonUp   (int ctrl, int shift, int x, int y);


  // Description:
  // Keyboard functions
  virtual void OnChar      (int ctrl, int shift, char keycode, 
                            int repeatcount);
  virtual void OnKeyPress  (int ctrl, int shift, char keycode, char *keysym,
                            int repeatcount);
  virtual void OnKeyRelease(int ctrl, int shift, char keycode, char *keysym,
                            int repeatcount);


  // Description:
  // These are more esoteric events, but are useful in some cases.
  virtual void OnExpose   (int x, int y, int width, int height);
  virtual void OnConfigure(int width, int height);
  virtual void OnEnter    (int x, int y);
  virtual void OnLeave    (int x, int y);

  virtual void OnTimer(void);


protected:
  vtkInteractorStyleUser();
  ~vtkInteractorStyleUser();

  int OldPos[2];

  int Char;
  char *KeySym;
  int Button;

  unsigned long MouseMoveTag;
  unsigned long KeyPressTag;
  unsigned long KeyReleaseTag;
  unsigned long CharTag;
  unsigned long EnterTag;
  unsigned long LeaveTag;
  unsigned long ExposeTag;
  unsigned long ConfigureTag;
  unsigned long TimerTag;
  unsigned long UserTag;

  void vtkSetOldCallback(unsigned long &tag, unsigned long event, 
                         void (*f)(void *), void *arg);
  void vtkSetOldDelete(unsigned long tag, void (*f)(void *));
private:
  vtkInteractorStyleUser(const vtkInteractorStyleUser&);  // Not implemented.
  void operator=(const vtkInteractorStyleUser&);  // Not implemented.
};

#endif
