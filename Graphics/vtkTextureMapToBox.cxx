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
#include "vtkTextureMapToBox.h"
#include "vtkTCoords.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkTextureMapToBox* vtkTextureMapToBox::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkTextureMapToBox");
  if(ret)
    {
    return (vtkTextureMapToBox*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkTextureMapToBox;
}




// Construct with r-s-t range=(0,1) and automatic box generation turned on.
vtkTextureMapToBox::vtkTextureMapToBox()
{
  this->Box[0] = this->Box[2] = this->Box[4] = 0.0;
  this->Box[1] = this->Box[3] = this->Box[5] = 1.0;

  this->RRange[0] = 0.0;
  this->RRange[1] = 1.0;

  this->SRange[0] = 0.0;
  this->SRange[1] = 1.0;

  this->TRange[0] = 0.0;
  this->TRange[1] = 1.0;

  this->AutomaticBoxGeneration = 1;
}


void vtkTextureMapToBox::Execute()
{
  float tc[3];
  int numPts;
  vtkTCoords *newTCoords;
  int i, j;
  float *box, *p;
  float min[3], max[3];
  vtkDataSet *input = this->GetInput();
  vtkDataSet *output = this->GetOutput();

  vtkDebugMacro(<<"Generating 3D texture coordinates!");
  //
  //  Allocate texture data
  //
  if ( (numPts=input->GetNumberOfPoints()) < 1 )
    {
    vtkErrorMacro(<<"No points to texture!");
    return;
    }

  newTCoords = vtkTCoords::New();
  newTCoords->SetNumberOfComponents(3);
  newTCoords->SetNumberOfTCoords(numPts);

  if ( this->AutomaticBoxGeneration ) 
    {
    box = input->GetBounds();
    }
  else
    {
    box = this->Box;
    }
  //
  // Loop over all points generating coordinates
  //
  min[0] = this->RRange[0]; min[1] = this->SRange[0]; min[2] = this->TRange[0]; 
  max[0] = this->RRange[1]; max[1] = this->SRange[1]; max[2] = this->TRange[1]; 

  for (i=0; i<numPts; i++) 
    {
    p = output->GetPoint(i);
    for (j=0; j<3; j++) 
      {
      tc[j] = min[j] + (max[j]-min[j]) * (p[j] - box[2*j]) / 
	(box[2*j+1] - box[2*j]);
      if ( tc[j] < min[j] )
	{
	tc[j] = min[j];
	}
      if ( tc[j] > max[j] )
	{
	tc[j] = max[j];
	}
      }
    newTCoords->SetTCoord(i,tc);
    }
  //
  // Update ourselves
  //
  output->GetPointData()->CopyTCoordsOff();
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());

  output->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();
}

// Specify the bounding box to map into.
void vtkTextureMapToBox::SetBox(float xmin, float xmax, float ymin, float ymax,
                                float zmin, float zmax)
{
  if ( xmin != this->Box[0] || xmax != this->Box[1] ||
  ymin != this->Box[2] || ymax != this->Box[3] ||
  zmin != this->Box[4] || zmax != this->Box[5] )
    {
    this->Modified();

    this->Box[0] = xmin; this->Box[1] = xmax; 
    this->Box[2] = ymin; this->Box[3] = ymax; 
    this->Box[4] = zmin; this->Box[5] = zmax; 

    for (int i=0; i<3; i++)
      {
      if ( this->Box[2*i] > this->Box[2*i+1] )
	{
         this->Box[2*i] = this->Box[2*i+1];
	}
      }
    }
}

void vtkTextureMapToBox::SetBox(float *bounds)
{
  this->SetBox(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4],
               bounds[5]);
}

void vtkTextureMapToBox::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToDataSetFilter::PrintSelf(os,indent);

  os << indent << "Box: " << "( " << this->Box[0] << ", " 
     << this->Box[1] << ", "
     << this->Box[2] << ", "
     << this->Box[3] << ", "
     << this->Box[4] << ", "
     << this->Box[5] << " )\n";

  os << indent << "R Range: (" << this->RRange[0] << ", "
                               << this->RRange[1] << ")\n";
  os << indent << "S Range: (" << this->SRange[0] << ", "
                               << this->SRange[1] << ")\n";
  os << indent << "T Range: (" << this->TRange[0] << ", "
                               << this->TRange[1] << ")\n";
  os << indent << "Automatic Box Generation: " << 
                  (this->AutomaticBoxGeneration ? "On\n" : "Off\n");
}

