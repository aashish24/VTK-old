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
#include "vtkOutputWindow.h"
#ifdef _WIN32
#include "vtkWin32OutputWindow.h"
#endif
#include "vtkObjectFactory.h"

vtkOutputWindow* vtkOutputWindow::Instance = 0;

void vtkOutputWindowDisplayText(const char* message)
{
  vtkOutputWindow::GetInstance()->DisplayText(message);
}

vtkOutputWindow::vtkOutputWindow()
{
  this->PromptUser = 0;
}

vtkOutputWindow::~vtkOutputWindow()
{
}

void vtkOutputWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkOutputWindow Single instance = "
     << (void*)vtkOutputWindow::Instance << endl;
  os << indent << "PromptUser Flag: " << this->PromptUser << endl;
}


// default implementation outputs to cerr only
void vtkOutputWindow::DisplayText(const char* txt)
{
  cerr << txt;
  if(this->PromptUser)
    {
    char c = 'n';
    cerr << "\nDo you want to suppress any further messages (y,n)?." << endl;
    cin >> c;
    if(c == 'y')
      {
      vtkObject::GlobalWarningDisplayOff(); 
      }
    }
}



class vtkOutputWindowSmartPointer
{
public:
  vtkOutputWindowSmartPointer() {};
  void SetPointer(vtkOutputWindow* obj)
    {
      Pointer = obj;
    }
  
  ~vtkOutputWindowSmartPointer() { Pointer->Delete(); }
private:
  vtkOutputWindow* Pointer;
};


// Up the reference count so it behaves like New
vtkOutputWindow* vtkOutputWindow::New()
{
  vtkOutputWindow* ret = vtkOutputWindow::GetInstance();
  ret->Register(NULL);
  return ret;
}


// Return the single instance of the vtkOutputWindow
vtkOutputWindow* vtkOutputWindow::GetInstance()
{
  // use this as a way of memory managment when the 
  // program exits the static will be deleted which
  // will delete the Instance singleton
  static vtkOutputWindowSmartPointer smartPointer;
  if(!vtkOutputWindow::Instance)
    {
    // Try the factory first
    vtkOutputWindow::Instance = (vtkOutputWindow*)
      vtkObjectFactory::CreateInstance("vtkOutputWindow");
    // if the factory did not provide one, then create it here
    if(!vtkOutputWindow::Instance)
      {
#ifdef _WIN32    
      vtkOutputWindow::Instance = vtkWin32OutputWindow::New();
#else
      vtkOutputWindow::Instance = new vtkOutputWindow;
#endif
      }
    // set the smart pointer to the instance, so
    // it will be UnRegister'ed at exit of the program
    smartPointer.SetPointer(vtkOutputWindow::Instance );
    }
  // return the instance
  return vtkOutputWindow::Instance;
}
