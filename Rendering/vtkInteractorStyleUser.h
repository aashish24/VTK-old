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

// .NAME vtkInteractorStyleUser - provides customizable interaction routines
// 
// .SECTION Description
// The most common way to customize user interaction is to write a subclass
// of vtkInteractorStyle: vtkInteractorStyleUser allows you to customize
// the interaction to without subclassing vtkInteractorStyle.  This is
// particularly useful for setting up custom interaction modes in
// scripting languages such as Tcl and Python.  To customize the
// interaction for a particular button, you must call the
// SetXXButtonPressMethod()/SetXXButtonReleaseMethod() (See the documentation
// for vtkInteractorStyle) as well as SetMouseMoveMethod().

#ifndef __vtkInteractorStyleUser_h
#define __vtkInteractorStyleUser_h

#include "vtkInteractorStyleSwitch.h"

// new motion flag
#define VTKIS_USERINTERACTION 8 

class VTK_EXPORT vtkInteractorStyleUser : public vtkInteractorStyleSwitch 
{
public:
  static vtkInteractorStyleUser *New();
  vtkTypeMacro(vtkInteractorStyleUser,vtkInteractorStyleSwitch);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set a method that will be called every time the mouse is
  // moved.  You can use GetLastPos() to determine the position of
  // the cursor in display coordinates, and GetOldPos() to determine
  // the previous position.  Use GetButton() to query which mouse
  // button is being held down.  This should be used in conjunction
  // with SetXXButtonPressMethod()/SetXXButtonReleaseMethod() in
  // vtkInteractorStyle.
  void SetMouseMoveMethod(void (*f)(void *), void *arg);
  void SetMouseMoveMethodArgDelete(void (*f)(void *));

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
  // This method behaves just like OnTimer, but is only called if the
  // mouse has moved.  Deprecated, do not use.
  void SetUserInteractionMethod(void (*f)(void *), void *arg);
  void SetUserInteractionMethodArgDelete(void (*f)(void *));

  // Description: 
  // Start/Stop user interaction mode.  You must not call these methods
  // before you have Initialized the vtkRenderWindowInteractor.  Deprecated,
  // do not use.
  void StartUserInteraction();
  void EndUserInteraction();

protected:
  vtkInteractorStyleUser();
  ~vtkInteractorStyleUser();
  vtkInteractorStyleUser(const vtkInteractorStyleUser&) {};
  void operator=(const vtkInteractorStyleUser&) {};

  void OnChar(int ctrl, int shift, char keycode, int repeatcount);
  void OnKeyPress(int ctrl, int shift, char keycode, char *keysym,
		  int repeatcount);
  void OnKeyRelease(int ctrl, int shift, char keycode, char *keysym,
		    int repeatcount);

  void OnLeftButtonDown(int ctrl, int shift, int X, int Y);
  void OnLeftButtonUp(int ctrl, int shift, int X, int Y);
  void OnMiddleButtonDown(int ctrl, int shift, int X, int Y);
  void OnMiddleButtonUp(int ctrl, int shift, int X, int Y);
  void OnRightButtonDown(int ctrl, int shift, int X, int Y);
  void OnRightButtonUp(int ctrl, int shift, int X, int Y);

  void OnMouseMove(int ctrl, int shift, int X, int Y);

  void OnConfigure(int width, int height);
 
  void OnEnter(int ctrl, int shift, int X, int Y);
  void OnLeave(int ctrl, int shift, int X, int Y);

  void OnTimer(void);

  int OldPos[2];
  int UserInteractionPos[2];

  int Char;
  char *KeySym;
  int Button;

  void (*MouseMoveMethod)(void *);
  void (*MouseMoveMethodArgDelete)(void *);
  void *MouseMoveMethodArg;

  void (*KeyPressMethod)(void *);
  void (*KeyPressMethodArgDelete)(void *);
  void *KeyPressMethodArg;

  void (*KeyReleaseMethod)(void *);
  void (*KeyReleaseMethodArgDelete)(void *);
  void *KeyReleaseMethodArg;  

  void (*CharMethod)(void *);
  void (*CharMethodArgDelete)(void *);
  void *CharMethodArg;

  void (*EnterMethod)(void *);
  void (*EnterMethodArgDelete)(void *);
  void *EnterMethodArg;

  void (*LeaveMethod)(void *);
  void (*LeaveMethodArgDelete)(void *);
  void *LeaveMethodArg;

  void (*ConfigureMethod)(void *);
  void (*ConfigureMethodArgDelete)(void *);
  void *ConfigureMethodArg;

  void (*TimerMethod)(void *);
  void (*TimerMethodArgDelete)(void *);
  void *TimerMethodArg;

  void (*UserInteractionMethod)(void *);
  void (*UserInteractionMethodArgDelete)(void *);
  void *UserInteractionMethodArg;
};

#endif
