/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


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
// .NAME vtkSource - abstract class specifies interface for visualization network source (or objects that generate output data)
// .SECTION Description
// vtkSource is an abstract object that specifies behavior and interface
// of source objects. Source objects are objects that begin visualization
// pipeline. Sources include readers (read data from file or communications
// port) and procedural sources (generate data programmatically). vtkSource 
// objects are also objects that generate output data. In this sense
// vtkSource is used as a superclass to vtkFilter.
//
// Concrete subclasses of vtkSource must define Update() and Execute() 
// methods. The public method Update() invokes network execution and will
// bring the network up-to-date. The protected Execute() method actually
// does the work of data creation/generation. The difference between the two
// methods is that Update() implements input consistency checks and modified
// time comparisons and then invokes the Execute() which is an implementation 
// of a particular algorithm.
//
// vtkSource provides a mechanism for invoking the methods StartMethod() and
// EndMethod() before and after object execution (via Execute()). These are
// convenience methods you can use for any purpose (e.g., debugging info,
// highlighting/notifying user interface, etc.) These methods accept a single
// void* pointer that can be used to send data to the methods. It is also
// possible to specify a function to delete the argument via 
// StartMethodArgDelete and EndMethodArgDelete.
//
// Another method, ProgressMethod can be specified. Some filters invoke this method
// periodically during their execution. The use is similar to that of the Start and End Methods.
//
// An important feature of subclasses of vtkSource is that it is possible 
// to control the memory-management model (i.e., retain output versus delete
// output data). If enabled the ReleaseDataFlag enables the deletion of the
// output data once the downstream process object finishes processing the
// data (please see text).

// .SECTION See Also
// vtkDataSetReader vtkFilter vtkPolyDataSource vtkStructuredGridSource
// vtkStructuredPointsSource vtkUnstructuredGridSource

#ifndef __vtkSource_h
#define __vtkSource_h

#include "vtkObject.h"
#include "vtkDataSet.h"

class VTK_EXPORT vtkSource : public vtkObject
{
public:
  vtkSource();
  virtual ~vtkSource() { if (this->Output) this->Output->Delete();};
  static vtkSource *New() {return new vtkSource;};
  const char *GetClassName() {return "vtkSource";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Bring object up-to-date before execution. Update() checks modified
  // time against last execution time, and re-executes object if necessary.
  virtual void Update();

  void SetStartMethod(void (*f)(void *), void *arg);
  void SetProgressMethod(void (*f)(void *), void *arg);
  void SetEndMethod(void (*f)(void *), void *arg);
  void SetStartMethodArgDelete(void (*f)(void *));
  void SetProgressMethodArgDelete(void (*f)(void *));
  void SetEndMethodArgDelete(void (*f)(void *));

  // Description:
  // Set/Get the AbortExecute flag for the filter. It's up to the filter writer
  // to handle premature ending of a filter
  vtkSetMacro(AbortExecute,int);
  vtkGetMacro(AbortExecute,int);
  vtkBooleanMacro(AbortExecute,int);

  // Description:
  // Specify progress of a filter.
  vtkSetClampMacro(Progress,float,0.0,1.0);
  vtkGetMacro(Progress,float);

  // Description:
  // Turn on/off flag to control whether this object's data is released
  // after being used by a source.
  virtual void SetReleaseDataFlag(int);
  virtual int GetReleaseDataFlag();
  vtkBooleanMacro(ReleaseDataFlag,int);

  // Description:
  // Set/Get flag indicating whether data has been released since last 
  // execution. Used during update method to determine whether to execute 
  // or not.
  virtual int GetDataReleased();
  virtual void SetDataReleased(int flag);

  // Description:
  // Update the progress of a filter. If a ProgressMEthod, exists, executes it. Then sets
  // the Progress ivar to amount.
  void UpdateProgress(float amount);

protected:
  virtual void Execute();
  void (*StartMethod)(void *);
  void (*StartMethodArgDelete)(void *);
  void *StartMethodArg;
  void (*ProgressMethod)(void *);
  void *ProgressMethodArg;
  void (*ProgressMethodArgDelete)(void *);
  void (*EndMethod)(void *);
  void (*EndMethodArgDelete)(void *);
  void *EndMethodArg;
  float Progress;
  int AbortExecute;
  vtkTimeStamp ExecuteTime;
  vtkDataSet *Output;
};

#endif

