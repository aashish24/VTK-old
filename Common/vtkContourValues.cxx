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
#include "vtkContourValues.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkContourValues* vtkContourValues::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkContourValues");
  if(ret)
    {
    return (vtkContourValues*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkContourValues;
}




// Construct object with a single contour value at 0.0.
vtkContourValues::vtkContourValues()
{
  this->Contours = vtkFloatArray::New();
  this->Contours->Allocate(64);
  this->Contours->InsertValue(0,0.0);
}

vtkContourValues::~vtkContourValues()
{
  this->Contours->Delete();
}

// Set the ith contour value.
void vtkContourValues::SetValue(int i, float value) 
{
  int numContours=this->Contours->GetMaxId()+1;
  i = (i < 0 ? 0 : i);

  if ( i >= numContours || value != this->Contours->GetValue(i) )
    {
    this->Modified();
    this->Contours->InsertValue(i,value);
    }
}

// Get the ith contour value. The return value will be clamped if the
// index i is out of range.
float vtkContourValues::GetValue(int i) 
{
  i = (i < 0 ? 0 : i);
  i = (i > this->Contours->GetMaxId() ? this->Contours->GetMaxId() : i);
  return this->Contours->GetValue(i);
}

// Return a pointer to a list of contour values. The contents of the
// list will be garbage if the number of contours <= 0.
float *vtkContourValues::GetValues() 
{
  return this->Contours->GetPointer(0);
}

// Fill a supplied list with contour values. Make sure you've
// allocated memory of size GetNumberOfContours().
void vtkContourValues::GetValues(float *contourValues)
{
  int i, numContours=this->Contours->GetMaxId()+1;

  for ( i=0; i < numContours; i++ )
    {
    contourValues[i] = this->Contours->GetValue(i);
    }
}

// Set the number of contours to place into the list. You only really
// need to use this method to reduce list size. The method SetValue()
// will automatically increase list size as needed.
void vtkContourValues::SetNumberOfContours(const int number)
{
  int currentNumber = this->Contours->GetMaxId()+1;
  int n = ( number < 0 ? 0 : number);
  int i;

  if ( n != currentNumber )
    {
    this->Modified();
    this->Contours->SetNumberOfValues(n);
    }
  // Zero out new values
  if (n > currentNumber)
    {
    for ( i = currentNumber; i < n; i++ )
      {
      this->Contours->SetValue (i, 0.0);
      }
    }
}

// Generate numContours equally spaced contour values between specified
// range. Contour values will include min/max range values.
void vtkContourValues::GenerateValues(int numContours, float rangeStart, 
				     float rangeEnd)
{
  float range[2];

  range[0] = rangeStart;
  range[1] = rangeEnd;
  this->GenerateValues(numContours,range);
}

// Generate numContours equally spaced contour values between specified
// range. Contour values will include min/max range values.
void vtkContourValues::GenerateValues(int numContours, float range[2])
{
  float val, incr;
  int i;

  this->SetNumberOfContours(numContours);
  if (numContours == 1)
    {
    incr = 0;
    }
  else
    {
    incr = (range[1] - range[0]) / (numContours-1);
    }
  for (i=0, val=range[0]; i < numContours; i++, val+=incr)
    {
    this->SetValue(i,val);
    }
}

// Return the number of contours in the
int vtkContourValues::GetNumberOfContours() 
{
  return this->Contours->GetMaxId()+1;
}

void vtkContourValues::PrintSelf(ostream& os, vtkIndent indent)
{
  int i, numContours=this->Contours->GetMaxId() + 1;

  os << indent << "Contour Values: \n";
  for ( i=0; i < numContours; i++)
    {
    os << indent << "  Value " << i << ": " << this->Contours->GetValue(i) << "\n";
    }
}
