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
// .NAME vtkStreamPoints - generate points along streamer separated by constant time increment
// .SECTION Description
// vtkStreamPoints is a filter that generates points along a streamer.
// The points are separated by a constant time increment. The resulting visual
// effect (especially when coupled with vtkGlyph3D) is an indication of particle
// speed.

#ifndef __vtkStreamPoints_h
#define __vtkStreamPoints_h

#include "vtkStreamer.hh"

class vtkStreamPoints : public vtkStreamer
{
public:
  vtkStreamPoints();
  char *GetClassName() {return "vtkStreamPoints";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the separation of points in terms of absolute time.
  vtkSetClampMacro(TimeIncrement,float,0.000001,VTK_LARGE_FLOAT);
  vtkGetMacro(TimeIncrement,float);

protected:
  // Convert streamer array into vtkPolyData
  void Execute();

  // the separation of points
  float TimeIncrement;
  
};

#endif


