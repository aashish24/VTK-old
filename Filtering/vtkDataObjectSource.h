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
// .NAME vtkDataObjectSource - abstract class specifies interface for
//  field source (or objects that generate field output)

// .SECTION Description
// vtkDataObjectSource is an abstract object that specifies behavior and
// interface of field source objects. Field source objects are source objects
// that create vtkFieldData (field data) on output.
//
// Concrete subclasses of vtkDataObjectSource must define Update() and
// Execute() methods. The public method Update() invokes network execution
// and will bring the network up-to-date. The protected Execute() method
// actually does the work of data creation/generation. The difference between
// the two methods is that Update() implements input consistency checks and
// modified time comparisons and then invokes the Execute() which is an
// implementation of a particular algorithm.
//
// vtkDataObjectSource provides a mechanism for invoking the methods
// StartMethod() and EndMethod() before and after object execution (via
// Execute()). These are convenience methods you can use for any purpose
// (e.g., debugging info, highlighting/notifying user interface, etc.) These
// methods accept a single void* pointer that can be used to send data to the
// methods. It is also possible to specify a function to delete the argument
// via StartMethodArgDelete and EndMethodArgDelete.
//
// Another method, ProgressMethod() can be specified. Some filters invoke this 
// method periodically during their execution. The use is similar to that of 
// StartMethod() and EndMethod().
//
// An important feature of subclasses of vtkDataObjectSource is that it is
// possible to control the memory-management model (i.e., retain output
// versus delete output data). If enabled the ReleaseDataFlag enables the
// deletion of the output data once the downstream process object finishes
// processing the data (please see text).

// .SECTION See Also
// vtkSource vtkFilter vtkFieldDataFilter

#ifndef __vtkDataObjectSource_h
#define __vtkDataObjectSource_h

#include "vtkSource.h"

class vtkDataObject;

class VTK_EXPORT vtkDataObjectSource : public vtkSource
{
public:
  static vtkDataObjectSource *New();
  vtkTypeMacro(vtkDataObjectSource,vtkSource);

  // Description:
  // Get the output field of this source.
  vtkDataObject *GetOutput();
  vtkDataObject *GetOutput(int idx)
    {return (vtkDataObject *) this->vtkSource::GetOutput(idx); };
  void SetOutput(vtkDataObject *);
  
protected:
  vtkDataObjectSource();
  ~vtkDataObjectSource() {};
  vtkDataObjectSource(const vtkDataObjectSource&) {};
  void operator=(const vtkDataObjectSource&) {};

};

#endif

