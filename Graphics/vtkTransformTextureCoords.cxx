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
#include "vtkTransformTextureCoords.h"
#include "vtkTransform.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkTransformTextureCoords* vtkTransformTextureCoords::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkTransformTextureCoords");
  if(ret)
    {
    return (vtkTransformTextureCoords*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkTransformTextureCoords;
}




// Create instance with Origin (0.5,0.5,0.5); Position (0,0,0); and Scale
// set to (1,1,1). Rotation of the texture coordinates is turned off.
vtkTransformTextureCoords::vtkTransformTextureCoords()
{
  this->Origin[0] = this->Origin[1] = this->Origin[2] = 0.5;
  this->Position[0] = this->Position[1] = this->Position[2] = 0.0;
  this->Scale[0] = this->Scale[1] = this->Scale[2] = 1.0;

  this->FlipR = 0;
  this->FlipS = 0;
  this->FlipT = 0;
}

// Incrementally change the position of the texture map (i.e., does a
// translate or shift of the texture coordinates).
void vtkTransformTextureCoords::AddPosition (float dPX, float dPY, float dPZ)
{
  float position[3];

  position[0] = this->Position[0] + dPX;
  position[1] = this->Position[1] + dPY;
  position[2] = this->Position[2] + dPZ;
  
  this->SetPosition(position);
}

void vtkTransformTextureCoords::AddPosition(float deltaPosition[3])
{ 
  this->AddPosition (deltaPosition[0], deltaPosition[1], deltaPosition[2]);
}

void vtkTransformTextureCoords::Execute()
{
  vtkDataSet *input = this->GetInput();
  vtkDataSet *output = this->GetOutput();
  vtkTCoords *inTCoords=input->GetPointData()->GetTCoords();
  vtkTCoords *newTCoords;
  int numPts=input->GetNumberOfPoints();
  int i, j, ptId, texDim;
  vtkTransform *transform = vtkTransform::New();
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  float *TC, newTC[3];

  vtkDebugMacro(<<"Transforming texture coordinates...");

  if ( inTCoords == NULL || numPts < 1 )
    {
    vtkErrorMacro(<<"No texture coordinates to transform");
    return;
    }

  // create same type as input
  texDim = inTCoords->GetNumberOfComponents();
  newTCoords = (vtkTCoords *)inTCoords->MakeObject();
  newTCoords->Allocate(numPts,texDim);

  // just pretend texture coordinate is 3D point and use transform object to
  // manipulate
  transform->PostMultiply();
  // shift back to origin
  transform->Translate(-this->Origin[0], -this->Origin[1], -this->Origin[2]);

  // scale
  transform->Scale(this->Scale[0], this->Scale[1], this->Scale[2]);

  // rotate about z, then x, then y
  if ( this->FlipT )
    {
    transform->RotateZ(180.0);
    }
  if ( this->FlipR )
    {
    transform->RotateX(180.0);
    }
  if ( this->FlipS )
    {
    transform->RotateY(180.0);
    }

  // move back from origin and translate
  transform->Translate(this->Origin[0] + this->Position[0],
                      this->Origin[1] + this->Position[1],
		      this->Origin[2] + this->Position[2]);

  matrix->DeepCopy(transform->GetMatrixPointer());

  newTC[0] = newTC[1] = newTC[2] = 0.0;
  newTC[0] = newTC[1] = newTC[2] = 0.0;

  for (ptId=0; ptId < numPts; ptId++)
    {
    TC = inTCoords->GetTCoord(ptId);
    for (i=0; i<texDim; i++)
      {
      newTC[i] = matrix->Element[i][3];
      for (j=0; j<texDim; j++)
        {
        newTC[i] += matrix->Element[i][j] * TC[j];
        }
      }

    newTCoords->InsertTCoord(ptId,newTC);
    }

  //
  // Update self
  //
  output->GetPointData()->CopyTCoordsOff();
  output->GetPointData()->PassData(input->GetPointData());

  output->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();
  matrix->Delete();
  transform->Delete();
}

void vtkTransformTextureCoords::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToDataSetFilter::PrintSelf(os,indent);

  os << indent << "Scale: (" 
     << this->Scale[0] << ", " 
     << this->Scale[1] << ", " 
     << this->Scale[2] << ")\n";

  os << indent << "Position: (" 
     << this->Position[0] << ", " 
     << this->Position[1] << ", " 
     << this->Position[2] << ")\n";

  os << indent << "Origin: (" 
     << this->Origin[0] << ", " 
     << this->Origin[1] << ", " 
     << this->Origin[2] << ")\n";

  os << indent << "FlipR: " << (this->FlipR ? "On\n" : "Off\n");
  os << indent << "FlipS: " << (this->FlipS ? "On\n" : "Off\n");
  os << indent << "FlipT: " << (this->FlipT ? "On\n" : "Off\n");
}

